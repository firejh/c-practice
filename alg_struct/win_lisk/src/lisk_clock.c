/**
 * FILE		:lisk_clock.c
 * DESC		:to to calc time cost of a bin file
 * AUTHOR	:v1.0 written by Alex Stocks
 * DATE		:on June 7, 2012
 * MOD		:modified by * on June 2, 2012
 * **/

#ifdef __cplusplus
extern "C" {
#endif

#include <Windows.h>
#include <stdio.h>
#include <lisk_public.h>
#include <lisk_clock.h>
#include <lisk_type.h>
#include <lisk_mem_pool.h>
//#include "lisk_list.h"	//timer node list

#ifdef __cplusplus
	}
#endif

u8 time_usec(vd);
u8 time_diff_usec(LARGE_INTEGER a, LARGE_INTEGER b, LARGE_INTEGER freq);

#define TIME_DIFF_SEC(a, b, freq)	(TIME_DIFF_USEC(a, b, freq) / 1000000)

#define TIME_DIFF_MSEC(a, b, freq)	(TIME_DIFF_USEC(a, b, freq) / 1000)

#define TIME_DIFF_USEC(a, b, freq)	time_diff_usec(a, b, freq)

vd msleep(n4 ms)
{
	Sleep(ms);
}

#ifdef WIN32

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

struct timezone
{
	n4  tz_minuteswest;
	n4  tz_dsttime;
};

n4 gettimeofday(struct timeval *tv, struct timezone *tz)
{
	FILETIME ft;
	u8 tmpres = 0;
	static n8 tzflag = 0;

	if (NULL != tv)	{
		GetSystemTimeAsFileTime(&ft);

		tmpres |= ft.dwHighDateTime;
		tmpres <<= 32;
		tmpres |= ft.dwLowDateTime;

		tmpres /= 10;

		tmpres -= DELTA_EPOCH_IN_MICROSECS;

		tv->tv_sec = (uw)(tmpres / 1000000UL);
		tv->tv_usec = (uw)(tmpres % 1000000UL);
	}

	if (NULL != tz)	{
		if (!tzflag)	{
			_tzset();
			tzflag++;
		}

		tz->tz_minuteswest = _timezone / 60;
		tz->tz_dsttime = _daylight;
	}

	return 0;
}

#endif

u8 time_diff_usec(LARGE_INTEGER a, LARGE_INTEGER b, LARGE_INTEGER freq)
{
	u8 a_usec = ((u8)(a.QuadPart * 1000000) / freq.QuadPart);
	u8 b_usec = ((u8)(b.QuadPart * 1000000) / freq.QuadPart);
	if (IS_LT(b_usec, a_usec)) {
		u8 tmp = a_usec;
		a_usec = b_usec;
		b_usec = tmp;
	}

	return (u8)(b_usec - a_usec);
}

n8 get_cur_time_sec()
{
	struct timeval tv;
	gettimeofday(&tv, (struct timezone*)(nil));

	return (n8)(tv.tv_sec);
}

