/**
 * FILE		:lisk_hash.h
 * DESC		:list some known hash funcs
 * AUTHOR	:v0.3.0 written by Alex Stocks
 * DATE		:on Sep 6, 2012
 * MOD		:modified by * on June 2, 2012
 **/

#ifndef __LISK_HASH_H__
#define __LISK_HASH_H__

#ifdef __cplusplus
	extern "C" {
#endif

#include <lisk_public.h>

#define	U4_MASK		(0X7FFFFFFF)
#define	U8_MASK		(u8)(0X7FFFFFFFFFFFFFFFLL)

/* bkdr hash function */
u4 bkdr32(n1* str, u4 str_len);
u8 bkdr64(n1 *str, u4 str_len);

/* crc hash function */
u4 crc32(n1* str, u4 str_len);
u8 crc64(n1* str, u4 str_len);

#ifdef __cplusplus
	}
#endif

#endif

