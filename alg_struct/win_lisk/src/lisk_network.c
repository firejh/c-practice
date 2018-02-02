/**
 * FILE		: lisk_network.c
 * DESC		: network related fuctions
 * AUTHOR	: v0.3.0 written by Alex Stocks
 * DATE		: on Dec 5, 2012
 * MOD		: modified by Alex Stocks on Jan 11, 2012
 * MOD		: modified by Alex Stocks on March 5, 2013
 **/

#include <lisk_deque.h>
#include <lisk_log.h>
#include <lisk_map.h>			//event
#include <lisk_network.h>
#include <lisk_ring.h>
#include <lisk_sys.h>			//is_little_endian
#include <lisk_thread.h>
#include <lisk_timer.h>			//get_cur_time_msec, timer
#include <lisk_vector.h>		//lisk_vector

#include <errno.h>				//errno
#include <stdio.h>
#include <unistd.h>				//close
#include <sys/types.h>			//geteuid
#include <sys/ioctl.h>
#include <sys/resource.h>		//getrlimit, setrlimit
#include <sys/epoll.h>			//epoll
#include <malloc.h>				//mallopt

n4 check_connection(n4 sock)
{
	n4			ret;
	u4			opt;
	socklen_t	len;

	len = sizeof(opt);
	ret = getsockopt(sock, SOL_SOCKET, SO_ERROR, &opt, &len);
	if (IS_MN(ret) || IS_NZR(opt))	{
		OUTPUT_ERROR("getsockopt(sock = %d, SO_ERROR) = %d, opt = %u", sock, ret, opt);
		RET_INT(-2, nil_str);
	}

	RET_INT(0, nil_str);
}

//////////////////////////////////////////
// event
//////////////////////////////////////////

#define	INIT_BUF_LEN	MAX_BUF_LEN
#define	READ_BUF_LEN	SIZE_K(64)

typedef enum EEVENT_STATE_TAG	{
	EEVENT_INIT				= 0x1 << 0,

	EEVENT_TCP_BIND			= 0x1 << 1,
	EEVENT_TCP_LISTEN		= 0x1 << 2,
	EEVENT_TCP_CONNECTING	= 0x1 << 4,
	EEVENT_TCP_CONNECTED	= 0x1 << 5,
	EEVENT_TCP_ACCEPT		= 0x1 << 6,
	EEVENT_TCP_CLOSING		= 0x1 << 7,

	EEVENT_UDP				= 0x1 << 8,
} EEVENT_STATE;

/**
 * a udp event has no rbuf and wbuf. os will allocate a
 * rbuf for a udp event but no wbuf.
 **/
typedef struct event_tag	{
	u2						index;
	volatile u2				state;
	u4						timeout;
	volatile n4				sock;
	lisk_atom_a				ref;
	SAEX					local;		// 16B
	SAEX					peer;		// 16B
	callback_t				callback;	// 6 * 8B
	uw						active;
	uw						tid;		// timer id
	lisk_vector_a			rbuf;		// 24 B
	lisk_vector_a			wbuf;
} event_t, *event_p, **event_pp, event_a[1];

vp s_event_ring = nil;
lisk_atom_t g_event_num = {0};

static n4 event_init(vpp event, sock_key_p key);
static vd event_uninit(vp event);

static inline n4 event_get_index(vp event, u4* index);

static inline n4 event_get_state(vp event, n4* state);
static inline n4 event_set_state(vp event, n4 state);

static inline n4 event_get_timeout(vp event, u4* timeout);
static inline n4 event_set_timeout(vp event, u4 timeout);

static inline n4 event_get_local_addr2(vp event, net_addr_p addr);
static inline n4 event_get_peer_addr(vp event, SAEX_P addr);
static inline n4 event_get_peer_addr2(vp event, net_addr_p addr);
static inline n4 event_set_peer_addr(vp event, SAEX_P addr);

static inline n4 event_get_callback(vp event, callback_p callback);
static inline n4 event_set_callback(vp event, callback_p callback);

static inline n4 event_get_sock(vp event, n4* sock);
static inline n4 event_set_sock(vp event, n4 sock);

static inline n4 event_get_tid(vp event, uw* id);
static inline n4 event_set_tid(vp event, uw id);

static inline n4 event_get_active(vp event, uw* active);
static inline n4 event_update_active(vp event);

static inline n4 event_read_buf(vp event, vpp buf);
static inline n4 event_write_buf_size(vp event, u4* size);

static n4 event_write(vp event, vp data);

const n4 DEF_SOCK_KEY_TIMEOUT = 900000;

n4 event_init(vp event_ptr, sock_key_p key)
{
	n4		ret;
	event_p	event;

	event = event_ptr;
	if (IS_NL(s_event_ring) || IS_NL(event_ptr))	{
		RET_INT(-1, "@s_event_ring = %p, @event_pptr = %p", s_event_ring, event_pptr);
	}

	if (IS_NL(event = (event_p)(*event_pptr))) {
	}
		if (IS_NL(key)) {
			RET_INT(-1, "@key = %p", key);
		}

		ret = ring_pop(s_event_ring, (vp)(&event));
		if (IS_NZR(ret) && IS_NL(ALLOC(event, sizeof(event_t))))	{
			RET_INT(-2, "ring_pop(&s_event_ring = %p, event = %p) = %d, "
						"ALLOC(size = %zu) = %p",
						s_event_ring, &event, ret, sizeof(event_t), event);
		}
		SET_ZERO(event, sizeof(*event));
		event->timeout	= DEF_SOCK_KEY_TIMEOUT;
		CPY(event, key, sizeof(*event));

	event->active	= ((uw)(get_cur_time_msec()));
	event->tid		= 0;
	lisk_sync_set(event->sock, INVALID_SOCK);
	lisk_lock_init(event->lock);
	SET_ZERO(&(event->rbuf), sizeof(event->rbuf));
	SET_ZERO(&(event->wbuf), sizeof(event->wbuf));

	*event_pptr = event;

	RET_INT(0, nil_str);
}

vd event_uninit(vpp event_pptr)
{
	n4			flag;
	n4			ret;
	event_p		event;

	if (IS_NL(event_pptr))	{
		RET_VD("@event_pptr = %p", event_pptr);
	}

	event = nil;
	if (IS_NL(event = *event_pptr))	{
		RET_VD("*@event_pptr = %p", event);
	}

	lisk_lock(event->lock);
	CPY(&flag, event->rbuf[0].reserve, sizeof(flag));
	if (IS_NEQ(event->state, EEVENT_INIT) && IS_NEQ(event->state, EEVENT_UDP))	{
		lisk_vector_uninit(event->wbuf);
		lisk_vector_uninit(event->rbuf);

		shutdown(event->sock, SHUT_WR);
		if (IS_NEQ(event->state, EEVENT_TCP_CLOSING))	{
			shutdown(event->sock, SHUT_RD);
		}
	}
	close(event->sock);
	lisk_sync_set(event->sock, INVALID_SOCK);
	lisk_unlock(event->lock);

	if (IS_EQ(flag, EEVENT_SOCK_DRIVE)) {
		ret = ring_push(s_event_ring, (vp)(&event));
		if (IS_NZR(ret))	{
			DEALLOC(*event_pptr);
		}
	}

	*event_pptr = nil;
	lisk_atom_sub(g_event_num, 1);

	RET_VD(nil_str);
}

inline n4 event_get_index(vp event_ptr, u4* index)
{
	event_p	event;

	if (IS_NL(event = ((event_p)(event_ptr))) || IS_NL(index))	{
		RET_INT(-1, "@event = %p, state = %p", event, index);
	}

	*index = event->index;

	RET_INT(0, nil_str);
}

inline n4 event_get_state(vp event_ptr, n4* state)
{
	event_p	event;

	if (IS_NL(event = ((event_p)(event_ptr))) || IS_NL(state))	{
		RET_INT(-1, "@event = %p, state = %p", event, state);
	}

	*state = event->state;

	RET_INT(0, nil_str);
}

