/**
 * FILE		: lisk_mem_pool.c
 * DESC		: memory pool
 * AUTHOR	: v1.0 written by Alex Stocks
 * DATE		: on Nov 5, 2012
 * MOD		: modified by * on June 2, 2012
 **/

#include <lisk_mem_pool.h>
#include <lisk_log.h>
#include <lisk_public.h>

#pragma pack(1)

typedef struct chunk_t_tag {
	struct chunk_t_tag*	pre;
	struct chunk_t_tag*	next;
} chunk_t, *chunk_p, chunk_a[1];

#pragma pack(1)

typedef struct block_t_tag {
	u4					size;
	u4					free;
	struct block_t_tag*	next;
	chunk_p				bottom;
	n1					data[1];
} block_t, *block_p, block_a[1];

#pragma pack()

typedef struct mem_pool_t_tag {
	u4					block_num;
	u4					block_size;
	block_p				block_list;		//utilization is greater than 80%
} mem_pool_t, *mem_pool_p, mem_pool_a[1];

#pragma pack()

#define	UNIT_SIZE					sizeof(chunk_t)
#define	BLOCK_END(block)			(vp)(block->data + block->size)
#define	CHUNK_FIRST(chunk)			ELEM_PTR(vp, chunk, UNIT_SIZE)
#define	MIN_CHUNK_SIZE				(UNIT_SIZE * 3)

n4	block_init(vpp block, u4 size);
vd	block_uninit(vpp block);
vp	block_malloc(block_p block, u4 size);
n4	block_free(block_p block, vp ptr);
vd	print_block_chunk(block_p block);
/**
 * description		: print information of memory pool @pool
 *
 * in-@pool			: memory pool handler
 *
 * out-@ret			: the ret is vd
 **/
vd	print_pool_info(vp pool);

static uw get_freebit(chunk_p chunk)
{
	return (PTR_2_NUM(chunk->next) & (uw)(0x01));
}

static vd set_freebit(chunk_p chunk)
{
	chunk->next = NUM_2_PTR(PTR_2_NUM(chunk->next) | (uw)(0x01));
}

static vd clr_freebit(chunk_p chunk)
{
	chunk->next = NUM_2_PTR(PTR_2_NUM(chunk->next) & (uw)(~0x01));
}

static chunk_p chunk_next(chunk_p chunk)
{
	return (chunk_p)NUM_2_PTR(PTR_2_NUM(chunk->next) & (uw)(~0x01));
}

static u4  chunk_mem_size(chunk_p chunk)
{
	chunk_p	next;

	if (IS_NL(chunk)) {
		return 0;
	}

	next = chunk_next(chunk);
	return 	(u4)PTR_OFFSET(chunk, next);
}

n4 block_init(vpp block_pptr, u4 size)
{
	chunk_p		chunk;
	block_p		block;

	if (IS_NL(block_pptr) || IS_ZR(size)) {
		RET_INT(-1, "@block = %p, @size = %u", block_pptr, size);
	}

	block = nil;
	size = ALIGN_SIZE(size, UNIT_SIZE);
	if (IS_NL(ALLOC(block_p, block, sizeof(block_t) - sizeof(n1) + size))) {
		RET_INT(-2, "ALLOC(block, %zu) = nil", sizeof(block_t) - sizeof(n1) + size);
	}

	block->size		= size;
	block->free		= size;
	block->next		= nil;
	block->bottom	= (chunk_p)(block->data);

	chunk = block->bottom;
	chunk->pre		= nil;
	chunk->next		= BLOCK_END(block);
	set_freebit(chunk);

	*block_pptr = block;

	RET_INT(0, nil_str);
}

vd block_uninit(vpp block_pptr)
{
	block_p		block;

	if (IS_NNL(block_pptr) && IS_NNL(block = (block_p)(*block_pptr)))	{
		DEALLOC(*block_pptr);
	}
}

