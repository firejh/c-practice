/**
 * FILE		: lisk_network.h
 * DESC		: network related functions
 * AUTHOR	: v0.3.0 written by Alex Stocks
 * DATE		: on Oct 1, 2012
 * MOD		: modified by Alex Stocks on Feb 17, 2014
 **/

#ifndef __LISK_NETWORK_H__
#define __LISK_NETWORK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <lisk_atom.h>
#include <lisk_sock.h>

//define a udp or tcp server
typedef enum ESOCK_TYPE_TAG	{
	ESOCK_INIT			= 0x1 << 0,
	ESOCK_TCP_CLIENT	= 0x1 << 1,
	ESOCK_TCP_SERVER	= 0x1 << 2,
	ESOCK_UDP			= 0x1 << 8,
} ESOCK_TYPE;

//callback functions of server, server interface for client
typedef struct callback_tag	{
	/**
	 * description		: handle connection. when the tcp client sock_key @key
	 *					  connects its server successfully, sock drive will
	 *					  invoke @handle_connect. please note that if the return
	 *					  value is -3, sock drive will close this connection. <br/><br/>
	 *
	 * in-@key			: tcp client socket key
	 *
	 * out-@ret			: if successful, ret is 0; otherwise -1 instead if @key
	 *					  is nil which is impossible or -2 if @key is illegal or
	 *					  -3 if you wanna close this connection.
	 **/
	n4 (*handle_connect)(vp key);
	/**
	 * description		: handle accept. when the tcp server @key accepts a
	 *					  new socket key @new_key, the sock drive will invoke
	 *					  this function. you cat set some parameters such as
	 *					  @timeout and @index for @new_key. please note that
	 *					  if the return value is -3, sock drive will close
	 *					  the @new_key. <br/><br/>
	 *
	 * in-@key			: tcp server socket key
	 * in-@new_key		: new socket key
	 *
	 * out-@ret			: if successful, ret is 0; otherwise -1 instead if some
	 *					  params are illegal or -2 if @key or @new_key is illegal.
	 **/
	n4 (*handle_accept)(vp key, vp new_key);

	/**
	 * description		: handle udp package. if its return value is -3, @key will
	 *					  be closed.<br/><br/>
	 *
	 * in-@key			: socket key
	 * in-@rcv_pkg		: received udp package buffer
	 * in-@rcv_pkg_len	: the length of @rcv_pkg
	 *
	 * out-@ret			: if successful, ret is 0; otherwise -1 instead if some
	 *					  params are illegal.
	 *					  this package is illegal.
	 **/
	n4 (*handle_udp_pkg)(vp key, const vp rcv_pkg, n4 rcv_pkg_len);
	/**
	 * description		: get the effective length of a received tcp package. the
	 *					  length of a tcp package is always stored in the @pkg's
	 *					  first 2 or 4 bytes. this "length value" should be its
	 *					  total length, including the length of "the length byets"
	 *					  (2 or 4 bytes) itself. if all parameters are legal but
	 *					  the @rcv_pkg_len is less than the length of "the length
	 *					  byets"(2 or 4 bytes), the return value @pkg_len should be 0.
	 *					  please note that if the return value is -3, sock drive
	 *					  will close the @key. <br/><br/>
	 *
	 * in-@key			: socket key
	 * in-@rcv_buf		: received tcp package buffer
	 * in-@rcv_buf_len	: the length of @rcv_buf
	 * in-out@pkg_len	: effective length of this package
	 *
	 * out-@ret			: if successful, ret is 0; otherwise -1 instead if some
	 *					  params are illegal.
	 **/
	n4 (*handle_tcp_stream)(vp key, const vp rcv_buf, n4 rcv_buf_len, n4* pkg_len);
	/**
	 * description		: handle tcp package. sock drive will invoke this function
	 *					  after it get get the length of the received tcp package
	 *					  @rcv_pkg by @handle_tcp_stream. please note that if the
	 *					  return value is -3, sock drive will close the @key. <br/><br/>
	 *
	 * in-@key			: socket key
	 * in-@rcv_pkg		: received tcp package buffer
	 * in-@rcv_pkg_len	: the length of @rcv_pkg
	 *
	 * out-@ret			: if successful, ret is 0; otherwise -1 instead if some
	 *					  params are illegal.
	 **/
	n4 (*handle_tcp_pkg)(vp key, const vp rcv_pkg, n4 rcv_pkg_len);
	/**
	 * description		: handle connection closed event. when a tcp connection is
	 *					  closed, sock drive will invoke @handle_close. <br/><br/>
	 *
	 * in-@key			: socket key
	 *
	 * out-@ret			: if successful, ret is 0; otherwise -1 instead if @key
	 *					  is nil or -2 if the @key is illegal.
	 **/
	n4 (*handle_close)(vp key);
	/**
	 * description		: handle timeout event. every sock_key has a life cycle
	 *					  "timeout". when it runs out, sock drive will invoke
	 *					  @handle_timeout. if its return value is -3 and the sock
	 *					  key type is not ESOCK_TCP_SERVER, sock drive will close
	 *					  the @key. <br/><br/>
	 *
	 * in-@key			: socket key
	 *
	 * out-@ret			: if successful, ret is 0; otherwise -1 instead if @key is
	 *					  nil or -2 if the @key is illegal.
	 **/
	n4 (*handle_timeout)(vp key);
} callback_t, *callback_p, callback_a[1];