inline n4 event_set_state(vp event_ptr, n4 state)
{
	n4		ret;
	n4		rval;
	event_p	event;

	if (IS_NL(event = ((event_p)(event_ptr))) ||
		IS_NBE(state, EEVENT_INIT, EEVENT_UDP))	{
		RET_INT(-1, "@event = %p, state = %d, Eevent_INIT = %u, Eevent_UDP = %u",
					event, state, EEVENT_INIT, EEVENT_UDP);
	}

	rval = 0;
	lisk_sync_set(event->state, state);
	switch (state) {
	case EEVENT_TCP_ACCEPT:
	case EEVENT_TCP_CONNECTING: {
		u4 size = INIT_BUF_LEN;
		ret = lisk_vector_init(event->rbuf, size, sizeof(n1));
		if (IS_NZR(ret))	{
			rval = -2;
			LISK_LOG_ERROR("lisk_vector_init(rbuf_ptr = %p, "
							"unit num = %u, unit len = %lu) = %d",
							event->rbuf, size, (uw)sizeof(n1), ret);
			break;
		}

		ret = lisk_vector_init(event->wbuf, size, sizeof(n1));
		if (IS_NZR(ret))	{
			rval = -2;
			lisk_vector_uninit(event->rbuf);
			LISK_LOG_ERROR("lisk_vector_init(wbuf_pptr = %p, "
							"unit num = %u, unit len = %lu) = %d",
							event->wbuf, size, (uw)sizeof(n1), ret);
			break;
		}
	}
	break;

	case EEVENT_TCP_CLOSING:	{
		shutdown(event->sock, SHUT_RD);
	}
	break;
	}

	RET_INT(rval, nil_str);
}

inline n4 event_get_timeout(vp event_ptr, u4* timeout)
{
	event_p	event;

	if (IS_NL(event = ((event_p)(event_ptr))) || IS_NL(timeout))	{
		RET_INT(-1, "@event = %p, timeout = %p", event, timeout);
	}

	*timeout = event->timeout;

	RET_INT(0, nil_str);
}

inline n4 event_set_timeout(vp event_ptr, u4 timeout)
{
	event_p	event;

	if (IS_NL(event = ((event_p)(event_ptr))) || IS_ZR(timeout))	{
		RET_INT(-1, "@event = %p, timeout = %d", event, timeout);
	}

	lisk_sync_set(event->timeout, timeout);

	RET_INT(0, nil_str);
}

inline n4 event_get_local_addr2(vp event_ptr, net_addr_p addr)
{
	event_p	event;

	if (IS_NL(event = ((event_p)(event_ptr))) || IS_NL(addr))	{
		RET_INT(-1, "@event = %p, addr = %p", event, addr);
	}

	sa_2_na(addr, &(event->local));

	RET_INT(0, nil_str);
}

inline n4 event_get_peer_addr(vp event_ptr, SAEX_P addr)
{
	event_p	event;

	if (IS_NL(event = ((event_p)(event_ptr))) || IS_NL(addr))	{
		RET_INT(-1, "@event = %p, addr = %p", event, addr);
	}

	CPY(addr, &(event->peer), sizeof(event->peer));

	RET_INT(0, nil_str);
}

inline n4 event_get_peer_addr2(vp event_ptr, net_addr_p addr)
{
	event_p	event;

	if (IS_NL(event = ((event_p)(event_ptr))) || IS_NL(addr))	{
		RET_INT(-1, "@event = %p, addr = %p", event, addr);
	}

	sa_2_na(addr, &(event->peer));

	RET_INT(0, nil_str);
}

inline n4 event_set_peer_addr(vp event_ptr, SAEX_P addr)
{
	event_p	event;

	if (IS_NL(event = ((event_p)(event_ptr))) || IS_NL(addr))	{
		RET_INT(-1, "@event = %p, addr = %p", event, addr);
	}

	CPY(&(event->peer), addr, sizeof(event->peer));

	RET_INT(0, nil_str);
}

inline n4 event_get_callback(vp event_ptr, callback_p callback)
{
	event_p	event;

	if (IS_NL(event = event_ptr) || IS_NL(callback))	{
		RET_INT(-1, "@event = %p, @callback = %p", event, callback);
	}

	CPY(callback, &(event->callback), sizeof(*callback));

	RET_INT(0, nil_str);
}

inline n4 event_set_callback(vp event_ptr, callback_p callback)
{
	event_p	event;

	if (IS_NL(event = event_ptr) || IS_NL(callback))	{
		RET_INT(-1, "@event = %p, @callback = %p", event, callback);
	}

	CPY(&(event->callback), callback, sizeof(*callback));

	RET_INT(0, nil_str);
}

inline n4 event_get_sock(vp event_ptr, n4* sock)
{
	event_p	event;

	if (IS_NL(event = ((event_p)(event_ptr))) || IS_NL(sock))	{
		RET_INT(-1, "@event = %p, sock = %p", event, sock);
	}

	*sock = event->sock;

	RET_INT(0, nil_str);
}

inline n4 event_set_sock(vp event_ptr, n4 sock)
{
	event_p	event;

	if (IS_NL(event = ((event_p)(event_ptr))) || IS_LT(sock, 3))	{
		RET_INT(-1, "@event = %p, sock = %d", event, sock);
	}

	lisk_sync_set(event->sock, sock);

	RET_INT(0, nil_str);
}

inline n4 event_get_tid(vp event_ptr, uw* id)
{
	event_p	event;

	if (IS_NL(event = event_ptr) || IS_NL(id))	{
		RET_INT(-1, "@event = %p, id = %p", event, id);
	}

	*id = event->tid;

	RET_INT(0, nil_str);
}

inline n4 event_set_tid(vp event_ptr, uw id)
{
	event_p	event;

	if (IS_NL(event = event_ptr))	{
		RET_INT(-1, "@event = %p", event);
	}

	lisk_sync_set(event->tid, id);

	RET_INT(0, nil_str);
}

inline n4 event_get_active(vp event_ptr, uw* active)
{
	event_p	event;

	if (IS_NL(event = ((event_p)(event_ptr))) || IS_NL(active))	{
		RET_INT(-1, "@event = %p, active_time = %p", event, active);
	}

	*active = event->active;

	RET_INT(0, nil_str);
}

inline n4 event_update_active(vp event_ptr)
{
	uw		active;
	event_p	event;

	active = (uw)get_cur_time_msec();
	if (IS_NL(event = event_ptr) || IS_LT(active, event->active))	{
		RET_INT(-1, "@event = %p, @active = %lu, @event->active = %lu",
					event, active, event->active);
	}

	event->active = active;

	RET_INT(0, nil_str);
}

inline n4 event_read_buf(vp event_ptr, vpp buf)
{
	event_p	event;

	if (IS_NL(event = event_ptr) || IS_NL(buf))	{
		RET_INT(-1, "@event = %p, @buf = %p", event, buf);
	}

	*buf = nil;

	if (IS_EQ(event->state, EEVENT_INIT) ||
		IS_EQ(event->state, EEVENT_TCP_LISTEN) ||
		IS_NL(event->rbuf[0].data))	{
		RET_INT(-1, "state = %d, Eevent_INIT = %u,"
				" Eevent_TCP_LISTEN = %u, event->rbuf.data = %p",
				event->state, EEVENT_INIT, EEVENT_TCP_LISTEN, event->rbuf[0].data);
	}

	*buf = &(event->rbuf);

	RET_INT(0, nil_str);
}

inline n4 event_write_buf_size(vp event_ptr, u4* size)
{
	event_p	event;

	event = (event_p)(event_ptr);
	if (IS_NL(event) || IS_NL(size))	{
		RET_INT(-1, "@event = %p, @size = %p", event, size);
	}

	*size = 0;
	if (IS_NEQ(event->state, EEVENT_INIT) && (IS_NEQ(event->state, EEVENT_UDP))) {
		lisk_lock(event->lock);
		*size = event->wbuf[0].number;
		lisk_unlock(event->lock);
	}

	RET_INT(0, nil_str);
}

