/**
 * FILE		:lisk_shm.cc
 * DESC		:shm inf
 * AUTHOR	:v1.0 written by Alex Stocks
 * DATE		:on Sep 5, 2012
 * MOD		:modified by * on June 2, 2012
 **/

#ifdef __cplusplus
extern "C" {
#endif

#include <Windows.h>
#include <WinBase.h>
#include <lisk_shm.h>
#include <lisk_public.h>
#include <lisk_log.h>

#ifdef __cplusplus
	}
#endif

int get_shm(shm_p shm)	{
	SECURITY_ATTRIBUTES sa;
	vp	shm_handle = nil;
	vp	shm_ptr = nil;
	char*	key_ptr = nil;

	if (IS_NL(shm) || IS_LE(shm->size, 0) || IS_NL(shm->key)) {
		return -1;
	}

	sa.nLength	=sizeof(sa);
	sa.lpSecurityDescriptor	=	nil;
	sa.bInheritHandle		=	TRUE;
	if (IS_PN(strlen(shm->key)))	{
		key_ptr = shm->key;
	}
	shm_handle = CreateFileMapping((HANDLE)(INVALID_HANDLE_VALUE), &sa, (DWORD)(PAGE_READWRITE), (DWORD)(0), (DWORD)(shm->size), (LPCTSTR)(key_ptr));
	if (IS_NL(shm_handle))	{
		return -2;
	}

	shm_ptr = (vp)MapViewOfFile((HANDLE)shm_handle, FILE_MAP_WRITE, 0, 0, 0);
	if (IS_NL(shm_ptr))	{
		CloseHandle((HANDLE)(shm_handle));
		return -2;
	}

	shm->handle = shm_handle;
	shm->ptr = shm_ptr;

    return 0;
}

vd release_shm(shm_p shm)	{
	vp shm_handle = nil;
	int ret = 0;
	if (IS_NNL(shm))	{
		shm_handle = shm->handle;
		if (IS_NNL(shm->ptr))	{
			ret = UnmapViewOfFile(shm->ptr);
			if (ret)	{
				if (IS_NNL(shm_handle))	{
					ret = CloseHandle(shm_handle);
				}			
			}
		}
	}
}

