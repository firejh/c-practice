/**
 * FILE		:lisk_string.h
 * DESC		:to instead of string.h
 * AUTHOR	:v0.3.0 written by Alex Stocks
 * DATE		:on Aug 29, 2012
 * MOD		:modified by * on June 2, 2012
 **/

#ifndef __LISK_STRING_H__
#define __LISK_STRING_H__

#if	(defined(__cplusplus))
extern	"C"	{
#endif

#include <lisk_public.h>

#include <string.h>
#include <stdio.h>

//////////////////////////////////////////
//basic string operations
//////////////////////////////////////////
/**
 * description	: check whether n1 @c is control n1acter <br/><br/>
 *
 * in-@c		: n1
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 _is_ctrl(n1 c);
/**
 * description	: check whether n1 @c is Arabic numbers <br/><br/>
 *
 * in-@c		: n1
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 _is_digit(n1 c);
/**
 * description	: check whether n1 @c is englist n1acter <br/><br/>
 *
 * in-@c		: n1
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 _is_alpha(n1 c);
/**
 * description	: check whether n1 @c is lowercase <br/><br/>
 *
 * in-@c		: n1
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 _is_lower(n1 c);
/**
 * description	: check whether n1 @c is capital(uppercase) <br/><br/>
 *
 * in-@c		: n1
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 _is_upper(n1 c);
/**
 * description	: check whether n1 @c is space n1acter,
 *				  such as \t \n \v \f \r space etc <br/><br/>
 *
 * in-@c		: n1
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 _is_space(n1 c);
/**
 * description	: check whether n1 @c is printable <br/><br/>
 *
 * in-@c		: n1
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 _is_print(n1 c);
/**
 * description	: if @c is a capital, get its lowercase n1 <br/><br/>
 *
 * in-@c		: n1
 *
 * out-@ret		: if successful, ret is lowercase of @c; otherwise -1 instead.
 **/
n1 to_lower(n1 c);
/**
 * description	: if @c is a lowercase, get its uppercase n1 <br/><br/>
 *
 * in-@c		: n1
 *
 * out-@ret		: if successful, ret is uppercase of @c; otherwise -1 instead.
 **/
n1 to_upper(n1 c);
/**
 * description	: check whether the word of this system is 32-bit <br/><br/>
 *
 * out-@ret		: if the word of this system is 32-bit, successful, 0;
 *				  otherwise -1 instead.
 **/
n4 _is_32(vd);
/**
 * description	: check whether the word of this system is 64-bit <br/><br/>
 *
 * out-@ret		: if the word of this system is 64-bit, successful, 0;
 *				  otherwise -1 instead.
 **/
n4 _is_64(vd);
/**
 * description	: trim right space of @pstr <br/><br/>
 *
 * in-@pstr		: string
 *
 * out-@ret		: the ret is @pstr except those @pstr's right side spaces
 **/
n1* str_trim_r(n1 *pstr);
/**
 * description	: trim left space of @pstr <br/><br/>
 *
 * in-@pstr		: string
 *
 * out-@ret		: the ret is @pstr except those @pstr's left side spaces
 **/
n1* str_trim_l(n1 *pstr);
/**
 * description	: trim spaces of @pstr <br/><br/>
 *
 * in-@pstr		: string
 *
 * out-@ret		: the ret is @pstr except those @pstr's side spaces
 **/
n1* str_trim(n1 *pstr);
/**
 * description	: locate n1acter@ch in string@str <br/><br/>
 *
 * in-@str		: string
 * in-@ch		: n1acter
 *
 * out-@ret		: return the first occurrence of @ch within @str,
 *				  return nil if not found.
 **/
n1* _index(n1* str, n1 ch);
/**
 * description	: Returns a pointer to the first occurrence of @pattern within. 
 *				  you can use it many times @str <br/><br/>
 *
 * in-@str		: string
 * in-@pattern	: sub string pattern
 *
 * out-@ret		: return the first occurrence of "needle" within @haystack,
 *				  no nil if not found.
 **/
n1* _strstr(const n1* str, const n1* pattern);
/**
 * description	: Returns a pointer to the last occurrence of @pattern
 *				  within @str <br/><br/>
 * in-@str		: string
 * in-@pattern	: sub string pattern
 *
 * out-@ret		: return the last occurrence of "needle" within @haystack,
 *				  no nil if not found.
 **/
n1* _strstr_r(const n1* str, const n1* pattern);
/**
 * description	: Reverse string @str
 * in-@str		: string
 *
 * out-@ret		: retrun null
 **/
vd _reverse_str(n1* str);
/**
 * description	: Insert @sub_str into @str at pos@pos and keep result in @dst
 * in-@str		: primary string
 * in-@pos		: insert pos
 * in-@sub_str	: sub string
 * in-out@dst	: destination memory
 * in-@dst_len	: @dst len
 *
 * out-@ret		: If insert successfully, ret is 0, or -1 instead if failed.
 **/
n4 _insert(n1* str, n4 pos, n1* sub_str, n1* dst, n4 dst_len);
/**
 * description	: the first string @first is merge to the second string @second. 
 *				: note that @first or @second could be null, but both of them 
 *				: should not be null.<br/><br/>
 *
 * in-@first	: the first string
 * in-@second	: the second string
 * in-out@dst	: destination memory
 * in-@dst_len	: @dst len
 *
 * out-@ret		: If merge successfully, ret is 0, or -1 instead if failed.
 **/
n4 _merge(n1* first, n1* second, n1* dst, n4 dst_len);
/**
 * description	: @rep shoule replace sub string of primary string
 *				  @str whose len is @len at @pos,
 *				: and keep result in @dst<br/><br/>
 *
 * in-@str		: primary string
 * in-@pos		: replaced string pos
 * in-@len		: replaced string len
 * in-@rep		: replacing string
 * in-out@dst	: destination memory
 * in-@dst_len	: @dst len
 *
 * out-@ret		: If insert successfully, ret is 0, or -1 instead if failed.
 **/
n4 _replace(n1* str, n4 pos, n4 len, n1* rep, n1* dst, n4 dst_len);
/**
 * description	: @rep shoule replace sub string @pattern of primary string @str,
 *				: and keep result in @dst<br/><br/>
 *
 * in-@str		: primary string
 * in-@pattern	: replaced string
 * in-@rep		: replacing string
 * in-out@dst	: destination memory
 * in-@dst_len	: @dst len
 *
 * out-@ret		: If insert successfully, ret is 0, or -1 instead if failed.
 **/
n4 _replace_str(n1* str, n1* pattern, n1* rep, n1* dst, n4 dst_len);
/**
 * description	: Delete sub string whose len is @len from @str at pos@pos
 *				  and keep result in @dst
 * in-@str		: primary string
 * in-@pos		: deleted sub string pos
 * in-@len		: deleted sub string len
 * in-out@dst	: destination memory
 * in-@dst_len	: @dst len
 *
 * out-@ret		: If insert successfully, ret is 0, or -1 instead if failed.
 **/
n4 _delete(n1* str, n4 pos, n4 len, n1* dst, n4 dst_len);

/**
 * description	: convert to a long integer from string @str<br/><br/>
 *
 * in-@str		: string
 *
 * out-@ret		: 32-bit number
 **/
n4 str2n4(n1* str);
/**
 * description	: convert to a unsigned long integer from string @str<br/><br/>
 *
 * in-@str		: string
 *
 * out-@ret		: 32-bit unsigned  number
 **/
u4 str2u4(n1* str);
/**
 * description	: convert to a long long integer from string @str<br/><br/>
 *
 * in-@str		: string
 *
 * out-@ret		: 64-bit number
 **/
n8 str2n8(n1* str);
/**
 * description	: convert to a unsigned long long integer from string @str<br/><br/>
 *
 * in-@str		: string
 *
 * out-@ret		: 64-bit unsigned  number
 **/
u8 str2u8(n1* str);
/**
 * description	: convert to a f4 from string @str<br/><br/>
 *
 * in-@str		: string
 *
 * out-@ret		: f4
 **/
f4 str2f4(n1* str);
/**
 * description	: convert to a f8 from string @str<br/><br/>
 *
 * in-@str		: string
 *
 * out-@ret		: f8
 **/
f8 str2f8(n1* str);

typedef struct lisk_string_tag {
	u4		capacity;
	u4		length;
	n1*		data;
} lisk_string_t, *lisk_string_p, **lisk_string_pp, lisk_string_a[1];

/**
 * description		: init lisk_string, you can also get a lisk_string by this func <br/><br/>
 *
 * in-out@str		: lisk_string
 * in-@size			: lisk_string initial capacity
 *
 * out-@ret			: if successful, ret is 0; ret is -1 or -2 when failed.
 **/
n4 lisk_string_init(vp str, u4 size);
/**
 * description		: allocate memory for @str and copy data from @buffer<br/><br/>
 *
 * in-out@str		: lisk_string
 * in-@buffer		: string buffer
 * in-@size			: @buffer len
 *
 * out-@ret			: if successful, ret is 0; ret is -1 or -2 when failed.
 **/
n4 lisk_string_init_copy(vp str, n1* buffer, u4 size);
/**
 * description	: init lisk_string by a format string<br/><br/>
 *
 * in-@str		: lisk_string
 * in-@fmt		: string format
 *
 * out-@ret		: if successful, ret is lisk_string; ret is nil when failed.
 **/
#define	lisk_string_fmt(str, fmt, ...)	({									\
	n1 fmt_str[SIZE_K(4)] = {0};										\
	snprintf(fmt_str, (n4)sizeof(fmt_str), fmt, ##__VA_ARGS__);			\
	lisk_string_append(str, fmt_str, (u4)strlen(fmt_str));				\
})
/**
 * description	: init lisk_string by a format string<br/><br/>
 *
 * in-@fmt		: string format
 *
 * out-@ret		: if successful, ret is lisk_string; ret is nil when failed.
 **/
