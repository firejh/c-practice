/** 
 * FILE		:lisk_type.h
 * DESC		:to supply standard variable type
 * AUTHOR	:v0.3.0 written by Alex Stocks
 * DATE		:on Sep 19, 2012
 * MOD		:modified by * on June 2, 2012
 * **/

#ifndef __LISK_TYPE_H__
#define __LISK_TYPE_H__

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
#define	vpp							void**

#define	N8_C(value)					value##LL
#define	U8_C(value)					value##ULL

#define	N1_MIN						(-N1_MAX - 1)
#define	N1_MAX						127
#define	N2_MIN						(-N2_MAX - 1)
#define	N2_MAX						32767
#define	N4_MIN						(-N4_MAX-1)
#define	N4_MAX						2147483647
#define	N8_MIN						(-N8_MAX-1)
#define	N8_MAX						9223372036854775807LL
#define	U1_MIN						((u1)(ABS_ZERO))
#define	U1_MAX						255
#define	U2_MIN						((u2)(ABS_ZERO))
#define	U2_MAX						65535
#define	U4_MIN						((u4)(ABS_ZERO))
#define	U4_MAX						4294967295U
#define	U8_MIN						((u8)(ABS_ZERO))
#define	U8_MAX						18446744073709551615ULL
#define	F4_EPSILON					1.192092896e-07F
#define	F4_MIN						1.175494351e-38F
#define	F4_MAX						3.402823466e+38F
#define	F8_EPSILON					2.2204460492503131E-016
#define	F8_MIN						2.2250738585072014E-308
#define	F8_MAX						1.7976931348623158E+308
#define	F12_EPSILON					1.0842021724855044E-19
#define	F12_MIN						3.3621031431120935E-4932
#define	F12_MAX						1.1897314953572318E+4932

#define	U2_HIGH(value)				(((value) >> 8) & 0XFF)
#define	U2_LOW(value)				((value) & 0XFF)
#define	U4_HIGH(value)				(((value) >> 16) & 0XFFFF)
#define	U4_LOW(value)				((value) & 0XFFFF)
#define	U8_HIGH(value)				(((value) >> 32) & 0XFFFFFFFF)
#define	U8_LOW(value)				((value) & 0XFFFFFFFF)
#define	U16_HIGH(value)				(((value) >> 64) & 0XFFFFFFFFFFFFFFFF)
#define	U16_LOW(value)				((value) & 0XFFFFFFFFFFFFFFFF)

#define	MAKE_U2(high, low)			({ u2  uv =  (u2)(high); uv = (uv <<  8) + low; })
#define	MAKE_U4(high, low)			({ u4  uv =  (u4)(high); uv = (uv << 16) + low; })
#define	MAKE_U8(high, low)			({ u8  uv =  (u8)(high); uv = (uv << 32) + low; })
#define	MAKE_U16(high, low)			({ u16 uv = (u16)(high); uv = (uv << 64) + low; })
#if defined(__x86_64__)
	#define	MAKE_UW					MAKE_U8
	#define	MAKE_UDW				MAKE_U16
#else
	#define	MAKE_UW					MAKE_U4
	#define	MAKE_UDW				MAKE_U8
#endif

#define	MAKE_N2(high, low)			({ n2  nv =  (n2)(high); nv = (nv <<  8) + low; })
#define	MAKE_N4(high, low)			({ n4  nv =  (n4)(high); nv = (nv << 16) + low; })
#define	MAKE_N8(high, low)			({ n8  nv =  (n8)(high); nv = (nv << 32) + low; })
#define	MAKE_N16(high, low)			({ n16 nv = (n16)(high); nv = (nv << 64) + low; })
#if defined(__x86_64__)
	#define	MAKE_NW					MAKE_N8
	#define	MAKE_NDW				MAKE_N16
#else
	#define	MAKE_NW					MAKE_N4
	#define	MAKE_NDW				MAKE_N8
#endif

#pragma pack(1)

typedef struct lisk_data_tag {
	u4		size;
	vp		data;
} lisk_data_t, *lisk_data_p, lisk_data_a[1];
/**
 * description	: the key of a value that user defined. type "key_t"
 *				  has been used on sys/type.h +123 <br/><br/>
 *
 * @size		: key length
 * @data		: key content
 **/
typedef	lisk_data_t				lisk_key_t;
typedef	lisk_data_p				lisk_key_p;
typedef	lisk_data_a				lisk_key_a;

/**
 * description	: value got from user .<br/><br/>
 *
 * @size		: value length
 * @data		: value content
 **/
typedef	lisk_data_t				lisk_value_t;
typedef	lisk_data_p				lisk_value_p;
typedef	lisk_data_a				lisk_value_a;

#pragma pack()

#endif