n4 event_write(vp event_ptr, vp data_ptr)
{
	n4			ret;
	n4			rval;
	u4			pos;
	u4			size;
	vp			buf;
	event_p		event;
	net_addr_a	local;
	net_addr_a	peer;
	lisk_buf_p	lisk_buf;

	event = event_ptr;
	lisk_buf = data_ptr;
	if (IS_NL(event))	{
		RET_INT(-1, "@event = %p", event);
	}

	if (IS_NPN(event->sock) || IS_NBE(event->state, EEVENT_TCP_CONNECTED, EEVENT_UDP)) {
		RET_INT(-3, "event->sock = %d, event->state = %d", event->sock, event->state);
	}

	buf = lisk_buf->data;
	size = lisk_buf->size;
	if (IS_EQ(event->state, EEVENT_UDP))	{
		if (IS_NNL(lisk_buf) && IS_NNL(buf) && IS_NZR(size))	{
			ret = sock_write(event->sock, buf, &(size), (SA_P)(&(event->peer)));
			if (IS_NZR(ret))	{
				event_get_local_addr2(event, local);
				event_get_peer_addr2(event, peer);
				RET_INT(ret, "sock_write(udp sock = %d, pkg = %p, "
							"size = %u, local{%s-%hu}, addr{%s-%hu}) = %d",
							event->sock, buf, size, local[0].ip,
							local[0].port, peer[0].ip, peer[0].port, ret);
			}
		}

		RET_INT(0, nil_str);
	}

	rval = 0;
	lisk_lock(event->lock);
	do {
		if (IS_NNL(lisk_buf) && IS_NNL(buf) && IS_NZR(size))	{
			ret = lisk_vector_push_back(event->wbuf, buf, size);
			if (IS_NZR(ret))	{
				rval = -2;
				LISK_LOG_ERROR("lisk_vector_push_back(wbuf = %p, "
								"pkg = %p, len = %d) = %d",
								event->wbuf, buf, size, ret);
				break;
			}
		}

		buf = event->wbuf[0].data;
		size = event->wbuf[0].number;
		ret = sock_write(event->sock, buf, &size, nil);
		if (IS_NZR(ret))	{
			rval = ret;
			event_get_local_addr2(event, local);
			event_get_peer_addr2(event, peer);
			LISK_LOG_ERROR("sock_write(tcp sock = %d, buf = %p, @size = %p, "
							"local = {%s-%hu}, peer = {%s-%hu}) = %d",
							event->sock, buf, &size, local[0].ip,
							local[0].port, peer[0].ip, peer[0].port, ret);
			break;
		}

		if (IS_NZR(size))	{
			pos = 0;
			lisk_vector_erase(event->wbuf, pos, size);
		}
	} while (0);
	lisk_unlock(event->lock);

	RET_INT(rval, nil_str);
}

//////////////////////////////////////////
// epoll
//////////////////////////////////////////

#define	EVENT					struct epoll_event
#define	EVENT_P					struct epoll_event*
#define	DEF_EVENT_TIMEOUT		100						// epoll_wait timeout

typedef enum ESOCK_EVENT_TAG	{
	ESOCK_READ		= 0x1 << 0,
	ESOCK_WRITE		= 0x1 << 1,
} ESOCK_EVENT;

typedef struct event_buf_tag	{
	vp				event;
	lisk_vector_a	buf;
} event_buf_t, *event_buf_p, event_buf_a[1];

typedef struct epoll_tag	{
	n4				ep_fd;		// epoll fd
	n4				timeout;	// time out
	vp				queue;		// event queue
	lisk_atom_t		event_num;	// event number
	EVENT			event;		// event array
	lisk_timer_a	timer;		// timer
} epoll_t, *epoll_p, epoll_a[1];

static n4 epoll_init(vpp epoll);
static vd epoll_uninit(vp epoll);

static inline n4 epoll_set_timeout(vp epoll, n4 timeout);
static inline n4 epoll_set_event_queue(vp epoll, vp event);
static inline n4 epoll_get_event_num(vp epoll, n4* num);

static n4 epoll_add_event(vp epoll, vp event, n4 event_type);
static n4 epoll_mod_event(vp epoll, vp event, n4 event_type);
static n4 epoll_del_event(vp epoll, vp event);
static n4 epoll_kill_event(vp timer, uw id, vp event);

static n4 epoll_run(vp epoll);

static n4 epoll_handle_error(vp epoll, vp event);
static n4 epoll_handle_tcp_snd(vp epoll, vp event);
static n4 epoll_handle_udp_snd(vp epoll, vp event);
static n4 epoll_handle_send(vp epoll, vp event);

static n4 epoll_handle_accept(vp epoll, vp event);
static n4 epoll_handle_tcp_rcv(vp epoll, vp event);
static n4 epoll_handle_udp_rcv(vp epoll, vp event);
static n4 epoll_handle_receive(vp epoll, vp data);

n4 epoll_init(vpp epoll_pptr)
{
	n4		fd;
	n4		ret;
	n4		type;
	epoll_p	epoll;

	if (IS_NL(epoll_pptr))	{
		RET_INT(-1, "@epoll_pptr = %p", epoll_pptr);
	}

	if (IS_NL(ALLOC(epoll, sizeof(epoll_t))))	{
		RET_INT(-2, "ALLOC(size = %zu) = %p", sizeof(epoll_t), epoll);
	}
	SET_ZERO(epoll, sizeof(*epoll));
	*epoll_pptr = nil;
	
	epoll->event = nil;
	lisk_atom_set(epoll->event_num, 0);
	epoll->timeout = DEF_EVENT_TIMEOUT;
	type = EPOLL_CLOEXEC;
	fd = epoll_create1(type);
	if (IS_EQ(fd, -1))	{
		DEALLOC(epoll);
		RET_INT(-2, "epoll_create(type = %d) = %d", type, fd);
	}
	epoll->ep_fd = fd;

	ret = timer_init(epoll->timer);
	if (IS_NZR(ret))	{
		close(epoll->ep_fd);
		epoll->ep_fd = -1;
		DEALLOC(epoll);
		RET_INT(-2, "timer_init(timer_pptr = %p) = %d", epoll->timer, ret);
	}

	*epoll_pptr = epoll;

	RET_INT(0, nil_str);
}

vd epoll_uninit(vp epoll_ptr)
{
	epoll_p	epoll;

	if (IS_NL(epoll = epoll_ptr))	{
		RET_VD("@epoll = %p", epoll);
	}

	timer_uninit(epoll->timer);
	close(epoll->ep_fd);
	epoll->ep_fd = -1;

	DEALLOC(epoll);
}

inline n4 epoll_set_timeout(vp epoll_ptr, n4 timeout)
{
	epoll_p	epoll;

	if (IS_NL(epoll = epoll_ptr) || IS_NPN(timeout))	{
		RET_INT(-1, "@epoll = %p, @timeout = %d", epoll, timeout);
	}

	epoll->timeout = timeout;

	RET_INT(0, nil_str);
}

inline n4 epoll_set_event_queue(vp epoll_ptr, vp queue)
{
	epoll_p	epoll;

	if (IS_NL(epoll = epoll_ptr) || IS_NPN(queue))	{
		RET_INT(-1, "@epoll = %p, @queue = %p", epoll, queue);
	}

	epoll->event = queue;

	RET_INT(0, nil_str);
}

inline n4 epoll_get_event_num(vp epoll_ptr, n4* num)
{
	epoll_p	epoll;

	if (IS_NL(epoll = epoll_ptr) || IS_NL(num))	{
		RET_INT(-1, "@epoll = %p, @num = %p", epoll, num);
	}

	*num = lisk_atom_read(epoll->event_num);

	RET_INT(0, nil_str);
}

