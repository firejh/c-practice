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

#include <lisk_public.h>

typedef	vd	(*event_callback)(n4 fd, n4 event, vp arg);

typedef enum ESOCK_EVENT_TAG    {
	ESOCK_INIT		= 0x1 << 0,
	ESOCK_READ		= 0x1 << 1,
	ESOCK_WRITE		= 0x1 << 2,
	ESOCK_TIMEOUT	= 0x1 << 3,
	ESOCK_ADD		= 0x1 << 4,
} ESOCK_EVENT;

vd event_init(vp event);
n4 event_dispatch(vp event);

vd event_set(vp event, n4, short, event_callback callback, vp arg);
vd event_add(vp event, struct timeval *);
vd event_del(vp event);

n4 timeout_list(struct timeval *);
vd timeout_process(vd);

#define timeout_add(ev, tv)		event_add(ev, tv)
#define timeout_set(ev, cb, arg)	event_set(ev, -1, 0, cb, arg)
#define timeout_del(ev)			event_del(ev)
#define timeout_pending(ev, tv)		event_pending(ev, ESOCK_TIMEOUT, tv)
#define timeout_initalized(ev)		((ev)->flag & ESOCK_INIT)

n4 event_pending(event_p, short, struct timeval *);

#define event_initalized(ev)		((ev)->flag & ESOCK_INIT)

#ifdef __cplusplus
}
#endif

#endif /* _EVENT_H_ */

