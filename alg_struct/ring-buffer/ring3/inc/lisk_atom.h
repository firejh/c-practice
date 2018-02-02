/**
 * FILE		:lisk_atom.h
 * DESC		:to supply atom
 * AUTHOR	:v0.3.0 written by Alex Stocks
 * DATE		:on Sep 17, 2012
 * MOD		:modified by * on June 2, 2012
 **/

#ifndef __LISK_ATOM_H__
#define __LISK_ATOM_H__

#include "lisk_public.h"

/**
 * Attention	: the type of @var and @delta and @old must be the same, and the return value
 *				  is of the same type of @var
 **/
#define	lisk_sync_zero(var)					__sync_lock_release(&(var))
#define	lisk_sync_add(var, delta)			__sync_fetch_and_add(&(var), delta)
#define	lisk_sync_sub(var, delta)			__sync_fetch_and_sub(&(var), delta)
#define	lisk_sync_set(var, value)			__sync_lock_test_and_set(&(var), value)
#define	lisk_sync_cas(var, old_v, new_v)	__sync_val_compare_and_swap(&(var), old_v, new_v)

typedef struct list_atom_tag {
	volatile n4 num;
} lisk_atom_t, *lisk_atom_p, lisk_atom_a[1];

#define	lisk_atom_read(at)					((at).num)
#define	lisk_atom_zero(atom)				lisk_sync_zero(atom.num)
#define	lisk_atom_add(atom, delta)			lisk_sync_add(atom.num, delta)
#define	lisk_atom_sub(atom, delta)			lisk_sync_sub(atom.num, delta)
#define	lisk_atom_set(atom, value)			lisk_sync_set(atom.num, value)
#define	lisk_atom_cas(atom, old_v, new_v)	lisk_sync_cas(atom.num, old_v, new_v)

/**
 * description	: get the bit value of the bit array @ptr at position @pos <br/><br/>
 *
 * @in-out@ptr	: bit array
 *
 * @out-ret		: if the value at that position is 1, the return value is 1; otherwise 0 instread.
 **/
n4 get_bit(n1* ptr, n4 pos);
/**
 * description	: set the bit value of the bit array @ptr at position @pos to be 1 <br/><br/>
 *
 * @in-out@ptr	: bit array
 *
 * @out-ret		: if all params are legal, the return value is 0; otherwise -1 instead.
 **/
n4 set_bit1(n1* ptr, n4 pos);
/**
 * description	: set the bit value of the bit array @ptr at position @pos to be 0 <br/><br/>
 *
 * @in-out@ptr	: bit array
 *
 * @out-ret		: if all params are legal, the return value is 0; otherwise -1 instead.
 **/
n4 set_bit0(n1* ptr, n4 pos);

#endif

