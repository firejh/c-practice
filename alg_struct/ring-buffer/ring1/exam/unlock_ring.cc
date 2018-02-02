/**
 * FILE		: tcmalloc.c
 * DESC		: tcmalloc test
 * AUTHOR	: v1.0 written by Alex Stocks
 * DATE		: on Dec 21, 2013
 * LICENCE	: GPL 2.0
 * MOD		:
 **/

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <lisk_public.h>
#include <lisk_ring.h>

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

typedef struct pc_queue_tag	{
	sem_t	sem[1];
	vp		queue;
} pc_queue_t, *pc_queue_p, pc_queue_a[1];

n4	pc_queue_init(vpp queue, u4 size);
vd	pc_queue_uninit(vpp queue);
n4	pc_queue_push(vp queue, vp data);
n4	pc_queue_pop(vp queue, vp data);
n4	pc_queue_size(vp queue, u4* size);

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
	vp				queue;
	struct timeval	start;
	struct timeval	end;
	pthread_t		producer_trd[20];
	pthread_t		consumer_trd[80];

	gettimeofday(&start, nil);

	size = sizeof(vp);
	ret = pc_queue_init(&queue, size);
	if (IS_NZR(ret))	{
		fprintf(stderr, "queue_init(&queue = %p, size = %u) = %d\n", &queue, size, ret);
		return -1;
	}

	do {
		arg = queue;
		ret = generate_thread(consumer_trd, ARRAY_SIZE(consumer_trd), (vp)(consumer), arg);
		if (IS_NZR(ret))	{
			printf("generate_thread(consumer, num = %lu) = %d\n",
					ARRAY_SIZE(consumer_trd), ret);
			break;
		}

		arg = queue;
		ret = generate_thread(producer_trd, ARRAY_SIZE(producer_trd), (vp)(producer), arg);
		if (IS_NZR(ret))	{
			printf("generate_thread(producer, num = %lu) = %d\n",
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
	} while(0);

	pc_queue_uninit(&queue);

	gettimeofday(&end, nil);
	printf("time diff:%llu\n", TIME_DIFF_USEC(start, end) / 1000);

	return 0;
}

n4	pc_queue_init(vpp queue_pptr, u4 size)
{
	n4			ret;
	pc_queue_p	queue;

	if (IS_NL(queue_pptr))	{
		RET_INT(-1, "@queue_pptr = %p", queue_pptr);
	}

	if (IS_NL(ALLOC(queue, sizeof(pc_queue_t))))	{
		RET_INT(-2, "@ALLOC(queue) = nil");
	}

	ret = sem_init(queue->sem, 0, 0);
	if (IS_NZR(ret))	{
		RET_INT(-2, "sem_init(&sem = %p) = %d", queue->sem, ret);
	}

	ret = queue_init(&(queue->queue), size);
	if (IS_NZR(ret))	{
		sem_destroy(queue->sem);
		RET_INT(-2, "queue_init(&queue = %p, size = %u) = %d",
					&(queue->queue), size, ret);
	}

	*queue_pptr = (vp)(queue);

	RET_INT(0, nil_str);
}

vd	pc_queue_uninit(vpp queue_pptr)
{
	pc_queue_p	queue;

	if (IS_NL(queue_pptr) ||
		IS_NL(queue = (pc_queue_p)(*queue_pptr)))	{
		RET_VD(nil_str);
	}

	sem_destroy(queue->sem);
	queue_uninit(&(queue->queue));
	DEALLOC(queue);
	*queue_pptr = nil;
}

n4	pc_queue_push(vp queue_ptr, vp data)
{
	n4			ret;
	pc_queue_p	queue;

	if (IS_NL(queue = (pc_queue_p)(queue_ptr)))	{
		RET_INT(-1, "@queue = %p", queue);
	}

	ret = queue_push(queue->queue, data);
	if (IS_NZR(ret))	{
		RET_INT(-2, "queue_push(queue = %p, data = %p) = %d",
					queue->queue, data, ret);
	}

	do {
		ret = sem_post(queue->sem);
		if (IS_ZR(ret))	{
			break;
		}
	} while(1);

	RET_INT(0, nil_str);
}

n4	pc_queue_pop(vp queue_ptr, vp data)
{
	n4			ret;
	pc_queue_p	queue;

	if (IS_NL(queue = (pc_queue_p)(queue_ptr)))	{
		RET_INT(-1, "@queue = %p", queue);
	}

	do {
		ret = sem_wait(queue->sem);
		if (IS_ZR(ret))	{
			break;
		}
	} while(1);

	ret = queue_pop(queue->queue, data);
	if (IS_NZR(ret))	{
		RET_INT(-2, "queue_pop(queue = %p, data = %p) = %d",
					queue->queue, data, ret);
	}

	RET_INT(0, nil_str);
}

n4	pc_queue_size(vp queue_ptr, u4* size)
{
	n4			ret;
	pc_queue_p	queue;

	if (IS_NL(queue = (pc_queue_p)(queue_ptr)))	{
		RET_INT(-1, "@queue = %p", queue);
	}

	ret = queue_size(queue->queue, size);
	if (IS_NZR(ret))	{
		RET_INT(-2, "queue_size(queue = %p, &size = %p) = %d",
					queue->queue, size, ret);
	}

	RET_INT(0, nil_str);
}

vp producer(vp arg)
{
	n4	idx;
	n4	ret;
	n4	top;
	u4	size;
	vp	pret;
	vp	ptr;
	vp	queue;

	queue = arg;

	size = MEM_SIZE;
	top = SIZE_K(4);
	for (idx = 0; IS_LT(idx, top); idx++)	{
		if (IS_NL(ALLOC(ptr, size))) {
			printf("Error: ALLOC(idx = %d) failed.\n", idx);
			idx--;
			continue;
		}

		ret = pc_queue_push(queue, (vp)(&ptr));
		if (IS_NZR(ret))	{
			printf("Error: queue.push(idx = %d) failed.\n", idx);
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
	vp	queue;

	queue = arg;
	//printf("consumer queue = %p\n", queue);

	top = SIZE_K(1);
	for (idx = 0; IS_LT(idx, top); idx++) {
		ptr = nil;
		ret = pc_queue_pop(queue, (vp)(&ptr));
		if (IS_NZR(ret) || IS_NL(ptr)) {
			printf("queue.pop(idx = %d) = %d\n", idx, ret);
			idx--;
			continue;
		}

		//printf("consumer ptr = %p\n", ptr);
		free(ptr);
		ptr = nil;
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

