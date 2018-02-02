/**
 * FILE		: lisk_sock.c
 * DESC		: network related fuctions
 * AUTHOR	: v0.3.0 written by Alex Stocks
 * DATE		: on Dec 5, 2012
 * MOD		: modified by Alex Stocks on Jan 11, 2012
 * MOD		: modified by Alex Stocks on March 5, 2013
 * MOD		: modified by Alex Stocks on Feb 17, 2014 (lisk_sock
 *			  + lisk_network = old lisk_network)
 **/

#include <lisk_log.h>
#include <lisk_sock.h>

#include <stdio.h>
#include <sys/types.h>
//#include <sys/socket.h>			//setsockopt, socket, setsockopt, bind,
								//sendmsg, recvmsg, getpeername, accept4
//#include <ifaddrs.h>			//getifaddrs
//#include <net/if.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>			//for in_addr, inet_ntop, inet_pton
//#include <netinet/tcp.h>		//TCP_NODELAY
#include <fcntl.h>				//fcntl
//#include <sys/time.h>
//#include <sys/select.h>			//select

//////////////////////////////////////////
//socket interface
//////////////////////////////////////////

n4 sa_2_na(net_addr_p net_addr, SAEX_P sa)
{
	if (IS_NL(net_addr) || IS_NL(sa)) {
		RET_INT(-1, "@net_addr = %p, @sa = %p", net_addr, sa);
	}

	SET_ZERO(net_addr, sizeof(*net_addr));
	inet_ntop(
			AF_INET,
			&(sa->sin_addr),
			net_addr->ip,
			sizeof(net_addr->ip)
			);
	net_addr->port = ntohs((u2)(sa->sin_port));

	RET_INT(0, nil_str);
}

n4 na_2_sa(net_addr_p net_addr, SAEX_P sa_p)
{
	if (IS_NL(net_addr) || IS_NL(sa_p)) {
		RET_INT(-1, "@net_addr = %p, @sa_p = %p", net_addr, sa_p);
	}

	SET_ZERO(sa_p, sizeof(*sa_p));
	sa_p->sin_family = AF_INET;
	inet_pton(AF_INET, net_addr->ip, &(sa_p->sin_addr));
	sa_p->sin_port = net_addr->port;
	sa_p->sin_port = htons(net_addr->port);

	RET_INT(0, nil_str);
}

n4 get_peer_addr(n4 sock, SAEX_P addr)
{
	n4			ret;
	socklen_t	salen;

	if (IS_LT(sock, 0) || IS_NL(addr)) {
		RET_INT(-1, "@sock = %d, @addr = %p", sock, addr);
	}

	salen = sizeof(*addr);
	ret = getpeername(sock, (SA*)addr, &salen);
	if (IS_EQ(ret, -1)) {
		RET_INT(-1, "getpeername(sock = %d) == %d", sock, ret);
	}

	RET_INT(0, nil_str);
}

n4 get_sock_addr(n4 sock, SAEX_P addr)
{
	n4			ret;
	socklen_t	sa_len;

	if (IS_LT(sock, 0) || IS_NL(addr)) {
		RET_INT(-1, "@sock = %d, @addr = %p", sock, addr);
	}

	sa_len = sizeof(*addr);
	ret = getsockname(sock, (SA*)addr, &sa_len);
	if (IS_EQ(ret, -1)) {
		RET_INT(-2, "getsockname(sock = %d) = %d", sock, ret);
	}

	RET_INT(0, nil_str);
}

n4 get_local_ip(n1 ip[32])
{
	n4				ret;
	const char		eth[] = {"eth"};
	u4				ip_len;
	struct ifaddrs*	addr;
	struct ifaddrs*	addr_idx;
	n1				buf[INET_ADDRSTRLEN];	//16

	if (IS_NL(ip)) {
		RET_INT(-1, "@ip = %p", ip);
	}

	ret = getifaddrs(&addr);
	if (IS_NZR(ret))	{
		RET_INT(-2, "failed to get local ip");
	}

	addr_idx = addr;
	while (IS_NNL(addr_idx)) {
		if (IS_EQ(addr_idx->ifa_addr->sa_family, AF_INET)) {
			inet_ntop(
					AF_INET,
					&(((SAEX_P)(addr_idx->ifa_addr))->sin_addr),
					buf,
					INET_ADDRSTRLEN
					);

			if (IS_ZR(strncmp(
							addr_idx->ifa_name,
							eth,
							strlen(eth)
							)))	{
				ip_len = 32 * sizeof(char) - 1;
				strncpy(ip, buf, ip_len);
				ip[ip_len] = '\0';
				break;
			}
		}

		addr_idx = addr_idx->ifa_next;
	}

	freeifaddrs(addr);

	//loop to the end of addr list
	if (IS_NL(addr_idx))	{
		RET_INT(-2, "failed to get local ip");
	}

	RET_INT(0, nil_str);
}

