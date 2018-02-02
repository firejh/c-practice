/**
 * FILE		:lisk_vector.c
 * DESC		:to instead of stl vector
 * AUTHOR	:v0.3.0 written by Alex Stocks
 * DATE		:on Aug 29, 2012
 * MOD		:modified by * on June 2, 2012
 **/

#include <lisk_log.h>
#include <lisk_math.h>
#include <lisk_vector.h>

#include <string.h>

/**
 * description	: change the sizeof of the memory block of @vect to a new size
 *				  @number * vect->length. the contents of @vect is unchanged
 *				  copied to the new memory block and primary memory os @vect is
 *				  released. if it failed to alloc new memory block, ret is nil
 *				  and primary memory block is unchanged <br/><br/>
 *
 * in-@number	: init max unit num
 * in-@length	: per unit len
 *
 * out-@ret		: if successful, ret is the new memory block pointed by @ret
 *				  otherwise ret is nil if those params are illegal or the OS
 *				  failed to alloc space. <br/><br/>
 **/
static n4 lisk_vector_realloc(vp vect, u4 number);

u4 lisk_vector_max_size(vp vector)
{
	lisk_vector_p	vect;

	if (IS_NL(vect = (lisk_vector_p)(vector)) || IS_ZR(vect->length)) {
		return 0;
	}

	return vect->capacity / vect->length;
}

u4 lisk_vector_capacity(vp vector)
{
	lisk_vector_p	vect;

	if (IS_NL(vect = (lisk_vector_p)(vector))) {
		return 0;
	}

	return vect->capacity;
}

vp lisk_vector_at(vp vector, u4 pos)
{
	lisk_vector_p	vect;

	if (IS_NL(vect = (lisk_vector_p)(vector))) {
		RET_NL("@vect = %p", vect);
	}

	if (IS_GE(pos, vect->number)) {
		RET_NL("@vect->number = %u, pos = %u", vect->number, pos);
	}

	return (vp)(vect->data + pos * vect->length);
}

vp lisk_vector_find(vp vector, vp value, compare cmp)
{
	u4				idx;
	vp				ret;
	vp				elem;
	lisk_vector_p	vect;

	if (IS_NL(vect = (lisk_vector_p)(vector)) || IS_NL(value)) {
		RET_PTR(nil, "@vect = %p, @value = %p", vect, value);
	}

	ret	= nil;
	if (IS_NNL(cmp)) {
		ret = bsearch(
					value,
					vect->data,
					vect->number,
					vect->length,
					(vp)cmp
					);
		RET_PTR(ret, nil_str);
	}

	for (idx = 0; IS_LT(idx, vect->number); idx++) {
		elem = (vp)(vect->data + idx * vect->length);
		if (IS_ZR(CMP(elem, value, vect->length))) {
			ret = elem;
			break;
		}
	}

	RET_PTR(ret, nil_str);
}

vp lisk_vector_first(vp vector)
{
	lisk_vector_p	vect;

	if (IS_NL(vect = (lisk_vector_p)(vector))) {
		RET_NL("@vect = %p", vect);
	}

	return lisk_vector_at(vect, 0);
}

vp lisk_vector_last(vp vector)
{
	lisk_vector_p	vect;

	if (IS_NL(vect = (lisk_vector_p)(vector))) {
		RET_NL("@vect = %p", vect);
	}

	if (IS_ZR(vect->number)) {
		RET_NL("@vect->number = %u", vect->number);
	}

	return lisk_vector_at(vect, vect->number - 1);
}

vp lisk_vector_next(vp vector, vp cur)
{
	u4				idx;
	n1*				ptr;
	lisk_vector_p	vect;

	if (IS_NL(vect = (lisk_vector_p)(vector))) {
		RET_NL("@vect = %p", vect);
	}

	if (IS_ZR(vect->length) || IS_ZR(vect->number)) {
		RET_NL("@vect->length = %u, @vect->number = %u", vect->length, vect->number);
	}

	idx = (u4)((n1*)(cur) - (n1*)(vect->data)) / vect->length;
	ptr = (n1*)(vect->data) + idx * vect->length;
	if (IS_GE(idx, vect->number-1) || IS_NEQ((n1*)cur, ptr))	{
		RET_NL("unit number of vector = %u, @cur = %p, @cur index = %u, while the pointer of that index = %p\n",
				vect->number, cur, idx, ptr);
	}

	return (vp)(vect->data + (idx + 1) * vect->length);
}

vp lisk_vector_pre(vp vector, vp cur)
{
	u4				idx;
	n1*				ptr;
	lisk_vector_p	vect;

	vect = (lisk_vector_p)(vector);
	if (IS_NL(vect)) {
		RET_NL("@vect = %p", vect);
	}

	if (IS_ZR(vect->length)) {
		RET_NL("@vect->length = %u", vect->length);
	}

	idx = (u4)((n1*)(cur) - (n1*)(vect->data)) / vect->length;
	ptr = (n1*)(vect->data) + idx * vect->length;
	if (IS_NBT(idx, 0, vect->number) || IS_NEQ((n1*)cur, ptr))	{
		RET_NL("unit number of vector = %u, @cur = %p, @cur index = %u, while the pointer of that index = %p\n",
				vect->number, cur, idx, ptr);
	}

	return (vp)(vect->data + (idx - 1) * vect->length);
}

