/**
 * FILE		: lisk_deque.h
 * DESC		: double-ended queue
 * AUTHOR	: v1.0 written by Alex Stocks
 * DATE		: on Apr 21, 2014
 * LICENCE	: GPL 2.0
 * MOD		:
 **/

#ifndef __LISK_DEQUE_H__
#define __LISK_DEQUE_H__

#if  (defined(__cplusplus))
	extern "C" {
#endif

#include <lisk_type.h>

#define	DEF_BLOCK_SIZE		SIZE_K(1)
#define	INITIAL_MAP_SIZE	8


/**
 *	deque init&uninit macro. please attention that the type of @deque is void*.
 **/
#define	deque_init(deque, unit_size)	\
			_deque_init(&deque, unit_size, INITIAL_MAP_SIZE, DEF_BLOCK_SIZE)
#define	deque_uninit(deque)	_deque_uninit(&deque)

/**
 * description		: initialise a double-ended queue object @deque <br/><br/>
 *
 * in-out@deque		: deque object pointer
 * in-@unit_size	: unit size.
 * in-@map_size		: block index array(map) initial size. it must be a power of 2.
 * in-@block_size	: block size
 *
 * out-@ret			: if successful, the return value is 0; otherwise -1 instead
 *					  if some parameters are illegal or -2 if failed to initialize it.
 **/
n4 _deque_init(vpp deque, u4 unit_size, u4 map_size, u4 block_size);
/**
 * description		: destroy a double-ended queue object @deque <br/><br/>
 *
 * in-out@deque		: deque object pointer
 *
 * out-@ret			: the return value is void.
 **/
vd _deque_uninit(vpp deque);
/**
 * description		: check whether @deque is empty. <br/><br/>
 *
 * in-out@deque		: deque object
 *
 * out-@ret			: if @deque is empty, the return value is 0; otherwise -1 instead
 *					  if some parameters are illegal.
 **/
n4 deque_empty(vp deque);
/**
 * description		: get element number in @deque. <br/><br/>
 *
 * in-out@deque		: deque object
 * in-out@size		: element number
 *
 * out-@ret			: if successful, the return value is 0; otherwise -1 instead
 *					  if some parameters are illegal.
 **/
n4 deque_size(vp deque, u4* size);
/**
 * description		: get the element at position @pos in @deque. <br/><br/>
 *
 * in-@deque		: deque object
 * in-@pos			: element position
 *
 * out-@ret			: if @deque is not nil, the return value is the element
 *					  that you wanna. otherwise nil instead.
 **/
vp deque_at(vp deque, u4 pos);
/**
 * description		: get the first element in @deque. <br/><br/>
 *
 * in-@deque		: deque object
 *
 * out-@ret			: if @deque is not nil, the return value is the first element.
 *					  otherwise nil instead.
 **/
vp deque_front(vp deque);
/**
 * description		: get the last element in @deque. <br/><br/>
 *
 * in-@deque		: deque object
 *
 * out-@ret			: if @deque is not nil, the return value is the last element.
 *					  otherwise nil instead.
 **/
vp deque_back(vp deque);
/**
 * description		: insert element @front into the beginning of @deque. <br/><br/>
 *
 * in-@deque		: deque object
 * in-@front		: element data
 *
 * out-@ret			: if inserts successful, the ret 0; otherwise -1 instead.
 **/
n4 deque_push_front(vp deque, vp front);
/**
 * description		: append element @front to the end of @deque. <br/><br/>
 *
 * in-@deque		: deque object
 * in-@front		: element data
 *
 * out-@ret			: if appends successful, the ret 0; otherwise -1 instead.
 **/
n4 deque_push_back(vp deque, vp back);
/**
 * description		: pop up the first element of @deque. <br/><br/>
 *
 * in-@deque		: deque object
 * in-out@front		: element data
 *
 * out-@ret			: if @deque is not nil and there are one or more elements in it,
 *					  the return value is 0 and @front stores the first element's
 *					  data. otherwise the return value is -1 if @deque or @front is nil.
 **/
n4 deque_pop_front(vp deque, vp front);
/**
 * description		: pop up the last element of @deque. <br/><br/>
 *
 * in-@deque		: deque object
 * in-out@back		: element data
 *
 * out-@ret			: if @deque is not nil and there are one or more elems in it,
 *					  the return value is 0 and @back is the last element. otherwise
 *					  the return value is -1 if @deque or @back is nil.
 **/
n4 deque_pop_back(vp deque, vp back);

#if  (defined(__cplusplus))
	}
#endif

#endif

