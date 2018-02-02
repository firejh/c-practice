/**
 * FILE		: lisk_deque.c
 * DESC		: double-ended queue
 * AUTHOR	: v1.0 written by Alex Stocks
 * DATE		: on Apr 21, 2014
 * LICENCE	: GPL 2.0
 * MOD		:
 **/

#include <lisk_deque.h>
//#include <lisk_log.h>
#include <lisk_public.h>

typedef struct block_tag	{
	vp			data;
} block_t, *block_p, block_a[1];

typedef struct deque_tag	{
	// map
	block_p		map;
	u4			map_size;
	// block
	// head.cur = first element index
	// tail.cur = tail element index + 1
	struct {
		u4		index;	// map index
		u4		cur;	// current element index
	} start, end;
	u4			unit_num;
	u4			unit_size;
} deque_t, *deque_p, deque_a[1];

n4 deque_new_map(vp deque);

n4 _deque_init(vpp deque_pptr, u4 unit_size, u4 map_size, u4 block_size)
{
	u4			size;
	deque_p		deque;

	if (IS_NL(deque_pptr) ||
		IS_ZR(unit_size) ||
		IS_ZR(map_size) ||
		IS_LT(block_size, unit_size))	{
		RET_INT(-1, "@deque_pptr = %p, @unit_size = %u, "
					"@map_size = %u, @block_size = %u",
					deque_pptr, unit_size, map_size, block_size);
	}

	if (IS_NL(ALLOC(deque, sizeof(*deque))))	{
		RET_INT(-2, "@ALLOC(sizeof(deque) = %zu) = nil", sizeof(*deque));
	}

	// map
	deque->map_size = map_size;
	size = deque->map_size * sizeof(block_t);
	if (IS_NL(ALLOC(deque->map, size)))	{
		DEALLOC(deque);
		RET_INT(-1, "ALLOC(deque->map, size = %u) = nil", size);
	}
	deque->start.index = map_size >> 1;
	deque->start.cur = 0;
	CPY(&(deque->end), &(deque->start), sizeof(deque->end));

	// block
	deque->unit_size = unit_size;
	deque->unit_num = block_size / deque->unit_size;
	size = deque->unit_num * deque->unit_size;
	if (IS_NL(ALLOC(deque->map[deque->start.index].data, size)))	{
		DEALLOC(deque->map);
		DEALLOC(deque);
		RET_INT(-2, "ALLOC(deque->start.first, size = %u) = nil", size);
	}

	*deque_pptr = (vp)(deque);

	RET_INT(0, nil_str);
}

vd _deque_uninit(vpp deque_pptr)
{
	u4		idx;
	deque_p	deque;

	if (IS_NL(deque_pptr) || IS_NL(deque = (deque_p)(*deque_pptr)))	{
		RET_VD("@deque_pptr = %p, deque = %p", deque_pptr, deque);
	}

	for (idx = deque->start.index; IS_LE(idx, deque->end.index); idx++)	{
		DEALLOC(deque->map[idx].data);
	}

	DEALLOC(deque->map);
	DEALLOC(deque);
	*deque_pptr = nil;
}

n4 deque_update_map(vp deque_ptr)
{
	u4		num;
	u4		size;
	u4		pos;
	block_p	map;
	deque_p	deque;

	if (IS_NL(deque = (deque_p)(deque_ptr)))	{
		RET_INT(-1, "@deque = %p", deque);
	}

	num = deque->end.index - deque->start.index + 1;
	if (IS_LT(num << 1, deque->map_size))	{
		pos = (deque->map_size - num) >> 1;
		CPY(deque->map + pos, deque->map + deque->start.index, num * sizeof(block_t));
	} else {
		size = (deque->map_size << 1) * sizeof(block_t);
		if (IS_NL(ALLOC(map, size)))	{
			RET_INT(-2, "ALLOC(new_map, size = %u) = nil", size);
		}
		pos = deque->map_size - (num >> 1);
		CPY(&(map[pos]), &(deque->map[deque->start.index]), num * sizeof(block_t));
		DEALLOC(deque->map);
		deque->map = map;
		deque->map_size <<= 1;
	}
	deque->start.index = pos;
	deque->end.index = deque->start.index + num - 1;

	RET_INT(0, nil_str);
}

