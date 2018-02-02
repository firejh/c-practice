/**
 * FILE		:lisk_atom.c
 * DESC		:to supply atom
 * AUTHOR	:v0.3.0 written by Alex Stocks
 * DATE		:on Sep 17, 2012
 * MOD		:modified by * on June 2, 2012
 ***/

#include <lisk_atom.h>
#include <pthread.h>

n4 lisk_lock(lisk_lock_p lock)	{
	if (!lock)	{
		return -1;
	}

	do	{
		if (ELOCK_FREE != lock->flag)	{
			continue;
		}

		if (ELOCK_FREE == lisk_sync_cas(lock->flag, ELOCK_FREE, ELOCK_BUSY))	{
			break;
		}

		sched_yield();
	} while(1);

	return 0;
}

n4 lisk_unlock(lisk_lock_p lock)	{
	if (!lock)	{
		return -1;
	}

	lisk_sync_set(lock->flag, ELOCK_FREE);

	return 0;
}

