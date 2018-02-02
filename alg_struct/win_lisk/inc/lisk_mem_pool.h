/**
 * FILE		: lisk_mem_pool.h
 * DESC		: memory pool
 * AUTHOR	: v1.0 written by Alex Stocks
 * DATE		: on Nov 5, 2012
 * MOD		: modified by * on June 2, 2012
 **/

#ifndef __LISK_MEM_POOL_H__
#define __LISK_MEM_POOL_H__

#ifdef  __cplusplus
extern "C"	{
#endif

#include <lisk_type.h>

/**
 * description		: init memory pool <br/><br/>
 *
 * in-out@pool		: memory pool
 * in-@block_size	: a memory block size
 *
 * out-@ret			: if successful, ret is memory pool;
 *					  otherwise nil if @block_size is
 *					  not more than 0 or fail to allocate memory block.
 **/
n4 mem_pool_init(vpp pool, u4 block_size);
/**
 * description		: deallocate all blocks of memory pool @pool <br/><br/>
 *
 * in-out@pool		: memory pool
 *
 * out-@ret			: the ret is nil
 **/
vd mem_pool_uninit(vpp pool);
/**
 * description		: apply to memory pool @pool for a memory space
 *					  whose size is @size <br/><br/>
 *
 * in-@pool			: memory pool handler
 * in-@size			: applied memory space size
 *
 * out-@ret			: if memory pool allocates successfully,
 *					  the ret is that memory space; or the ret is nil
 **/
vp mem_pool_alloc(vp pool, u4 size);
/**
 * description		: deallocate memory space @ptr that
 *					  you applied to memory pool @pool <br/><br/>
 *
 * in-@pool			: memory pool handler
 * in-out@ptr		: a memory space that you applied to @pool
 *
 * out-@ret			: if successful, ret is 0; otherwise -1 if
 *					  @mem_pool is nil or @ptr is nil or -2
 *					  if can not find your memory space @ptr in memory pool @pool
 **/
n4	mem_pool_dealloc(vp pool, vp ptr);
/**
 * description		: get the size of the @pool memory <br/><br/>
 *
 * in-@pool			: memory pool handler
 * in-out@size		: memory size
 *
 * out-@ret			: if successful, ret is 0; otherwise -1 instead.
 **/
n4	mem_pool_mem_size(vp pool, u8* size);
/**
 * description		: get the free size of @pool that has not been occupied <br/><br/>
 *
 * in-@pool			: memory pool handler
 * in-out@size		: memory free size
 *
 * out-@ret			: if successful, ret is 0; otherwise -1 instead.
 **/
n4	mem_pool_free_size(vp pool, u8* size);

#ifdef  __cplusplus
}
#endif

#endif

