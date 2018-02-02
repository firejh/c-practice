/**
 * FILE		:lisk_ring.cc
 * DESC		:a FIFO(first coming, first output) queue, also can be uesed as a ring buffer
 * AUTHOR	:v1.0 written by Alex Stocks
 * DATE		:on Oct 8, 2012
 * MOD		:modified by Alex Stocks on May 21, 2013
 **/

#include <lisk_atom.h>
//#include <lisk_log.h>
#include <lisk_ring.h>

#include <semaphore.h>
#include <string.h>
#include <sched.h>				//sched_yield

//////////////////////////////////////////
// array based lock-free ring
//////////////////////////////////////////

#pragma pack(8)

typedef struct ring_tag {
	volatile u4		size;
	volatile u4		unit_size;
	volatile u4		unit_num;
	volatile u4		write_pos;
	volatile u4		read_pos;
	volatile u4		max_read_idx;
	volatile n1		data[0];
} ring_t, *ring_p, ring_a[1];

#pragma pack()

n4 ring_empty(vp ring);
n4 ring_full(vp ring);
n4 ring_unit_num(vp ring, u4* num);

n4 ring_init(vpp ring_pptr, u4 unit_num, u4 unit_size)
{
	u4		size;
	ring_p	ring;

	/* size must be a power of 2 */
	if (IS_NL(ring_pptr) || IS_ZR(unit_num) || IS_ZR(unit_size) ||
		IS_NZR(unit_num & (unit_num - 1))) {
		RET_INT(-1, "@ring = %p, @unit_num = %u, @unit_size = %u",
				ring_pptr, unit_num, unit_size);
	}

	size = sizeof(ring_t) + unit_size * unit_num;
	if (IS_NL(ALLOC(ring, size))) {
		RET_INT(-2, "ALLOC(size = %u) = %p", size, ring);
	}

	SET_ZERO(ring, sizeof(ring_t));
	ring->unit_num	= unit_num;
	ring->unit_size	= unit_size;

	*ring_pptr = ring;

	RET_INT(0, nil_str);
}

vd ring_uninit(vpp ring_pptr)
{
	ring_p	ring;

	if (IS_NNL(ring_pptr) && IS_NNL(ring = (ring_p)(*ring_pptr))) {
		DEALLOC(ring);
	}
}

n4 ring_size(vp ring_ptr, u4* size)
{
	ring_p	ring;

	ring = ring_ptr;
	if (IS_NL(ring) || IS_NL(size)) {
		RET_INT(-1, nil_str);
	}

	*size = ring->size;

	RET_INT(0, nil_str);
}

n4 ring_unit_num(vp ring_ptr, u4* num)
{
	ring_p	ring;

	ring = ring_ptr;
	if (IS_NL(ring) || IS_NL(num)) {
		RET_INT(-1, nil_str);
	}

	*num = ring->unit_num;

	RET_INT(0, nil_str);
}

//unit_num should be a power of 2
u4 ring_index(u4 count, u4 unit_num)
{
	if (IS_NZR(unit_num)) {
		RET_INT(count & (unit_num - 1), nil_str);
	}

	RET_INT(0, nil_str);
}

n4 ring_empty(vp ring_ptr)
{
	n4		ret;
	ring_p	ring;

	ret = -1;
	if (IS_NL(ring = ring_ptr))	{
		RET_INT(ret, nil_str);
	}

	ret = -2;
	if (IS_NEQ(ring_index(ring->read_pos, ring->unit_num), ring_index(ring->max_read_idx, ring->unit_num))) {
		ret = 0;
	}

	RET_INT(ret, nil_str);
}

n4 ring_full(vp ring_ptr)
{
	n4		ret;
	ring_p	ring;

	ret = -1;
	if (IS_NL(ring = ring_ptr))	{
		RET_INT(ret, nil_str);
	}

	ret = -2;
	if (IS_EQ(ring_index(ring->write_pos, ring->unit_num), ring_index(ring->read_pos, ring->unit_num))) {
		ret = 0;
	}

	RET_INT(ret, nil_str);
}

