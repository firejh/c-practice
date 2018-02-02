/**
 * FILE		: lisk_skiplist.h
 * DESC		: skip list
 * AUTHOR	: v0.3 written by Alex Stocks
 * DATE		: on Oct 15, 2014
 * LICENCE	: GPL 2.0
 * MOD		:
 **/

#ifndef __LISK_SKIPLIST_H__
#define __LISK_SKIPLIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <lisk_public.h>

/**
 * description		: define a skiplist and initilize it. <br/><br/>
 *
 * in-out@skip_list	: skip list
 * in-@cmp			: compare function, its a dual-params function
 *
 * out-@ret			: if successful, ret is 0; otherwise -1 instead.
 **/
n4 skip_list_init(vpp skip_list, compare cmp);
/**
 * description		: destruct skip list @skip_list <br/><br/>
 *
 * in-out@skip_list	: skip list
 *
 * out-@ret			: the return value is void.
 **/
vd skip_list_uninit(vpp skip_list);
/**
 * description		: find a element whose key is @key in @skip_list <br/><br/>
 *
 * in-out@skip_list	: skip list
 * in-@key			: the key of target element
 * out-@value		: target element
 *
 * out-@ret			: if @skip_list has initilized, ret is 0; otherwise -1
 *					  instead if some params are illegal or -2 if can not find
 *					  the target element.
 **/
n4 skip_list_find(vp skip_list, key_p key, value_p* value);
/**
 * description		: insert a element with @key and @value into @skip_list <br/><br/>
 *
 * in-out@skip_list	: skip list
 * in-@key			: the key of new element
 * in-@value		: @key's value
 *
 * out-@ret			: if insert successful, ret is 0; otherwise -1 when some params
 *					  are illegal or -2 when lisk fails to insert @key->@value or -3
 *					  when lisk find the element with @key has existed.
 **/
n4 skip_list_insert(vp skip_list, key_p key, value_p value);
/**
 * description		: update a element with new value @value <br/><br/>
 *
 * in-out@skip_list	: skip list
 * in-@key			: the key of target element
 * in-@value	: new value of target element
 *
 * out-@ret			: if update successful, ret is 0; if @skip_list do not have
 *					  the element with @key, lisk insert a element{key->value}
 *					  into the @skip_list and ret is 1; otherwise -1 when some params
 *					  are illegal or -2 when lisk fails to insert @key->@value.
 **/
n4 skip_list_update(vp skip_list, key_p key, value_p value);
/**
 * description		: delete a element whose key is @key from @skip_list <br/><br/>
 *
 * in-out@skip_list	: skip list
 * in-@key			: the key of erasing element
 *
 * out-@ret			: if delete successful, ret is 0; otherwise -1 when some params are
 *					  illegal or -2 when fail to find that element.
 **/
vd skip_list_erase(vp skip_list, key_p key);

#ifdef __cplusplus
}
#endif

#endif

