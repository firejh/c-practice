/** 
 * FILE		:lisk_type.h
 * DESC		:to supply standard variable type
 * AUTHOR	:v1.0 written by Alex Stocks
 * DATE		:on Sep 19, 2012
 * MOD		:modified by * on June 2, 2012
 * **/

#ifndef __LISK_TYPE_H__
#define __LISK_TYPE_H__

#define	n1							char
#define	n2							short int
#define	n4							int
#define	n8							long long int
#define	nw							long int
#define	un1							unsigned char
#define	u2							unsigned short int
#define	u4							unsigned int
#define	u8							unsigned long long int
#define	uw							unsigned long int
#define	f4							float
#define	f8							double
#define	fw							long double
#define	real_num					f4
#define	vd							void
#define	vp							void*
#define	vpp							void**
#define	N8_FMT						""lld""
#define	U8_FMT						""llu""

extern const n1* vt[];
extern const n1* vtf[];
extern const n1* vtx[];
#if (defined(BITS32) || defined(_WIN32))
#define	f12							long double
#define	N8_C(value)					value##LL
#define	U8_C(value)					value##ULL
typedef enum tagVAR_TYPE { vt_n1 = 0, vt_n2, vt_n4, vt_n8, vt_u1, vt_u2, vt_u4, vt_un8, vt_f4, vt_f8, vt_f12, vt_ptr, vt_str}VAR_TYPE;
#elif (defined(BITS64) || defined(_WIN64))
#define	f16							long double
#define	N8_C(value)					value##L
#define	U8_C(value)					value##UL
typedef enum tagVAR_TYPE { vt_n1 = 0, vt_n2, vt_n4, vt_n8, vt_u1, vt_u2, vt_u4, vt_un8, vt_f4, vt_f8, vt_f16, vt_ptr, vt_str }VAR_TYPE;
#endif

#define	N1_BIT_SIZE					8
#define	N2_BIT_SIZE					16
#define	N4_BIT_SIZE					32
#define	N8_BIT_SIZE					64
#define	U1_BIT_SIZE					8
#define	U2_BIT_SIZE					16
#define	U4_BIT_SIZE					32
#define	U8_BIT_SIZE					64
#define	F4_BIT_SIZE					32
#define	F8_BIT_SIZE					64
#define	F12_BIT_SIZE				96

#define	N1_MIN						(-128)
#define	N1_MAX						127
#define	N2_MIN						(-32768)
#define	N2_MAX						32767
#define	N4_MIN						(-N4_MAX-1)
#define	N4_MAX						2147483647
#define	N8_MIN						(-N8_MAX-1)
#define	N8_MAX						9223372036854775807LL
#define	U1_MIN						((un1)(ABS_ZERO))
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

#define	TYPE_U1(value)				((un1)(value))
#define	TYPE_U2(value)				((u2)(value))
#define	TYPE_U4(value)				((u4)(value))
#define	TYPE_U8(value)				((u8)(value))
#define	TYPE_N1(value)				((n1)(value))
#define	TYPE_N2(value)				((n2)(value))
#define	TYPE_N4(value)				((n4)(value))
#define	TYPE_N8(value)				((n8)(value))
#define	TYPE_F4(value)				((f4)(value))
#define	TYPE_F8(value)				((f8)(value))
#define	TYPE_U1_PTR(value)			((un1*)(value))
#define	TYPE_U2_PTR(value)			((u2*)(value))
#define	TYPE_U4_PTR(value)			((u4*)(value))
#define	TYPE_U8_PTR(value)			((u8*)(value))
#define	TYPE_N1_PTR(value)			((n1*)(value))
#define	TYPE_N2_PTR(value)			((n2*)(value))
#define	TYPE_N4_PTR(value)			((n4*)(value))
#define	TYPE_N8_PTR(value)			((n8*)(value))
#define	TYPE_VD_PTR(value)			((vp)(value))
#define	TYPE_F4_PTR(value)			((f4*)(value))
#define	TYPE_F8_PTR(value)			((f8*)(value))
#define	TYPE_VD_PTR(value)			((vp)(value))

#pragma pack(1)
/**
 * description	: the key of a value that user defined .<br/><br/>
 *
 * @size		: key length
 * @data		: key content
 **/
typedef struct skey_tag {
	u4		size;
	n1*		data;
} key_t, *key_p, key_a[1];

/**
 * description	: value got from user .<br/><br/>
 *
 * @size		: value length
 * @data		: value content
 **/
typedef struct value_tag {
	u4		size;
	n1*		data;
} value_t, *value_p, value_a[1];

#pragma pack()

#endif