n4 get_local_ip_ex(n1 ip[64])
{
	const char		eth[] = {"eth"};
	u4				ip_len;
	n4				ret;
	struct ifaddrs*	addr;
	struct ifaddrs*	addr_idx;
	n1				buf[INET6_ADDRSTRLEN];	//46

	if (IS_NL(ip)) {
		RET_INT(-1, "@ip = %p", ip);
	}

	ret = getifaddrs(&addr);
	if (IS_NZR(ret))	{
		RET_INT(-2, "failed to get local ip");
	}

	addr_idx = addr;
	while (IS_NNL(addr_idx)) {
		if (IS_EQ(addr_idx->ifa_addr->sa_family, AF_INET6)) {
			inet_ntop(
					AF_INET6,
					&(((SAEX_P)(addr_idx->ifa_addr))->sin_addr),
					buf,
					INET6_ADDRSTRLEN
					);
			if (IS_ZR(strncmp(
							addr_idx->ifa_name,
							eth,
							strlen(eth)
							)))	{
				ip_len = 64 * sizeof(char) - 1;
				strncpy(ip, buf, ip_len);
				ip[ip_len] = '\0';
				break;
			}
		}

		addr_idx	= addr_idx->ifa_next;
	}

	freeifaddrs(addr);

	//loop to the end of addr list
	if (IS_NL(addr_idx))	{
		RET_INT(-2, "failed to get local ip");
	}

	RET_INT(0, nil_str);
}

n4 set_sock_nonblock(n4 sock)
{
	n4	stat;
	n4	ret;

	stat = fcntl(sock, F_GETFL, 0);
	stat |= O_NONBLOCK;
	ret = fcntl(sock, F_SETFL, stat);
	if (IS_LT(ret, 0)) {
		OUTPUT_ERROR("fcntl(sock = %d, F_SETFL, O_NONBLOCK) = %d", sock, ret);
		RET_INT(-1, nil_str);
	}

	stat = fcntl(sock, F_GETFD, 0);
	stat |= FD_CLOEXEC;
	fcntl(sock, F_SETFD, stat);

	RET_INT(0, nil_str);
}

n4 set_sock_addr_reuse(n4 sock)
{
	n4	opt;
	n4	ret;

	opt = 1;
	ret = setsockopt(
					sock,
					SOL_SOCKET,
					SO_REUSEADDR,
					&opt,
					sizeof(opt)
					);
	if (IS_LT(ret, 0)) {
		OUTPUT_ERROR("setsockopt(sock = %d, SO_REUSEADDR) = %d", sock, ret);
		RET_INT(-1, nil_str);
	}

	RET_INT(0, nil_str);
}

n4 set_sock_nonnagle(n4 sock)
{
	n4	opt;
	n4	ret;

	opt = 1;
	ret = setsockopt(
					sock,
					IPPROTO_TCP,
					TCP_NODELAY,
					&opt,
					sizeof(opt)
					);
	if (IS_LT(ret, 0)) {
		OUTPUT_ERROR("setsockopt(sock = %d, TCP_NODELAY) = %d", sock, ret);
		RET_INT(-1, nil_str);
	}

	RET_INT(0, nil_str);
}

n4 get_sock_snd_timeout(n4 sock, n4* timeout)
{
	n4				ret;
	u4				len;
	struct	timeval	tv;

	if (IS_LT(sock, 0) || IS_NL(timeout)) {
		RET_INT(-1, "@sock = %d, @timeout = %p", sock, timeout);
	}

	len = sizeof(tv);
	ret = getsockopt(
					sock,
					SOL_SOCKET,
					SO_SNDTIMEO,
					(n1*)(&tv),
					&len
					);
	if (IS_LT(ret, 0)) {
		OUTPUT_ERROR("getsockopt(sock = %d, SO_SNDTIMEO) = %d", sock, ret);
		RET_INT(-2, nil_str);
	}
	*timeout = tv.tv_sec * 1000 + tv.tv_usec / 1000;

	RET_INT(0, nil_str);
}

