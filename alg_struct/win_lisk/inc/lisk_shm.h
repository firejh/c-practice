/**
 * FILE		:lisk_shm.h
 * DESC		:shm inf
 * AUTHOR	:v1.0 written by Alex Stocks
 * DATE		:on Sep 5, 2012
 * MOD		:modified by * on June 2, 2012
 * **/

#ifndef __LISK_SHM_H__
#define __LISK_SHM_H__

#ifdef __cplusplus
	extern "C" {
#endif

#include <lisk_public.h>

typedef struct shm_tag {
	char		key[256];
	uw			size;
	vp			handle;
	vp			ptr;
} shm_t, *shm_p, shm[1];


/**
 * description	: get shared memory<br/><br/>
 *
 * out-@ret		: if successful, return sys shm num; otherwise -1 instead.
 **/
int get_shm(shm_p shm);
/**
 * description	: get shm size by key <br/><br/>
 *
 * in-@key		: shm key
 *
 * out-@ret		: if successful, return sys shm size ; otherwise -1 instead.
 **/
vd release_shm(shm_p shm_ptr);

#ifdef __cplusplus
	}
#endif

#endif