n4 lisk_vector_init(vp vector, u4 number, u4 length)
{
	n1*				data;
	u4				capacity;
	lisk_vector_p	vect;

	vect = vector;
	if (IS_NL(vect) || IS_ZR(number) || IS_ZR(length)) {
		RET_INT(-1, "@vect = %p, @number = %u, @length = %u",
					vect, number, length);
	}
	vect->data = nil;

	capacity = number * length;
	if (IS_NZR(is_pow_of_2(capacity)))	{
		capacity = (u4)next_pow_of_2(capacity);
	}
	if (IS_NL(ALLOC(n1*, data, capacity))) {
		RET_INT(-2, "ALLOC(data = %p, size = %u)", data, capacity);
	}

	vect->capacity	= capacity;
	vect->length	= length;
	vect->number	= 0;
	vect->data		= data;

	RET_INT(0, nil_str);
}

n4 lisk_vector_init_copy(vp vector, vp array, u4 number, u4 length)
{
	n4				ret;
	u4				len;
	lisk_vector_p	vect;

	vect = vector;
	len = number * length;
	if (IS_NL(vect) || IS_NL(array) || IS_ZR(len)) {
		RET_INT(-1, "@vector = %p, array = %p, len = %u", vect, array, len);
	}
	vect->data = nil;

	ret = lisk_vector_init(vect, number, length);
	if (IS_NZR(ret))	{
		RET_INT(-2, "lisk_vector_init(vect = %p, @number = %u, @length = %u) = %d",
					vect, number, length, ret);
	}

	if (IS_NZR(CPY(vect->data, array, len))) {
		lisk_vector_uninit(vect);
		RET_INT(-2, "CPY(vect->data = %p, array = %p, len = %u) = nil",
					vect->data, array, len);
	}
	vect->number = number;

	RET_INT(0, nil_str);
}

vd lisk_vector_uninit(vp vect_ptr)
{
	lisk_vector_p	vect;

	vect = vect_ptr;
	if (IS_NNL(vect) && IS_NNL(vect->data))	{
		DEALLOC(vect->data);
		SET_ZERO(vect, sizeof(lisk_vector_t));
	}
}

n4 lisk_vector_realloc(vp vect_ptr, u4 number)
{
	u4				vect_len;
	lisk_vector_p	vect;

	vect = nil;
	if (IS_NL(vect = (lisk_vector_p)(vect_ptr))) {
		RET_INT(-1, "@vect = %p", vect);
	}

	if (IS_ZR(vect->capacity)) {
		RET_INT(-1, "*@vect_ptr->capacity = %u", vect->capacity);
	}

	if (IS_ZR(number))	{
		lisk_vector_uninit(vect_ptr);
		RET_INT(0, nil_str);
	}

	vect_len = vect->length * number;
	if (IS_NL(REALLOC(n1*, vect->data, vect_len))) {
		RET_INT(-2, "REALLOC(vect = %p, size = %u) = nil", vect, vect_len);
	}

	vect->capacity = vect_len;
	if (IS_LT(number, vect->number))	{
		vect->number = number;
	}

	RET_INT(0, nil_str);
}

n4 lisk_vector_assign(vp vect, vp array, u4 size)
{
	if (IS_NL(vect) || IS_ZR(size))	{
		RET_INT(-1, "@vect_ptr = %p, @size = %u", vect, size);
	}

	lisk_vector_clear(vect);

	return lisk_vector_push_back(vect, array, size);
}

//pos should be in range[0, vect->number]
n4 lisk_vector_insert(vp vect_ptr, u4 pos, vp array, u4 size)
{
	n1*					cursor;
	n4					ret;
	u4					len;
	lisk_vector_p		vect;

	vect	= nil;
	if (IS_NL(vect = (lisk_vector_p)(vect_ptr))) {
		RET_INT(-1, "@vect = %p", vect);
	}

	if (IS_ZR(size) || IS_GT(pos, vect->number)) {
		RET_INT(-1, "@size = %u, @pos = %u, @vect->length %u ,@vect->number %u",
				size, pos, vect->length, vect->number);
	}

	if (IS_GT((vect->number + size) * vect->length, vect->capacity))	{
		len = vect->number + size;
		if (IS_NZR(is_pow_of_2(len)))	{
			len = (u4)next_pow_of_2(len);
		}
		ret = lisk_vector_realloc(vect, len);
		if (IS_NZR(ret))	{
			RET_INT(-2, "lisk_vector_realloc(@vector = %p, size = %u) = %d",
						&vect, len, ret);
		}
	}

	cursor = vect->data + pos * vect->length;
	if (IS_LT(pos, vect->number))	{
		len = (vect->number - pos) * vect->length;
		if (IS_NZR(CPY(cursor + size * vect->length, cursor, len)))	{
			RET_INT(-2, "CPY(dst = %p, src = %p, len = %u) = -1",
						cursor + size * vect->length, cursor, len);
		}
	}

	if (IS_NNL(array))	{
		len = size * vect->length;
		if (IS_NEQ(cursor, array))	{
			if (IS_NZR(CPY(cursor, array, len)))	{
				RET_INT(-2, "CPY(dst = %p, src = %p, len = %u) = -1",
							cursor, array, len);
			}
		}
	}
	vect->number += size;

	RET_INT(0, nil_str);
}

