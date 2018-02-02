/**
 * FILE		: atom_test.c
 * DESC		: atom test
 * AUTHOR	: v1.0 written by Alex Stocks
 * DATE		: on Jan 7, 2014
 * LICENCE	: GPL 2.0
 * MOD		:
 **/

#include <lisk_atom.h>

#include <pthread.h>
#include <stdio.h>

void*	test(void*);
volatile int	num = 0;
lisk_lock_t		lock;

int main(int argc, char** argv)	{
	int				idx;
	int				ret;
	int				top;
	void*			arg;
	void*			pret;
	pthread_attr_t*	attr;

	LISK_LOCK_INIT(lock);

	top = 3;
	arg = NULL;
	attr = NULL;
	pthread_t trd[top];
	for (idx = 0; idx < top; idx++)   {
		arg = &idx;		//local varialble as thread argument ^_^
		ret = pthread_create(trd + idx, attr, test, arg);
		if (ret)    {
			break;
		}
	}
	top = idx;

	for (idx = 0; idx < top; idx++)   {
		pret = NULL;
		pthread_join(trd[idx], &pret);
	}

	return 0;
}

void* test(void* arg)	{
	int			id;
	int			idx;
	int			top;
	void*		pret;

	id = *(int*)(arg);
	top = 10;
	for (idx = 0; idx < top; idx++) {
		lisk_lock(&lock);
		num++;
		printf("thread id = %d, number = %d\n", id, num);
		lisk_unlock(&lock);
	}

	pret = NULL;
	pthread_exit(pret);

	return NULL;
}