n4 ring_push(vp ring_ptr, vp elem, u4 elem_num)
{
	u4		ridx;
	u4		widx;
	ring_p	ring;

	if (IS_NL(ring = ring_ptr) || IS_NL(elem) || IS_ZR(elem_num)) {
		RET_INT(-1, nil_str);
	}

	ridx = 0;
	widx = 0;
	do {
		ridx = ring->read_pos;
		widx = ring->write_pos;
		//the ring is full
		if (IS_EQ(ring_index(widx + elem_num, ring->unit_num), ring_index(ridx, ring->unit_num))) {
			//RET_INT(-2, "push widx = %u, ridx = %u", widx, ridx);
			RET_INT(-2, nil_str);
		}
	} while(IS_NEQ(lisk_sync_cas(ring->write_pos, widx, widx + elem_num), widx));

	CPY(ring->data + ring_index(widx, ring->unit_num) * ring->unit_size, elem, elem_num * (ring->unit_size));

	while (IS_NEQ(lisk_sync_cas(ring->max_read_idx, widx, widx + elem_num), widx)) {
		sched_yield();
	}

	lisk_sync_add(ring->size, elem_num);

	RET_INT(0, nil_str);
}

n4 ring_pop(vp ring_ptr, vp elem, u4* elem_num)
{
	u4		ridx;
	u4		num;
	u4		max_ridx;
	ring_p	ring;

	if (IS_NL(ring = ring_ptr) || IS_NL(elem) || IS_NL(elem_num) || IS_ZR(num = *elem_num)) {
		RET_INT(-1, nil_str);
	}

	ridx = 0;
	max_ridx = 0;
	do {
		ridx = ring->read_pos;
		max_ridx = ring->max_read_idx;

		//the ring is empty or a procuder has got space from the ring and dumping data into it now
		if (IS_EQ(ring_index(ridx, ring->unit_num), ring_index(max_ridx, ring->unit_num))) {
			RET_INT(-2, nil_str);
		}

		if (IS_GT(num, ring->size))	{
			num = ring->size;
		}

		CPY(elem, ring->data + ring_index(ridx, ring->unit_num) * ring->unit_size, num * (ring->unit_size));
		*elem_num = num;

		if (IS_EQ(lisk_sync_cas(ring->read_pos, ridx, ridx + num), ridx)) {
			lisk_sync_sub(ring->size, num);
			RET_INT(0, nil_str);
		}
	} while(1);

	RET_INT(-2, nil_str);
}

//////////////////////////////////////////
// lock free ring based on list
//////////////////////////////////////////

#pragma pack(1)

typedef struct queue_node_tag	{
	struct queue_node_tag*	next;
	n1						data[0];
} queue_node_t, *queue_node_p, queue_node_a[1];

#pragma pack()

n4 queue_node_init(vpp node, vp data, u4 size);
vd queue_node_uninit(vpp node);

n4 queue_node_init(vpp node_pptr, vp data, u4 size)
{
	u4				len;
	queue_node_p	node;

	if (IS_NL(node_pptr))	{
		RET_INT(-1, "@node = %p", node_pptr);
	}

	*node_pptr = nil;

	len = sizeof(queue_node_t) + size;
	if (IS_NL(ALLOC(node, len)))	{
		RET_INT(-2, "ALLOC(len = %u) = nil", len);
	}
	if (IS_NNL(data) && IS_ZR(size))	{
		CPY(node->data, data, size);
	}
	node->next = nil;

	*node_pptr = node;

	RET_INT(0, nil_str);
}

vd queue_node_uninit(vpp node)
{
	if (IS_NNL(node) && IS_NNL(*node))	{
		DEALLOC(*node);
	}
}