n4 epoll_add_event(vp epoll_ptr, vp event, n4 event_type)
{
	n4			ret;
	n4			rval;
	n4			sock;
	u4			expire;
	u4			index;
	uw			id;
	EVENT		event;
	epoll_p		epoll;
	timer_key_t	tkey;

	if (IS_NL(epoll = epoll_ptr) || IS_NL(event) ||
		IS_NBE(event_type, ESOCK_READ, ESOCK_WRITE))	{
		RET_INT(-1, "@epoll = %p, @event = %p, @event_type = %d",
					epoll, event, event_type);
	}

	event.data.ptr = (vp)(event);
	if (IS_NZR(event_type & ESOCK_READ))	{
		event.events = EPOLLIN;
	}
	if (IS_NZR(event_type & ESOCK_WRITE))	{
		event.events = EPOLLOUT;
	}
	event.events |= (EPOLLONESHOT | EPOLLRDHUP);

	rval = 0;
	event_get_sock(event, &sock);
	ret = epoll_ctl(epoll->ep_fd, EPOLL_CTL_ADD, sock, &event);
	if (IS_MN(ret))	{
		if (IS_EQ(errno, EEXIST))	{
			ret = epoll_ctl(epoll->ep_fd, EPOLL_CTL_MOD, sock, &event);
			if (IS_MN(ret))	{
				OUTPUT_ERROR("epoll_ctl(epoll fd = %d, EPOLL_CTL_MOD,"
							" sock = %d, &event = %p) = %d",
							epoll->ep_fd, sock, &event, ret);
				rval = -2;
			}
		} else {
			OUTPUT_ERROR("epoll_ctl(epoll fd = %d, EPOLL_CTL_ADD,"
						" sock = %d, &event = %p) = %d",
						epoll->ep_fd, sock, &event, ret);
			rval = -2;
		}
	} // if

	if (IS_ZR(rval))	{
		lisk_atom_add(epoll->event_num, 1);
		event_update_active(event);
		event_get_timeout(event, &expire);
		tkey.type = ETIMER_LOOP;
		tkey.expire = expire;
		tkey.arg = event;
		tkey.func = epoll_kill_event;
		timer_add(epoll->timer, &tkey, &id);
		event_set_tid(event, id);
		event_get_index(event, &index);
		LISK_LOG_INFO("event{index = %u, sock = %d, tid = %lu}", index, sock, id);
	}

	RET_INT(rval, nil_str);
}

n4 epoll_mod_event(vp epoll_ptr, vp event, n4 event_type)
{
	n4		ret;
	n4		rval;
	n4		sock;
	EVENT	event;
	epoll_p	epoll;

	if (IS_NL(epoll = epoll_ptr) || IS_NL(event) ||
		IS_NBE(event_type, EEVENT_INIT, EEVENT_UDP))	{
		RET_INT(-1, "@epoll = %p, @event = %p, @event_type = %d",
					epoll, event, event_type);
	}

	event.data.ptr = (vp)(event);
	if (IS_NZR(event_type & ESOCK_READ))	{
		event.events = EPOLLIN;
	}
	if (IS_NZR(event_type & ESOCK_WRITE))	{
		event.events = EPOLLOUT;
	}
	event.events |= (EPOLLONESHOT | EPOLLRDHUP);

	rval = 0;
	event_get_sock(event, &sock);
	event_update_active(event);
	ret = epoll_ctl(epoll->ep_fd, EPOLL_CTL_MOD, sock, &event);
	if (IS_MN(ret))	{
		if (IS_EQ(errno, ENOENT))	{
			ret = epoll_ctl(epoll->ep_fd, EPOLL_CTL_ADD, sock, &event);
			if (IS_MN(ret))	{
				OUTPUT_ERROR("epoll_ctl(epoll fd = %d, EPOLL_CTL_ADD, "
							"sock = %d, event = %p) = %d",
							epoll->ep_fd, sock, &event, ret);
				rval = -2;
			}
		} else {
			OUTPUT_ERROR("epoll_ctl(epoll fd = %d, EPOLL_CTL_MOD, "
						"sock = %d, event = %p) = %d",
						epoll->ep_fd, sock, &event, ret);
			rval = -2;
		}	//if
	} // if

	RET_INT(rval, nil_str);
}

n4 epoll_del_event(vp epoll_ptr, vp event)
{
	n4				ret;
	n4				sock;
	n4				state;
	n4				type;
	u4				index;
	u4				size;
	uw				tid;
	epoll_p			epoll;

	if (IS_NL(epoll = epoll_ptr) || IS_NL(event))	{
		RET_INT(-1, "@epoll = %p, @event = %p", epoll, event);
	}

	//epoll
	event_get_sock(event, &sock);
	ret = epoll_ctl(epoll->ep_fd, EPOLL_CTL_DEL, sock, nil);
	if (IS_MN(ret))	{
		if (IS_NEQ(errno, ENOENT))	{
			OUTPUT_ERROR("epoll_ctl(epoll fd = %d, EPOLL_CTL_DEL, sock = %d) = %d",
							epoll->ep_fd, sock, ret);
		}
	}

	event_get_state(event, &state);
	if (IS_EQ(state, EEVENT_TCP_CONNECTED))	{
		ret = event_write_buf_size(event, &size);
		if (IS_ZR(ret) && IS_NZR(size))	{
			event_set_state(event, EEVENT_TCP_CLOSING);
			type = ESOCK_WRITE;
			ret = epoll_mod_event(epoll, event, type);
			if (IS_ZR(ret))	{
				RET_INT(0, nil_str);
			}
		}
	}

	event_get_index(event, &index);
	event_get_tid(event, &tid);
	if (IS_NZR(tid))	{
		timer_del(epoll->timer, tid);
		event_set_tid(event, 0);
	}
	event_uninit((vpp)(&event));
	lisk_atom_sub(epoll->event_num, 1);
	LISK_LOG_INFO("event{index = %u, sock = %d, tid = %lu}", index, sock, tid);

	RET_INT(0, nil_str);
}

n4 epoll_kill_event(vp timer, uw id, vp event_ptr)
{
	n4			ret;
	n4			rval;
	n4			sock;
	n4			state;
	u4			index;
	u4			timeout;
	uw			active;
	uw			now;
	uw			tid;
	event_p		event;
	epoll_p		epoll;
	callback_t	callback;

	if (IS_NL(event = event_ptr))	{
		RET_INT(-1, "@event = %p", event);
	}

	event_get_index(event, &index);
	event_get_sock(event, &sock);
	event_get_state(event, &state);
	event_get_tid(event, &tid);
	if (IS_NEQ(id, event->tid))	{
		RET_INT(-3, "@id = %lu, @event{index = %u, sock = %d, state = %d, tid = %lu}",
					id, index, sock, state, tid);
	}

	if (IS_NL(timer)) {
		event_uninit((vpp)(&event));
		RET_INT(0, nil_str);
	}

	epoll = container_of(timer, epoll_t, timer);
	event_get_active(event, &active);
	event_get_timeout(event, &timeout);

	rval = 0;
	tid = 0;
	now = (uw)get_cur_time_msec();
	if (IS_GT(now, active) && IS_GE(now - active, timeout))	{
		ret = 0;
		event_get_callback(event, &callback);
		if (IS_NNL(callback.handle_timeout))	{
			ret = callback.handle_timeout(event);
			if (IS_NZR(ret))	{
				LISK_LOG_ERROR("handle_timeout(event{index = %u, "
								"sock = %d, state = %d}) = %d",
								index, sock, state, ret);
			}
		}

		if (IS_EQ(ret, -3) && IS_NEQ(state, EEVENT_TCP_LISTEN))	{
			event_set_tid(event, tid);
			epoll_del_event(epoll, event);
			rval = -3;
		}
	}

	RET_INT(rval, nil_str);
}

