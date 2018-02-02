/**
 * FILE		:lisk_ring.cc
 * DESC		:a FIFO(first coming, first output) queue, also can be uesed as a ring buffer
 * AUTHOR	:v1.0 written by Alex Stocks
 * DATE		:on Oct 8, 2012
 * MOD		:modified by Alex Stocks on May 21, 2013
 **/

#include <lisk_atom.h>
#include <lisk_ring.h>

#include <semaphore.h>
#include <string.h>
#include <sched.h>				//sched_yield

//////////////////////////////////////////
// array based lock-free ring
//////////////////////////////////////////

typedef struct ring_tag {
	u4				unit_size;
	u4				unit_num;
	u4				type;			// ring type
	u4				extra;
	volatile u4		size;
	volatile u4		write_pos;
	volatile u4		read_pos;
	volatile u4		max_read_idx;
	volatile n1		data[0];
} ring_t, *ring_p, ring_a[1];

n4 ring_init(vpp ring_pptr, u4 unit_num, u4 unit_size)
{
	u8		size;
	ring_p	ring;

	/* size must be a power of 2 */
	if (IS_NL(ring_pptr) ||
		IS_ZR(unit_num) ||
		IS_ZR(unit_size) ||
		IS_NZR(unit_num & (unit_num - 1))) {
		RET_INT(-1, "@ring = %p, @unit_num = %u, @unit_size = %u",
				ring_pptr, unit_num, unit_size);
	}

	size = sizeof(ring_t) + (u8)(unit_size) * (u8)(unit_num);
	if (IS_NL(ALLOC(ring, size))) {
		RET_INT(-2, "ALLOC(size = %llu) = %p", size, ring);
	}

	SET_ZERO(ring, sizeof(ring_t));
	ring->type		= RING_MP_MC;
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

n4 ring_set_type(vp ring_ptr, u4 type)
{
	ring_p	ring;

	ring = ring_ptr;
	if (IS_NL(ring) || IS_NBE(type, RING_SP_SC, RING_MP_MC))	{
		RET_INT(-1, "@ring = %p, @type = %d", ring, type);
	}

	ring->type = type;

	RET_INT(0, nil_str);
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

//unit_num should be a power of 2
u4 ring_index(u4 count, u4 unit_num)
{
	if (IS_NZR(unit_num)) {
		//RET_INT((n4)(count & (unit_num - 1)), nil_str);
		return (u4)(count & (unit_num - 1));
	}

	RET_INT(0, nil_str);
}

n4 mp_ring_push(vp ring_ptr, vp elem)
{
	u4		ridx;
	u4		widx;
	ring_p	ring;

	ridx = 0;
	widx = 0;
	ring = ring_ptr;
	do {
		ridx = ring->read_pos;
		widx = ring->write_pos;
		//the ring is full
		if (IS_EQ(ring_index(widx + 1, ring->unit_num),
			ring_index(ridx, ring->unit_num))) {
			// RET_INT(-2, "push widx = %u, ridx = %u", widx, ridx);
			RET_INT(-2, nil_str);
		}
	} while(IS_NEQ(lisk_sync_cas(ring->write_pos, widx, widx + 1), widx));

	CPY(ring->data + ring_index(widx, ring->unit_num) * ring->unit_size,
		elem,
		ring->unit_size);

	while (IS_NEQ(lisk_sync_cas(ring->max_read_idx, widx, widx + 1), widx)) {
		sched_yield();
	}

	lisk_sync_add(ring->size, 1);

	RET_INT(0, nil_str);
}

n4 sp_mc_ring_push(vp ring_ptr, vp elem)
{
	u4		ridx;
	u4		widx;
	ring_p	ring;

	ring = ring_ptr;
	ridx = ring->read_pos;
	widx = ring->write_pos;
	//the ring is full
	if (IS_EQ(ring_index(widx + 1, ring->unit_num),
		ring_index(ridx, ring->unit_num))) {
		// RET_INT(-2, "push widx = %u, ridx = %u", widx, ridx);
		RET_INT(-2, nil_str);
	}

	CPY(ring->data + ring_index(widx, ring->unit_num) * ring->unit_size,
		elem,
		ring->unit_size);

	lisk_sync_add(ring->write_pos, 1);
	lisk_sync_add(ring->size, 1);

	RET_INT(0, nil_str);
}

n4 sp_sc_ring_push(vp ring_ptr, vp elem)
{
	u4		ridx;
	u4		widx;
	ring_p	ring;

	ring = ring_ptr;
	ridx = ring->read_pos;
	widx = ring->write_pos;
	//the ring is full
	if (IS_EQ(ring_index(widx + 1, ring->unit_num),
		ring_index(ridx, ring->unit_num))) {
		// RET_INT(-2, "push widx = %u, ridx = %u", widx, ridx);
		RET_INT(-2, nil_str);
	}

	CPY(ring->data + ring_index(widx, ring->unit_num) * ring->unit_size,
		elem,
		ring->unit_size);

	// lisk_sync_add(ring->write_pos, 1);
	// lisk_sync_add(ring->size, 1);
	ring->write_pos++;
	ring->size++;

	RET_INT(0, nil_str);
}

n4 ring_push(vp ring_ptr, vp elem)
{
	n4		ret;
	ring_p	ring;

	if (IS_NL(ring = ring_ptr) || IS_NL(elem)) {
		RET_INT(-1, nil_str);
	}

	switch (ring->type) {
	case RING_MP_MC:
	case RING_MP_SC: {
		ret = mp_ring_push(ring, elem);
		break;
	}

	case RING_SP_MC: {
		ret = sp_mc_ring_push(ring, elem);
		break;
	}

	case RING_SP_SC: {
		ret = sp_sc_ring_push(ring, elem);
		break;
	}

	default: {
		ret = -2;
		//LISK_LOG_ERROR("illegal ring type = %u", ring->type);
		break;
	}
	}

	RET_INT(ret, nil_str);
}

n4 sp_mc_ring_pop(vp ring_ptr, vp elem)
{
	u4		ridx;
	u4		widx;
	ring_p	ring;

	ridx = 0;
	widx = 0;
	ring = ring_ptr;
	do {
		ridx = ring->read_pos;
		widx = ring->write_pos;
		// the ring is empty
		if (IS_EQ(ring_index(ridx, ring->unit_num),
			ring_index(widx, ring->unit_num))) {
			// RET_INT(-2, "push widx = %u, ridx = %u", widx, ridx);
			RET_INT(-2, nil_str);
		}

		CPY(elem,
			ring->data + ring_index(ridx, ring->unit_num) * ring->unit_size,
			ring->unit_size);

		if (IS_EQ(lisk_sync_cas(ring->read_pos, ridx, ridx + 1), ridx)) {
			lisk_sync_sub(ring->size, 1);
			RET_INT(0, nil_str);
		}
	} while(1);

	RET_INT(-2, nil_str);
}

n4 mp_mc_ring_pop(vp ring_ptr, vp elem)
{
	u4		ridx;
	u4		max_ridx;
	ring_p	ring;

	ridx = 0;
	max_ridx = 0;
	ring = ring_ptr;
	do {
		ridx = ring->read_pos;
		max_ridx = ring->max_read_idx;
		// the ring is empty or a procuder has got space
		// from the ring and dumping data into it now
		if (IS_EQ(ring_index(ridx, ring->unit_num),
			ring_index(max_ridx, ring->unit_num))) {
			// RET_INT(-2, "push ridx = %u, max_ridx = %u", ridx, max_ridx);
			RET_INT(-2, nil_str);
		}

		CPY(elem,
			ring->data + ring_index(ridx, ring->unit_num) * ring->unit_size,
			ring->unit_size);

		if (IS_EQ(lisk_sync_cas(ring->read_pos, ridx, ridx + 1), ridx)) {
			lisk_sync_sub(ring->size, 1);
			RET_INT(0, nil_str);
		}
	} while(1);

	RET_INT(-2, nil_str);
}

n4 sp_sc_ring_pop(vp ring_ptr, vp elem)
{
	u4		ridx;
	u4		widx;
	ring_p	ring;

	ring = ring_ptr;
	ridx = ring->read_pos;
	widx = ring->write_pos;
	// the ring is empty
	if (IS_EQ(ring_index(ridx, ring->unit_num),
		ring_index(widx, ring->unit_num))) {
		// RET_INT(-2, "push widx = %u, ridx = %u", widx, ridx);
		RET_INT(-2, nil_str);
	}

	CPY(elem,
		ring->data + ring_index(ridx, ring->unit_num) * ring->unit_size,
		ring->unit_size);

	// lisk_sync_add(ring->read_pos, 1);
	// lisk_sync_sub(ring->size, 1);
	ring->read_pos++;
	ring->size++;

	RET_INT(0, nil_str);
}

n4 mp_sc_ring_pop(vp ring_ptr, vp elem)
{
	u4		ridx;
	u4		max_ridx;
	ring_p	ring;

	ring = ring_ptr;
	ridx = ring->read_pos;
	max_ridx = ring->max_read_idx;
	// the ring is empty
	if (IS_EQ(ring_index(ridx, ring->unit_num),
		ring_index(max_ridx, ring->unit_num))) {
		// RET_INT(-2, "push ridx = %u, max_ridx = %u", ridx, max_ridx);
		RET_INT(-2, nil_str);
	}

	CPY(elem,
		ring->data + ring_index(ridx, ring->unit_num) * ring->unit_size,
		ring->unit_size);

	lisk_sync_add(ring->read_pos, 1);
	lisk_sync_sub(ring->size, 1);

	RET_INT(0, nil_str);
}

n4 ring_pop(vp ring_ptr, vp elem)
{
	n4		ret;
	ring_p	ring;

	if (IS_NL(ring = ring_ptr) || IS_NL(elem)) {
		RET_INT(-1, nil_str);
	}

	switch (ring->type) {
	case RING_MP_MC: {
		ret = mp_mc_ring_pop(ring, elem);
		break;
	}

	case RING_SP_MC: {
		ret = sp_mc_ring_pop(ring, elem);
		break;
	}

	case RING_MP_SC: {
		ret = mp_sc_ring_pop(ring, elem);
		break;
	}

	case RING_SP_SC: {
		ret = sp_sc_ring_pop(ring, elem);
		break;
	}

	default: {
		ret = -2;
		//LISK_LOG_ERROR("illegal ring type = %u", ring->type);
		break;
	}
	}

	RET_INT(ret, nil_str);
}

