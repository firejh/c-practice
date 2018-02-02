/** 
 * FILE		: lisk_map.h
 * DESC		: map interface
 * AUTHOR	: v1.0 written by Alex Stocks
 * DATE		: on Dec 15, 2012
 * LICENCE	: GPL 2.0
 * MOD		: modified by * on June 2, 2012
 **/

#ifndef __LISK_MAP_H__
#define __LISK_MAP_H__


#ifdef __cplusplus
	extern "C" {
#endif

#include <lisk_public.h>

typedef struct iterator_tag {
	key_p	first;
	value_p	second;
} iterator_t, *iterator_p, iterator_a[1];

#define MAP_FOR_EACH_BEGIN(map, it) \
do {    \
	iterator_t  it;     \
	n4          __map_ret__;    \
	if (IS_NL(map)) {   \
	break;  \
	}   \
	__map_ret__ = map_begin(map, &it);  \
	for (; IS_ZR(__map_ret__) && IS_NZR(map_is_end(map, &it)); __map_ret__ = map_next(map, &it))    {

#define MAP_FOR_EACH_END    \
	}   \
} while (0);

#define MAP_FOR_EACH_RBEGIN(map, it)    \
do {    \
	iterator_t  it;     \
	n4          __map_ret__;    \
	if (IS_NL(map)) {   \
	break;  \
	}   \
	__map_ret__ = map_rbegin(map, &it); \
	for (; IS_ZR(__map_ret__) && IS_NZR(map_is_rend(map, &it)); __map_ret__ = map_rnext(map, &it))  {

#define MAP_FOR_EACH_REND   \
	}   \
} while (0);

/**
 * description		: define a map and initilize it. Code to init a map might be like the following.
 * 					  vp map = nil;
 * 					  compare cmp = cmp_func;
 * 					  n4 ret = map_init(&map, cmp);
 * 					  if (IS_NZR(ret)) {
 * 					  	LOG_ERROR();
 * 					  }
 * in-out@map		: map object.
 * in-@cmp			: compare function, its a dual-params function
 *
 * out-@ret			: if successful, ret is 0; otherwise -1 instead.
 **/
n4 map_init(vpp map, compare cmp);
/**
 * description	: destruct map @ht <br/><br/>
 *
 * in-out@map	: map object
 *
 * out-@ret		: if @map has been initilized, ret is 0; otherwise -1 instead.
 **/
vd map_uninit(vpp map);
/**
 * description	: find a element whose key is @key in map @map <br/><br/>
 *
 * in-out@map	: map object
 * in-@key		: the key of target element
 * out-@value	: target element
 *
 * out-@ret		: if @map has initilized, ret is 0; otherwise -1 instead.
 **/
n4 map_find(vp map, key_p key, value_p* value);
/**
 * description	: insert a element with @key and @value into map @map <br/><br/>
 *
 * in-out@map	: map object
 * in-@key		: the key of a new element
 * in-@value	: the value of a new element
 *
 * out-@ret		: if insert successful, ret is 0; otherwise -1 when the params are illegal
 * 				  or -2 when fail to insert new element.
 **/
n4 map_insert(vp map, key_p key, value_p value);
/**
 * description	: update a element with new value @value in map @map <br/><br/>
 *
 * in-out@map	: map object
 * in-@key		: the key of target element
 * in-@value	: new value of target element
 *
 * out-@ret		: if update successful, ret is 0; otherwise -1 when the params are illegal
 * 				  or -2 when fail to find that element.
 **/
n4 map_update(vp map, key_p key, value_p value);
/**
 * description	: delete a element whose key is @key from map @map <br/><br/>
 *
 * in-out@map	: map object
 * in-@key		: the key of erasing element
 *
 * out-@ret		: if delete successful, ret is 0; otherwise -1 when the params are illegal
 * 				  or -2 when fail to find that element.
 **/
n4 map_erase(vp map, key_p key);
/**
 * description  : get element number of map<br/><br/>
 *
 * in-out@map   : map object
 * in-out@size  : element number
 *
 * out-@ret     : if successful, ret is 0; otherwise -1 instead.
 **/
n4 map_size(vp map, u8* size);
/**
 * description  : get an iterator that points to the first element of the map<br/><br/>
 *
 * in-out@map	: map object
 * in-out@it	: the iterator
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 map_begin(vp map, iterator_p it);
/**
 * description  : get an iterator that just passed the end of the map<br/><br/>
 *
 * in-out@map	: map object
 * in-out@it	: the iterator
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 map_end(vp map, iterator_p it);
/**
 * description  : get an iterator that points to the next element <br/><br/>
 *
 * in-out@map	: map object
 * in-out@it	: the iterator
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 map_next(vp map, iterator_p it);
/**
 * description  : check whether an iterator @it that points to the end of the map  <br/><br/>
 *
 * in-out@map	: map object
 * in-out@it	: the iterator
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 map_is_end(vp map, iterator_p it);
/**
 * description  : get an reverse-iterator that points to the last element of the map<br/><br/>
 *
 * in-out@map	: map object
 * in-out@it	: the iterator
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 map_rbegin(vp map, iterator_p it);
/**
 * description  : get an reverse-iterator that points to the beginning of the map<br/><br/>
 *
 * in-out@map	: map object
 * in-out@it	: the iterator
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 map_rend(vp map, iterator_p it);
/**
 * description  : check whether an reverse-iterator @it that points to the beginning of the map  <br/><br/>
 *
 * in-out@map	: map object
 * in-out@it	: the iterator
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 map_is_rend(vp map, iterator_p it);
/**
 * description  : get an reverse-iterator that points to the preceding  element <br/><br/>
 *
 * in-out@map	: map object
 * in-out@it	: the iterator
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 map_rnext(vp map, iterator_p it);

#ifdef __cplusplus
	}
#endif

#endif