vp block_malloc(block_p block, u4 size)
{
	u4			chunk_size;
	u4			cur_chunk_size;
	vp			block_end;
	chunk_p		next;
	chunk_p		ret;
	chunk_p		chunk;
	chunk_p		cur_chunk;

	if (IS_NL(block) || IS_ZR(size)) {
		RET_NL("@block = %p, @size = %u", block, size);
	}

	size += UNIT_SIZE;
	if (IS_GT(ALIGN_SIZE(size, UNIT_SIZE), block->free)) {
		RET_NL("ALIGN_SIZE(%u) = %zu is greater than block free size %u",
				size, ALIGN_SIZE(size, UNIT_SIZE), block->free);
	}
	size = ALIGN_SIZE(size, UNIT_SIZE);

	//the most adaptive memory allocation method
	chunk = nil;
	chunk_size = 0;
	cur_chunk_size = 0;
	cur_chunk = block->bottom;
	block_end = BLOCK_END(block);
	while (IS_NEQ(cur_chunk, block_end)) {
		cur_chunk_size = chunk_mem_size(cur_chunk);
		if (IS_NZR(get_freebit(cur_chunk)) && IS_GE(cur_chunk_size, size)) {
			if (IS_NL(chunk)) {
				chunk = cur_chunk;
				chunk_size = cur_chunk_size;
			} else if (IS_LT(cur_chunk_size, chunk_size)) {
				chunk = cur_chunk;
				chunk_size = cur_chunk_size;
			}
		}
		cur_chunk = chunk_next(cur_chunk);
	}

	if (IS_NL(chunk)) {
		RET_NL("can not get a chunk whose size is not less than size %u", size);
	}

	ret = nil;
	if (IS_LT(chunk_mem_size(chunk) - size, MIN_CHUNK_SIZE)) {
		ret = chunk;
	} else {
		ret = (chunk_p)(chunk_next(chunk) - size);
		ret->next = chunk->next;
	}
	clr_freebit(ret);
	if (IS_NEQ(chunk, ret)) {
		ret->pre = chunk;
		next = chunk_next(chunk);
		if (IS_NEQ(next, block_end)) {
			next->pre = ret;
		}
		chunk->next = ret;
		set_freebit(chunk);
	}
	block->free -= size;

	RET_PTR(CHUNK_FIRST(ret), nil_str);
}

n4 block_free(block_p block, vp ptr)
{
	chunk_p	pre;
	chunk_p	next;
	chunk_p	chunk;
	vp		block_end;

	if (IS_NL(block) || IS_NL(ptr)) {
		RET_INT(-1, "@block = %p, @ptr = %p", block, ptr);
	}

	block_end = BLOCK_END(block);
	if (IS_NBT(ptr, (vp)(block->data), block_end)) {
		//RET_INT(-1, "block->data = %p, block_end = %p, ptr = %p",
		//		block->data, block_end, ptr);
		RET_INT(-1, nil_str);
	}

	chunk = ELEM_PRE(chunk_t, ptr, UNIT_SIZE);
	set_freebit(chunk);
	block->free += (chunk_mem_size(chunk) - UNIT_SIZE);
	next = chunk_next(chunk);
	if (IS_NEQ(next, block_end) && IS_NZR(get_freebit(next))) {
		chunk->next = next->next;
		block->free += UNIT_SIZE;
	}
	pre = chunk->pre;
	if (IS_NNL(pre) && IS_NZR(get_freebit(pre))) {
		pre->next = chunk->next;
		block->free += UNIT_SIZE;
	}

	RET_INT(0, nil_str);
}

//if dynamic allocation size is not less than 128k, the malloc is equal to mmap
#define	BLOCK_SIZE	(4 * 1024)

n4 mem_pool_init(vpp pool_pptr, u4 block_size)
{
	n4			ret;
	block_p		block;
	mem_pool_p	pool;

	if (IS_NL(pool_pptr))	{
		RET_INT(-1, "@pool = %p", pool_pptr);
	}

	if (IS_ZR(block_size)) {
		RET_INT(-1, "@block_size = %u", block_size);
	}

	if (IS_NL(ALLOC(mem_pool_p, pool, sizeof(mem_pool_t)))) {
		RET_INT(-2, "ALLOC(pool = %p, size = %zu", pool, sizeof(mem_pool_t));
	}

	block_size = (block_size + BLOCK_SIZE - 1) & (~(BLOCK_SIZE - 1));
	ret = block_init((vpp)(&block), block_size);
	if (IS_NZR(ret)) {
		RET_INT(-2, "block_init(block = %p, size = %u) = %d", &block, block_size, ret);
	}

	pool->block_num		= 1;
	pool->block_size	= block_size;
	pool->block_list	= block;

	*pool_pptr = (vp)pool;

	RET_INT(0, nil_str);
}

vd mem_pool_uninit(vpp pool_pptr)
{
	block_p		block;
	block_p		next;
	mem_pool_p	pool;

	if (IS_NL(pool_pptr))	{
		RET_VD("@pool = %p", pool_pptr);
	}

	pool = (mem_pool_p)(*pool_pptr);
	if (IS_NL(pool)) {
		RET_VD("*@pool = %p", pool);
	}

	block = pool->block_list;
	next = nil;
	while (IS_NNL(block)) {
		next = block->next;
		block_uninit((vpp)(&block));
		block = next;
	}

	DEALLOC(*pool_pptr);
}

#include <sys/types.h>