#pragma pack(1)

typedef struct queue_tag	{
	u4				size;			// element size
	lisk_atom_t		num;			// element number
	vp				ring;
	queue_node_p	head;
	queue_node_p	tail;
} queue_t, *queue_p, queue_a[1];

#pragma pack()

n4 queue_fill(vp queue);
n4 queue_clear(vp queue);

n4 queue_init(vpp queue_pptr, u4 size)
{
	n4		ret;
	u4		num;
	vp	node;
	queue_p	queue;

	if (IS_NL(queue_pptr) || IS_ZR(size))	{
		RET_INT(-1, "@queue_pptr = %p, @size = %u", queue_pptr, size);
	}

	*queue_pptr = nil;

	ret = queue_node_init(&node, nil, size);
	if (IS_NZR(ret))	{
		RET_INT(-2, "queue_node_init(&node = %p) = %d", &node, ret);
	}

	if (IS_NL(ALLOC(queue, sizeof(queue_t))))	{
		queue_node_uninit(&node);
		RET_INT(-2, "ALLOC(size = %zu) = %p", sizeof(queue_t), queue);
	}

	num = SIZE_K(16);
	ret = ring_init(&(queue->ring), num, sizeof(vp));
	if (IS_NZR(ret))	{
		queue_node_uninit(&node);
		DEALLOC(queue);
		RET_INT(-2, "ring_init(&ring = %p, num = %u, size = %zu) = %d",
				&(queue->ring), num, sizeof(vp), ret);
	}

	queue->size = size;
	queue->head = node;
	queue->tail = node;
	lisk_atom_zero(queue->num);

	ret = queue_fill(queue);
	if (IS_NZR(ret))	{
		queue_clear(queue);
		ring_uninit(&(queue->ring));
		queue_node_uninit(&node);
		DEALLOC(queue);
		RET_INT(-2, "ring_init(&ring = %p, num = %u, size = %zu) = %d",
				&(queue->ring), num, sizeof(queue_t) + size, ret);
	}

	*queue_pptr = queue;

	RET_INT(0, nil_str);
}

vd queue_uninit(vpp queue_pptr)
{
	queue_p			queue;
	queue_node_p	next;
	queue_node_p	node;

	if (IS_NL(queue_pptr) || IS_NL(queue = (queue_p)(*queue_pptr)))	{
		RET_VD("@queue_pptr = %p, @queue = %p", queue_pptr, queue);
	}

	node = queue->head;
	while (IS_NNL(node))	{
		next = node->next;
		queue_node_uninit((vpp)(&node));
		node = next;
	}

	queue_clear(queue);
	ring_uninit(&(queue->ring));

	DEALLOC(*queue_pptr);
}

n4 queue_fill(vp queue_ptr)
{
	n4		ret;
	u4		idx;
	u4		max;
	u4		num;
	u4		size;
	vp		node;
	vp		data;
	queue_p	queue;

	queue = queue_ptr;
	if (IS_NL(queue))	{
		RET_INT(-1, "@queue = %p", queue);
	}

	ret = ring_unit_num(queue->ring, &num);
	if (IS_NZR(ret) || IS_ZR(num))	{
		RET_INT(-2, "ring_unit_num(ring = %p, num = %u) = %d", queue->ring, num, ret);
	}

	ret = ring_size(queue->ring, &size);
	if (IS_NZR(ret))	{
		RET_INT(-2, "ring_unit_num(ring = %p, &size = %p) = %d", queue->ring, &size, ret);
	}

	num >>= 1;
	if (IS_GE(size, num))	{
		RET_INT(0, nil_str);
	}

	data = nil;
	max = num;
	for (idx = size; idx < max; idx++)	{
		ret = queue_node_init(&node, data, queue->size);
		if (IS_NZR(ret))	{
			break;
		}
		num = 1;
		ret = ring_push(queue->ring, (vp)(&node), num);
		if (IS_NZR(ret))	{
			queue_node_uninit((vpp)(&node));
			break;
		}
	}

	RET_INT(0, nil_str);
}

