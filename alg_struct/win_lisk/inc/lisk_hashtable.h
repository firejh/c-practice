/**
 * FILE		: lisk_hashtable.h
 * DESC		: standard data struct hashtable like dict of redis
 * AUTHOR	: v1.0 written by Alex Stocks
 * DATE		: on Nov 28, 2012
 * MOD		: modified by * on June 2, 2012
 **/

#ifndef __LISK_HASHTABLE_H__
#define __LISK_HASHTABLE_H__

#ifdef __cplusplus
	extern "C" {
#endif

#include <lisk_public.h>

/**
 * description		: define a hashtable and initilize it. Code to init a hashtable might be like the following.
 * 					  vp hashtable = nil;
 * 					  u8 init_ht_size = SIZE_M;
 * 					  compare ht_cmp = cmp_func;
 * 					  n4 ret = hashtable_init(&hashtable, init_ht_size, ht_cmp);
 * 					  if (IS_NZR(ret)) {
 * 					  	LOG_ERROR();
 * 					  }
 * in-out@ht		: hashtable object.
 * in-@size			: its init size
 * in-@cmp			: compare function, its a dual-params function
 *
 * out-@ret			: if successful, ret is 0; otherwise -1 instead.
 **/
n4 hashtable_init(vpp ht, u8 size, compare cmp);
/**
 * description	: destruct hashtable @ht <br/><br/>
 *
 * in-out@ht	: hashtable object
 *
 * out-@ret		: if @ht has been initilized, ret is 0; otherwise -1 instead.
 **/
n4 hashtable_free(vpp ht);
/**
 * description	: find a element whose key is @key in hashtable @ht <br/><br/>
 *
 * in-out@ht	: hashtable object
 * in-@key		: the key of target element
 * out-@value	: target element
 *
 * out-@ret		: if @ht has initilized, ret is 0; otherwise -1 when your params
 *				  are illegal or -2 when can't find the element with key @key in
 *				  hashtable @ht.
 **/
n4 hashtable_find(vp ht, key_p key, value_p* value);
/**
 * description	: insert a element with @key and @value into hashtable @ht.
 *				  if a element with the same key is still existing in the hashtable,
 *				  it will refuse to insert your element into hashtable.<br/><br/>
 *
 * in-out@ht	: hashtable object
 * in-@key		: the key of new element
 * in-@value	: the value of new element
 *
 * out-@ret		: if insert successful, ret is 0; otherwise -1 when the params are illegal
 * 				  or -2 when fail to insert new element or -3 when a element with the same
 * 				  key is still exsiting in the hashtable.
 **/
n4 hashtable_insert(vp ht, key_p key, value_p value);
/**
 * description	: delete a element whose @key from hashtable @ht <br/><br/>
 *
 * in-out@ht	: hashtable object
 * in-@key		: the key of erasing element
 *
 * out-@ret		: if delete successful, ret is 0; otherwise -1 when the params are illegal
 * 				  or -2 when fail to find that element.
 **/
n4 hashtable_erase(vp ht, key_p key);
/**
 * description	: update a element with new value @value in hashtable @ht <br/><br/>
 *
 * in-out@ht	: hashtable object
 * in-@key		: the key of target element
 * in-@value	: new value of target element
 *
 * out-@ret		: if update successful, ret is 0; otherwise -1 when the params are illegal
 * 				  or -2 when fail to find that element or -3 when fail to allocate new memory
 * 				  space to copy the data of @value.
 **/
n4 hashtable_update(vp ht, key_p key, value_p value);
/**
 * description	: get element number of hashtable @ht<br/><br/>
 *
 * in-out@map	: hashtable object
 * in-out@size	: element number
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 hashtable_size(vp ht, u4* size);
n4 hashtable_memory_size(vp ht, u4* size);
#ifdef __cplusplus
	}
#endif

#endif
