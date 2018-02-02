/** 
 * FILE		:lisk_clock.h
 * DESC		:to calc time cost of a bin file
 * AUTHOR	:v1.0 written by Alex Stocks
 * DATE		:on June 7, 2012
 * MOD		:modified by * on June 2, 2012
 * **/


#ifndef __LISK_CLOCK_H_
#define __LISK_CLOCK_H_

#if  (defined(__cplusplus))
	extern "C" {
#endif

#include <lisk_type.h>
#include <time.h>


#define	timeradd(t1, t2, t3) do {								\
	(t3)->tv_sec = (t1)->tv_sec + (t2)->tv_sec;					\
	(t3)->tv_usec = (t1)->tv_usec + (t2)->tv_usec % (1000000);	\
	if (IS_GT((t1)->tv_usec + (t2)->tv_usec, (1000000))) {		\
		(t3)->tv_sec++;											\
	}															\
} while (0)

#define	timersub(t1, t2, t3) do {								\
	if (IS_GT((t1)->tv_sec, (t2)->tv_sec)) {					\
		(t3)->tv_sec = (t1)->tv_sec - (t2)->tv_sec;				\
		(t3)->tv_usec = (t1)->tv_usec - (t2)->tv_usec;			\
		if (IS_GT((t1)->tv_usec, (t2)->tv_usec)) {				\
			(t3)->tv_sec--;										\
			(t3)->tv_usec += (1000000);							\
		}														\
	} else {													\
		(t3)->tv_sec = (t2)->tv_sec - (t1)->tv_sec;				\
		(t3)->tv_usec = (t2)->tv_usec - (t1)->tv_usec;			\
		if (IS_GT((t2)->tv_usec, (t1)->tv_usec)) {				\
			(t3)->tv_sec--;										\
			(t3)->tv_usec += (1000000);							\
		}														\
	}															\
} while (0)


/**
 * description	: sleep in unit millisecond    <br/><br/>
 *
 * in-@ms		: ms millisecond
 *
 * out-@ret		: ret is vd
 **/
vd msleep(int ms);
#define sched_yield()	msleep(0)
/**
 * description	: windows version of linux function gettimeofday
 *
 * in-out@tv	: current time
 * in-@tz		: time zone, no usage. so it should be nil.
 *
 * out-@ret		: the return value is 0.
 **/
#ifdef WIN32
#include <windows.h>
n4 gettimeofday(struct timeval* tv, struct timezone* tz);
#endif
/**
 * description	: get currcent time in second <br/><br/>
 *
 * out-@ret		: ret is currcent time in second
 **/
n8 get_cur_time_sec(vd);
/**
 * description	: get currcent time in millisecond <br/><br/>
 *
 * out-@ret		: ret is currcent time in millisecond
 **/
n8 get_cur_time_msec(vd);
/**
 * description	: get currcent time in microsecond <br/><br/>
 *
 * out-@ret		: ret is currcent time in microsecond
 **/
n8 get_cur_time_usec(vd);

/**
 * description: get currcent time string
 * @in-out buf: output time string buffer
 * @in buf_len: input buf len
 * @out       : reture vd
 **/
vd get_cur_time_string(n1* buf, n4 buf_len);

/**
 * description: get time string by time string
 * @in-out buf: output time string buffer
 * @in buf_len: input buf len
 * @out       : reture vd
 **/
vd get_time_string(struct timeval tv, n1* buf, n4 buf_len);

/**
 * description	: get morning second of the day @sec<br/><br/>
 *
 * in-@sec		: time second, to get the day of @sec
 *
 * out-@ret		: morning second
 **/
n8 get_sec_of_morning(n8 sec);

/**
 * description	: get time difference between time second @sec and morning second of that day @sec<br/><br/>
 *
 * in-@sec		: time second, to get its morning second of @sec
 *
 * out-@ret		: time difference
 **/
n8 get_sec_of_day(n8 sec);

//////////////////////////////////////////
//note: the following functions such as clock_start&clock_record&time_diff_sys&
//time_diff_usr&time_diff_real&time_diff_progam calls system function times, which costs time
//////////////////////////////////////////
/**
 * description	: start the timer <br/><br/>
 **/
vd clock_start(vd);
/**
 * description	: record the time at current moment <br/><br/>
 **/
vd clock_record(vd);
/**
 * description	: reset the timer <br/><br/>
 **/
vd clock_reset(vd);
/**
 * description	: after calls clock_record, you can get the program running time <br/><br/>
 *
 * out-@ret		: program running time in microsecond
 **/
u4 time_diff(vd);

#if  (defined(__cplusplus))
	}
#endif

#endif