n4 lisk_vector_push_back(vp vect_ptr, vp array, u4 size)
{
	lisk_vector_p	vect;

	vect = vect_ptr;
	if (IS_NL(vect))	{
		RET_INT(-1, "@vect = %p", vect);
	}

	return lisk_vector_insert(vect, vect->number, array, size);
}

n4 lisk_vector_empty(vp vect_ptr)
{
	lisk_vector_p	vect;

	if (IS_NNL(vect = (lisk_vector_p)(vect_ptr)) && IS_ZR(vect->number)) {
		RET_INT(0, "");
	}

	RET_INT(-1, nil_str);
}

vp lisk_vector_erase(vp vect_ptr, u4 pos, u4 size)
{
	n4				ret;
	n1*				dst;
	n1*				src;
	u4				buf_len;
	u4				cpy_len;
	vp				ret_buf;
	lisk_vector_p	vect;

	vect = nil;
	if (IS_NL(vect = (lisk_vector_p)(vect_ptr)) ||IS_ZR(size)) {
		RET_NL("@vector = %p, size = %u", vect, size);
	}

	if (IS_ZR(lisk_vector_empty(vect))) {
		RET_NL("lisk_vector_empty(%p) = %d", vect, lisk_vector_empty(vect));
	}

	buf_len = vect->number * vect->length;
	if (IS_GE(pos, vect->number) ||
		IS_GT(pos + size, vect->number) ||
		IS_ZR(buf_len)) {
		RET_NL("@vect's data len = %u, vect->number %u, "
				"the elem pos your wanna to erase is %u",
				buf_len, vect->number, pos);
	}

	ret_buf	= nil;
	cpy_len	= (vect->number - (pos + size)) * vect->length;
	if (IS_GT(cpy_len, 0)) {
		dst = vect->data + pos * vect->length;
		src = dst + size * vect->length;
		ret = CPY(dst, src, cpy_len);
		if (IS_NZR(ret)) {
			RET_NL("CPY(%p, %p, %u) = %d", dst, src, cpy_len, ret);
		}
		ret_buf = dst;
	}
	vect->number -= size;

	RET_PTR(ret_buf, "");;
}

vp lisk_vector_pop_up(vp vector)
{
	lisk_vector_p	vect;

	vect = nil;
	if (IS_NL(vect = (lisk_vector_p)(vector))) {
		RET_NL("@vector = %p", vect);
	}

	if (IS_ZR(lisk_vector_empty(vect))) {
		RET_NL("lisk_vector_empty(%p) = %d", vect, lisk_vector_empty(vect));
	}

	return lisk_vector_erase(vector, 0, 1);
}

vp lisk_vector_pop_back(vp vector)
{
	lisk_vector_p	vect;

	vect = nil;
	if (IS_NL(vect = (lisk_vector_p)(vector))) {
		RET_NL("@vector = %p", vect);
	}

	if (IS_ZR(lisk_vector_empty(vect))) {
		RET_NL("@vect = %p, lisk_vector_empty(%p) = %d", vect, vect, lisk_vector_empty(vect));
	}

	return lisk_vector_erase(vector, vect->number - 1, 1);
}

n4 lisk_vector_reverse(vp vector)
{
	u4				lf;
	u4				rg;
	n1				buf[SIZE_K(1)];
	n1*				temp_buf;
	lisk_vector_p	vect;

	vect = nil;
	if (IS_NL(vect = (lisk_vector_p)(vector))) {
		RET_INT(-1, "@vector = %p", vect);
	}

	if (IS_ZR(vect->number)) {
		RET_INT(-1, "@vect->number = %u", vect->number);
	}

	temp_buf = buf;
	if (IS_GT(vect->length, sizeof(buf)) && IS_NL(ALLOC(n1*, temp_buf, vect->length)))	{
		RET_INT(-2, "@ALLOC(size = %u) = nil", vect->length);
	}

	lf = 0;
	rg = vect->number - 1;
	while (lf < rg) {
		CPY(temp_buf, vect->data + lf * vect->length, vect->length);
		CPY(
			vect->data + lf * vect->length,
			vect->data + rg * vect->length,
			vect->length
			);
		CPY(vect->data + rg * vect->length, temp_buf, vect->length);
		lf++;
		rg--;
	}

	if (IS_NEQ(temp_buf, buf))	{
		DEALLOC(temp_buf);
	}

	RET_INT(0, nil_str);
}

vd lisk_vector_clear(vp vector)
{
	lisk_vector_p	vect;

	vect = nil;
	if (IS_NL(vect = (lisk_vector_p)(vector))) {
		return;
	}

	vect->number = 0;
}

