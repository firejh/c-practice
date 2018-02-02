/**
 * FILE		:lisk_mul_hash.c
 * DESC		:multy hash inf
 * AUTHOR	:v1.0 written by Alex Stocks
 * DATE		:on Sep 5, 2012
 * MOD		:modified by * on June 2, 2012
 **/

#ifdef __cplusplus
extern "C" {
#endif

#include <lisk_public.h>
#include <lisk_mul_hash.h>
#include <lisk_hash.h>
#include <lisk_math.h>
#include <lisk_shm.h>
#include <string.h>

#ifdef __cplusplus
	}
#endif

#pragma pack(1)

typedef struct _MUL_HASH {
	n8		row;
	n8		col;
	u2		max_key_size;
	u4		max_value_size;
	compare	cmp;
	n8*		prime;
	n1*		data;
}mul_hash_t, *mul_hash_p, mul_hash_a[1];

#pragma pack()

#define	GET_MUL_HASH(shm)		(mul_hash_t*)((n1*)shm - sizeof(mul_hash_t))

static u8 get_data_key(n1* data, n8 len)
{
	if (IS_NL(data) || IS_ZR(len)) {
		return 0;
	}

	return bkdr64((n1*)data, len * sizeof(n1));
}

n4 mul_hash_init(vpp mul_hash,
				  n8 row, n8 col,
				  u2 max_key_size, u4 max_value_size,
				  n1* file_key,
				  compare cmp)	{
	n8 prime[MAX_HASH_ROW] = {0};
	n8 num = 0;
	n8 elem_num = 0;
	n4 idx = 0;
	u4 max_elem_size = sizeof(key_t) + sizeof(value_t) +  max_key_size + max_value_size;
	n8 shm_size  = 0;
	n8 total_size = 0;
	shm_t shm;
	n4 ret = 0;
	mul_hash_p mh = nil;

	if (IS_NL(mul_hash) || IS_NBT(row, 1, MAX_HASH_ROW) || IS_LT(col, 1) || IS_ZR(max_key_size) || IS_ZR(max_value_size) || IS_NL(cmp)) {
		return -1;
	}

	num = get_prime_arr(col, row, prime);
	if (num < (row)) {
		return -2;
	}

	for (idx = 0; idx < row; idx++) {
		elem_num += prime[idx];
	}

	shm_size = elem_num * max_elem_size * sizeof(n1);
	total_size = sizeof(shm_t) + sizeof(mul_hash_t) + sizeof(n8) * row + shm_size;	//shm_t + mul_hash_t + prime + data
	SET_ZERO(&shm, sizeof(shm_t));
	shm.size = total_size;

	ret = get_shm(&shm);
	//mh = (mul_hash_t*)malloc(total_size);
	if (IS_NZR(ret) || IS_NL(shm.ptr)) {
		return -2;
	}

	SET_ZERO(shm.ptr, shm.size);
	memcpy(shm.ptr, &shm, sizeof(shm_t));
	mh = (mul_hash_t*)((n1*)shm.ptr + sizeof(shm_t));
	mh->row = row;
	mh->col = col;
	mh->max_key_size = max_key_size;
	mh->max_value_size = max_value_size;
	mh->cmp = cmp;
	mh->prime = (n8*)((n1*)mh + sizeof(mul_hash_t));
	mh->data = (n1*)((n1*)mh->prime + sizeof(n8) * row);
	memcpy(mh->prime, prime, row * sizeof(n8));
	//for (idx = 0; idx < mh->row; idx++) {
	//	mh->prime[idx] = prime[idx];
	//}
	*mul_hash = (vp)(mh);

	return 0;
}

n4 mul_hash_free(vpp mul_hash)
{
	mul_hash_p mh = nil;
	shm_p shm_ptr = nil;

	if (IS_NNL(mul_hash) && IS_NNL(*mul_hash)) {
		mh = (mul_hash_p)(*mul_hash);
		shm_ptr = (shm_p)((n1*)mh - sizeof(shm_t));
		release_shm(shm_ptr);
		//free(mh);
		*mul_hash = nil;
	}

	return 0;
}