n4 epoll_run(vp epoll_ptr)
{
	n4			ret;
	n4			rval;
	n4			num;
	n4			sock;
	n4			type;
	n4			size;
	u4			index;
	epoll_p		epoll;
	EVENT_P		event;
	net_addr_a	local;
	net_addr_a	peer;
	event_p		event;

	if (IS_NL(epoll = epoll_ptr))	{
		RET_INT(-1, "@epoll_ptr = %p", epoll);
	}

	rval = 0;
	do {
		event = &(epoll->event);
		size = sizeof(epoll->event) / sizeof(EVENT);
		num = epoll_wait(epoll->ep_fd, event, size, epoll->timeout);
		if (IS_MN(num))	{
			if (IS_EQ(errno, EINTR) || IS_EQ(errno, EAGAIN))	{
				break;
			}
			rval = -2;
			OUTPUT_ERROR("epoll_wait(fd) = %d:", num);
			break;
		} else if (IS_ZR(num))	{
			break;
		}

		event = event->data.ptr;
		if ((event->events) & (EPOLLHUP | EPOLLERR))	{
			rval = -2;
			event_get_index(event, &index);
			event_get_sock(event, &sock);
			event_get_local_addr2(event, local);
			event_get_peer_addr2(event, peer);
			OUTPUT_ERROR("epoll_wait event->events = %u, EPOLLHUP = %u, EPOLLERR = %u, "
						"event{index = %u, sock = %d, local{%s-%hu}, peer{%s-%hu}}",
						(u4)(EPOLLHUP | EPOLLERR), EPOLLHUP, EPOLLERR, index, sock,
						local[0].ip, local[0].port, peer[0].ip, peer[0].port);
			epoll_handle_error(epoll, event);
			epoll_del_event(epoll, event);
			break;
		}

		if ((event->events) & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))	{
			ret = epoll_handle_receive(epoll, event);
			if (IS_NZR(ret))	{
				LISK_LOG_ERROR("epoll_handle_receive(epoll = %p, event = %p) = %d",
								epoll, event, ret);
				if (IS_EQ(ret, -3))	{
					epoll_del_event(epoll, event);
				}
				rval = -2;
				break;
			}
		}

		if ((event->events) & EPOLLOUT)	{
			ret = epoll_handle_send(epoll, event);
			if (IS_NZR(ret))	{
				LISK_LOG_ERROR("epoll_handle_send(epoll = %p, event = %p) = %d",
								epoll, event, ret);
				if (IS_EQ(ret, -3))	{
					epoll_del_event(epoll, event);
				}
				rval = -2;
				break;
			}
		}

		type = ESOCK_READ;
		ret = event_write_buf_size(event, (u4*)&(size));
		if (IS_ZR(ret) && IS_NZR(size))	{
			type |= ESOCK_WRITE;
		}
		ret = epoll_mod_event(epoll, event, type);
		if (IS_NZR(ret))	{
			LISK_LOG_ERROR("epoll_mod_event(epoll = %p, event = %p, type = %d) = %d",
							epoll, event, type, ret);
		}
	} while(0);

	timer_loop(epoll->timer);

	RET_INT(rval, nil_str);
}

n4 epoll_handle_error(vp epoll_ptr, vp event)
{
	n4			ret;
	n4			sock;
	epoll_p		epoll;
	callback_t	callback;
	net_addr_a	local;
	net_addr_a	peer;

	if (IS_NL(epoll = epoll_ptr) || IS_NL(event))	{
		RET_INT(-1, "@epoll_ptr = %p, @event_ptr = %p", epoll, event);
	}

	event_get_sock(event, &sock);
	event_get_local_addr2(event, local);
	event_get_peer_addr2(event, peer);
	LISK_LOG_ERROR("epoll error:event{sock = %d, local = {%s-%hu}, peer = {%s-%hu}}",
					sock, local[0].ip, local[0].port, peer[0].ip, peer[0].port);
	ret = event_get_callback(event, &callback);
	if (IS_NZR(ret))	{
		RET_INT(-1, "event_get_callback(event = %p, &callback = %p) = %d",
					event, &callback, ret);
	}

	if (IS_NNL(callback.handle_close))	{
		callback.handle_close(event);
	}

	RET_INT(0, nil_str);
}

n4 epoll_handle_receive(vp epoll_ptr, vp event)
{
	n4			ret;
	n4			state;
	epoll_p		epoll;

	if (IS_NL(epoll = epoll_ptr) || IS_NL(event))	{
		RET_INT(-1, "@epoll_ptr = %p, @event_ptr = %p", epoll, event);
	}

	event_get_state(event, &state);
	switch (state)	{
	case EEVENT_TCP_ACCEPT:
	case EEVENT_TCP_CONNECTED:
		ret = epoll_handle_tcp_rcv(epoll, event);
		break;

	case EEVENT_TCP_LISTEN:
		ret = epoll_handle_accept(epoll, event);
		break;

	case EEVENT_UDP:
		ret = epoll_handle_udp_rcv(epoll, event);
		break;

	default:
		ret = -2;
		LISK_LOG_ERROR("state = %d", state)
		break;
	}

	if (IS_NZR(ret))	{
		RET_INT(ret, "state = %d, ret = %d", state, ret)
	}

	RET_INT(0, nil_str);
}

n4 epoll_handle_accept(vp epoll_ptr, vp event_ptr)
{
	n4			ret;
	SA			addr;
	socklen_t	len;
	event_p		event;
	epoll_p		epoll;
	n4			local_sock;
	n4			peer_sock;
	callback_t	callback;

	if (IS_NL(epoll = epoll_ptr) || IS_NL(event_ptr))	{
		RET_INT(-1, "@epoll_ptr = %p, @event_ptr = %p", epoll, event_ptr);
	}

	peer_sock = INVALID_SOCK;
	len = sizeof(addr);
	event_get_sock(event_ptr, &local_sock);
	event_get_callback(event_ptr, &callback);
	while (1)	{
		peer_sock = accept(local_sock, &addr, &len);
		if (IS_EQ(peer_sock, -1))	{
			if (IS_EQ(errno, EINTR))	{
				continue;
			} if (IS_EQ(errno, EAGAIN) ||
					IS_EQ(errno, EWOULDBLOCK))	{
				break;
			}
			OUTPUT_ERROR("accept()");
			break;
		}

		ret = set_sock_nonblock(peer_sock);
		if (IS_NZR(ret))	{
			LISK_LOG_ERROR("set_sock_nonblock(sock = %d) = %d", peer_sock, ret);
			continue;
		}

		event = nil;
		ret = event_init((vpp)(&event), event_ptr);
		if (IS_MN(ret))	{
			LISK_LOG_ERROR("event_init(&event = %p, key = %p) = %d", event, event_ptr, ret);
			continue;
		}

		event_set_sock(event, peer_sock);
		event_set_state(event, EEVENT_TCP_ACCEPT);
		event_set_peer_addr(event, ((SAEX_P)(&addr)));

		if (callback.handle_accept)	{
			ret = callback.handle_accept(event_ptr, event);
			if (IS_NZR(ret))	{
				LISK_LOG_ERROR("handle_accept(local sock = %d, peer sock = %d) = %d",
								local_sock, peer_sock, ret);
				event_uninit((vp)(event));
				continue;
			}
		}

		ret = deque_push_back(epoll->event, ((vp)(&event)));
		if (IS_NZR(ret))	{
			LISK_LOG_ERROR("deque_push_back(event_ptr = %p, event_ptr = %p) = %d",
							epoll->event, ((vp)(&event)), ret);
			event_uninit((vp)(event));
		}
	}

	RET_INT(0, nil_str);
}

n4 epoll_handle_tcp_rcv(vp epoll_ptr, vp event)
{
	n4			ret;
	n4			sock;
	n4			buf_len;
	n4			pkg_len;
	n4			prev_len;
	n4			left;
	u4			size;
	vp			buf;
	vp			pkg;
	vp			addr;
	vp			rbuf;
	epoll_p		epoll;
	net_addr_a	local;
	net_addr_a	peer;
	n1			str[READ_BUF_LEN];
	callback_t	callback;

	if (IS_NL(epoll = epoll_ptr) || IS_NL(event))	{
		RET_INT(-1, "@epoll = %p, @event = %p", epoll, event);
	}

	event_read_buf(event, &rbuf);
	event_get_sock(event, &sock);
	event_get_callback(event, &callback);

	addr = nil;
	size = sizeof(str);
	ret = sock_read(sock, str, &size, addr);
	if (IS_NZR(ret))	{
		if (IS_EQ(ret, -3) && IS_NNL(callback.handle_close))	{
			// peer socket has been closed.
			callback.handle_close(event);
		}

		event_get_local_addr2(event, local);
		event_get_peer_addr2(event, peer);
		RET_INT(ret, "sock_read(sock = %d, buf = %p, size = %u, "
					"local = {%s-%hu}, peer = {%s-%hu}) = %d",
					sock, str, size, local[0].ip, local[0].port,
					peer[0].ip, peer[0].port, ret);
	}

	if (IS_ZR(size))	{
		RET_INT(0, nil_str);
	}

	ret = lisk_vector_push_back(rbuf, str, size);
	if (IS_NZR(ret))	{
		LISK_LOG_ERROR("lisk_vector_push_back(buffer = %p, str = %p, size = %u) = %d",
						rbuf, str, size, ret);
		RET_INT(-2, nil_str)
	}

	pkg_len = 0;
	prev_len = 0;
	buf = ((lisk_vector_p)(rbuf))->data;
	buf_len = ((lisk_vector_p)(rbuf))->number;
	do {
		pkg = buf + prev_len;
		left = buf_len - prev_len;

		ret = callback.handle_tcp_stream(event, (const vp)(pkg), left, &pkg_len);
		if (IS_NZR(ret) || IS_GT(pkg_len, READ_BUF_LEN))	{
			RET_INT(ret, "callback.handle_tcp_stream(event = %p, "
						"pkg = %p, left = %d, &pkg_len:{%u} = %p) = %d",
						event, pkg, left, pkg_len, &pkg_len, ret);
		}
		if (IS_ZR(pkg_len) || IS_GT(pkg_len, left))	{
			break;
		}

		ret = callback.handle_tcp_pkg(event, (vp)(pkg), pkg_len);
		if (IS_NZR(ret))	{
			RET_INT(ret, "callback.handle_tcp_pkg(event = %p, "
						"pkg = %p, &pkg_len = %p) = %d",
						event, pkg, &left, ret);
			break;
		}
		prev_len += pkg_len;
	} while(IS_LT(prev_len, buf_len));

	if (IS_NZR(prev_len))	{
		lisk_vector_erase(rbuf, 0, prev_len);
	}

	RET_INT(0, nil_str);
}