n4 set_sock_snd_timeout(n4 sock, n4 timeout)
{
	n4				ret;
	struct	timeval	tv;

	if (IS_NPN(sock)) {
		RET_INT(-1, "@sock = %d", sock);
	}

	tv.tv_sec	= timeout / 1000;
	tv.tv_usec	= (timeout % 1000) * 1000000;
	ret = setsockopt(
					sock,
					SOL_SOCKET,
					SO_SNDTIMEO,
					(const n1*)(&tv),
					sizeof(tv)
					);
	if (IS_LT(ret, 0)) {
		OUTPUT_ERROR("setsockopt(sock = %d, timeout = %d) = %d", sock, timeout, ret);
		RET_INT(-2, nil_str);
	}

	RET_INT(0, nil_str);
}

n4 get_sock_rcv_timeout(n4 sock, n4* timeout)
{
	n4				ret;
	u4				len;
	struct	timeval	tv;

	if (IS_LT(sock, 0) || IS_NL(timeout)) {
		RET_INT(-1, "@sock = %d, @timeout = %p", sock, timeout);
	}

	len = sizeof(tv);
	ret = getsockopt(
					sock,
					SOL_SOCKET,
					SO_RCVTIMEO,
					(n1*)(&tv),
					&len
					);
	if (IS_LT(ret, 0)) {
		OUTPUT_ERROR("getsockopt(sock = %d, SO_SNDTIMEO) = %d", sock, ret);
		RET_INT(-2, nil_str);
	}
	*timeout = tv.tv_sec * 1000 + tv.tv_usec / 1000;

	RET_INT(0, nil_str);
}

// for blocking socket
n4 set_sock_rcv_timeout(n4 sock, n4 timeout)
{
	n4				ret;
	struct	timeval	tv;

	if (IS_NPN(sock)) {
		RET_INT(-1, "@sock = %d", sock);
	}

	tv.tv_sec	= timeout / 1000;
	tv.tv_usec	= (timeout % 1000) * 1000000;
	ret = setsockopt(
					sock,
					SOL_SOCKET,
					SO_RCVTIMEO,
					(const n1*)(&tv),
					sizeof(tv)
					);
	if (IS_LT(ret, 0)) {
		OUTPUT_ERROR("setsockopt(sock = %d, timeout = %d) = %d", sock, timeout, ret);
		RET_INT(-2, nil_str);
	}

	RET_INT(0, nil_str);
}

n4 get_sock_sndbuf_size(n4 sock, n4* size)
{
	n4	ret;
	u4	size_len;

	if (IS_LT(sock, 0) || IS_NL(size)) {
		RET_INT(-1, "@sock = %d, @size = %p", sock, size);
	}

	size_len = sizeof(*size);
	ret = getsockopt(
					sock,
					SOL_SOCKET,
					SO_SNDBUF,
					(n1*)size,
					&size_len
					);
	if (IS_LT(ret, 0)) {
		OUTPUT_ERROR("getsockopt(sock = %d, SO_SNDBUF) = %d", sock, ret);
		RET_INT(-2, nil_str);
	}

	RET_INT(0, nil_str);
}

n4 set_sock_sndbuf_size(n4 sock, n4 size)
{
	u4	size_len;
	n4	ret;
	n4	cur_buf_size;

	cur_buf_size = 0;
	size_len = sizeof(cur_buf_size);
	ret = getsockopt(
					sock,
					SOL_SOCKET,
					SO_SNDBUF,
					&cur_buf_size,
					&size_len
					);
	if (IS_LT(ret, 0)) {
		OUTPUT_ERROR("getsockopt(sock = %d, SO_SNDBUF) = %d", sock, ret);
		RET_INT(-1, nil_str);
	}

	if (IS_LT(size, cur_buf_size)) {
		//no socket data buffer, send data to peer imediately
		size = 0;
	}

	ret = setsockopt(
					sock,
					SOL_SOCKET,
					SO_SNDBUF,
					(const n1*)&size,
					sizeof(size)
					);
	if (IS_LT(ret, 0)) {
		OUTPUT_ERROR("getsockopt(sock = %d, SO_SNDBUF) = %d", sock, ret);
		RET_INT(-1, nil_str);
	}

	RET_INT(0, nil_str);
}

n4 get_sock_rcvbuf_size(n4 sock, n4* size)
{
	if (IS_LT(sock, 0) || IS_NL(size)) {
		RET_INT(-1, "@sock = %d, @size = %p", sock, size);
	}

	u4 size_len = sizeof(*size);
	n4 ret = getsockopt(
						sock,
						SOL_SOCKET,
						SO_RCVBUF,
						size,
						&size_len
						);
	if (IS_LT(ret, 0)) {
		OUTPUT_ERROR("getsockopt(sock = %d, SO_RCVBUF) = %d", sock, ret);
		RET_INT(-2, nil_str);
	}

	RET_INT(0, nil_str);
}

