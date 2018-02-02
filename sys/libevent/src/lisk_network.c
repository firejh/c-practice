/**
 * FILE		: lisk_network.c
 * DESC		: network related fuctions
 * AUTHOR	: v0.3.0 written by Alex Stocks
 * DATE		: on Dec 5, 2012
 * MOD		: modified by Alex Stocks on Jan 11, 2012
 * MOD		: modified by Alex Stocks on March 5, 2013
 **/

#include <lisk_network.h>
#include <lisk_list.h>
#include <lisk_log.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define TIMEOUT_DEFAULT	5

typedef struct event_tag	{
	vp				read_list;
	vp				write_list;
	vp				timeout_list;
	vp				pending_list;
	n4				fd;
	n4				event;
	struct timeval	timeout;
	event_callback	callback;
	vp				arg;
	n4				flag;
} event_t, *event_p, event_a[1];

typedef struct event_config_tag	{
	vp		timeout_list;
	vp		write_list;
	vp		read_list;
	vp		pending_list;
	n4		event_inloop;
	n4		event_max_fd;		/* Highest fd in fd set */
	n4		event_fd_size;
	fd_set*	event_readset;
	fd_set*	event_writeset;
} event_config_t, *event_config_p, event_config_a[1];

static event_config_t s_config = {0};

/* Prototypes */
//vd event_add_post(event_p );

vd event_init(vd)
{
	list_init(&s_config.timeout_list);
	lisk_init(&s_config.write_list);
	lisk_init(&s_config.read_list);
	lisk_init(&s_config.pending_list);
}

/*
 * Called with the highest fd that we know about.  If it is 0, completely
 * recalculate everything.
 */

n4 events_recalc(n4 max)
{
	n4		fd_size;
	fd_set*	readset;
	fd_set*	writeset;

	s_config.event_max_fd = max;

	if (IS_ZR(event_max_fd)) {
		LIST_FOR_EACH_BEGIN(s_config.write_list, event_p, event)
			if (IS_GT(event->fd, s_config.event_max_fd)) {
				s_config.event_max_fd = event->fd;
			}
		LIST_FOR_EACH_END

		LIST_FOR_EACH_BEGIN(s_config.read_list, event_p, event)
			if (IS_GT(event->fd, s_config.event_max_fd)) {
				s_config.event_max_fd = event->fd;
			}
		LIST_FOR_EACH_END
	}

#if !(defined(howmany))
#define	howmany(x, y)		(((x) + ((y) - 1)) / (y))
#endif
	fd_size = howmany(s_config.event_max_fd + 1, NFDBITS) * sizeof(fd_mask);
	if (IS_GT(fd_size, s_config.event_fd_size)) {
		if (IS_NL(REALLOC(s_config.event_readset, fd_size))) {
			RET_INT(-1, "REALLOC(s_config.event_readset, fd_size = %d) = nil", fd_size);
		}

		if (IS_NL(REALLOC(s_config.event_writeset, fd_size))) {
			RET_INT(-1, "REALLOC(s_config.event_writeset, fd_size = %d) = nil", fd_size);
		}

		memset(
				s_config.readset + s_config.event_fd_size,
				0,
				fd_size - s_config.event_fd_size
			);
		memset(
				s_config.writeset + s_config.event_fd_size,
				0,
				fd_size - s_config.event_fd_size
			);

		s_config.event_fd_size = fd_size;
	}

	RET_INT(0, nil_str);
}

n4 event_dispatch(vd)
{
	struct timeval tv;
	event_p ev, *old;
	n4 res, maxfd;

	/* Calculate the initial events that we are waiting for */
	if (events_recalc(0) == -1)
		return (-1);

	while (1) {
		memset(event_readset, 0, event_fd_size);
		memset(event_writeset, 0, event_fd_size);

		TAILQ_FOREACH(ev, &s_config.write_list, write_list)
				FD_SET(ev->fd, event_writeset);

		TAILQ_FOREACH(ev, &s_config.read_list, read_list)
				FD_SET(ev->fd, event_readset);

		timeout_list(&tv);

		if ((res = select(event_max_fd + 1, event_readset, 
				  event_writeset, NULL, &tv)) == -1) {
			if (errno != EINTR) {
				perr("select");
				return (-1);
			}
			continue;
		}

		pinfo((LOG_MISC, 80, __FUNCTION__": select reports %d",
			 res));

		maxfd = 0;
		event_inloop = 1;
		for (ev = TAILQ_FIRST(&s_config.read_list); ev;) {
			old = TAILQ_NEXT(ev, read_list);
			if (FD_ISSET(ev->fd, event_readset)) {
				event_del(ev);
				(*ev->callback)(ev->fd, ESOCK_READ,
						   ev->arg);
			} else if (ev->fd > maxfd)
				maxfd = ev->fd;

			ev = old;
		}

		for (ev = TAILQ_FIRST(&s_config.write_list); ev;) {
			old = TAILQ_NEXT(ev, read_list);
			if (FD_ISSET(ev->fd, event_writeset)) {
				event_del(ev);
				(*ev->callback)(ev->fd, ESOCK_WRITE,
						   ev->arg);
			} else if (ev->fd > maxfd)
				maxfd = ev->fd;

			ev = old;
		}
		event_inloop = 0;

		for (ev = TAILQ_FIRST(&s_config.pending_list); ev; 
		     ev = TAILQ_FIRST(&s_config.pending_list)) {
			TAILQ_REMOVE(&s_config.pending_list, ev, pending_list);
			ev->flag &= ~ESOCK_ADD;
			
			event_add_post(ev);

			if (ev->fd > maxfd)
				maxfd = ev->fd;
		}

		if (events_recalc(maxfd) == -1)
			return (-1);

		timeout_process();
	}

	return (0);
}