extern const n4 DEF_SOCK_KEY_TIMEOUT;

//define socket key to distinguish differnt sockets
typedef struct sock_key_tag	{
	u2					index;			// socket key index.
	// the type of server. its value range is [ESOCK_TCP_CLIENT, ESOCK_UDP].
	volatile u2			type;
	// socket key life circle. it unit is in milliseconds. its
	// default value is DEF_SOCK_KEY_TIMEOUT.
	u4					timeout;
	lisk_atom_a			ref;
	volatile n4			sock;			// local socket
	SAEX				local;			// local ip address
	SAEX				peer;			// peer ip address
	callback_t			callback;		// callback functions
} sock_key_t, *sock_key_p, sock_key_a[1];

/**
 * description	: bind a server to a socket key @key <br/><br/>
 *
 * in-out@key	: socket key. if you do not wanna bind any ip or port, you can set
 *				  the ip or port of @key->local_addr to be zero.
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead if all params
 *				  are illegal or -2 because some other errors.
 **/
n4 sock_key_bind(vp key);
/**
 * description	: connect @key to its peer address @peer_addr. if the connection
 *				  fails, the @key will be destroyed. <br/><br/>
 *
 * in-out@key	: socket key
 * in-@retry	: retry flag. if the @retry is non-zero and the sock drive fails to
 *				  connect its peer at the first time, it will retry later.
 *
 * out-@ret		: if the connection sucessds, the return value is 0; otherwise -1
 *				  instead if some params are illegal or -2 if the connetion fails
 *				  or -3 if the connection fails but you can retry later.
 **/
n4 sock_key_connect(vp key, n4 retry);
/**
 * description	: check whether the sock_key @key is available to send out
 *				  data.<br/><br/>
 *
 * in-@key		: socket key
 *
 * out-@ret		: if successful, the return value is 0. otherwise -1 instead if the
 *				  @key is nil or -2 if @key's sock is invalid or 1 if the @key has not
 *				  been ready to send out data.
 **/
n4 sock_key_check(vp key);
/**
 * description	: send data buffer to its peer <br/><br/>
 *
 * in-@key		: socket key
 * in-@pkg		: data buffer
 * in-@len		: buffer len
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead if all params
 *				  are illegal or -2 if the lisk fail to send @pkg or -3
 *				  if @key which is a tcp sock_key has not been ready to send @pkg.
 **/
n4 sock_key_send(vp key, vp pkg, n4 len);

/**
 * description	: define a socket drive <br/><br/>
 *
 * in-out@drive	: socket drive object
 *
 * out-@ret		: if successful, ret is 0 and *@drive is the socket drive;
 *				  otherwise -1 instead if all params are illegal
 *				  or -2 if this function fails to initialize a sock drive object.
 **/
n4 sock_drive_init(vpp drive);
/**
 * description	: destruct a socket drive <br/><br/>
 *
 * in-out@drive	: socket drive object
 *
 * out-@ret		: the return is vd.
 **/
vd sock_drive_uninit(vpp drive);
/**
 * description	: socket drive's loop function. <br/><br/>
 *
 * in-@drive	: socket drive object
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead if some params are
 *				  illegal or -2 if @drive get some errors or -3 if it exits.
 **/
n4 sock_drive_run(vp drive);
/**
 * description	: add a socket key to the socket drive @drive <br/><br/>
 *
 * in-out@drive	: socket drive object
 * in-@key		: socket key
 *
 * out-@ret		: if successful, the return value is 0. otherwise the return value
 *				  is -1 if @drive or @key is nil or -2 if @drive fails to push @key
 *				  into its event queue.
 **/
n4 sock_drive_add_key(vp drive, vp key);

#ifdef __cplusplus
}
#endif

#endif