n4 set_sock_rcvbuf_size(n4 sock, n4 size)
{
	n4	ret;
	n4	cur_buf_size = 0;
	u4	size_len = sizeof(cur_buf_size);

	ret = getsockopt(
					sock,
					SOL_SOCKET,
					SO_RCVBUF,
					&cur_buf_size,
					&size_len
					);
	if (IS_LT(ret, 0)) {
		OUTPUT_ERROR("getsockopt(sock = %d, SO_RCVBUF) = %d",
						sock, ret);
		RET_INT(-1, nil_str);
	}

	if (IS_LT(size, cur_buf_size)) {
		//no socket data buffer, send data to app imediately
		size = 0;
	}

	ret = setsockopt(
					sock,
					SOL_SOCKET,
					SO_RCVBUF,
					(const n1*)&size,
					sizeof(size)
					);
	if (IS_LT(ret, 0)) {
		OUTPUT_ERROR("getsockopt(sock = %d, SO_RCVBUF) = %d",
						sock, ret);
		RET_INT(-1, nil_str);
	}

	RET_INT(0, nil_str);
}

n4 create_udp_sock(n4* sock_ptr)
{
	n4	sock;

	if (IS_NL(sock_ptr)) {
		RET_INT(-1, "@sock = %p", sock_ptr);
	}

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (IS_EQ(sock, -1)) {
		OUTPUT_ERROR("socket(IPPROTO_UDP) = -1");
		RET_INT(-2, nil_str);
	}

	*sock_ptr = sock;

	RET_INT(0, nil_str);
}

n4 create_tcp_sock(n4* sock_ptr)
{
	n4	sock;

	if (IS_NL(sock_ptr)) {
		RET_INT(-1, "@sock = %p", sock_ptr);
	}

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (IS_EQ(sock, -1)) {
		OUTPUT_ERROR("socket(IPPROTO_TCP) = -1");
		RET_INT(-2, nil_str);
	}

	*sock_ptr = sock;

	RET_INT(0, nil_str);
}

n4 bind_addr(n4 sock, SAEX_P addr)
{
	n4			ret;
	net_addr_t	na;

	if (IS_LT(sock, 3) || IS_NL(addr)) {
		RET_INT(-1, "@sock = %d, @addr = %p", sock, addr);
	}

	ret = bind(sock, (SA_P)(addr), sizeof(*addr));
	if (IS_LT(ret, 0)) {
		sa_2_na(&na, addr);
		OUTPUT_ERROR("bind(sock = %d, ip = %s, port = %hu) = %d",
					sock, na.ip, na.port, ret);
		RET_INT(-2, nil_str);
	}

	RET_INT(0, nil_str);
}

n4 sock_listen(n4 sock)
{
	n4	ret;

	if (IS_LT(sock, 3)) {
		RET_INT(-1, "@sock = %d", sock);
	}

	/**
	 * Use a backlog of 1024 entries. We pass 1023 to the
	 * listen() call because the kernel does:
	 * backlogsize = roundup_pow_of_two(backlogsize + 1);
	 * which will thus give us a backlog of 1024 entries
	 **/
	ret = listen(sock, 1023);
	if (IS_EQ(ret, -1)) {
		RET_INT(-2, "listen(sock = %d, 1023) = %d",
					sock, ret);
	}

	RET_INT(0, nil_str);
}

n4 connect_server(n4 sock, SAEX_P srv_addr)
{
	n4			ret;
	n4			rval;
	net_addr_a	addr;

	if (IS_LT(sock, 0) || IS_NL(srv_addr)) {
		RET_INT(-1, "@sock = %d, @srv_addr = %p", sock, srv_addr);
	}

	rval = 0;
	ret = connect(sock, (SA_P)(srv_addr), sizeof(*srv_addr));
	if (IS_MN(ret)) {
		switch (errno)	{
		case EISCONN: {
			rval = 0;
		}
		break;

		case EINPROGRESS:
		// can not restart connect, page 115 of unp vol 3 version 3
		case EINTR:	{
			rval = 1;
		}
		break;

		case EAGAIN: {
			rval = -3;
		}
		break;

		case EALREADY:
		case ECONNRESET:
		case EHOSTUNREACH:
		case ENETUNREACH:
		case ETIMEDOUT:	{
			rval = -2;
		}
		break;

		default:	{
			rval = -2;
		}
		break;
		}
	}

	if (IS_NZR(rval))	{
		sa_2_na(addr, srv_addr);
		OUTPUT_ERROR("connect(sock = %d, addr{%s-%u} = %p) = %d, errno = %d",
					sock, addr[0].ip, addr[0].port, srv_addr, ret, errno);
	}

	RET_INT(rval, nil_str);
}