n4 deque_empty(vp deque_ptr)
{
	deque_p	deque;

	if (IS_NL(deque = (deque_p)(deque_ptr)))	{
		RET_INT(-1, "@deque = %p", deque);
	}

	if (IS_NEQ(deque->start.index, deque->end.index) ||
		IS_NEQ(deque->start.cur, deque->end.cur))	{
		RET_INT(-2, nil_str);
	}

	RET_INT(0, nil_str);
}

n4 deque_size(vp deque_ptr, u4* size)
{
	deque_p	deque;

	if (IS_NL(deque = (deque_p)(deque_ptr)) || IS_NL(size))	{
		RET_INT(-1, "@deque = %p, @size = %p", deque, size);
	}

	if (IS_EQ(deque->start.index, deque->end.index))	{
		*size = deque->end.index - deque->start.index;
	} else {
		*size = deque->unit_num - deque->start.cur +
				deque->end.cur +
				(deque->end.index - deque->start.index - 1) * deque->unit_num;
	}

	RET_INT(0, nil_str);
}

vp deque_at(vp deque_ptr, u4 pos)
{
	vp		ptr;
	block_p	block;
	deque_p	deque;

	if (IS_NL(deque = (deque_p)(deque_ptr)))	{
		RET_PTR(nil, "@deque = %p", deque);
	}

	pos += deque->start.cur;
	block = &(deque->map[deque->start.index + pos / deque->unit_num]);
	ptr = ELEM_PTR(vp, block->data, (pos % deque->unit_num) * deque->unit_size);

	RET_PTR(ptr, nil_str);
}

vp deque_front(vp deque_ptr)
{
	vp		ptr;
	block_p	block;
	deque_p	deque;

	if (IS_NL(deque = (deque_p)(deque_ptr)))	{
		RET_PTR(nil, "@deque = %p", deque);
	}

	ptr = nil;
	if (IS_EQ(deque->start.index, deque->end.index) &&
		IS_EQ(deque->start.cur, deque->end.cur))	{
		// deque is nil
		ptr = nil;
	} else {
		block = &(deque->map[deque->start.index]);
		ptr = ELEM_PTR(vp, block->data, deque->start.cur * deque->unit_size);
	}

	RET_PTR(ptr, nil_str);
}

vp deque_back(vp deque_ptr)
{
	vp		ptr;
	block_p	block;
	deque_p	deque;

	if (IS_NL(deque = (deque_p)(deque_ptr)))	{
		RET_PTR(nil, "@deque = %p", deque);
	}

	ptr = nil;
	if (IS_EQ(deque->start.index, deque->end.index) &&
		IS_EQ(deque->start.cur, deque->end.cur))	{
		// deque is nil
		ptr = nil;
	} else {
		block = &(deque->map[deque->end.index]);
		if (IS_NZR(deque->end.cur)) {
			ptr = ELEM_PTR(vp, block->data, (deque->end.cur - 1) * deque->unit_size);
		} else {
			block = &(deque->map[deque->end.index - 1]);
			ptr = ELEM_PTR(vp, block->data, (deque->unit_num - 1) * deque->unit_size);
		}
	}

	RET_PTR(ptr, nil_str);
}

n4 deque_push_front(vp deque_ptr, vp front)
{
	n4		ret;
	u4		size;
	vp		ptr;
	block_p	block;
	deque_p	deque;

	if (IS_NL(deque = (deque_p)(deque_ptr)) || IS_NL(front))	{
		RET_INT(-1, "@deque = %p, @front = %p", deque, front);
	}

	block = &(deque->map[deque->start.index]);
	if (IS_NZR(deque->start.cur)) {
		deque->start.cur--;
	} else {
		if (IS_ZR(deque->start.index))	{
			ret = deque_update_map(deque);
			if (IS_NZR(ret))	{
				RET_INT(-2, "deque_update_map(@deque = %p) = %d", deque, ret);
			}
		}

		size = deque->unit_num * deque->unit_size;
		if (IS_NL(ALLOC(ptr, size)))	{
			RET_INT(-2, "ALLOC(new block, size = %u) = nil", size);
		}

		deque->start.index--;
		block = &(deque->map[deque->start.index]);
		block->data = ptr;
		deque->start.cur = deque->unit_num - 1;
	}
	ptr = ELEM_PTR(vp, block->data, deque->start.cur * deque->unit_size);
	CPY(ptr, front, deque->unit_size);

	RET_INT(0, nil_str);
}

