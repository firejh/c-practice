/**
 * FILE		:lisk_string.c
 * DESC		:to instead of string.h
 * AUTHOR	:v0.3.0 written by Alex Stocks
 * DATE		:on Aug 29, 2012
 * MOD		:modified by * on June 2, 2012
 **/

#define _ISOC99_SOURCE
#include <lisk_string.h>
#include <lisk_type.h>	//U1_MAX
#include <lisk_math.h>	//next_pow_of_2
#include <string.h>
#include <stdlib.h>		// strtoul

//////////////////////////////////////////
//check string
//////////////////////////////////////////
n4 _is_ctrl(n1 c)
{
	if ((n1)(0x00) <= c && c <= (n1)(0x1F)) {
		return 0;
	}

	return -1;
}

n4 _is_digit(n1 c)
{
	if ('0' <= c && c <= '9') {
		return 0;
	}

	return -1;
}

n4 _is_alpha(n1 c)
{
	if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z')) {
		return 0;
	}

	return -1;
}

n4 _is_lower(n1 c)
{
	if ('a' <= c && c <= 'z') {
		return 0;
	}

	return -1;
}

n4 _is_upper(n1 c)
{
	if ('A' <= c && c <= 'Z') {
		return 0;
	}

	return -1;
}

n4 _is_space(n1 c)
{
	n4	flag;

	flag = 0;
	switch (c) {
	case ' ':
	case '\f': //switch page
	case '\t':
	case '\r':
	case '\n':
	case '\v': {//like \t, its vertical tabs
		flag = 0;
	}
	break;

	default: {
		flag = -1;
	}
	break;
	}

	return flag;
}

n4 _is_print(n1 c)
{
	if ((un1)(0x20) <= (un1)(c) && (un1)(c) <= (un1)(0x7F)) {
		return 0;
	}

	return -1;
}

n1 to_lower(n1 c)
{
	if (0 == _is_upper(c)) {
		c ^= (n1)(0x20);
	}

	return c;
}

n1 to_upper(n1 c)
{
	if (0 == _is_lower(c)) {
		c ^= (n1)(0x20);
	}

	return c;
}

n4 _is_32(vd)
{
	if (sizeof(uw) == 4) {
		return 0;
	}

	return -1;
}

n4 _is_64(vd)
{
	if (sizeof(uw) == 8) {
		return 0;
	}

	return -1;
}

n1* str_trim_r(n1 *pstr)
{
	n4	len;
	n4	idx;

	if (IS_NL(pstr)) {
		return nil;
	}

	len = strlen(pstr);
	if (IS_LE(len, 0)) {
		*pstr = '\0';
		return pstr;
	}

	idx = len - 1;
	while (IS_GE(idx, 0)) {
		if (IS_NZR(_is_space(pstr[idx]))) {
			break;
		}
		pstr[idx--] = '\0';
	}

	return pstr;
}

n1* str_trim_l(n1 *pstr)
{
	n4	len;
	n4	idx;

	if (IS_NL(pstr)) {
		return nil;
	}

	len = strlen(pstr);
	if (IS_LE(len, 0)) {
		*pstr = '\0';
		return pstr;
	}

	idx = 0;
	while (IS_LT(idx, len)) {
		if (IS_NZR(_is_space(pstr[idx]))) {
			break;
		}
		idx++;
	}
	if (IS_GE(idx, len)) {
		*pstr = '\0';
		return pstr;
	}

	if (IS_GT(idx, 0)) {
		memmove(pstr, pstr + idx, len + 1 - idx);
		pstr[len+1-idx] = '\0';
	}

	return pstr;
}

n1* str_trim(n1 *pstr)
{
	n4	len;
	n4	cursor;
	n4	idx;

	if (IS_NL(pstr)) {
		return nil;
	}

	len = strlen(pstr);
	if (IS_LE(len, 0)) {
		*pstr = '\0';
		return pstr;
	}

	idx = 0;
	cursor = 0;
	while (idx < len) {
		if (IS_NZR(_is_space(pstr[idx]))) {
			pstr[cursor++] = pstr[idx];
		}
		idx++;
	}
	pstr[cursor] = '\0';

	return pstr;
}

un1* bh_search(un1* str, u4 str_len, un1* sub_str, u4 sub_str_len)
{
	u4		idx;
	u4		anchor;
	u4		skip_buf[255 + 1];

	if (IS_LE(sub_str_len, 0) || IS_LT(str_len, sub_str_len) || IS_NL(str) || IS_NL(sub_str)) {
		return nil;
	}

	for (idx = 0; IS_LE(idx, U1_MAX); idx++)	{
		skip_buf[idx] = sub_str_len;
	}

	anchor = sub_str_len - 1;
	for (idx = 0; IS_LT(idx, anchor); idx = idx + 1) {
		skip_buf[sub_str[idx]] = anchor - idx;
	}

	while (IS_LE(sub_str_len, str_len)) {
		for (idx = anchor; IS_EQ(str[idx], sub_str[idx]); idx = idx - 1) {
			if (IS_ZR(idx))  {
				return str;
			}
		}

		str_len	-= skip_buf[str[anchor]];
		str		+= skip_buf[str[anchor]];
	}

	return nil;
}

