/**
 * FILE		: protobuf_c_public.h
 * DESC		: to simplify protobuf c functions
 * AUTHOR	: v1.0 written by Alex Stocks
 * DATE		: on Dec 12, 2013
 * LICENCE	: GPL 2.0
 * MOD		: #modified by Alex Stocks on June 2, 2012
 **/

#ifndef __PROTOBUF_C_PUBLIC_H__
#define __PROTOBUF_C_PUBLIC_H__

#define	PC_INIT(type, obj)				type obj = type##_Init
#define	PC_ARR_INIT(type, arr, num)		type arr[num];							\
										do {									\
											unsigned __pb_c_arr_idx__ = 0;		\
											for (__pb_c_arr_idx__ = 0;			\
											__pb_c_arr_idx__ < (unsigned)(num);	\
											__pb_c_arr_idx__++)	{				\
												type##_init(arr +				\
															__pb_c_arr_idx__);	\
											}									\
										} while(0)

#define	PC_PACK(type, ptr, buf, size)	unsigned size = (unsigned)				\
										type##_get_packed_size((ptr));			\
										unsigned char buf[size + 1];			\
										buf[size] = '\0';						\
										size = (unsigned)type##_pack((ptr), buf)

#define	PC_UNPACK(type, ptr, buf, size)	type* ptr = type##_unpack(NULL,			\
										(size_t)(size), (unsigned char*)(buf))
#define	PC_UNPACK_FREE(type, ptr)		type##_free_unpacked(ptr, NULL)

#endif