n4 epoll_handle_udp_rcv(vp epoll, vp event_ptr)
{
	n4			ret;
	n4			sock;
	u4			size;
	SA			addr;
	event_p		event;
	net_addr_a	local;
	net_addr_a	peer;
	n1			str[READ_BUF_LEN];
	callback_t	callback;

	if (IS_NL(epoll) || IS_NL(event = event_ptr))	{
		RET_INT(-1, "@epoll = %p, @event = %p", epoll, event);
	}

	event_get_sock(event, &sock);
	event_get_callback(event, &callback);
	while (1) {
		size = sizeof(str);
		ret = sock_read(sock, str, &size, &addr);
		if (IS_MN(ret)) {
			event_get_local_addr2(event, local);
			event_get_peer_addr2(event, peer);
			LISK_LOG_ERROR("sock_read(sock = %d, buffer = %p, size = %u, "
							"local = {%s-%hu}, peer = {%s-%hu}) = %d",
							sock, str, size, local[0].ip,
							local[0].port, peer[0].ip, peer[0].port, ret);
			break;
		}
		if (IS_ZR(size))	{
			break;
		}
		event_set_peer_addr(event, ((SAEX_P)(&addr)));
		ret = callback.handle_udp_pkg(event, (const vp)(str), (n4)(size));
		if (IS_NZR(ret))	{
			RET_INT(ret, "callback.handle_udp_pkg(event = %p,"
						" buf = %p, buf_len = %u) = %d",
						event, str, size, ret);
		}
	}

	RET_INT(0, nil_str);
}

n4 epoll_handle_send(vp epoll, vp event)
{
	n4		ret;
	n4		state;

	if (IS_NL(epoll) || IS_NL(event))	{
		RET_INT(-1, "@epoll = %p, @event = %p", epoll, event);
	}

	event_get_state(event, &state);
	switch (state)	{
	case EEVENT_TCP_ACCEPT:
	case EEVENT_TCP_CONNECTING:
	case EEVENT_TCP_CONNECTED:
	case EEVENT_TCP_CLOSING: {
		ret = epoll_handle_tcp_snd(epoll, event);
	}
	break;

	case EEVENT_UDP: {
		ret = epoll_handle_udp_snd(epoll, event);
	}
	break;

	default: {
		ret = -2;
		LISK_LOG_ERROR("illegal state = %d", state);
	}
	break;
	}

	if (IS_NZR(ret))	{
		RET_INT(ret, "state = %d, ret = %d", state, ret)
	}

	RET_INT(0, nil_str);
}

n4 epoll_handle_tcp_snd(vp epoll, vp event)
{
	n4			ret;
	n4			sock;
	n4			state;
	u4			index;
	u4			size;
	lisk_buf_p	buf;
	net_addr_a	local;
	net_addr_a	peer;
	callback_t	callback;

	if (IS_NL(epoll) || IS_NL(event))	{
		RET_INT(-1, "@epoll = %p, @event = %p", epoll, event);
	}

	event_get_callback(event, &callback);
	event_get_index(event, &index);
	event_get_sock(event, &sock);
	event_get_state(event, &state);
	// handle client connection
	if (IS_EQ(state, EEVENT_TCP_CONNECTING))	{
		ret = check_connection(sock);
		if (IS_NZR(ret)) {
			event_get_local_addr2(event, local);
			event_get_peer_addr2(event, peer);
			RET_INT(-3, "check_connection(event{index = %u, state = %hu, "
						"sock = %d, local{%s-%hu}, peer{%s-%hu}}) = %d",
						index, state, sock, local[0].ip,
						local[0].port, peer[0].ip, peer[0].port, ret);
		}

		event_set_state(event, EEVENT_TCP_CONNECTED);
		if (IS_NNL(callback.handle_connect))	{
			ret = callback.handle_connect(event);
			if (IS_NZR(ret))	{
				RET_INT(ret, "handle_connect(event{index = %u, state = %hu, "
							"sock = %d, local{%s-%hu}, peer{%s-%hu}}) = %d",
							index, state, sock, local[0].ip,
							local[0].port, peer[0].ip, peer[0].port, ret);
			}
		}

		RET_INT(0, nil_str);
	}

	buf = nil;
	ret = event_write(event, buf);
	if (IS_NZR(ret))	{
		if (IS_EQ(ret, -3) && IS_NNL(callback.handle_close))	{
			callback.handle_close(event);
		}

		RET_INT(ret, "event_write(event = %p, buf = %p) = %d", event, buf, ret);
	}

	ret = event_write_buf_size(event, &size);
	if (IS_NZR(ret))	{
		size = 0;
	}
	if (IS_ZR(size) && IS_EQ(state, EEVENT_TCP_CLOSING))	{
		RET_INT(-3, nil_str);
	}

	RET_INT(0, nil_str);
}

n4 epoll_handle_udp_snd(vp epoll, vp event)
{
	if (IS_NL(epoll) || IS_NL(event))	{
		RET_INT(-1, "@epoll = %p, @event = %p", epoll, event);
	}

	//a udp package should be sent out when you handle a received udp package.
	//sock epollr can not send out a udp package for EPOLLOUT event because udp
	//type event did not store peer sock address.
	//delete write event

	RET_INT(0, nil_str);
}

//////////////////////////////////////////
// socket end
//////////////////////////////////////////

n4 set_max_sock_num(n4 max_sock_num);
n4 create_udp_end(n4* sock_ptr, SAEX_P addr);
n4 create_tcp_end(n4* sock_ptr, SAEX_P addr);
n4 create_tcp_client(n4* sock_ptr, SAEX_P addr);
n4 create_tcp_server(n4* sock_ptr, SAEX_P addr);

n4 set_max_sock_num(n4 max_sock_num)
{
	n4				ret;
	u4				uid;
	struct rlimit	limit;

	uid = geteuid();
	if (IS_NZR(uid))	{
		// root's uid is 0
		RET_INT(0, nil_str);
	}

	//to invoke @setrlimit, your should have root's right
	if (IS_ZR(getrlimit(RLIMIT_NOFILE, &limit)))	{
		limit.rlim_max = limit.rlim_cur = max_sock_num;
		ret = setrlimit(RLIMIT_NOFILE, &limit);
		if (IS_NZR(ret))	{
			OUTPUT_ERROR("setrlimit(RLIMIT_NOFILE, &limit = %p) = %d", &limit, ret);
			RET_INT(-2, nil_str);
		}
	}

	RET_INT(0, nil_str);
}

n4 create_udp_end(n4* sock_ptr, SAEX_P addr)
{
	n4		ret;
	n4		sock;

	if (IS_NL(sock_ptr) || IS_NL(addr))	{
		RET_INT(-1, "@sock_ptr = %p, @addr = %p", sock_ptr, addr);
	}

	sock = INVALID_SOCK;
	ret = create_udp_sock(&sock);
	if (IS_NZR(ret))	{
		RET_INT(-2, "create_udp_sock(sock = %d) = %d", sock, ret);
	}

	ret = set_sock_addr_reuse(sock);
	if (IS_NZR(ret))	{
		close(sock);
		RET_INT(-2, "set_sock_addr_reuse(sock = %d) = %d", sock, ret);
	}

	ret = set_sock_nonblock(sock);
	if (IS_MN(ret))	{
		close(sock);
		RET_INT(-2, "set_sock_nonblock(sock = %d) = %d",sock, ret);
	}

	ret = bind_addr(sock, addr);
	if (IS_NZR(ret))	{
		close(sock);
		RET_INT(-2, "bind_addr(sock = %d, addr = %p) = %d", sock, addr, ret);
	}

	*sock_ptr = sock;

	RET_INT(0, nil_str);
}