n1* _index(n1* str, n1 ch)
{
	if (IS_NL(str)) {
		return nil;
	}

	return (n1*)bh_search((un1*)(str), strlen(str), (un1*)&(ch), 1);
}

//search string multiple times
n1* _strstr(const n1* str, const n1* pattern)
{
	static n1*	last_str = nil;
	static n1*	last_pat = nil;
	static n1*	cursor = nil;
	n1*			ret;
	n1*			start;
	n1*			end;

	if (IS_NL(str) || IS_NL(pattern) || IS_GE(strlen(pattern), strlen(str))) {
		return nil;
	}

	ret = nil;
	if (IS_NNL(last_str) && IS_NNL(last_pat) && IS_EQ(str, last_str) && IS_EQ(pattern, last_pat)) {
		end = (n1*)(str) + strlen(str) - 1;
		if(IS_NL(cursor) || IS_LT(cursor, str) || IS_GT(cursor, end)) {
			return nil;
		}
		start = cursor;
		ret = (n1*)bh_search((un1*)start, strlen(start), (un1*)pattern, strlen(pattern));
		cursor = ret;
		if (IS_NNL(cursor)) {
			cursor += strlen(pattern);
			//cursor++;
		} else {
			//have found the last res
			last_str = nil;
			last_pat = nil;
			cursor = nil;
		}
	} else {
		ret = (n1*)bh_search((un1*)str, strlen(str), (un1*)pattern, strlen(pattern));
		last_str = (n1*)(str);
		last_pat = (n1*)(pattern);
		cursor = ret;
		if (IS_NNL(cursor)) {
			cursor += strlen(pattern);
		}
	}

	return ret;
}

n1* _strstr_r(const n1* str, const n1* pattern)
{
	n1*	ret;
	n1*	last_ret;
	n1*	start;
	n1*	end;

	if (IS_NL(str) || IS_NL(pattern) || IS_GE(strlen(pattern), strlen(str))) {
		return nil;
	}

	ret = nil;
	last_ret = ret;
	start = (n1*)(str);
	end = (n1*)(start + strlen(start));
	while (IS_NNL(start) && IS_LT(start, end)) {
		ret = (n1 *)bh_search((un1*)start, strlen(start), (un1*)pattern, strlen(pattern));
		if (IS_NL(ret)) {
			break;
		}
		last_ret = ret;
		start = ret + strlen(pattern);
	}

	return last_ret;
}

vd _reverse_str(n1* str)
{
	n1*	first;
	n1*	last;

	if (IS_NL(str) || IS_LE(strlen(str), 1)) {
		return;
	}

	first = str;
	last = first + strlen(first) - 1;
	while (first < last) {
		*first = *first ^ *last;
		*last = *first ^ *last;
		*first = *first ^ *last;
		first++;
		last--;
	}
}

n4 _insert(n1* str, n4 pos, n1* sub_str, n1* dst, n4 dst_len)
{
	if (IS_NL(str) || IS_ZR(strlen(str)) || IS_NBE(pos, 0, strlen(str)) ||
		IS_NL(sub_str) || IS_ZR(strlen(sub_str)) || IS_NL(sub_str) ||
		IS_LT(dst_len, strlen(str)+strlen(sub_str)+1)) {
		return -1;
	}

	SET_ZERO(dst, dst_len);
	dst[--dst_len] = '\0';
	strncat(dst, str, pos);
	strncat(dst, sub_str, strlen(sub_str));
	strncat(dst, str+pos, strlen(str)-pos);

	return 0;
}

n4 _merge(n1* first, n1* second, n1* dst, n4 dst_len)
{
	n4	flag1;
	n4	flag2;
	n4	flag3;
	n4	min_dst_len;

	flag1 = IS_NL(first) || IS_ZR(strlen(first));
	flag2 = IS_NL(second) || IS_ZR(strlen(second));
	min_dst_len = 1;
	if (IS_NNL(first)) {
		min_dst_len += strlen(first);
	}
	if (IS_NNL(second)) {
		min_dst_len += strlen(second);
	}
	flag3 = IS_NL(dst) || IS_LT(dst_len, min_dst_len);
	if ((IS_NZR(flag1) && IS_NZR(flag2)) || IS_NZR(flag3)) {
		return -1;
	}

	SET_ZERO(dst, dst_len);
	dst[--dst_len] = '\0';
	if ((IS_ZR(flag1) && IS_NZR(flag2))) {
		strncat(dst, first, strlen(first));
		return 0;
	}
	if ((IS_NZR(flag1) && IS_ZR(flag2))) {
		strncat(dst, second, strlen(second));
		return 0;
	}

	return _insert(first, strlen(first), second, dst, dst_len);
}

