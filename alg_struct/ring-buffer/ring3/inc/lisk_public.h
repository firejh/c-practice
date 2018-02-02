//filename		: lisk_public.h
//func			: global macros, global variables, global functions
//version		: v0.3.0
//author		: Alex Stocks
//date			: 2012-07
//verify		:
//license		: GPL 2.0
//declaration	: Long live C! C++, C'est la merde!

#ifndef __LISK_PUBLIC_H__
#define __LISK_PUBLIC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <lisk_type.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>		// memcmp, memmove
#include <sys/epoll.h>	// epoll_data_t

#ifdef __cplusplus
}
#endif

#define	nil							NULL
#define	nil_str						""

#define	LESS						(1e-6f)
#define	ZERO						LESS
#define	ABS_ZERO					0
#define	LEAST						(1e-9f)
#define	MINUS_LESS					(-1e-6f)
#define	PTR_2_NUM(_ptr_)			({	\
	epoll_data_t	__epoll_data__;		\
	__epoll_data__.ptr = _ptr_;			\
	(uw)(__epoll_data__.u64);			\
})
#define	NUM_2_PTR(num)				({	\
	epoll_data_t	_epoll_data_;		\
	_epoll_data_.u64 = num;				\
	_epoll_data_.ptr;					\
})
#define	IS_ZR(_value)				((_value) == ABS_ZERO)
#define	IS_NZR(_value)				((_value) != ABS_ZERO)
#define	IS_FZ(_value)				({				\
	f4 __zr__ = (f4)(_value);						\
	(MINUS_LESS + 1.0f - 1.0f < (__zr__)) &&		\
	((__zr__)  < LESS + 1.0f - 1.0f);				\
})
#define	IS_NFZ(_value)				({				\
	f4 __zr__ = (f4)(_value);						\
	(__zr__) < (MINUS_LESS + 1.0f - 1.0f) ||		\
	(LESS + 1.0f - 1.0f) < (__zr__);				\
})

#define	IS_MN(_value)				((_value) < 0)
#define	IS_NMN(_value)				(0 <= (_value))
#define	IS_PN(_value)				(0 < (_value))
#define	IS_NPN(_value)				((_value) <= 0)
#define	IS_GT(_value, min)			((min) < (_value))
#define	IS_GE(_value, min)			((min) <= (_value))
#define	IS_EQ(_value, _eq)			((_value) == (_eq))
#define	IS_NEQ(_value, _eq)			((_value) != (_eq))
#define	IS_NL(ptr)					((ptr) == NULL)
#define	IS_NNL(ptr)					((ptr) != NULL)
#define	IS_LT(_value, max)			((_value) < (max))
#define	IS_LE(_value, max)			((_value) <= (max))
#define	IS_BT(_value, min, max)		({			\
	typeof(_value) __is_bt_value__ = (_value);	\
	IS_GT(__is_bt_value__, min) &&				\
	IS_LT(__is_bt_value__, max);				\
})
#define	IS_BE(_value, min, max)		({			\
	typeof(_value) __is_be_value__ = (_value);	\
	IS_GE(__is_be_value__, min) &&				\
	IS_LE(__is_be_value__, max);				\
})
#define	IS_NBT(_value, min, max)	({			\
	typeof(_value) __is_nbt_value__ = (_value);	\
	IS_LE(__is_nbt_value__, min) ||				\
	IS_GE(__is_nbt_value__, max);				\
})
#define	IS_NBE(_value, min, max)	({			\
	typeof(_value) __is_nbe_value__ = (_value);	\
	IS_LT(__is_nbe_value__, min) ||				\
	IS_GT(__is_nbe_value__, max);				\
})

#define	PTR_OFFSET(ptr0, ptr1)		({			\
	uw __offset_p0__ = (uw)(ptr0);				\
	uw __offset_p1__ = (uw)(ptr1);				\
	(IS_LT( __offset_p0__, __offset_p1__) ?		\
	(__offset_p1__ - __offset_p0__) :			\
	(__offset_p0__ - __offset_p1__));			\
})

#define	SIZE_K(x)					((x) << 10)
#define	SIZE_M(x)					((x) << 20)
#define	SIZE_G(x)					((x) << 30)
#define	MAX_BUF_LEN					SIZE_K(1)
#define	SET_ZERO(ptr, size)			memset(ptr, 0, size)