n8 get_cur_time_msec()
{
	struct timeval tv;
	gettimeofday(&tv, (struct timezone*)(nil));

	return (n8)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

n8 get_cur_time_usec()
{
	struct timeval tv;
	gettimeofday(&tv, (struct timezone*)(nil));

	return (n8)(tv.tv_sec * 1000000 + tv.tv_usec);
}

typedef struct _program_time_t {
	LARGE_INTEGER	prog_start;
	LARGE_INTEGER	prog_cur;
	LARGE_INTEGER	clock_ticks;
} program_time_t;

static program_time_t pgm_time;

vd clock_start()
{
	QueryPerformanceCounter(&pgm_time.prog_start);
	CPY(&(pgm_time.prog_cur), &(pgm_time.prog_start), sizeof(LARGE_INTEGER));

	QueryPerformanceFrequency(&pgm_time.clock_ticks);
	if (IS_ZR(pgm_time.clock_ticks.QuadPart)) {
		pgm_time.clock_ticks.QuadPart = 1;
	}
}

vd clock_record()
{
	QueryPerformanceCounter(&pgm_time.prog_cur);
}

vd clock_reset()
{
	QueryPerformanceCounter(&pgm_time.prog_start);
	CPY(&(pgm_time.prog_cur), &(pgm_time.prog_start), sizeof(LARGE_INTEGER));
}

u4 time_diff()
{
	return TIME_DIFF_USEC(pgm_time.prog_cur, pgm_time.prog_start, pgm_time.clock_ticks);
}

vd get_cur_time_string(n1* buf, n4 buf_len)
{
	struct timeval tv;
	if (IS_NL(buf) || IS_ZR(buf_len)) {
		return;
	}

	gettimeofday(&tv, (struct timezone*)(nil));
	get_time_string(tv, buf, buf_len);
}

vd get_time_string(struct timeval tv, n1* buf, n4 buf_len)
{
	struct tm* tm_ptr =	nil;
	if (IS_NL(buf) || IS_ZR(buf_len)) {
		return;
	}

	tm_ptr =	localtime((time_t*)&(tv.tv_sec));
	snprintf(buf, buf_len, "%04d-%02d-%02d_%02d-%02d-%02d_%06d",
			tm_ptr->tm_year+1900, tm_ptr->tm_mon + 1, tm_ptr->tm_mday,
			tm_ptr->tm_hour, tm_ptr->tm_min, tm_ptr->tm_sec, (n4)(tv.tv_usec));
}

//////////////////////////////////////////
//timer
//////////////////////////////////////////
/*
#define	IS_TRIG_LOOP(expire)	(expire & 0X80000000)
#define	SET_TRIG_LOOP(expire)	(expire |= 0X80000000)
#define	EXPIRE_TIME(expire)		(expire & ~0X80000000)

#define	MAX_MS					(1000)
#define	MAX_SECOND				(60)
#define	MAX_MINUTE				(60)
#define	MAX_HOUR				(24)
#define	MAX_DAY					(31)
#define	MS						(1)
#define	SECOND_MS				(1 * MAX_MS * (MS))
#define	MINUTE_MS				(1 * MAX_SECOND * (SECOND_MS))
#define	HOUR_MS					(1 * MAX_MINUTE * (MINUTE_MS))
#define	DAY_MS					(1 * MAX_HOUR * (HOUR_MS))
#define	MS_NUM(expire)			(expire / MS)
#define	SECOND_NUM(expire)		(expire / SECOND_MS)
#define	MINUTE_NUM(expire)		(expire / MINUTE_MS)
#define	HOUR_NUM(expire)		(expire / HOUR_MS)
#define	DAY_NUM(expire)			(expire / DAY_MS)

#pragma pack(1)

typedef struct timer_node_tag {
	u8			trig_time;
	n4			expire;
	time_func	on_timer;
} timer_node_t, timer_node_a[1];

typedef enum {
	MAX_TIMER_LEVEL = 5
} TIMER_LEVEL;

typedef struct {
	n8		clock;		//its unit is ms
	//0: micro-second list;
	//1: second list
	//2: minute list
	//3: hour list
	//4: day list
	u4		size;					//list size
	u4		hand[MAX_TIMER_LEVEL];
	u4		hand_max[MAX_TIMER_LEVEL];
	vp		list[MAX_TIMER_LEVEL];	//timer list
} lisk_timer_t;

#pragma pack()

n4 cmp_timer_node(vp first, vp second)
{
	timer_node_t* node_first = TYPE(timer_node_t*, first);
	timer_node_t* node_second = TYPE(timer_node_t*, second);
	n4 ret = 0;
	if (IS_LT(node_first->trig_time, node_second->trig_time)) {
		ret = -1;
	} else if (IS_GT(node_first->trig_time, node_second->trig_time)) {
		ret = 1;
	}

	RET_INT(ret, nil_str);
}

n4 timer_add_node(lisk_timer_t* timer, timer_node_t* node)
{
	if (IS_NL(timer) || IS_NL(node)) {
		RET_INT(-1, "@timer = %p, node = %p", timer, node);
	}

	n8 clock_diff = node->trig_time - timer->clock;
	if (IS_LE(node->trig_time, timer->clock)) {
		list_insert(timer->list[0], node, SIZE(timer_node_t), cmp_timer_node);
	} else if (IS_NZR(DAY_NUM(clock_diff))) {
		list_insert(timer->list[4], node, SIZE(timer_node_t), cmp_timer_node);
	} else if(IS_NZR(HOUR_NUM(clock_diff))) {
		list_insert(timer->list[3], node, SIZE(timer_node_t), cmp_timer_node);
	} else if (IS_NZR(MINUTE_NUM(clock_diff))) {
		list_insert(timer->list[2], node, SIZE(timer_node_t), cmp_timer_node);
	} else if (IS_NZR(SECOND_NUM(clock_diff))) {
		list_insert(timer->list[1], node, SIZE(timer_node_t), cmp_timer_node);
	} else {
		list_insert(timer->list[0], node, SIZE(timer_node_t), cmp_timer_node);
	}

	RET_INT(0, nil_str);
}

vd timer_del_node(vpp list, vpp node)
{
	list_erase(list, node);
}

n4 timer_cascade(lisk_timer_t* timer, u4 level)
{
	if (IS_NL(timer) || IS_NBT(level, 0, MAX_TIMER_LEVEL)) {
		RET_INT(-1, "@timer = %p, level = %u", timer, level);
	}

	n4 flag = 0;
	timer_node_a tmp_node;
	LIST_FOR_EACH_BEGIN(timer_node_t, timer->list[level], cur)
		switch (level) {
			case 1:
				flag = IS_GE(SECOND_NUM(cur->trig_time - timer->clock), 1);
				break;

			case 2:
				flag = IS_GE(MINUTE_NUM(cur->trig_time - timer->clock), 1);
				break;

			case 3:
				flag = IS_GE(HOUR_NUM(cur->trig_time - timer->clock), 1);
				break;

			case 4:
				flag = IS_GE(HOUR_NUM(cur->trig_time - timer->clock), 1);
				break;
		}
		if (IS_NZR(flag)) {
			break;
		}
		CPY(tmp_node, cur, SIZE(tmp_node));
		timer_del_node(TYPE(vpp, &(timer->list[level])), TYPE(vpp, &(cur)));
		timer_add_node(timer, tmp_node);
		cur = timer->list[level];
	LIST_FOR_EACH_END

	RET_INT(0, nil_str);
}

n4 timer_update_time(lisk_timer_t* timer)
{
	if (IS_NL(timer)) {
		RET_INT(-1, "@timer = %p", timer);
	}

	//update clock
	n8 cur_clock = get_cur_time_msec();
	u4 clock_diff = TYPE_U4(cur_clock - timer->clock);
	timer->clock = cur_clock;

	//update timer->hand[0]
	//hypothesis: a process may run once every second
	cur_clock = (timer->hand[0] + clock_diff) % timer->hand_max[0];
	if (IS_GE(cur_clock, timer->hand[0])) {
		timer->hand[0] = cur_clock;
		RET_INT(0, nil_str);
	}
	timer->hand[0] = cur_clock;

	//update timer->hand
	u4 idx = 0;
	u4 hand = 0;
	for (idx = 1; idx < MAX_TIMER_LEVEL; idx++) {
		hand = timer->hand[idx];
		timer->hand[idx] = (hand++) % timer->hand_max[idx];
		timer_cascade(timer, idx);
		if (IS_NZR(timer->hand[idx])) {
			break;
		}
	}

	RET_INT(0, nil_str);
}

n4 timer_init(vpp tmr)
{
	if (IS_NL(tmr)) {
		RET_INT(-1, "@timer = %p", tmr);
	}
	*tmr = nil;

	lisk_timer_t* timer = nil;
	POOL_ALLOC(timer, SIZE(lisk_timer_t));
	if (IS_NL(timer)) {
		RET_INT(-2, "POOL_ALLOC(timer = %p, size = %lu)", timer, (uw)SIZE(lisk_timer_t));
	}
	SET_ZERO(timer, SIZE(lisk_timer_t));
	u4 idx = 0;
	for (idx = 0; idx < MAX_TIMER_LEVEL; idx++) {
		LIST_DEF(tmp);
		if (IS_NL(tmp)) {
			u4 idx_i = 0;
			for (idx_i = 0; idx_i < idx; idx_i++) {
				timer->list[idx_i] = list_head(4);
			}
			RET_INT(-2, "idx %u: LIST_DEF(tmp) = nil", idx);
		}
		timer->list[idx] = tmp;
	}
	timer->size = MAX_TIMER_LEVEL;
	timer->hand_max[0] = TYPE_U4(MAX_MS);
	timer->hand_max[1] = TYPE_U4(MAX_SECOND);
	timer->hand_max[2] = TYPE_U4(MAX_MINUTE);
	timer->hand_max[3] = TYPE_U4(MAX_HOUR);
	timer->hand_max[4] = TYPE_U4(MAX_DAY);
	timer->clock = get_cur_time_msec();
	*tmr = TYPE(vp, timer);

	RET_INT(0, nil_str);
}

vd timer_uninit(vpp tmr)
{
	if (IS_NL(tmr)) {
		RET_VD(nil_str);
	}
	lisk_timer_t* timer = TYPE(lisk_timer_t*, *tmr);
	if (IS_NL(timer)) {
		RET_VD(nil_str);
	}

	u4 idx = 0;
	for (idx = 0; idx < timer->size; idx++) {
		LIST_UNDEF(timer->list[idx]);
	}
	SET_ZERO(timer, SIZE(*timer));
	POOL_DEALLOC(timer);
	*tmr = nil;
}

n4 timer_add(vp tmr, u4 expire, time_func func, u4 loop)
{
	lisk_timer_t* timer = TYPE(lisk_timer_t*, tmr);
	if (IS_NL(timer) || IS_NL(func) || IS_NBE(expire, MIN_EXPIRE, MAX_EXPIRE)) {
		RET_INT(-1, "@timer = %p, @func = %p, @expire = %u", timer, func, expire);
	}

	if (IS_ZR(timer->size)) {
		RET_INT(-1, "timer->size = %u", timer->size);
	}

	timer_node_a node;
	node[0].expire = expire;
	node[0].trig_time = timer->clock + node[0].expire;
	node[0].on_timer = func;
	if (IS_NZR(loop)) {
		SET_TRIG_LOOP(node[0].expire);
	}

	RET_INT(0, nil_str);
}

n4 timer_loop(vp tmr)
{
	lisk_timer_t* timer = TYPE(lisk_timer_t*, tmr);
	if (IS_NL(timer)) {
		RET_INT(-1, "timer = %p", timer);
	}

	if (IS_ZR(timer->clock) || IS_ZR(timer->size)) {
		RET_INT(-1, "timer->clock = %lld, timer->size = %u", timer->clock, timer->size);
	}

	timer_update_time(timer);

	vp list = timer->list[0];
	timer_node_t* prev = nil;
	LIST_FOR_EACH_BEGIN(timer_node_t, list, node)
		if (IS_LE(node->trig_time, timer->clock)) {
			node->on_timer();
			if (IS_TRIG_LOOP(node->expire)) {
				timer_add(tmr, EXPIRE_TIME(node->expire), node->on_timer, TYPE_U4(1));
			}
			prev = TYPE(timer_node_t*, list_prev_elem(node));
			timer_del_node(TYPE(vpp, &list), TYPE(vpp, &node));
			node = prev;
		}
	LIST_FOR_EACH_END

	RET_INT(0, nil_str);
}
*/
