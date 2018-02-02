/**
 * FILE		:lisk_mul_hash.h
 * DESC		:multy hash inf
 * AUTHOR	:v1.0 written by Alex Stocks
 * DATE		:on Sep 5, 2012
 * MOD		:modified by * on June 2, 2012
 * **/

#ifndef __LISK_MULTI_HASH_H__
#define __LISK_MULTI_HASH_H__

#ifdef __cplusplus
	extern "C" {
#endif

//optimum value range of row: [16, 32]
#define	MAX_HASH_ROW	1000

#include <lisk_public.h>

/**
 * description			: define a multi-hash table and initialize it. Code to init a multi-hash table might be like the following.
 *						  n4 cmp(key_p key0, key_p key1)	{
 *						  	n4 ret = 0;
 *						  	if (IS_NL(key0) || IS_NL(key1) || IS_NEQ(key0->size, key1->size) ||
 *						  		IS_NL(key0->data) || IS_NL(value0->data))	{
 *						  		return -2;
 *						  	}
 *
 *						  	ret = memcmp(key0->data, key1->data);
 *						  	return ret;
 *						  }
 *
 * 						  vp mul_hash = nil;
 * 						  u8 row_num = 33;
 * 						  u8 col_num = 1000000;
 * 						  u2 max_key_size = 4;
 * 						  u4 max_value_size = 32;
 * 						  n4 ret = mul_hash_init(&mul_hash, row_num, col_num, max_key_size, max_value_size, nil, cmp);
 * 						  if (IS_NZR(ret)) {
 * 						  	LOG_ERROR("mul_hash_init() = %d", ret);
 * 						  }
 * in-out@mul_hash		: multi-hash table object.
 * in-@row				: row number
 * in-@col				: column number
 * in-@max_key_size		: maximum length of key
 * in-@max_value_size	: maximum length of value
 * in-@file_key			: file key for multi-hash table. its used for multiple processes.
 * in-@cmp				: compare function, its a dual-params function
 *
 * out-@ret				: if successful, ret is 0; otherwise -1 instead.
 **/
n4 mul_hash_init(vpp mul_hash,
				  n8 row, n8 col,
				  u2 max_key_size, u4 max_value_size,
				  n1* file_key,
				  compare cmp);
/**
 * description			: destruct multi-hash table @ht <br/><br/>
 *
 * in-out@mul_hash		: multi-hash table object
 *
 * out-@ret				: if @mul_hash has been initilized, ret is 0; otherwise -1 instead.
 **/
n4 mul_hash_free(vpp mul_hash);
/**
 * description			: find a element whose key is @key in @mul_hash <br/><br/>
 *
 * in-out@mul_hash		: multi-hash table object
 * in-@key				: the key of target element
 * out-@value			: target element
 *
 * out-@ret				: if @mul_hash has initilized, ret is 0; otherwise -1 instead.
 **/
n4 mul_hash_find(vp mul_hash, key_p key, value_p value);
/**
 * description			: insert a element with @key-@value into multi-hash table @mul_hash<br/><br/>
 *
 * in-out@mul_hash		: multi-hash table object
 * in-@key				: the key of a new element
 * in-@value			: the value of a new element
 *
 * out-@ret				: if insert successful, ret is 0; otherwise -1 when the params are illegal
 *						  or -2 when fail to insert new element.
 **/
n4 mul_hash_insert(vp mul_hash, key_p key, value_p value);
/**
 * description			: delete a element whose key is @key from @mul_hash <br/><br/>
 *
 * in-out@mul_hash		: multi-hash table object
 * in-@key				: the key of that erasing element
 *
 * out-@ret				: if delete successful, ret is 0; otherwise -1 when the params are illegal
 *						  or -2 when fail to find that element.
 **/
n4 mul_hash_erase(vp mul_hash, key_p key);

#ifdef __cplusplus
	}
#endif

#endif