n4 create_tcp_end(n4* sock_ptr, SAEX_P addr)
{
	n4		ret;
	n4		sock;

	if (IS_NL(sock_ptr))	{
		RET_INT(-1, "@sock_ptr = %p", sock_ptr);
	}

	sock = INVALID_SOCK;
	ret = create_tcp_sock(&sock);
	if (IS_NZR(ret))	{
		RET_INT(-2, "create_tcp_sock(sock = %d) = %d", sock, ret);
	}

	ret = set_sock_addr_reuse(sock);
	if (IS_NZR(ret))	{
		close(sock);
		RET_INT(-2, "set_sock_addr_reuse(sock = %d) = %d", sock, ret);
	}

	ret = set_sock_nonblock(sock);
	if (IS_MN(ret))	{
		close(sock);
		RET_INT(-2, "set_sock_nonblock(sock = %d) = %d", sock, ret);
	}

	ret = set_sock_nonnagle(sock);
	if (IS_MN(ret))	{
		close(sock);
		RET_INT(-2, "set_sock_nonnagle(sock = %d) = %d", sock, ret);
	}

	if (IS_NNL(addr))	{
		ret = bind_addr(sock, addr);
		if (IS_NZR(ret)) {
			close(sock);
			RET_INT(-2, "bind_addr(sock = %d, addr = %p) = %d", sock, addr, ret);
		}
	}

	*sock_ptr = sock;

	RET_INT(0, nil_str);
}

n4 create_tcp_client(n4* sock_ptr, SAEX_P addr)
{
	n4	ret;

	if (IS_NL(sock_ptr))	{
		RET_INT(-1, "@sock_ptr = %p", sock_ptr);
	}

	ret = create_tcp_end(sock_ptr, addr);

	RET_INT(ret, nil_str);
}

n4 create_tcp_server(n4* sock_ptr, SAEX_P addr)
{
	n4	ret;

	if (IS_NL(sock_ptr) || IS_NL(addr))	{
		RET_INT(-1, "@sock_ptr = %p, @addr = %p",
					sock_ptr, addr);
	}

	ret = create_tcp_end(sock_ptr, addr);
	if (IS_ZR(ret))	{
		ret = sock_listen(*sock_ptr);
	}

	RET_INT(ret, nil_str);
}

//////////////////////////////////////////
//sock drive
//////////////////////////////////////////

//unlimit -n. max connection number for a tcp server at a time
#define	MAX_END_NUM		64
#define	TCP_PEER_NUM	1152
#define	MAX_SOCK_NUM	500 * TCP_PEER_NUM
#define	MAX_event_NUM	64

typedef enum EDRIVE_STATE_TAG	{
	EDRIVE_INIT	= 0x1 << 0,
	EDRIVE_LOOP	= 0x1 << 1,
	EDRIVE_EXIT	= 0x1 << 2,
} EDRIVE_STATE;

typedef struct sock_drive_tag	{
	vp				event;			// event queue
	pthread_key_t	key;			// epoll key
} sock_drive_t, *sock_drive_p, sock_drive_a[1];

n4 sock_drive_init(vpp drive_pptr)
{
	n4				ret;
	u4				len;
	u4				size;
	sock_drive_p	drive;

	if (IS_NL(drive_pptr))	{
		RET_INT(-1, "@drive = %p", drive_pptr);
	}

	*drive_pptr = nil;
	// drive
	if (IS_NL(ALLOC(drive, sizeof(sock_drive_t))))	{
		RET_INT(-2, "drive ALLOC(%zu) = nil", sizeof(sock_drive_t));
	}
	SET_ZERO(drive, sizeof(*drive));

	// to avoid sock drive invoke mmap / sbrk too frequently
	mallopt(M_MMAP_THRESHOLD, SIZE_M(2));	// mmap
	mallopt(M_TRIM_THRESHOLD, SIZE_M(8));	// sbrk

	// to insure the current process donot exhuast the number of file description
	set_max_sock_num(MAX_SOCK_NUM);

	// epoll key
	ret = pthread_key_create(&(drive->key), epoll_uninit);
	if (IS_NZR(ret))	{
		OUTPUT_ERROR("pthread_key_create(&key = %p, @func = %p) = %d",
					&(drive->key), epoll_uninit, ret);
		sock_drive_uninit((vpp)(&drive));
		RET_INT(-2, nil_str);
	}

	// event queue
	len = sizeof(vp);
	ret = deque_init(drive->event, len);
	if (IS_NZR(ret))	{
		sock_drive_uninit((vpp)(&drive));
		RET_INT(-2, "deque_init(event_ptr, len = %u) = %d", len, ret);
	}

	size = SIZE_K(8);
	len = sizeof(vp);
	ret = ring_init(((vpp)&(s_event_ring)), size, len);
	if (IS_NZR(ret))	{
		sock_drive_uninit((vpp)(&drive));
		RET_INT(-2, "ring_init(&s_event_ring = %p, num = %u, len = %u) = %d",
					&s_event_ring, size, len, ret);
	}

	*drive_pptr = drive;

	RET_INT(0, nil_str);
}

vd sock_drive_uninit(vpp drive_pptr)
{
	n4				ret;
	vp				event;
	vp				ring;
	sock_drive_p	drive;

	drive = (sock_drive_p)(*drive_pptr);
	if (IS_NL(drive_pptr) || IS_NL(drive))	{
		RET_VD("@drive_pptr = %p, @drive = %p", drive_pptr, drive);
	}

	ring = s_event_ring;
	s_event_ring = nil;
	do	{
		ret = ring_pop(ring, ((vp)(&event)));
		if (IS_NZR(ret))	{
			break;
		}

		event_uninit(&event);
	} while(1);
	ring_uninit(((vpp)(&ring)));

	// event queue
	if (IS_NNL(drive->event))	{
		do	{
			ret = deque_pop_front(drive->event, ((vp)(&event)));
			if (IS_NZR(ret))	{
				break;
			}
			event_uninit(&event);
		} while(1);

		deque_uninit(drive->event);
	}

	// drive
	DEALLOC(*drive_pptr);

	RET_VD(nil_str);
}

n4 sock_drive_add_key(vp drive_ptr, vp key)
{
	n4				ret;
	vp				queue;
	sock_drive_p	drive;

	if (IS_NL(drive = drive_ptr) || IS_NL(key))	{
		RET_INT(-1, "@drive_ptr = %p, @key_ptr = %p", drive, key);
	}

	queue = drive->event;
	ret = deque_push_back(queue, ((vp)(&key)));
	if (IS_NZR(ret))	{
		RET_INT(-2, "deque_push_back(event_ptr = %p, key_ptr = %p) = %d",
					drive->event, ((vp)(&key)), ret);
	}

	RET_INT(0, nil_str);
}