n4 _replace(n1* str, n4 pos, n4 len, n1* rep, n1* dst, n4 dst_len)
{
	n4	min_dst_len;

	min_dst_len = 1;
	if (IS_NNL(str)) {
		min_dst_len += strlen(str);
		if (IS_GT(len, 0)) {
			min_dst_len -= len;
			if (IS_NNL(rep)) {
				min_dst_len += strlen(rep);
			}
		}
	}
	if (IS_NL(str) || IS_ZR(strlen(str)) || IS_NBE(pos, 0, strlen(str)-1) || IS_LE(len, 0) || IS_GT(pos+len, strlen(str)) || IS_NL(dst) || IS_LT(dst_len, min_dst_len))  {
		return -1;
	}

	SET_ZERO(dst, dst_len);
	dst[--dst_len] = '\0';

	strncat(dst, str, pos);
	if (IS_NNL(rep) && IS_NZR(strlen(rep))) {
		strncat(dst, rep, strlen(rep));
	}
	strncat(dst, str+pos+len, strlen(str)-(pos+len));

	return 0;
}

n4 _replace_str(n1* str, n1* pattern, n1* rep, n1* dst, n4 dst_len)
{
	n4 del_flag;
	n1*	start;
	n1*	sub;
	n1*	str_end;

	if (IS_NL(str) || IS_NL(pattern) || IS_NL(dst) || IS_GE(strlen(pattern), strlen(str)) || IS_LT(dst_len, 2) || IS_LT(dst_len, strlen(str))) {
		return -1;
	}

	SET_ZERO(dst, dst_len);
	dst[--dst_len] = '\0';
	del_flag = 0;
	if (IS_NL(rep) || IS_ZR(strlen(rep))) {
		del_flag = 1;
	}

	start = str;
	sub = nil;
	str_end = str + strlen(str);
	while (IS_LT(start, str_end)) {
		sub = _strstr(str, pattern);
		if (IS_NL(sub)) {
			if (IS_GT((strlen(dst) + (size_t)(str_end - start)), dst_len)) {
				break;
			}
			strncat(dst, start, (size_t)(str_end - start));
			break;
		}
		if (IS_GT(sub, start)) {
			if (IS_GT((strlen(dst) + (size_t)(sub - start)), dst_len)) {
				//to be case of next substr search
				while (IS_NNL(_strstr(str, pattern)));
				break;
			}

			strncat(dst, start, (size_t)(sub - start));
			if (IS_ZR(del_flag)) {
				if (IS_GT((strlen(dst) + strlen(rep)), dst_len)) {
					//to be case of next substr search
					while (IS_NNL(_strstr(str, pattern)));
					break;
				}
				strncat(dst, rep, strlen(rep));
			}
		}

		start = sub + strlen(pattern);
	}

	return 0;
}

n4 _delete(n1* str, n4 pos, n4 len, n1* dst, n4 dst_len)
{
	n1*	rep;

	rep = nil;
	return _replace(str, pos, len, rep, dst, dst_len);
}

//////////////////////////////////////////
//str & num
//////////////////////////////////////////
n4 str2n4(n1* str)
{
	return (n4)strtoul(str, (n1**)nil, 10);
}

u4 str2u4(n1* str)
{
	return (u4)strtoul(str, (n1**)nil, 10);
}

n8 str2n8(n1* str)
{
	return (n8)strtoul(str, (n1**)nil, 10);
}

u8 str2u8(n1* str)
{
	return (u8)strtoul(str, (n1**)nil, 10);
}

f4 str2f4(n1* str)
{
	return strtod(str, (n1**)nil);
}

f8 str2f8(n1* str)
{
	return strtod(str, (n1**)nil);
}

//////////////////////////////////////////
//std::string like
//////////////////////////////////////////

/**
 * description	: realloc memory space for lisk_string @buffer<br/><br/>
 *
 * in-@str		: lisk_string
 * in-@size		: new len
 *
 * out-@ret		: if successful, the ret is 0; othersize -1 if params are illegal or -2 if
 *				  the system fail to allocate memory space.
 **/
n4 lisk_string_realloc(vp str, u4 size);

n4 lisk_string_init(vp string, u4 size)
{
	u4				capacity;
	vp				data;
	lisk_string_p	str;

	str = string;
	if (IS_NL(str) || IS_ZR(size))	{
		size = 1;
	}

	capacity = size;
	if (IS_NZR(is_pow_of_2(capacity)))	{
		capacity = (u4)next_pow_of_2(capacity);
	}
	if (IS_NL(ALLOC(vp, data, capacity * sizeof(char)))) {
		return -2;
	}

	str->capacity	= capacity;
	str->length		= 0;
	str->data		= data;
	str->data[0]	= '\0';

	return 0;
}

