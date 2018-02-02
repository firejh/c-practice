/**
 * FILE		: lisk_sock.h
 * DESC		: network related functions
 * AUTHOR	: v0.3.0 written by Alex Stocks
 * DATE		: on Oct 1, 2012
 * MOD		: modified by Alex Stocks on Feb 17, 2014 (lisk_sock
 *			  + lisk_network = old lisk_network)
 **/

#ifndef __LISK_SORK_H__
#define __LISK_SORK_H__

#ifdef __cplusplus
extern "C" {
#endif

//#include <netinet/in.h>				//struct sockaddr_in, struct sockaddr
#include <lisk_public.h>

#define	IP_STR_LEN					60
#define	SA							struct sockaddr
#define	SA_P						struct sockaddr*
#define	SAEX						struct sockaddr_in
#define	SAEX_P						struct sockaddr_in*
#if !defined(INVALID_SOCK)
	#define	INVALID_SOCK			-1
#endif
#define	IS_INVALID_SOCK(sock)		((sock) < 3)

#pragma pack(1)

//ip address
typedef struct net_addr_tag {
	n1		ip[IP_STR_LEN];		//16 for ipv4, 46 for ipv6
	u2		port;
} net_addr_t, *net_addr_p, net_addr_a[1];

#pragma pack()

/**
 * description	: get the local ipv4 ip address <br/><br/>
 *
 * in-out@fd	: local ip buffer. its length should not be less than 32 bytes.
 *
 * out-@ret		: ret is 0 if get the local ip. ret is -1 when @ip is nil.
 *				  ret is -2 when fail to get local ip.
 **/
n4 get_local_ip(n1 ip[32]);
/**
 * description	: get the local ipv6 ip address <br/><br/>
 *
 * in-out@fd	: local ip buffer. its length should be not less than 64 bytes.
 *
 * out-@ret		: ret is 0 if get the local ip. ret is -1 when @ip is nil.
 *				  ret is -2 when fail to get local ip.
 **/
n4 get_local_ip_ex(n1 ip[64]);
/**
 * description		: convert form of SA address <br/><br/>
 *
 * in-out@net_addr	: address of net_addr_p type
 * in-@sock_addr	: address of SAEX_P type
 *
 * out-@ret			: if successful the ret is 0.
 *					  ret is -1 when some params are illegal.
 **/
n4 sa_2_na(net_addr_p net_addr, SAEX_P sock_addr);
/**
 * description		: convert form of net address <br/><br/>
 *
 * in-@net_addr		: address of net_addr_t type
 * in-out@sock_addr	: address of SAEX type
 *
 * out-@ret			: if successful the ret is 0.
 *					  ret is -1 when some params are illegal.
 **/
n4 na_2_sa(net_addr_p net_addr, SAEX_P sa_p);
/**
 * description		: get local socket address of @sock <br/><br/>
 *
 * in-@sock			: socket fd
 * in-out@addr		: local address
 *
 * out-@ret			: if successful, ret is 0; otherwise -1 instead.
 **/
n4 get_sock_addr(n4 sock, SAEX_P addr);
/**
 * description		: get peer socket address of @sock <br/><br/>
 *
 * in-@sock			: socket fd
 * in-out@addr		: local address
 *
 * out-@ret			: if successful, ret is 0; otherwise -1 instead.
 **/
n4 get_peer_addr(n4 sock, SAEX_P addr);
/**
 * description	: get a udp socket file from os <br/><br/>
 *
 * in-out@sock	: socket object. it shoule be a pointer of "n4 socket".
 *
 * out-@ret		: if successful, the return is 0 and @sock is a udp socket;
 *				  otherwise -1 instead.
 **/
n4 create_udp_sock(n4* sck);
/**
 * description	: get a tcp socket file from os <br/><br/>
 *
 * in-out@sock	: socket object. it shoule be a pointer of "n4 socket".
 *
 * out-@ret		: if successful, the return is 0 and @sock is a tcp socket;
 *				  otherwise -1 instead.
 **/
n4 create_tcp_sock(n4* sock);
/**
 * description	: set the address of @sock reusable <br/><br/>
 *
 * in-@socd		: socket object
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 set_sock_addr_reuse(n4 sock);
/**
 * description	: to do not use nagle algrithm for tcp protocol <br/><br/>
 *
 * in-@socd		: socket object
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 set_sock_nonnagle(n4 sock);
/**
 * description	: to make sure the nonblock flag of @sock is enable <br/><br/>
 *
 * in-@socd		: socket object
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 set_sock_nonblock(n4 sock);
/**
 * description	: set @sock's sending out times-out in milliseconds. please
 *				  attention that @sock should be blocking socket. <br/><br/>
 *
 * in-@sock		: socket object
 * in-@timeout	: sending out timeout
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead if @sock or
 *				  @timeout is invalid or -2 if lisk fails to set @timeout.
 **/
n4 set_sock_snd_timeout(n4 sock, n4 timeout);
/**
 * description	: set @sock's waiting data times-out in milliseconds. please
 *				  attention that @sock should be a blocking socket. <br/><br/>
 *
 * in-@sock		: socket object
 * in-@timeout	: waiting data times-out
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead if @sock or
 *				  @timeout is invalid or -2 if lisk fails to set @timeout.
 **/
n4 set_sock_rcv_timeout(n4 sock, n4 timeout);
/**
 * description	: bind address @addr to socket @sock <br/><br/>
 *
 * in-@socd		: socket object
 * in-@addr		: ip address
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 bind_addr(n4 sock, SAEX_P addr);
/**
 * description	: listen for connections on it <br/><br/>
 *
 * in-@socd		: socket object
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 sock_listen(n4 sock);
/**
 * description	: connect socket to its peer address @srv_addr <br/><br/>
 *
 * in-@socd		: socket object
 * in-@srv_addr	: peer ip address
 *
 * out-@ret		: if successful, ret is 0. if @sock is less than 3 or
 *				  @srv_addr is nil, the return valus is -1. if @sock
 *				  fails to connect @srv_addr, the return value is -2.
 *				  if the return valus is -3, you can retry to connect
 *				  the peer address. if the return value is 1, you should
 *				  verify the writability of @sock.
 **/
n4 connect_server(n4 sock, SAEX_P srv_addr);
/**
 * description	: read stream from socket @sock <br/><br/>
 *
 * in-@sock		: socket decription
 * in-out@buf	: input buffer
 * in-out@size	: input buffer length. its output value is the
 *				  size of buf that got from @sock
 * in-out@addr	: socket address. if the @addr is not nil, the @sock
 *				  is considered as a udp socket and its address will
 *				  be stored in @addr.
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead if
 *				  params are illegal or -2 if got critical error from
 *				  system or -3 if got zero byte from network.
 **/
n4 sock_read(n4 sock, n1* buf, u4* buf_size, SA_P addr);
/**
 * description	: write buf to the buf of socket @sock <br/><br/>
 *
 * in-@sock		: socket decription
 * in-out@buf	: output buffer
 * in-out@size	: input buffer length. its output value is the size
 *				  of buf that written to @sock
 * in-@addr		: socket address. if the @addr is not nil, the @sock
 *				  is considered as a udp socket.
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 sock_write(n4 sock, n1* buf, u4* buf_size, SA_P addr);

#ifdef __cplusplus
}
#endif

#endif