n4 sock_drive_run(vp drive_ptr)
{
	n4					ep_num;
	n4					num;
	n4					ret;
	n4					state;
	n4					type;
	f4					avg;
	event_p				event;
	epoll_p				epoll;
	sock_drive_p		drive;
	static lisk_atom_t	epoll_num = {0};

	if (IS_NL(drive = drive_ptr))	{
		RET_INT(-1, "@drive = %p", drive);
	}

	if (IS_NL(epoll = pthread_getspecific(drive->key)))	{
		ret = epoll_init((vpp)(&epoll));
		if (IS_NZR(ret))	{
			// thread exit
			RET_INT(-3, nil_str);
		}
		epoll_set_event_queue(epoll, drive->event);
		ret = pthread_setspecific(drive->key, epoll);
		if (IS_NZR(ret))	{
			// thread exit
			RET_INT(-3, nil_str);
		}

		lisk_atom_add(epoll_num, 1);
	}

	num = lisk_atom_read(g_event_num);	// sock_drive's total event_t number
	ep_num = lisk_atom_read(epoll_num);	// total epoll_t number
	avg = (f4)(num) / ep_num;			// average event_t number
	epoll_get_event_num(epoll, &num);
	if (IS_GT(avg, num) || IS_FZ(avg - num) || IS_ZR(num) || IS_EQ(ep_num, 1))	{
		ret = deque_pop_front(drive->event, ((vp)(&event)));
		if (IS_ZR(ret))	{
			type = ESOCK_READ;
			event_get_state(event, &state);
			if (IS_EQ(state, EEVENT_TCP_CONNECTING))	{
				type = ESOCK_WRITE;
			}
			epoll_add_event(epoll, event, type);
			lisk_atom_add(g_event_num, 1);
		}
	}

	ret = epoll_run(epoll);
	if (IS_NZR(ret))	{
		RET_INT(-2, "epoll_run(epoll_ptr = %p) = %d", epoll, ret);
	}

	RET_INT(0, nil_str);
}

//////////////////////////////////////////
// sock_key
//////////////////////////////////////////

n4 sock_key_bind(vp key_ptr)
{
	n4			ret;
	n4			sock;
	n4			state;
	n4			type;
	SAEX		addr;
	sock_key_p	key;
	callback_p	callback;

	if (IS_NL(key = key_ptr))	{
		RET_INT(-1, "@key_ptr = %p", key);
	}

	type = (n4)(key->type);
	if (IS_NBE(type, ESOCK_TCP_CLIENT, ESOCK_UDP) || IS_ZR(key->timeout))	{
		RET_INT(-1, "type = %d, type value range[%u %u], timeout = %u",
					type, ESOCK_TCP_CLIENT, ESOCK_UDP, key->timeout);
	}
	callback = &(key->callback);
	if (IS_NL(callback->handle_udp_pkg) &&
		(IS_NL(callback->handle_tcp_stream) ||
		 IS_NL(callback->handle_tcp_pkg)))	{
			RET_INT(-1, "handle_udp_pkg = %p, "
						"handle_tcp_stream = %p, handle_tcp_pkg = %p",
						callback->handle_udp_pkg,
						callback->handle_tcp_stream, callback->handle_tcp_pkg)
	}

	//creat sock && bind addr
	ret = -2;
	state = EEVENT_INIT;
	switch (type)	{
	case ESOCK_TCP_SERVER:	{
		ret = create_tcp_server(&sock, &(key->local));
		state = EEVENT_TCP_LISTEN;
	}
	break;

	case ESOCK_UDP:	{
		ret = create_udp_end(&sock, &(key->local));
		state = EEVENT_UDP;
	}
	break;

	default:	{ // ESOCK_TCP_CLIENT
		ret = create_tcp_client(&sock, &(key->local));
		state = EEVENT_TCP_BIND;
	}
	break;
	}
	if (IS_NZR(ret))	{
		RET_INT(-2, "type = %d, create_end ret = %d", type, ret);
	}

	ret = get_sock_addr(sock, &addr);
	if (IS_NZR(ret))	{
		close(sock);
		RET_INT(-2, "get_sock_addr(sock = %d, &addr = %p) = %d",
					sock, &addr, ret);
	}
	CPY(&(key->local), &addr, sizeof(addr));

	ret = event_init((vpp)(&key), nil);
	if (IS_NZR(ret))	{
		close(sock);
		RET_INT(-2, "event_init(event = %p, &key = %p) = %d", &key, nil, ret);
	}

	ret = event_set_sock(key_ptr, sock);
	if (IS_NZR(ret))	{
		event_uninit(&key_ptr);
		RET_INT(-2, "event_set_sock(event = %p, sock = %d) = %d",
					key, sock, ret);
	}

	ret = event_set_state(key_ptr, state);
	if (IS_NZR(ret))	{
		event_uninit(&key_ptr);
		RET_INT(-2, "event_set_state(event = %p, state = %d) = %d",
					&key_ptr, state, ret);
	}

	RET_INT(0, nil_str);
}

#define	MAX_RETRY_TIME	6
n4 sock_key_connect(vp key_ptr, n4 retry)
{
	n4				idx;
	n4				ret;
	n4				sock;
	n4				state;
	n4				interval;
	event_p			event;
	net_addr_a		addr;

	if (IS_NL(event = key_ptr))	{
		RET_INT(-1, "@key_ptr = %p", event);
	}

	event_get_state(event, &state);
	if (IS_NEQ(state, EEVENT_TCP_BIND))	{
		RET_INT(-1, "event state = %d, Eevent_TCP_BIND = %u",
					state, EEVENT_TCP_BIND);
	}

	// check self-connect
	if (IS_EQ(event->local.sin_addr.s_addr, event->peer.sin_addr.s_addr) &&
		IS_EQ(event->local.sin_port, event->peer.sin_port))	{
		na_2_sa(addr, &(event->peer));
		RET_INT(-1, "the @key want to connect itself. local addr{%s-%hu}",
					addr[0].ip, addr[0].port);
	}

	ret = event_get_sock(event, &sock);
	if (IS_NZR(ret) || IS_NPN(sock))	{
		RET_INT(-1, "event_get_sock(event_ptr = %p, &sock = %p) = %d, sock = %d",
					event, &sock, ret, sock);
	}

	ret = connect_server(sock, &(event->peer));
	if (IS_MN(ret))	{
		do {
			if (IS_EQ(ret, -3) && IS_NZR(retry))	{
				interval = 300;		// 300ms
				for (idx = 0; IS_LT(idx, MAX_RETRY_TIME); idx++) {
					msleep(interval, 0);
					ret = connect_server(sock, &(event->peer));
					if (IS_NEQ(ret, -3)) {
						break;
					}
					interval *= 2;
				}
			}

			if (IS_MN(ret))	{
				na_2_sa(addr, &(event->peer));
				event_uninit((vpp)(&event));
				RET_INT(ret, "connect_server(sock = %d, peer = {%s-%u}) = %d",
								sock, addr[0].ip, addr[0].port, ret);
			}
		} while (0);
	}

	state = IS_ZR(ret) ? EEVENT_TCP_CONNECTED : EEVENT_TCP_CONNECTING;
	ret = event_set_state(event, state);
	if (IS_NZR(ret))	{
		event_uninit((vpp)(&event));
		RET_INT(-2, "event_set_state(event = %p, state = %d) = %d", event, state, ret);
	}

	RET_INT(0, nil_str);
}

n4 sock_key_check(vp key)
{
	n4			ret;
	n4			sock;
	n4			state;
	u4			index;
	net_addr_a	local;
	net_addr_a	peer;

	if (IS_NL(key))	{
		RET_INT(-1, "@key = %p", key);
	}

	event_get_sock(key, &sock);
	event_get_state(key, &state);
	if (IS_INVALID_SOCK(sock) ||
		IS_NEQ(state, EEVENT_TCP_CONNECTED))	{
		ret = -2;
		if (IS_EQ(state, EEVENT_TCP_CONNECTING)) {
			ret = 1;
		}

		event_get_index(key, &index);
		event_get_local_addr2(key, local);
		event_get_peer_addr2(key, peer);
		RET_INT(ret, "@key(index = %hu, sock = %d, local{%s-%hu}, peer{%s-%hu})",
					index, sock, local[0].ip, local[0].port, peer[0].ip, peer[0].port);
	}

	RET_INT(0, nil_str);
}

n4 sock_key_send(vp key, vp pkg, n4 len)
{
	n4			ret;
	event_p		event;
	lisk_buf_a	buf;

	if (IS_NL(event = key) || IS_NL(pkg) || IS_NPN(len))	{
		RET_INT(-1, "@key = %p, @pkg = %p, @len = %d", key, pkg, len);
	}

	buf[0].data = pkg;
	buf[0].size = len;
	ret = event_write(event, buf);
	if (IS_NZR(ret))	{
		RET_INT(ret, "event_write(event = %p, buf{data = %p, size = %u}) = %d",
					event, buf[0].data, buf[0].size, ret);
	}

	RET_INT(0, nil_str);
}

// linux memory: .text, .rodata, .data(global scope var->file->function)
// .bss(global scope var->file->function), heap, mmap, stack, kernel(1G)