#define	lisk_string_fmt_init(str, fmt, ...)	({								\
	n1 fmt_str[SIZE_K(4)] = {0};										\
	n4 ret = 0;															\
	snprintf(fmt_str, (n4)sizeof(fmt_str)), fmt, ##__VA_ARGS__);		\
	ret = lisk_string_init_copy((vp)(str), fmt_str, (u4)strlen(fmt_str));	\
	ret;																\
})
/**
 * description	: dealloc lisk_string <br/><br/>
 *
 * in-@str		: lisk_string
 *
 * out-@ret		: the ret is vd
 **/
vd lisk_string_uninit(vp str);
/**
 * description	: copy from another buffer @buf <br/><br/>
 *
 * in-out@str	: destination lisk_string
 * in-out@buf	: source string buffer
 * in-@buf_len	: source string buffer len
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 lisk_string_copy(vp str, n1* buf, u4 buf_len);
/**
 * description	: append the @buf to the end of @str <br/><br/>
 *
 * in-out@str	: destination lisk_string
 * in-out@buf	: another string buffer
 * in-@buf_len	: @buf len
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 lisk_string_append(vp str, n1* buf, u4 buf_len);
/**
 * description	: insert steam string @buf whose length is @buf_len
 *				  into lisk_string @str_ptr at location @pos<br/><br/>
 *
 * in-@str		: lisk_string
 * in-@pos		: insert position
 * in-@buf		: stream string
 * in-@buf_len	: @buf length
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 or -2 instead.
 **/
n4 lisk_string_insert(vp str, u4 pos, n1* buf, u4 buf_len);
/**
 * description	: Reverse string@str	<br/><br/>
 * in-@str		: string
 *
 * out-@ret		: if successful, ret is 0; otherwise @str is nil
 *				  or its len is 0, ret is -1.
 **/
n4 lisk_string_reverse(vp str);
/**
 * description	: trim all spaces of lisk_string@str <br/><br/>
 *
 * in-@str		: lisk_string
 *
 * out-@ret		: the ret is 0 except the input param @str is illegal
 **/
int lisk_string_trim(vp str);
/**
 * description	: clear buffer <br/><br/>
 *
 * in-out@buf	: buffer
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
vd lisk_string_clear(vp buf);

#if	(defined(__cplusplus))
}
#endif

#endif