vd
event_set(event_p ev, n4 fd, short events,
	  vd (*callback)(n4, short, vd *), vd *arg)
{
	ev->callback = callback;
	ev->arg = arg;
	ev->fd = fd;
	ev->event = events;
	ev->flag = ESOCK_INIT;
}

/*
 * Checks if a specific event is pending or scheduled.
 */

n4
event_pending(event_p ev, short event, struct timeval *tv)
{
	n4 flags = ev->flag;

	/*
	 * We might not have been able to add it to the actual queue yet,
	 * check if we will enqueue later.
	 */
	if (ev->flag & ESOCK_ADD)
		flags |= (ev->event & (ESOCK_READ|ESOCK_WRITE));

	event &= (ESOCK_TIMEOUT|ESOCK_READ|ESOCK_WRITE);

	/* See if there is a timeout that we should report */
	if (tv != NULL && (flags & event & ESOCK_TIMEOUT))
		*tv = ev->timeout;

	return (flags & event);
}

vd
event_add(event_p ev, struct timeval *tv)
{
	pinfo((LOG_MISC, 55,
		 "event_add: event: %p, %s%s%scall %p",
		 ev,
		 ev->event & ESOCK_READ ? "ESOCK_READ " : " ",
		 ev->event & ESOCK_WRITE ? "ESOCK_WRITE " : " ",
		 tv ? "ESOCK_TIMEOUT " : " ",
		 ev->callback));
	if (tv != NULL) {
		struct timeval now;
		event_p tmp;

		gettimeofday(&now, NULL);
		timeradd(&now, tv, &ev->timeout);

		pinfo((LOG_MISC, 55,
			 "event_add: timeout in %d seconds, call %p",
			 tv->tv_sec, ev->callback));
		if (ev->flag & ESOCK_TIMEOUT)
			TAILQ_REMOVE(&s_config.timeout_list, ev, timeout_list);

		/* Insert in right temporal order */
		for (tmp = TAILQ_FIRST(&s_config.timeout_list); tmp;
		     tmp = TAILQ_NEXT(tmp, timeout_list)) {
		     if (timercmp(&ev->timeout, &tmp->timeout, <=))
			     break;
		}

		if (tmp)
			TAILQ_INSERT_BEFORE(tmp, ev, timeout_list);
		else
			TAILQ_INSERT_TAIL(&s_config.timeout_list, ev, timeout_list);

		ev->flag |= ESOCK_TIMEOUT;
	}

	if (event_inloop) {
		/* We are in the event loop right now, we have to
		 * postpone the change until later.
		 */
		if (ev->flag & ESOCK_ADD)
			return;

		TAILQ_INSERT_TAIL(&s_config.pending_list, ev, pending_list);
		ev->flag |= ESOCK_ADD;
	} else
		event_add_post(ev);
}

vd
event_add_post(event_p ev)
{
	if ((ev->event & ESOCK_READ) && !(ev->flag & ESOCK_READ)) {
		TAILQ_INSERT_TAIL(&s_config.read_list, ev, read_list);
		
		ev->flag |= ESOCK_READ;
	}

	if ((ev->event & ESOCK_WRITE) && !(ev->flag & ESOCK_WRITE)) {
		TAILQ_INSERT_TAIL(&s_config.write_list, ev, write_list);

		ev->flag |= ESOCK_WRITE;
	}
}

vd
event_del(event_p ev)
{
	pinfo((LOG_MISC, 80, "event_del: %p, callback %p",
		 ev, ev->callback));

	if (ev->flag & ESOCK_ADD) {
		TAILQ_REMOVE(&s_config.pending_list, ev, pending_list);

		ev->flag &= ~ESOCK_ADD;
	}

	if (ev->flag & ESOCK_TIMEOUT) {
		TAILQ_REMOVE(&s_config.timeout_list, ev, timeout_list);

		ev->flag &= ~ESOCK_TIMEOUT;
	}

	if (ev->flag & ESOCK_READ) {
		TAILQ_REMOVE(&s_config.read_list, ev, read_list);

		ev->flag &= ~ESOCK_READ;
	}

	if (ev->flag & ESOCK_WRITE) {
		TAILQ_REMOVE(&s_config.write_list, ev, write_list);

		ev->flag &= ~ESOCK_WRITE;
	}
}

n4
timeout_list(struct timeval *tv)
{
	struct timeval now;
	event_p ev;

	if ((ev = TAILQ_FIRST(&s_config.timeout_list)) == NULL) {
		timerclear(tv);
		tv->tv_sec = TIMEOUT_DEFAULT;
		return (0);
	}

	if (gettimeofday(&now, NULL) == -1)
		return (-1);

	if (timercmp(&ev->timeout, &now, <=)) {
		timerclear(tv);
		return (0);
	}

	timersub(&ev->timeout, &now, tv);

	pinfo((LOG_MISC, 60, "timeout_list: in %d seconds", tv->tv_sec));
	return (0);
}

vd
timeout_process(vd)
{
	struct timeval now;
	event_p ev;

	gettimeofday(&now, NULL);

	while ((ev = TAILQ_FIRST(&s_config.timeout_list)) != NULL) {
		if (timercmp(&ev->timeout, &now, >))
			break;

		TAILQ_REMOVE(&s_config.timeout_list, ev, timeout_list);
		ev->flag &= ~ESOCK_TIMEOUT;

		pinfo((LOG_MISC, 60, "timeout_process: call %p",
			 ev->callback));
		(*ev->callback)(ev->fd, ESOCK_TIMEOUT, ev->arg);
	}
}

