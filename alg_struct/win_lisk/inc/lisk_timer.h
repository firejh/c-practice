/**
 * FILE		: lisk_timer.h
 * DESC		: lisk timer (old lisk_clock = base/lisk_clock + ext/lisk_timer)
 * AUTHOR	: v1.0 written by Alex Stocks
 * DATE		: on June 7, 2012
 * LICENCE	: GPL 2.0
 * MOD		:
 **/

#ifndef __LISK_TIMER_H__
#define __LISK_TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <lisk_clock.h>

#define	MAX_TIMER_LEVEL			5

/**
 * description	: get current lisk time in microseconds <br/><br/>
 *
 * out-@ret		: the return value is current lisk time.
 **/
n8 lisk_time_get(vd);
/**
 * description	: update current lisk time. please attention that you should
 *				  invoke it in a while|for loop codes block. lisk timer depends
 *				  on this function. <br/><br/>
 *
 * out-@ret		: the return value is void.
 **/
vd lisk_time_update(vd);

/**
 * list
 * 0: micro-second list;
 * 1: second list
 * 2: minute list
 * 3: hour list
 * 4: day list
 **/
typedef struct lisk_timer_tag {
	n8			clock;					// current time in microseconds
	u4			hand[MAX_TIMER_LEVEL];	// clock
	u4			size;					// list size
	vp			list[MAX_TIMER_LEVEL];	// timer list
	u4			num;					// timer node number
	vp			ring;					// timer list node ring
} lisk_timer_t, *lisk_timer_p, lisk_timer_a[1];

/**
 * description	: declare a timer and initialize it, you can use it as follows,
 *				  vp timer = nil;
 *				  n4 ret = timer_init(&timer);
 *				  if (IS_NZR(ret)) {
 *				      printf("timer_init failed, ret = %d", ret);
 *				  }
 *				  <br/><br/>
 *
 * in-out@timer	: timer
 *
 * out-@ret		: if successful, ret is 0; if @timer is nil,
 *				  the return value is -1; or if fails to allocate
 *				  memory space for timer, the return value is -2.
 **/
n4 timer_init(vp timer);
/**
 * description	: free the memory space that timer @timer takes up <br/><br/>
 *
 * in-out@timer	: timer
 *
 * out-@ret		: the return value is void
 **/
vd timer_uninit(vp timer);

enum TASK_TYPE	{
	ETIMER_ONCE	= 0X1 << 0,
	ETIMER_LOOP	= 0X1 << 1,
};

/**
 * description	: timer process function. if the @timer is nil,
 *				  lisk will destroy the timer after it invoke
 *				  this function. if its return value is -3,
 *				  lisk will kill its timer.  <br/><br/>
 **/
typedef	n4 (*timer_func)(vp timer, uw id, vp arg);

typedef struct timer_key_tag	{
	u4			type;	// task type
	u4			expire;	// its time range is [1, 0XEFFFFFF]. its unit is ms.
	vp			arg;	// @func argument
	// time responding function, it should not be nil. if its
	// return value is -3 or its node type is ETIMER_ONCE,
	// its timer node will be deleted.
	timer_func	func;
} timer_key_t, *timer_key_p, timer_key_a[1];

/**
 * description	: add a timer <br/><br/>
 *
 * in-out@timer	: timer
 * in-@key		: timer key
 * out-@id		: timer id
 *
 * out-@ret		: if params are illegal, the return value is 0;
 *				  otherwise -1 instead.
 **/
n4 timer_add(vp timer, timer_key_p key, uw* id);
/**
 * description	: del a timer whose id is @id. if you do not
 *				  need a timer, you can delete it. please attention
 *				  that your timer function should not invoke
 *				  @timer_del. <br/><br/>
 *
 * in-out@timer	: timer
 * in-@id		: timer id
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 timer_del(vp timer, uw id);
/**
 * description	: timer loop function, you should put it
 *				  in a loop phrase <br/><br/>
 *
 * in-out@timer	: timer
 *
 * out-@ret		: if params are illegal, the return value is 0;
 *				  otherwise -1 instead.
 **/
n4 timer_loop(vp timer);
/**
 * description	: get memory size of @timer and its timer node @num<br/><br/>
 *
 * in-@timer	: timer
 * in-out@num	: timer node number
 * in-out@size	: memory size
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 timer_mem_size(vp timer, u8* num, u8* size);

#ifdef __cplusplus
}
#endif

#endif

