/**
 * FILE		:lisk_atom.h
 * DESC		:to supply atom
 * AUTHOR	:v0.3.0 written by Alex Stocks
 * DATE		:on Sep 17, 2012
 * MOD		:
 **/


#ifndef __LISK_ATOM_H__
#define __LISK_ATOM_H__

#ifdef __cplusplus
extern "C" {
#endif

#define	n1							char
#define	n2							short int
#define	n4							int
#define	n8							long long int
#define	n16							__int128_t
#if defined(__x86_64__)
#define	ndw							n16					// a gcc number type
#else
#define	ndw							n8
#endif
#define	nw							long int
#define	u1							unsigned char
#define	u2							unsigned short int
#define	u4							unsigned int
#define	u8							unsigned long long int
#define	u16							__uint128_t
#if defined(__x86_64__)
#define	udw							u16					// a gcc number type
#else
#define	udw							u8
#endif
#define	uw							unsigned long int
#define	f4							float
#define	f8							double
#define	fw							long double
#define	vd							void
#define	vp							void*

/**
 * Attention	: the type of @var and @delta and @old must be the same,
 *				  and the return value is of the same type of @var
 **/
#define	lisk_sync_zero(var)				__sync_lock_release(&(var))
#define	lisk_sync_add(var, delta)		__sync_fetch_and_add(&(var), delta)
#define	lisk_sync_sub(var, delta)		__sync_fetch_and_sub(&(var), delta)
#define	lisk_sync_set(var, value)		__sync_lock_test_and_set(&(var), value)
#define	lisk_sync_cas(var, _old, _new)	__sync_val_compare_and_swap(&(var), _old, _new)

typedef struct lisk_atom_tag {
	volatile n4 num;
} lisk_atom_t, *lisk_atom_p, lisk_atom_a[1];

#define	lisk_atom_read(at)				((at).num)
#define	lisk_atom_zero(atom)			lisk_sync_zero(atom.num)
#define	lisk_atom_add(atom, delta)		lisk_sync_add(atom.num, delta)
#define	lisk_atom_sub(atom, delta)		lisk_sync_sub(atom.num, delta)
#define	lisk_atom_set(atom, value)		lisk_sync_set(atom.num, value)
#define	lisk_atom_cas(atom, _old, _new)	lisk_sync_cas(atom.num, _old, _new)

typedef enum ELOCK_STATE_TAG {
	ELOCK_FREE		= 0X01 << 0,
	ELOCK_BUSY		= 0X01 << 1,
} ELOCK_STATE;

typedef struct lisk_lock_tag	{
	volatile u1		flag;
} lisk_lock_t, *lisk_lock_p, lisk_lock_a[1];

#define	LISK_LOCK_INIT(lock)			lisk_sync_set((lock).flag, ELOCK_FREE)

n4 lisk_lock(lisk_lock_p lock);
n4 lisk_unlock(lisk_lock_p lock);

#ifdef __cplusplus
	}
#endif

#endif