n4 queue_clear(vp queue_ptr)
{
	n4				ret;
	u4				num;
	queue_node_p	node;
	queue_p			queue;

	queue = queue_ptr;
	if (IS_NL(queue))	{
		RET_INT(-1, "@queue = %p", queue);
	}

	do	{
		num = 1;
		ret = ring_pop(queue->ring, (vp)(&node), &num);
		if (IS_NZR(ret) || IS_NEQ(num, 1))	{
			break;
		}
		queue_node_uninit((vp)(&node));
	} while(1);

	RET_INT(0, nil_str);
}

n4 queue_push(vp queue_ptr, vp data)
{
	n4				ret;
	u4				num;
	vp				node;
	queue_p			queue;
	queue_node_p	next;
	queue_node_p	tail;

	queue = queue_ptr;
	if (IS_NL(queue) || IS_NL(data))	{
		RET_INT(-1, "@queue = %p, @data = %p", queue, data);
	}

	num = 1;
	ret = ring_pop(queue->ring, (vp)(&node), &num);
	if (IS_ZR(ret))	{
		CPY(((queue_node_p)node)->data, data, queue->size);
		((queue_node_p)node)->next = nil;
	} else {
		ret = queue_node_init(&node, data, queue->size);
		if (IS_NZR(ret))	{
			RET_INT(-2, "queue_node_init(&node = %p, @data = %p, @size = %d) = %d",
						&node, data, queue->size, ret);
		}
	}

	while (1)	{
		tail = queue->tail;
		next = tail->next;
		if (IS_NEQ(tail, queue->tail))	{
			continue;
		}

		if (IS_NNL(next))	{
			(vd)lisk_sync_cas(queue->tail, tail, next);
			continue;
		}

		if (IS_EQ(lisk_sync_cas(tail->next, nil, node), nil))	{
			break;
		}
	}

	(vd)lisk_sync_cas(queue->tail, tail, node);

	queue_fill(queue);
	lisk_atom_add(queue->num, 1);

	RET_INT(0, nil_str);
}

n4 queue_pop(vp queue_ptr, vp data_ptr)
{
	n4				ret;
	u4				num;
	vp				data;
	queue_p			queue;
	queue_node_p	head;
	queue_node_p	next;
	queue_node_p	tail;

	if (IS_NL(queue = queue_ptr) || IS_NL(data_ptr))	{
		RET_INT(-1, "@queue = %p, @data_pptr = %p", queue, data_ptr);
	}

	while (1)	{
		head = queue->head;
		tail = queue->tail;
		next = head->next;

		data = nil;

		if (IS_NEQ(head, queue->head))	{
			continue;
		}

		if (IS_NL(next))	{
			break;
		}

		if (IS_EQ(head, tail))	{
			(vd)lisk_sync_cas(queue->tail, tail, next);
			continue;
		}

		data = next->data;

		if (IS_EQ(lisk_sync_cas(queue->head, head, next), head))	{
			break;
		}
	}

	if (IS_NL(data))	{
		RET_INT(-2, nil_str);
	}

	CPY(data_ptr, data, queue->size);

	num = 1;
	ret = ring_push(queue->ring, (vp)(&head), num);
	if (IS_NZR(ret))	{
		queue_node_uninit((vpp)(&head));
	}

	lisk_atom_sub(queue->num, 1);

	RET_INT(0, nil_str);
}

n4 queue_size(vp queue_ptr, u4* size)
{
	queue_p	queue;

	if (IS_NL(queue = queue_ptr) || IS_NL(size))	{
		RET_INT(-1, "@queue_ptr = %p, @size = %p", queue, size);
	}

	*size = (u4)lisk_atom_read(queue->num);

	RET_INT(0, nil_str);
}

