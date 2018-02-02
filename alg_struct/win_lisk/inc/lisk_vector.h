/**
 * FILE		: lisk_vector.h
 * DESC		: vector in c style
 * AUTHOR	: v0.3.0 written by Alex Stocks
 * DATE		: on Oct 1, 2012
 * MOD		: modified by * on June 2, 2012
 **/

#ifndef __LISK_VECTOR_H__
#define __LISK_VECTOR_H__

#if	(defined(__cplusplus))
extern	"C"	{
#endif

#include <lisk_public.h>

typedef struct lisk_vector_tag {
	u4		capacity;
	u4		number;
	u4		length;
	un1		reserve[sizeof(u4)];
	n1*		data;
} lisk_vector_t, *lisk_vector_p, lisk_vector_a[1];

/**
 * description	: allocate a memory space with wize @number * @length bytes
 *				  and fill it with zero. <br/><br/>
 *
 * in-out@vect	: vector object
 * in-@number	: init unit num
 * in-@length	: unit length
 *
 * out-@ret		: if successful, ret is 0; otherwise ret is -1 if those params are
 *				  illegal or -2 if failed to alloc space.
 **/
n4 lisk_vector_init(vp vect, u4 number, u4 length);
/**
 * description	: allocate a momory space with size @number * @length bytes and
 *				  copy the data of @array into it. <br/><br/>
 *
 * in-out@vect	: vector object
 * in-@array	: copied data array
 * in-@number	: init max unit num
 * in-@length	: per unit len
 *
 * out-@ret		: if successful, ret is 0; otherwise the return value is -1 if
 *				  those params are illegal or -2 if the lisk fails to allocate
 *				  momery space.
 **/
n4 lisk_vector_init_copy(vp vect, vp array, u4 number, u4 length);
/**
 * description	: free the memory block of @vect. if it is not allocated
 *				  by lisk_vector_init or lisk_vector_init_copy or lisk_vector_realloc
 *				  or they have been released by @lisk_vector_uninit before,
 *				  undefined behavior may occur. <br/><br/>
 *
 * in-@vect		: vector
 *
 * out-@ret		: the ret is void.
 **/
vd lisk_vector_uninit(vp vect);
/**
 * description	: clear the memory space of @vect and fill it with zero. <br/><br/>
 *
 * in-@vect		: vector
 *
 * out-@ret		: the ret is void.
 **/
vd lisk_vector_clear(vp vect);
/**
 * description	: copy @num * @length bytes from memory area of @array into @vect.
 *				  all contents of @array will be copied. if the memory block of @vect is
 *				  not full enough, lisk_vector_assign2 will realloc its memory block. <br/><br/>
 *
 * in-@vect		: vector
 * in-@array		: data array. its unit length should be equal to *@vect->length
 * in-@num		: element number of data buffer
 *
 * out-@ret		: if copy successful, the ret is 0. if params are illegal, ret is -1.
 *				  if realloc memory block fails, ret is -2.
 **/
n4 lisk_vector_assign(vp vect, vp array, u4 num);
/**
 * description	: insert @array's data arrfer into @vect at position @pos.
 *				  you should make sure that the unit length of @array
 *				  should be the equal to @vect->length. lisk will reallocate
 *				  memory for @vect if its length is not long enough. <br/><br/>
 *
 * in-@vect		: vector
 * in-@pos		: insert position
 * in-@array	: source data array. if the @array is nil, the lisk_vector @vect
 *				  will just add its size @vect->number by @num.
 * in-@num		: unit number of @array
 *
 * out-@ret		: if inerts successful, the return value is 0 otherwise -1 if params
 *				  are illegal or realloc or -2 if lisk fails to allocate memory space.
 **/
n4 lisk_vector_insert(vp vect, u4 pos, vp array, u4 num);
/**
 * description	: append @array to the end of @vect. lisk will reallocate memory
 *				  for @vect if its memory space is not long enough. <br/><br/>
 *
 * in-@vect		: vector
 * in-@array	: source data buffer. it should not be nil.
 * in-@num		: unit number of @array
 *
 * out-@ret		: if appends successful, the ret 0; otherwise -1 instead.
 **/
n4 lisk_vector_push_back(vp vect, vp array, u4 num);
/**
 * description	: erase @num elements of the @vect starting at position. <br/><br/>
 *
 * in-@vect		: vector
 * in-@pos		: start postition
 * in-@num		: elements number
 *
 * out-@ret		: if vect is not nil and there are one or more elems in it,
 *				  the ret is the new memory block of @vect at position @pos.
 *				  otherwise the ret is nil.
 **/
vp lisk_vector_erase(vp vect, u4 pos, u4 num);
/**
 * description	: pop up the first elem of vector @vect. <br/><br/>
 *
 * in-@vect		: vector
 *
 * out-@ret		: if @vect is not nil and there are one or more elems in it,
 *				  the return value is the new first memory block of @vect.
 *				  otherwise the return value is nil.
 **/
vp lisk_vector_pop_up(vp vect);
/**
 * description	: pop up the last elem of vector @vect. <br/><br/>
 *
 * in-@vect		: vector
 *
 * out-@ret		: if @vect is not nil and there are one or more elems in it,
 *				  the return is the last memory block of @vect. otherwise
 *				  the return value is nil.
 **/
vp lisk_vector_pop_back(vp vect);
/**
 * description	: reverse the elements of vector @vect. <br/><br/>
 *
 * in-@vect		: vector
 *
 * out-@ret		: if vect is not nil and there are one or more elems in it,
 *				  ret is 0. otherwise -1 instead.
 **/
n4 lisk_vector_reverse(vp vect);
/**
 * description	: retrun if the vector @vect has element or not. <br/><br/>
 *
 * in-@vect		: vector
 *
 * out-@ret		: if it stores one or more elements, the return is 0.
 *				  if vect is not nil or it has no elements, the ret is -1.
 **/
n4 lisk_vector_empty(vp vect);
/**
 * description	: get the elem at position @pos in @vect. <br/><br/>
 *
 * in-@vect		: vector
 * in-@pos		: element position
 *
 * out-@ret		: if vect is not nil and the @pos is not more than @vec->number,
 *				  the ret is element that you wanna. otherwise nil instead.
 **/
vp lisk_vector_at(vp vect, u4 pos);
/**
 * description	: find an elem of vector @vect which is equal to @value <br/><br/>
 *
 * in-@vect		: vector
 * in-@value	: reference value
 * in-@cmp		: compare functioin
 *
 * out-@ret		: if vect is not nil and the @value is not nil and find that value,
 *				  the ret is element that you wanna. otherwise nil instead.
 **/
vp lisk_vector_find(vp vect, vp value, compare cmp);
/**
 * description	: get the first elem of @vect. <br/><br/>
 *
 * in-@vect		: vector
 *
 * out-@ret		: if vect is not nil and @vec->number is not less than 0,
 *				  the ret is the element that you wanna. otherwise nil instead.
 **/
vp lisk_vector_first(vp vect);
/**
 * description	: get the last elem of @vect. <br/><br/>
 *
 * in-@vect		: vector
 *
 * out-@ret		: if vect is not nil and @vec->number is not less than 0,
 *				  the ret is element that you wanna. otherwise nil instead.
 **/
vp lisk_vector_last(vp vect);
/**
 * description	: get the next elem after current element @cur in vector @vect.
 *				  <br/><br/>
 *
 * in-@vect		: vector
 * in-@cur		: current element.
 *
 * out-@ret		  if @cur is the last element of @vect or in the right position
 *				  or @vect has no element, the return value is nil.
 **/
vp lisk_vector_next(vp vector, vp cur);
/**
 * description	: get the next elem before current element @cur in vector @vect.
 *				  <br/><br/>
 *
 * in-@vect		: vector
 * in-@cur		: current element.
 *
 * out-@ret		  if @cur is the first element of @vect or in the right position
 *				  or @vect has no element, the return value is nil.
 **/
vp lisk_vector_pre(vp vector, vp cur);

#if	(defined(__cplusplus))
}
#endif

#endif