n4 deque_push_back(vp deque_ptr, vp back)
{
	n4		ret;
	u4		size;
	vp		ptr;
	block_p	block;
	deque_p	deque;

	if (IS_NL(deque = (deque_p)(deque_ptr)) || IS_NL(back))	{
		RET_INT(-1, "@deque = %p, @back = %p", deque, back);
	}

	block = &(deque->map[deque->end.index]);
	ptr = ELEM_PTR(vp, block->data, deque->end.cur * deque->unit_size);
	CPY(ptr, back, deque->unit_size);
	deque->end.cur++;
	if (IS_EQ(deque->end.cur, deque->unit_num)) {
		if (IS_EQ(deque->end.index, deque->map_size - 1))	{
			ret = deque_update_map(deque);
			if (IS_NZR(ret))	{
				deque->end.cur--;
				RET_INT(-2, "deque_update_map(deque = %p) = %d", deque, ret);
			}
		}

		size = deque->unit_num * deque->unit_size;
		if (IS_NL(ALLOC(ptr, size))) {
			deque->end.cur--;
			RET_INT(-2, "ALLOC(size = %u) = nil", size);
		}

		deque->end.index++;
		block = &(deque->map[deque->end.index]);
		block->data = ptr;
		deque->end.cur = 0;
	}

	RET_INT(0, nil_str);
}

n4 deque_pop_front(vp deque_ptr, vp front)
{
	block_p	block;
	deque_p	deque;

	if (IS_NL(deque = (deque_p)(deque_ptr)))	{
		RET_INT(-1, "@deque = %p", deque);
	}

	if (IS_EQ(deque->start.index, deque->end.index) &&
		IS_EQ(deque->start.cur, deque->end.cur))	{
		// deque is nil
		RET_INT(-1, "@deque is empty");
	}

	block = &(deque->map[deque->start.index]);
	if (IS_NNL(front))	{
		CPY(
			front,
			ELEM_PTR(vp, block->data, deque->start.cur * deque->unit_size),
			deque->unit_size
			);
	}
	deque->start.cur++;
	if (IS_EQ(deque->start.cur, deque->unit_num))	{
		DEALLOC(block->data);
		deque->start.index++;
		deque->start.cur = 0;
	}

	RET_INT(0, nil_str);
}

n4 deque_pop_back(vp deque_ptr, vp back)
{
	vp		ptr;
	block_p	block;
	deque_p	deque;

	if (IS_NL(deque = (deque_p)(deque_ptr)))	{
		RET_INT(-1, "@deque = %p", deque);
	}
	
	if (IS_EQ(deque->start.index, deque->end.index) &&
		IS_EQ(deque->start.cur, deque->end.cur))	{
		// deque is nil
		RET_INT(-1, "@deque is empty");
	}

	block = &(deque->map[deque->end.index]);
	if (IS_NZR(deque->end.cur)) {
		deque->end.cur--;
		if (IS_NNL(back)) {
			ptr = ELEM_PTR(vp, block->data, deque->end.cur * deque->unit_size);
			CPY(back, ptr, deque->unit_size);
		}
	} else {
		DEALLOC(block->data);
		deque->end.index--;
		block = &(deque->map[deque->end.index]);
		deque->end.cur = deque->unit_num;
		if (IS_NNL(back)) {
			ptr = ELEM_PTR(vp, block->data, (deque->end.cur - 1) * deque->unit_size);
			CPY(back, ptr, deque->unit_size);
		}
		deque->end.cur--;
	}

	RET_INT(0, nil_str);
}