#define	ALLOC(ptr, size)			({			\
	ptr = (typeof(ptr))(malloc(size));			\
})
#define	REALLOC(ptr, size)			({			\
	vp buf = realloc(ptr, size);				\
	ptr = (buf != NULL) ?						\
		((typeof(ptr))(buf)) : ptr;				\
	buf;										\
})
#define	DEALLOC(ptr)				({	free(ptr);	ptr = nil;	})

#define	ALIGN_SIZE(size, unit_size)	(((size) + (unit_size)-1) & ~((unit_size) - 1))
#define	ARRAY_SIZE(array)			(sizeof(array) / sizeof(array[0]))
#define	ARRAY_ELEM_PTR(t, a, o)		((t)(a) + o)		//@a array, @o offset, @t type
#define	ELEM_PTR(type, arr, len)	((type)((u1*)(arr) + len))
#define	ELEM_PRE(type, arr, len)	((type)((u1*)(arr) - len))

#define	CMP(dst, src, len)			({			\
	n4 __cmp_flag__ = -2;						\
	n1* __cmp_dst__ = (n1*)(dst);				\
	n1* __cmp_src__ = (n1*)(src);				\
	u4 __cmp_len__ = (u4)(len);					\
	if (IS_NEQ(__cmp_src__, __cmp_dst__) &&		\
		IS_GT(__cmp_len__, 0)) {				\
		__cmp_flag__ = memcmp(					\
						(vp)__cmp_dst__,		\
						(vp)__cmp_src__,		\
						__cmp_len__				\
						);						\
	}											\
	__cmp_flag__;								\
})

#define	CPY(dst, src, len)			({			\
	n4 __cpy_flag__ = -1;						\
	n1* __cpy_dst__ = (n1*)(dst);				\
	n1* __cpy_src__ = (n1*)(src);				\
	u4 __cpy_len__ = (u4)(len);					\
	if (IS_LE(__cpy_len__, 0)) {				\
		__cpy_flag__ = 0;						\
	} else {									\
		memmove(__cpy_dst__,					\
				__cpy_src__, __cpy_len__);		\
		__cpy_flag__ = 0;						\
	}											\
	__cpy_flag__;								\
})

#define	TO_STR(var)					#var

#ifndef	offsetof
#if 4 <= __GNUC__
#define offsetof(type, member)		__builtin_offsetof(type, member)
#else
#define	offsetof(stt, mem)			(uw)((&((typeof(stt)*)0)->mem))
//__cplusplus #define	offsetof(stt, mem)	((n4)(&((typeof(stt)*)1)->mem)-1)
#endif
#define	container_of(p, t, m)		({			\
	const typeof(((t*)0)->m)* __mptr__ = (p);	\
	(t*)((n1*)__mptr__ - offsetof(t, m));		\
})
#endif
#define	st_mem_size(stt, mem)		sizeof(((typeof(stt)*)0)->mem)

//////////////////////////////////////////
//public fuction typedef
//////////////////////////////////////////

typedef	n4							(*compare)(vp, vp);
typedef	vp							(*allocate)(u4);
typedef	vp							(*reallocate)(vp, u4);
typedef	vd							(*deallocate)(vp);
typedef	n4							(*thread_func)(vp);

#define RET_PTR(RET, FMT, ...)		 \
	do {    \
		if (IS_NL(RET) && IS_NZR(strlen(FMT))) {    \
			printf("%s-%s-%d ", __FILE__, __FUNCTION__, __LINE__);	\
			printf("failed, ret is nil, "FMT"\n", ##__VA_ARGS__);    \
		}   \
		return RET; \
	} while (0);

#define RET_INT(RET, FMT, ...)		\
	do {    \
		if (IS_NZR(RET) && IS_NZR(strlen(FMT))) {   \
			printf("%s-%s-%d ", __FILE__, __FUNCTION__, __LINE__);	\
			printf("ret = %d, "FMT"\n", RET, ##__VA_ARGS__); \
		}   \
		return (RET);   \
	} while (0);

#define	RET_VD(fmt, ...)	\
	do {	\
		if (strlen(fmt))	{ \
			printf(fmt"\n", ##__VA_ARGS__);	\
		}	\
		return;	\
	} while(0);

#define	perr(fmt, ...)		printf("%s-%s-%d "fmt"\n", __FILE__,	\
								__FUNCTION__, __LINE__, ##__VA_ARGS__)
#define pinfo				perr

#endif

