/**
 * FILE		: tcmalloc.c
 * DESC		: tcmalloc test
 * AUTHOR	: v1.0 written by Alex Stocks
 * DATE		: on Dec 21, 2013
 * LICENCE	: GPL 2.0
 * MOD		:
 **/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <lisk_public.h>
#include <muduo_ring.h>

#define TIME_DIFF_USEC(a,b) ({						\
	u8 a_usec =										\
			((u8)a.tv_sec *							\
			1000000 + a.tv_usec);					\
	u8 b_usec =										\
			((u8)b.tv_sec * 1000000 + b.tv_usec);	\
	if (b_usec < a_usec) {							\
	u8 tmp = a_usec;								\
	a_usec = b_usec;								\
	b_usec = tmp;									\
	}												\
	(u8)(b_usec - a_usec);							\
})

#define	MEM_SIZE	SIZE_K(1)
typedef	vp (*thread_function)(vp);

vp	producer(vp);
vp	consumer(vp);
n4	generate_thread(pthread_t* trd, n4 num, vp func, vp arg);

n4 main(n4 argc, n1** argv)
{
	n4				idx;
	n4				ret;
	n4				top;
	u4				size;
	vp				arg;
	vp				pret;
	vp				ptr;
	struct timeval	start;
	struct timeval	end;
	pthread_t		producer_trd[20];
	pthread_t		consumer_trd[80];

	gettimeofday(&start, nil);

	do {
		block_queue2<vp> queue;

		arg = &queue;
		ret = generate_thread(consumer_trd, ARRAY_SIZE(consumer_trd), (vp)(consumer), arg);
		if (IS_NZR(ret))	{
			perr("generate_thread(consumer, num = %lu) = %d",
					ARRAY_SIZE(consumer_trd), ret);
			break;
		}

		arg = &queue;
		ret = generate_thread(producer_trd, ARRAY_SIZE(producer_trd), (vp)(producer), arg);
		if (IS_NZR(ret))	{
			perr("generate_thread(producer, num = %lu) = %d",
					ARRAY_SIZE(producer_trd), ret);
			break;
		}

		top = ARRAY_SIZE(producer_trd);
		for (idx = 0; IS_LT(idx, top); idx++) {
			pret = nil;
			pthread_join(producer_trd[idx], &pret);
		}

		top = ARRAY_SIZE(consumer_trd);
		for (idx = 0; IS_LT(idx, top); idx++) {
			pret = nil;
			pthread_join(consumer_trd[idx], &pret);
		}

		do {
			ret = queue.size(&size);
			//printf("queue size = %u, ret = %d\n", size, ret);
			if (IS_NZR(ret) || IS_ZR(size))	{
				break;
			}

			ptr = nil;
			ret = queue.pop(&ptr);
			if (IS_NZR(ret))	{
				break;
			}

			free(ptr);
		} while(1);
	} while(0);

	gettimeofday(&end, nil);
	pinfo("time diff:%llu", TIME_DIFF_USEC(start, end) / 1000);

	return 0;
}

vp producer(vp arg)
{
	n4	idx;
	n4	ret;
	n4	top;
	u4	size;
	vp	pret;
	vp	ptr;

	block_queue2<vp> &queue = *static_cast<block_queue2<vp>*>(arg);

	size = MEM_SIZE;
	top = SIZE_K(4);
	for (idx = 0; IS_LT(idx, top); idx++)	{
		if (IS_NL(ALLOC(ptr, size))) {
			perr("Error: ALLOC(idx = %d) failed.", idx);
			idx--;
			continue;
		}
		//printf("malloc ptr = %p\n", ptr);

		ret = queue.push(&ptr);
		if (IS_NZR(ret))	{
			perr("Error: queue.push(idx = %d) failed.", idx);
			free(ptr);
			idx--;
			continue;
		}
	}

	pret = nil;
	pthread_exit(pret);

	return nil;
}

vp consumer(vp arg)
{
	n4	idx;
	n4	ret;
	n4	top;
	vp	pret;
	vp	ptr;

	block_queue2<vp> &queue = *static_cast<block_queue2<vp>*>(arg);

	top = SIZE_K(1);
	for (idx = 0; IS_LT(idx, top); idx++) {
		ptr = nil;
		ret = queue.pop(&ptr);
		if (IS_NZR(ret) || IS_NL(ptr)) {
			perr("queue.pop(idx = %d) = %d\n", idx, ret);
			idx--;
			continue;
		}
		//printf("consumer ptr = %p\n", ptr);

		free(ptr);
	}

	pret = nil;
	pthread_exit(pret);

	return nil;
}

n4	generate_thread(pthread_t* trd, n4 num, vp func, vp arg)
{
	n4				idx;
	n4				ret;
	// vp			pret;
	pthread_attr_t*	attr;

	if (IS_NL(trd) || IS_NPN(num) || IS_NL(func))	{
		printf("Arg error: trd = %p, num = %d, func = %p\n", trd, num, func);
		return -1;
	}

	attr = nil;
	for (idx = 0; IS_LT(idx, num); idx++) {
		ret = pthread_create(trd + idx, attr, (thread_function)(func), arg);
		if (IS_NZR(ret))	{
			printf("Error: pthread_create(idx = %d) = %d\n", idx, ret);
			break;
		}
	}

	if (IS_LT(idx, num))	{
		num = idx;
		for (idx = 0; IS_LT(idx, num); idx++)   {
			// pret = nil;
			// pthread_join(trd[idx], &pret);
		}

		return -2;
	}

	return 0;
}