#define	SYS_ALLOC_FLAG	(0XF012F105)
vp mem_pool_alloc(vp pool_ptr, u4 size)
{
	n4			ret;
	u4*			arr;
	vp			ptr;
	block_p		block;
	mem_pool_p	pool;

	pool = pool_ptr;
	if (IS_NL(pool) || IS_ZR(size)) {
		RET_NL("@pool = %p, @size = %u", pool, size);
	}

	arr = nil;
	if (IS_GE(size, pool->block_size)) {
		size += 2 * sizeof(u4);
		if (IS_NL(ALLOC(u4*, arr, size))) {
			RET_NL("ALLOC(%p, %u)", arr, size);
		}
		arr[0] = (u4)SYS_ALLOC_FLAG;
		arr[1] = size - 2 * sizeof(n4);
		RET_PTR(&(arr[2]), nil_str);
	}

	//iterate block list
	ptr = nil;
	block = pool->block_list;
	while (IS_NNL(block)) {
		if (IS_GT(block->free, size)) {
			ptr = block_malloc(block, size);
			if (IS_NNL(ptr)) {
				RET_PTR(ptr, nil_str);
			}
		}
		block = block->next;
	}

	ret = block_init((vpp)(&block), pool->block_size);
	if (IS_NZR(ret)) {
		RET_NL("block_init(&block = %p, @pool->block_size = %u) = %d",
				&block, pool->block_size, ret);
	}
	block->next = pool->block_list;
	pool->block_list = block;
	pool->block_num++;
	ptr = block_malloc(block, size);

	if (IS_NL(ptr)) {
		RET_NL("block_malloc(%p, %u) = %p", block, size, ptr);
	}

	RET_PTR(ptr, nil_str);
}

n4	mem_pool_dealloc(vp pool_ptr, vp ptr)
{
	n4			ret;
	u4*			arr;
	block_p		pre;
	block_p		block;
	mem_pool_p	pool;

	if (IS_NL(pool = pool_ptr) || IS_NL(ptr)) {
		RET_INT(-1, "@pool = %p, @ptr = %p", pool, ptr);
	}

	arr = ELEM_PRE(u4, ptr, 2 * sizeof(u4));
	if (IS_EQ(arr[0], SYS_ALLOC_FLAG) && IS_GE(arr[1], pool->block_size)) {
		DEALLOC(arr);
		RET_INT(0, nil_str);
	}

	if (IS_NL(pool)) {
		RET_INT(-1, "@pool = %p", pool);
	}

	ret = 0;
	pre = nil;
	block = pool->block_list;
	while (IS_NNL(block)) {
		ret = block_free(block, ptr);
		if (IS_ZR(ret)) {
			if (IS_EQ(block->size, block->free)) {
				if (IS_NNL(pre)) {
					pre->next = block->next;
					//block->next = pool->block_list;
					//pool->block_list = block;
					// release this block to os to reduce the size of @pool
					block->next = nil;
					block_uninit((vp)(&block));
					pool->block_num--;
				}
			}
			break;
		}
		pre = block;
		block = block->next;
	}

	if (IS_NZR(ret)) {
		RET_INT(-2, "can not find %p in pool %p(%u, %p)",
				ptr, pool, pool->block_size, pool->block_list);
	}

	RET_INT(0, nil_str);
}

vd print_block_chunk(block_p block)
{
	chunk_p	chunk;
	vp		block_end;

	if (IS_NL(block)) {
		RET_VD("@block = %p", block);
	}

	chunk = block->bottom;
	block_end = BLOCK_END(block);
	while (IS_NEQ(chunk, block_end)) {
#if defined(DEBUG)
		u4 chunk_size = chunk_mem_size(chunk);
		u4 free_flag = get_freebit(chunk);
		LISK_LOG_INFO("cur chunk %p, pre chunk %p, next chunk %p, "
					"chunk size %u, free flag %u",
					chunk, chunk->pre, chunk->next, chunk_size, free_flag);
#endif
		chunk = chunk_next(chunk);
	}
}

vd print_pool_info(vp pool_ptr)
{
	block_p		block;
	mem_pool_p	pool;

	if (IS_NL(pool = pool_ptr)) {
		RET_VD(nil_str);
	}

	block = pool->block_list;
	while (IS_NNL(block)) {
		print_block_chunk(block);
		block = block->next;
	}
}

n4	mem_pool_mem_size(vp pool_ptr, u8* size)
{
	mem_pool_p	pool;
	if (IS_NL(pool = pool_ptr) || IS_NL(size))	{
		RET_INT(-1, "@pool = %p, @size = %p", pool, size);
	}

	*size = sizeof(mem_pool_t);
	*size += (pool->block_num * (pool->block_size + sizeof(block_t)));

	RET_INT(0, nil_str);
}

n4	mem_pool_free_size(vp pool_ptr, u8* size)
{
	block_p		block;
	mem_pool_p	pool;

	if (IS_NL(pool = pool_ptr) || IS_NL(size)) {
		RET_INT(-1, "@pool = %p, @size = %p", pool, size);
	}

	*size = 0;
	block = pool->block_list;
	while (IS_NNL(block)) {
		*size += block->free;
		block = block->next;
	}

	RET_INT(0, nil_str);
}

