//filename		: lisk_public.h
//func			: global macros, global variables, global functions
//version		: v1.0
//author		: Alex Stocks
//date			: 2012-07
//verify		:
//license		: GPL 2.0
//declaration	: Long live C!


#ifndef __LISK_PUBLIC_H__
#define __LISK_PUBLIC_H__


#ifdef __cplusplus
	extern "C" {
#endif

#include <lisk_type.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>		//memcmp, memmove
#include <stdio.h>		//snprintf

#include <windows.h>

#define	nil							NULL
#define	nil_str						""
#define	_STR(x)						#x
#define	STR(x)						_STR(x)
#define	STR_SELF(x)					_STR(x)
#define	STR_VALUE(x)				STR(x)
#define	_CAT(x, y)					x##y
#define	CAT(x, y)					_CAT(x, y)
#define	CAT_SELF(x, y)				_CAT(x, y)
#define	CAT_VALUE(x, y)				CAT(x, y)

#define	LESS						(1e-6f)
#define	ZERO						LESS
#define	ABS_ZERO					0
#define	LEAST						(1e-9f)
#define	MINUS_LESS					(-1e-6f)
#define	IS_ZR(_value)				((_value) == ABS_ZERO)
#define	IS_NZR(_value)				((_value) != ABS_ZERO)
#define	IS_FZ(_value)				((MINUS_LESS + 1.0f - 1.0f < ((float)(_value))) &&	\
										(((float)(_value))  < LESS + 1.0f - 1.0f) )
#define	IS_NFZ(_value)				( ((float)(_value)) < (MINUS_LESS + 1.0f - 1.0f) ||	\
										(LESS + 1.0f - 1.0f) < ((float)(_value)) )

#define	IS_MN(_value)				((_value) < 0)
#define IS_NMN(_value)              (0 <= (_value))
#define	IS_PN(_value)				(0 < (_value))
#define IS_NPN(_value)              ((_value) <= 0)
#define	IS_GT(_value, min)			((min) < (_value))
#define	IS_GE(_value, min)			((min) <= (_value))
#define	IS_EQ(_value, _eq)			((_value) == (_eq))
#define	IS_NEQ(_value, _eq)			((_value) != (_eq))
#define	IS_NL(ptr)					((ptr) == NULL)
#define	IS_NNL(ptr)					((ptr) != NULL)
#define	IS_LE(_value, max)			((_value) <= (max))
#define	IS_LT(_value, max)			((_value) < (max))
#define	IS_BT(_value, min, max)		(IS_GT(_value, min) && IS_LT(_value, max))
#define	IS_BE(_value, min, max)		(IS_GE(_value, min) && IS_LE(_value, max))
#define	IS_NBT(_value, min, max)	(IS_LE(_value, min) || IS_GE(_value, max))
#define	IS_NBE(_value, min, max)	(IS_LT(_value, min) || IS_GT(_value, max))

#define PTR_OFFSET(ptr0, ptr1)		(IS_LT((ptr0), (ptr1)) ? ((ptr1) - (ptr0)) : ((ptr0) - (ptr1)))

#define	SIZE_K(x)					((x) << 10)
#define	SIZE_M(x)					((x) << 20)
#define	SIZE_G(x)					((x) << 30)
#define	MAX_BUF_LEN					SIZE_K(1)
#define	MAX_PATH_LEN				MAX_BUF_LEN
#define	SET_ZERO(arr, len)			memset(arr, 0, len)

//#define	ALIGN_SIZE(size)		(UNIT_SIZE * ((size + UNIT_SIZE - 1) / UNIT_SIZE + 1))
#define	ALIGN_SIZE(size, unit_size)	(((size) + (unit_size) - 1) & ~((unit_size) - 1))
#define	ARRAY_SIZE(array)			(sizeof(array) / sizeof(array[0]))
#define	ARR_OFFSET(e, a)			((size_t)(OFFSET(e, a) / sizeof(a[0])))
#define	ARR_ELEM(arr, pos, type)	(*(ARR_ELEM_PTR(arr, pos, type)))
#define	ARR_ELEM_PTR(a, p, t)		((t*)(a) + p)							//@a array, @p position, @t type
#define	NEXT_ELEM(ptr, off, type)	(*(NEXT_ELEM_PTR(ptr, off, type)))
#define	NEXT_ELEM_PTR(p, o, t)		((t*)(p) + (o))							//@p ptr, @o offset, @t type
#define	ELEM_PTR(type, arr, len)	(type*)((un1*)arr + len)
#define	ELEM_PRE(type, arr, len)	(type*)((un1*)arr - len)

#define ALLOC(type, ptr, size)		ptr = (type)malloc(size)
#define REALLOC(type, ptr, size)	ptr = (type)realloc((vp)(ptr), size)
#define DEALLOC(ptr)				free(ptr);	ptr = nil;
#define	WIN_POOL_ALLOC				ALLOC
#define	WIN_POOL_DEALLOC			DEALLOC

//ternary conditional
#define	TERN_CMP(c, x, y, v1, v2)	({ c(x, y) ? (v1) : (v2); })

#define CMP(dst, src, len)          memcmp((vp)(dst), (vp)(src), (size_t)(len))
#define	CPY(dst, src, len)			copy_data(dst, src, len)

#define	snprintf					_snprintf



//////////////////////////////////////////
//public fuction typedef
//////////////////////////////////////////

typedef	n4 (*compare)(vp v0, vp v1);
typedef u4 (*hash_func)(n1*, n4);
typedef	n8	(*hash_function)(n1*, n8);
n4 copy_data(vp dst, vp src, u4 len);

uw PTR_2_NUM(vp ptr);
vp NUM_2_PTR(uw num);

#ifdef __cplusplus
	}
#endif

#endif

