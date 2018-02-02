/**
 * FILE		:lisk_atom.h
 * DESC		:to supply atom
 * AUTHOR	:v0.3.0 written by Alex Stocks
 * DATE		:on Sep 17, 2012
 * MOD		:modified by * on June 2, 2012
 **/

#ifndef __LISK_ATOM_H__
#define __LISK_ATOM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <lisk_public.h>


/**
 * Attention	: the type of @var and @delta and @old must be the same,
 *				  and the return value is of the same type of @var
 **/
#define	lisk_sync_zero8(var)			lisk_sync_set8(var, 0)
#define	lisk_sync_add8(var, delta)		InterlockedExchangeAdd64(&(var), delta)
#define	lisk_sync_sub8(var, delta)		InterlockedExchangeSub64(&(var), delta)
#define	lisk_sync_set8(var, value)		InterlockedExchange64(&(var), value)
#define	lisk_sync_cas8(var, _old, _new)	InterlockedCompareExchange64(&(var), _old, _new)

#define	lisk_sync_zero(var)				lisk_sync_set(var, 0)
#define	lisk_sync_add(var, delta)		InterlockedExchangeAdd(&(var), delta)
#define	lisk_sync_sub(var, delta)		InterlockedExchangeSub(&(var), delta)
#define	lisk_sync_set(var, value)		InterlockedExchange(&(var), value)
#define	lisk_sync_cas(var, _old, _new)	InterlockedCompareExchange(&(var), _old, _new)

typedef struct lisk_atom_tag {
	volatile n4 num;
} lisk_atom_t, *lisk_atom_p, lisk_atom_a[1];

#define	lisk_atom_read(at)				((at).num)
#define	lisk_atom_zero(atom)			lisk_atom_set(atom, 0)
#define	lisk_atom_add(atom, delta)		InterlockedAdd(&(atom.num), delta)
#define	lisk_atom_sub(atom, delta)		InterlockedSub(&(atom.num), delta)
#define	lisk_atom_set(atom, value)		InterlockedExchange(&(atom.num), value)
#define	lisk_atom_cas(atom, _old, _new)	InterlockedCompareExchange(&(atom.num), _old, _new)

/**
 * description	: get the bit value of the bit array @ptr
 *				  at position @pos <br/><br/>
 *
 * @in-out@ptr	: bit array
 *
 * @out-ret		: if the value at that position is 1,
 *				  the return value is 1; otherwise 0 instread.
 **/
n4 get_bit(n1* ptr, n4 pos);
/**
 * description	: set the bit value of the bit array @ptr
 *				  at position @pos to be 1 <br/><br/>
 *
 * @in-out@ptr	: bit array
 *
 * @out-ret		: if all params are legal, the return value is 0;
 *				  otherwise -1 instead.
 **/
n4 set_bit1(n1* ptr, n4 pos);
/**
 * description	: set the bit value of the bit array @ptr at
 *				  position @pos to be 0 <br/><br/>
 *
 * @in-out@ptr	: bit array
 *
 * @out-ret		: if all params are legal, the return value is 0;
 *				  otherwise -1 instead.
 **/
n4 set_bit0(n1* ptr, n4 pos);

#ifdef __cplusplus
}
#endif

#endif

