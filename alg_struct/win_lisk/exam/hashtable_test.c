/**
 * FILE		: hashtable_test.c
 * DESC		: test hashtable
 * AUTHOR	: v1.0 written by Alex Stocks
 * DATE		: on Dec 13, 2012
 * LICENCE	: GPL 2.0
 * MOD		: modified by * on June 2, 2012
 **/

#ifdef __cplusplus
	extern "C" {
#endif

#include "lisk_hashtable.h"
#include <time.h>

#ifdef __cplusplus
	}
#endif

n4 cmp(vd* v0, vd* v1);

n4 main(n4 argc, n1** argv) {
	//init hashtable
	vd* hashtable = nil;
	n4 size = 5;
	n4 elem_num = 0;
	u4 start_time = 0;
	n4 ret = 0;

	u4 ins_elem_size = 0;
	u4 del_elem_size = 0;
	u4 free_elem_size = 0;
	u4 ins_ht_size = 0;
	u4 del_ht_size = 0;
	u4 free_ht_size = 0;
	n4 idx = 0;

	u8 key_value = 0;
	key_t ht_key;
	value_t ht_value;
	n1 buf[1024] = {0};
	value_p ht_value_ptr = nil;

	if (IS_GT(argc, 1)) {
		elem_num = (n4)strtoul(argv[1], (char**)nil, 10);
	}

	size = 3;
	start_time = (u4)clock();
	ret = hashtable_init(&hashtable, size, cmp);
	if (IS_NZR(ret)) {
		return -1;	
	}

	//insert
	elem_num = 1000000;
	for (idx = 0; IS_LT(idx, elem_num); idx++) {
		ht_key.size = sizeof(idx);
		ht_key.data = TYPE_N1_PTR(&idx);
		ht_value.size = sizeof(idx);
		ht_value.data = TYPE_N1_PTR(&idx);
		ret = hashtable_insert(hashtable, &ht_key, &ht_value);
		if (IS_NZR(ret)) {
			goto EXIT0;
		}
	}
	hashtable_memory_size(hashtable, &ins_ht_size);
	hashtable_size(hashtable, &ins_elem_size);
	printf("insert costs: time %u, size %u\n", clock() - start_time, ins_ht_size);
	start_time = (u4)clock();

	//find
	for (idx = 0; IS_LT(idx, elem_num); idx++) {
		ht_key.size = sizeof(idx);
		ht_key.data = TYPE_N1_PTR(&idx);
		ret = hashtable_find(hashtable, &ht_key, &ht_value_ptr);
		if (IS_ZR(ret)) {
			//printf("key: %d, value: %d \n", idx, *(int*)(ht_value_ptr->data));
		}
	}

	printf("find costs: time %u, size %u\n", clock() - start_time);
	start_time = (u4)clock();

	//erase
	for (idx = 0; IS_LT(idx, elem_num); idx++) {
		if (idx % 3 == 0) {
			continue;
		}
		ht_key.size = sizeof(idx);
		ht_key.data = TYPE_N1_PTR(&idx);
		ret = hashtable_erase(hashtable, &ht_key);
		if (IS_NZR(ret)) {
			printf("hashtable_erase(hashtable, %d) = %d\n", idx, ret);
			//printf("key: %llu, value: %d \n", key_value, *(int*)(ht_value_ptr->data));
		}
	}
	hashtable_memory_size(hashtable, &del_ht_size);
	hashtable_size(hashtable, &del_elem_size);
	printf("delete costs: time %u, size %u\n", clock() - start_time, del_ht_size);
	start_time = (u4)clock();

	//find
// 	for (idx = 0; IS_LT(idx, elem_num); idx++) {
// 		ht_key.size = sizeof(idx);
// 		ht_key.data = TYPE_N1_PTR(&idx);
// 		ret = hashtable_find(hashtable, &ht_key, &ht_value_ptr);
// 		if (IS_ZR(ret)) {
// 			//printf("key: %d, value: %d \n", idx, *(int*)(ht_value_ptr->data));
// 		}
// 	}
EXIT0:
	//free hashtable
	hashtable_free(&hashtable);
	hashtable_memory_size(hashtable, &free_ht_size);
	hashtable_size(hashtable, &free_elem_size);

	printf("free costs: time %u\n", clock() - start_time);
//	printf("hashtable memory size: ins %u, del %u, free %u\n", ins_ht_size, del_ht_size, free_ht_size);
	printf("hashtable element size: ins %u, del %u, free %u\n", ins_elem_size, del_elem_size, free_elem_size);

	return 0;
}

n4 cmp(vd* v0, vd* v1) {
	key_p key0 = (key_p)(v0);
	key_p key1 = (key_p)(v1);

	n4 uv0 = *TYPE_N4_PTR(key0->data);
	n4 uv1 = *TYPE_N4_PTR(key1->data);
	n4 ret = 1;

	if (IS_LT(uv0, uv1)) {
		ret = -1;
	} else if (IS_EQ(uv0, uv1)) {
		ret = 0;
	}

	return ret;
}