n4 mul_hash_insert(vp mul_hash, key_p key, value_p value)
{
	mul_hash_t* mh = nil;
	n8 idx = 0;
	u4 elem_size = 0;
	key_p hash_key_p = nil;
	value_p hash_value_p = nil;
	u8 hash_key = 0;
	u8 pos = 0;
	u8 start_pos = 0;
	n1* root = nil;

	if (IS_NL(mul_hash) || IS_NL(key) || IS_NL(value) ||
		IS_ZR(key->size) || IS_NL(key->data) || IS_ZR(value->size) || IS_NL(value->data))	{
		return -1;
	}

	mh = (mul_hash_p)(mul_hash);

	if (IS_GT(key->size, mh->max_key_size) || IS_GT(value->size, mh->max_value_size))	{
		return -1;
	}

	hash_key = get_data_key(key->data, key->size);
	elem_size = sizeof(key_t) + sizeof(value_t) + mh->max_key_size + mh->max_value_size;
	start_pos = 0;
	for (idx = 0; idx < mh->row; idx++) {
		pos = hash_key % mh->prime[idx];
		root = mh->data + (start_pos + pos) * elem_size;
		hash_key_p = (key_p)((n1*)root);
		hash_value_p = (value_p)((n1*)(root) + sizeof(*hash_key_p));
		if (IS_ZR(hash_key_p->size)) {
			hash_key_p->size = key->size;
			hash_value_p->size = value->size;
			hash_key_p->data = (n1*)(hash_value_p) + sizeof(*hash_value_p);
			memcpy(hash_key_p->data, key->data, key->size);
			hash_value_p->data = (n1*)(hash_key_p->data) + hash_key_p->size;
			memcpy(hash_value_p->data, value->data, value->size);
			break;
		}
		start_pos += mh->prime[idx];
	}

	if (IS_GE(idx, mh->row)) {
		return -2;
	}

	return 0;
}

n4 mul_hash_find(vp mul_hash, key_p key, value_p value)
{
	mul_hash_p mh = nil;
	n8 idx = 0;
	key_p hash_key_p = nil;
	value_p hash_value_p = nil;
	u4 elem_size = 0;
	u8 hash_key = 0;
	u8 start_pos = 0;
	u8 pos = 0;
	n1* root = nil;
	n4 ret = 0;

	if (IS_NL(mul_hash) || IS_NL(key) || IS_NL(value) || IS_ZR(key->size) || IS_NL(key->data))	{
		return -1;
	}

	mh = (mul_hash_p)(mul_hash);
	hash_key = get_data_key(key->data, key->size);
	elem_size = sizeof(key_t) + sizeof(value_t) + mh->max_key_size + mh->max_value_size;
	start_pos = 0;
	for (idx = 0; idx < mh->row; idx++) {
		pos = hash_key % mh->prime[idx];
		root = mh->data + (start_pos + pos) * elem_size;
		hash_key_p = (key_p)(root);
		hash_value_p = (value_p)((n1*)(hash_key_p) + sizeof(*hash_key_p));
		if (IS_NZR(hash_value_p->size) && IS_EQ(hash_key_p->size, key->size))	{
			ret = mh->cmp(hash_key_p, key);
			if (IS_ZR(ret))	{
				value->size = hash_value_p->size;
				value->data = hash_value_p->data;
				break;
			}
		}
		start_pos += mh->prime[idx];
	}

	if (IS_GE(idx, mh->row)) {
		return -2;
	}

	return 0;
}

n4 mul_hash_erase(vp mul_hash, key_p key)
{
	mul_hash_p mh = nil;
	n8 idx = 0;
	key_p hash_key_p = nil;
	u4 elem_size = 0;
	u8 hash_key = 0;
	u8 start_pos = 0;
	u8 pos = 0;
	n1* root = nil;
	n4 ret = 0;

	if (IS_NL(mul_hash) || IS_NL(key) || IS_ZR(key->size) || IS_NL(key->data))	{
		return -1;
	}

	mh = (mul_hash_p)(mul_hash);
	hash_key = get_data_key(key->data, key->size);
	elem_size = sizeof(key_t) + sizeof(value_t) + mh->max_key_size + mh->max_value_size;
	start_pos = 0;
	for (idx = 0; idx < mh->row; idx++) {
		pos = hash_key % mh->prime[idx];
		root = mh->data + (start_pos + pos) * elem_size;
		hash_key_p = (key_p)(root);
		if (IS_EQ(hash_key_p->size, key->size))	{
			ret = mh->cmp(hash_key_p, key);
			if (IS_ZR(ret))	{
				SET_ZERO(root, elem_size);
				break;
			}
		}
		start_pos += mh->prime[idx];
	}

	if (IS_GE(idx, mh->row)) {
		return -2;
	}

	return 0;
}

