/** 
 * FILE		: lisk_ring.h
 * DESC		: a FIFO(first coming, first output) queue,
 *			   also can be uesed as a queue buffer
 * AUTHOR	: v1.0 written by Alex Stocks
 * DATE		: on Oct 8, 2012
 * MOD		: modified by Alex Stocks on May 21, 2013
 **/

#ifndef __SDK_RING_H__
#define	__SDK_RING_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <lisk_public.h>

/**
 * description	: initilise a ring buffer. after uses it,
 *				  you should destroy it by ring_uninit.
 *				  the following codes shows how to declare a ring buffer.
 *				  vp ring = nil;
 *				  u4 num = 1024;
 *				  u4 len = sizeof(n4);
 *				  n4 ret = ring_init(&ring, num, len);
 *				  if (IS_NZR(ret)) {
 *						LOG_ERROR("ring_init(num = %u, len = %u) = %d",
 *									num, len, ret);
 *				  }
 *				  <br/><br/>
 *
 * in-out@ring	: ring object
 * in-@unit_num	: its max element num. its value should be a power of 2
 *				  and greater than 2 itself.
 * in-@unit_len	: unit length of every element
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 ring_init(vpp ring, u4 unit_num, u4 unit_len);
/**
 * description	: destroy ring <br/><br/>
 *
 * in-out@ring	: ring object
 *
 * out-@ret		: the ret is void
 **/
vd ring_uninit(vpp ring);
/**
 * description	: get @ring's element number <br/><br/>
 *
 * in-out@ring	: ring object
 * in-out@size	: element number of the @ring
 *
 * out-@ret		: if @ring has been initialised, the ret @size is its
 *				  unit_num and the return value is 0; otherwise -1 instead.
 **/
n4 ring_size(vp ring, u4* size);
/**
 * description	: push data into the @ring <br/><br/>
 *
 * in-@ring		: ring object
 * in-@elem		: element pointer
 * in-@elem_num	: the number of the @elem
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 ring_push(vp ring, vp elem, u4 elem_num);
/**
 * description	: pop up data from the ring <br/><br/>
 *
 * in-@ring		: ring object
 * in-out@elem	: output element pointer
 * in-out@num	: wanna element number
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 ring_pop(vp ring, vp elem, u4* num);

/**
 * description	: initilise a lisk queue buffer based on list.
 *				  after uses it, you should destroy it by
 *				  queue_uninit. the difference between the
 *				  queue and the queue is that queue do not
 *				  have limit on maximum element number because
 *				  the queue is based on list. <br/><br/>
 *
 * in-out@queue	: lisk queue object
 * in-@size		: unit size of every element
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 queue_init(vpp queue, u4 size);
/**
 * description	: destroy lisk queue <br/><br/>
 *
 * in-out@queue	: lisk queue object
 *
 * out-@ret		: the ret is void
 **/
vd queue_uninit(vpp queue);
/**
 * description	: push data into the @queue <br/><br/>
 *
 * in-@queue	: lisk queue object
 * in-@elem		: element pointer
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 queue_push(vp queue, vp data);
/**
 * description	: pop up data from the queue <br/><br/>
 *
 * in-@queue	: lisk queue object
 * in-out@elem	: output element pointer
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 queue_pop(vp queue, vp data);
/**
 * description	: get @queue's element number <br/><br/>
 *
 * in-out@queue	: lisk queue object
 * in-out@size	: element number
 *
 * out-@ret		: the @size is its unit_num and the return value is 0
 *				  if successful; otherwise -1 instead if some parameters
 *				  are illegal.
 **/
n4 queue_size(vp queue, u4* size);

#ifdef __cplusplus
}
#endif

#endif

