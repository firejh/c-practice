
#include <BlockingQueue.h>

#include <pthread.h>
#include <stdio.h>
#include <sys/epoll.h>

#include <unistd.h>
#include <sys/syscall.h>
#if !(defined(gettid))
    #define gettid()    syscall(__NR_gettid)
#endif

#define	nil		NULL
#define	u8		unsigned long long

BlockingQueue<u8>	queue;

void* input(void*);
void* output(void*);

int main(int argc, char** argv)	{
	int				idx;
	int				ret;
	int				input_num;
	int				output_num;
	void*			arg;
	void*			pret;
	pthread_attr_t*	attr;

	attr = nil;
	input_num = 7;
	pthread_t	input_trd[input_num];
	for (idx = 0; idx < input_num; idx++)	{
		arg = nil;
		ret = pthread_create(input_trd + idx, attr, input, arg);
		if (ret)	{
			break;
		}
	}
	input_num = idx;

	arg = NULL;
	output_num = 3;
	pthread_t	output_trd[output_num];
	for (idx = 0; idx < output_num; idx++)	{
		ret = pthread_create(output_trd + idx, attr, output, arg);
		if (ret)	{
			break;
		}
	}
	output_num = idx;

	for (idx = 0; idx < output_num; idx++)	{
		pret = nil;
		pthread_join(output_trd[idx], &pret);
	}

	for (idx = 0; idx < input_num; idx++)	{
		pret = nil;
		pthread_join(input_trd[idx], &pret);
	}

	return 0;
}

void* input(void* arg)	{
	u8		idx;
	u8		start;
	u8		top;
	void*	pret;

	start = 1;
	top = 1000000;
	for (idx = start; idx < top; idx++)	{
		queue.put(idx);
	}

	pret = NULL;
	pthread_exit(pret);

	return NULL;
}

void* output(void* arg)	{
	void*	pret;

	while (queue.size())	{
		printf("%10lu   %10llu\n", gettid(), queue.take());
	}

	pret = NULL;
	pthread_exit(pret);

	return NULL;
}