n4 lisk_string_init_copy(vp string, n1* buffer, u4 size)
{
	n4				ret;
	lisk_string_p	str;

	str = string;
	if (IS_NL(str) || IS_NL(buffer)) {
		return -1;
	}

	if (IS_ZR(size)) {
		size = 1;
	}

	ret = lisk_string_init(str, size + 1);
	if (IS_NZR(ret))	{
		return -2;
	}

	if (IS_NZR(CPY(str->data, buffer, size)))	{
		lisk_string_uninit(str);
		return -2;
	}

	str->data[size]	= '\0';
	str->length		= size;

	return 0;
}

vd lisk_string_uninit(vp str)
{
	lisk_string_p	string;

	string = str;
	if (IS_NL(string))	{
		return;
	}

	if (IS_NNL(string->data))	{
		DEALLOC(string->data);
		string->data = nil;
		string->capacity = 0;
		string->length = 0;
	}
}

n4 lisk_string_realloc(vp str_ptr, u4 size)
{
	u4				capacity;
	lisk_string_p	str;

	str = (lisk_string_p)(str_ptr);
	if (IS_NL(str))	{
		return -1;
	}

	if (IS_ZR(size))	{
		lisk_string_uninit(str);
		return 0;
	}

	capacity = str->capacity;
	if (IS_ZR(capacity)) {
		return -1;
	}

	if (IS_NL(REALLOC(n1*, str->data, size)) )	{
		return -2;
	}

	str->capacity = size;
	if (IS_GE(str->length, str->capacity))	{
		str->length = str->capacity - 1;
		str->data[str->length] = '\0';
	}

	return 0;
}

n4 lisk_string_copy(vp str_ptr, n1* buf, u4 buf_len)
{
	lisk_string_p	str;

	str = (lisk_string_p)(str_ptr);
	if (IS_NL(str) || IS_NL(buf) || IS_EQ(buf_len, 0)) {
		return -1;
	}

	str->length = 0;
	str->data[str->length] = '\0';

	return lisk_string_append(str_ptr, buf, buf_len);
}

n4 lisk_string_append(vp str_ptr, n1* buf, u4 buf_len)
{
	lisk_string_p	str;

	str = str_ptr;
	if (IS_NL(str))	{
		return -1;
	}

	return lisk_string_insert(str, str->length, buf, buf_len);
}

n4 lisk_string_insert(vp str_ptr, u4 pos, n1* buf, u4 buf_len)
{
	n4				ret;
	u4				capacity;
	u4				str_len;
	lisk_string_p	str;

	str = (lisk_string_p)(str_ptr);
	if (IS_NL(str) || IS_NL(buf) || IS_ZR(buf_len)) {
		return -1;
	}

	str_len = str->length;
	if (IS_ZR(buf_len) || IS_GT(pos, str_len))	{
		return -1;
	}

	capacity = str_len + buf_len + 1;
	//buf_len = length(buf), 1 = length('\0')
	if (IS_LT(str->capacity, capacity))	{
		if (IS_NZR(is_pow_of_2(capacity)))	{
			capacity = next_pow_of_2(capacity);
		}
		ret = lisk_string_realloc(str, capacity);
		if (IS_NZR(ret))	{
			return -2;
		}
	}

	if (IS_LT(pos, str_len))	{
		if (IS_NZR(CPY(str->data + pos + buf_len, str->data + pos, str_len - pos)))	{
			return -2;
		}
	}

	if (IS_NZR(CPY(str->data + pos, buf, buf_len)))	{
		return -2;
	}

	str->length += buf_len;
	str->data[str->length] = '\0';

	return 0;
}

n4 lisk_string_reverse(vp str_ptr)
{
	lisk_string_p	str;

	if (IS_NL(str = str_ptr)) {
		return -1;
	}

	if (IS_ZR(str->capacity)) {
		return -1;
	}

	_reverse_str(str->data);

	return 0;
}

vd lisk_string_clear(vp str_ptr)
{
	lisk_string_p	str;

	if (IS_NL(str = str_ptr) || IS_NL(str->data)) {
		return;
	}

	str->length = 0;
	str->data[str->length] = '\0';
}

n4 lisk_string_trim(vp str_ptr)
{
	n1*				ret;
	lisk_string_p	str;

	if (IS_NL(str = str_ptr)) {
		return -1;
	}

	if (IS_ZR(str->length)) {
		return -1;
	}

	ret = str_trim(str->data);
	if (IS_NL(ret) || IS_NEQ(ret, str->data)) {
		return -2;
	}

	str->length = strlen(str->data);

	return 0;
}