n4 sock_read(n4 sock, n1* buf, u4* buf_size, SA_P addr)
{
	n4				ret;
	n4				rval;
	net_addr_t		na;
	struct iovec	iov[1];
	struct msghdr	msg;

	if (IS_MN(sock) || IS_NL(buf) ||
		IS_NL(buf_size) || IS_NPN(*buf_size))	{
		RET_INT(-1, "@sock = %d, @buf = %p, "
					"@buf_size = %p, @addr = %p",
					sock, buf, buf_size, addr);
	}

	SET_ZERO(&msg, sizeof(msg));
	iov[0].iov_base		= (vp)(buf);
	iov[0].iov_len		= *buf_size;
	msg.msg_iov			= (vp)(iov);
	msg.msg_iovlen		= sizeof(iov) / sizeof(struct iovec);
	msg.msg_control		= nil;
	msg.msg_controllen	= 0;
	msg.msg_flags		= 0;
	if (IS_NNL(addr))	{
		msg.msg_name	= (vp)(addr);
		msg.msg_namelen	= sizeof(*addr);
	}

	ret = recvmsg(sock, &msg, 0);
	if (IS_PN(ret))	{
		*buf_size = (u4)ret;
		rval = 0;
	} else if (IS_ZR(ret))	{
		*buf_size = 0;
		rval = -3;
	} else {
		if (IS_EQ(errno, EINTR) ||
			IS_EQ(errno , EAGAIN))	{
			rval = 0;
		} else {
			SET_ZERO(&na, sizeof(na));
			if (IS_NNL(addr))	{
				sa_2_na(&na, (SAEX_P)(addr));
			}
			OUTPUT_ERROR("recvmsg(sock = %d, &msg:{buf = %p, size = %u, "
						"addr:{ip = %s, port = %hu}}= %p, 0) = %d",
						sock, buf, *buf_size, na.ip, na.port, &msg, ret);
			rval = -2;
		}
		*buf_size = 0;
	}

	RET_INT(rval, nil_str);
}

n4 sock_write(n4 sock, n1* buf, u4* buf_size, SA_P addr)
{
	n4				ret;
	n4				rval;
	net_addr_t		na;
	struct iovec	iov[1];
	struct msghdr	msg;

	if (IS_MN(sock) || IS_NL(buf) ||
		IS_NL(buf_size) || IS_NPN(*buf_size))	{
		RET_INT(-1, "@sock = %d, @buf = %p, @buf_size = %p, @addr = %p",
					sock, buf, buf_size, addr);
	}

	SET_ZERO(&msg, sizeof(msg));
	iov[0].iov_base		= (vp)(buf);
	iov[0].iov_len		= *buf_size;
	msg.msg_iov			= (vp)(iov);
	msg.msg_iovlen		= sizeof(iov) / sizeof(struct iovec);
	msg.msg_control		= nil;
	msg.msg_controllen	= 0;
	msg.msg_flags		= 0;
	if (IS_NNL(addr))	{
		msg.msg_name	= (vp)(addr);
		msg.msg_namelen	= sizeof(*addr);
	}

	ret = sendmsg(sock, &msg, 0);
	if (IS_PN(ret))	{
		*buf_size = (u4)ret;
		rval = 0;
	} else if (IS_ZR(ret))	{
		//this may be impossible
		*buf_size = 0;
		rval = 0;
	} else {
		if (IS_EQ(errno, EINTR) || IS_EQ(errno , EAGAIN))	{
			rval = 0;
		} else {
			SET_ZERO(&na, sizeof(na));
			if (IS_NNL(addr))	{
				sa_2_na(&na, (SAEX_P)(addr));
			}
			OUTPUT_ERROR("sendmsg(sock = %d, &msg:{buf = %p, size = %u, "
						"addr:{ip = %s, port = %hu}} = %p, 0) = %d",
						sock, buf, *buf_size, na.ip, na.port, &msg, ret);
			rval = -3;
		}
		*buf_size = 0;
	}

	RET_INT(rval, nil_str);
}

