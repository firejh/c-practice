/**
 * FILE		: skiplist_test.c
 * DESC		: test skip list interface
 * AUTHOR	: v1.0 written by Alex Stocks
 * DATE		: on Oct 16, 2014
 * LICENCE	: GPL 2.0
 * MOD		:
 **/

#include <lisk_log.h>
#include <lisk_skiplist.h>

n4 cmp(vd* v0, vd* v1);

n4	values[1700];

n4 main(int argc, char* argv[])
{
	n4				idx;
	n4				ret;
	n4				size;
	vp				list;
	key_a		key;
	value_a	value;
	value_p	value_ptr;

	ret = skip_list_init(&list, cmp);
	if (IS_NZR(ret))	{
		RET_INT(-1, "skip_list_init() = %d", ret);
	}

	size = (n4)ARRAY_SIZE(values);
	for (idx = 0; IS_LT(idx, size); idx ++) {
		key[0].size = sizeof(idx);
		// key[0].data = &idx;
		key[0].data = (n1*)(&idx);

		values[idx] = rand();
		value[0].size = sizeof(values[idx]);
		// value[0].data = values + idx;
		value[0].data = (n1*)(values + idx);

		ret = skip_list_insert(list, key, value);
		assert(IS_ZR(ret));
	}

	for (idx = 0; IS_LT(idx, size); idx ++) {
		key[0].size = sizeof(idx);
		// key[0].data = &idx;
		key[0].data = (n1*)(&idx);
		ret = skip_list_find(list, key, &value_ptr);
		assert(IS_ZR(ret));
		assert(IS_EQ(value_ptr -> size, sizeof(n4)));
		assert(IS_EQ(*(n4*)(value_ptr -> data), values[idx]));
		printf("%d : %d\n", idx, *(n4*)(value_ptr -> data));

		if (IS_ZR(idx % 3)) {
			key[0].size = sizeof(idx);
			// key[0].data = &idx;
			key[0].data = (n1*)(&idx);
			skip_list_erase(list, key);
		}
	}

	for (idx = 0; IS_LT(idx, size); idx ++) {
		// if (IS_NZR(idx % 3)) {
			key[0].size = sizeof(idx);
			// key[0].data = &idx;
			key[0].data = (n1*)(&idx);
			ret = skip_list_find(list, key, &value_ptr);
		if (IS_ZR(ret)) {	
			assert(IS_ZR(ret));
			assert(IS_EQ(value_ptr -> size, sizeof(n4)));
			assert(IS_EQ(*(n4*)(value_ptr -> data), values[idx]));
			printf("%d : %d\n", idx, *(n4*)(value_ptr -> data));
		}
	}

	for (idx = 0; IS_LT(idx, size); idx ++) {
		key[0].size = sizeof(idx);
		// key[0].data = &idx;
		key[0].data = (n1*)(&idx);
		skip_list_erase(list, key);
	}

	skip_list_uninit(&list);

	RET_INT(0, nil_str);
}

n4 cmp(vd* v0, vd* v1)
{
	n4			uv0;
	n4			uv1;
	n4			ret;
	key_p	key0;
	key_p	key1;

	key0 = (key_p)(v0);
	key1 = (key_p)(v1);
	assert(IS_EQ(key0 -> size, key1 -> size));
	assert(IS_EQ(key0 -> size, sizeof(n4)));

	ret = 1;
	uv0 = *(n4*)(key0 -> data);
	uv1 = *(n4*)(key1 -> data);
	if (IS_LT(uv0, uv1)) {
		ret = -1;
	} else if (IS_EQ(uv0, uv1)) {
		ret = 0;
	}

	RET_INT(ret, nil_str);
}

