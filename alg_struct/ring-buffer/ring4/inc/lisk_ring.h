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
 * description	: initilise a ring buffer. after invokes it, you should destroy it
 *				  by ring_uninit. in default, @ring is a multi-producer-multi-consumer
 *				  ring. the following codes show how to declare a ring buffer.
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

typedef enum RING_TYPE_TAG {
	RING_SP_SC = 0X01 << 0,	// single producer & single consumer
	RING_SP_MC = 0X01 << 1,	// single producer & multiple consumers
	RING_MP_SC = 0X01 << 2,	// multiple producers & single consumer
	RING_MP_MC = 0X01 << 3,	// multiple producers & multiple consumers
} RING_TYPE;
/**
 * description	: set ring type. please attention that you should
 *				  invoke this function before @ring_push or @ring_pop.<br/><br/>
 *
 * in-out@ring	: ring object
 * in-@type		: ring type
 *
 * out-@ret		: if successful, the return value is 0. if  @ring
 *				  is nil or @type is not between RING_SP_SC and RING_MP_MC,
 *				  the return value is -1.
 **/
n4 ring_set_type(vp ring, u4 type);
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
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 ring_push(vp ring, vp elem);
/**
 * description	: pop up data from the ring <br/><br/>
 *
 * in-@ring		: ring object
 * in-out@elem	: output element pointer
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 ring_pop(vp ring, vp elem);

#ifdef __cplusplus
}
#endif

#endif

