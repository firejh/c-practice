/** 
  FILE		:group_feeds_attr.c
 * DESC		:to store group feeds attribution
 * AUTHOR:	:v1.0 written by Alex Stocks
 * DATE		:on Sep 18, 2012
 * MOD		:modified by * on June 2, 2012
 * **/

#include <lisk_mul_hash.h>
#include <lisk_math.h>
#include <string.h>

#pragma pack(1)

typedef struct gfeeds_attr_tag {
	unsigned int group_code;
	unsigned int feeds_num;
} gfeeds_attr_t, *gfeeds_attr_p, gfeeds_attr_a[1];

#pragma pack()

int  compare_key(vp key0, vp key1);
vp garr_init(int row, int col, int max_key_size, int max_value_size);
vd garr_free(vpp arr);
vd garr_ins(vp arr, int group_num);
vd garr_output(vp arr, int group_num);

int main(int argc, char** argv) {
	int row = 0;
	int col = 0;
	int key_size = 0;
	int value_size = 0;
	vp garr = nil;
	int group_num = 0;

	row = 3;
	col = 10;
	key_size = sizeof(unsigned int);
	value_size = sizeof(gfeeds_attr_t);
	garr = garr_init(row, col, key_size, value_size);
	if (IS_NL(garr))	{
		return -1;
	}

	group_num = row * col;
	garr_ins(garr, group_num);
	garr_output(garr, group_num);
	garr_free(&garr);

	return 0;
}

int  compare_key(vp key0, vp key1)	{
	key_p first_key = nil;
	key_p second_key = nil;
	int ret = 0;
	unsigned int group0 = 0;
	unsigned int group1 = 0;

	if (IS_NL(key0) || IS_NL(key1))	{
		return -2;
	}

	first_key = (key_p)(key0);
	second_key = (key_p)(key1);
	if (IS_ZR(first_key->size) || IS_ZR(second_key->size) || IS_NL(first_key->data) ||
		IS_NL(second_key->data) || IS_NEQ(first_key->size, second_key->size))	{
		return -2;
	}

	group0 = *TYPE_U4_PTR(first_key->data);
	group1 = *TYPE_U4_PTR(second_key->data);
	if (IS_LT(group0, group1))	{
		ret = -1;
	} else if (IS_EQ(group0, group1))	{
		ret = 0;
	} else	{
		ret = 1;
	}

	return ret;
}

vp garr_init(int row, int col, int max_key_size, int max_value_size) {
	int ret = 0;
	vp mul_hash = nil;

	ret = mul_hash_init(&mul_hash, row, col, max_key_size, max_value_size, nil, compare_key);
	if (IS_NZR(ret)) {
		return nil;
	}

	return mul_hash;
}

vd garr_ins(vp arr, int group_num) {
	gfeeds_attr_t elem;
	key_t key;
	value_t value;
	int fail_num = 0;
	int ret = 0;
	int idx = 0;

	for (idx = 0; idx < group_num; idx++) {
		elem.group_code = idx;
		elem.feeds_num = idx+1;

		key.size = sizeof(idx);
		key.data = (n1*)(&idx);
		value.size = sizeof(elem);
		value.data = (n1*)(&elem);
		ret = mul_hash_insert(arr, &key, &value);
		if (IS_NZR(ret)) {
			fail_num++;
			fprintf(stderr, "idx = %d, mul_hash_ins() failed, ret = %d. \n", idx, ret);
		}
	}
	printf("all group num %d, fail num %d, utilition %f.\n", group_num, fail_num, (float)(group_num - fail_num) / group_num);
}

vd garr_free(vpp arr) {
	mul_hash_free(arr);
}

vd garr_output(vp arr, int group_num) {
	key_t key;
	value_t value;
	gfeeds_attr_p elem = nil;
	int cur_group_num = 0;
	int ret = 0;
	int idx = 0;
	int fail_num = 0;

	fail_num = 0;
	for (idx = 0; idx < group_num; idx++) {
		key.size = sizeof(idx);
		key.data = (n1*)(&idx);
		ret = mul_hash_find(arr, &key, &value);
		if (IS_NZR(ret))	{
			fail_num++;
			fprintf(stderr, "idx = %d, mul_hash_find() = %d\n", ret);
			continue;
		}
		elem = (gfeeds_attr_p)(value.data);
		printf("group = %d, feeds num = %d\n", elem->group_code, elem->feeds_num);
	}
	printf("all group num %d, fail num %d, utilition %f.\n", group_num, fail_num, (float)(group_num - fail_num) / group_num);
}

