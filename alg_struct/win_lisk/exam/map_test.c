/**
 * FILE		: map_test.c
 * DESC		: test map infterface
 * AUTHOR	: v1.0 written by Alex Stocks
 * DATE		: on Dec 17, 2012
 * LICENCE	: GPL 2.0
 * MOD		: modified by * on June 2, 2012
 **/

#ifdef __cplusplus
extern "C" {
#endif

#include "lisk_public.h"
#include "lisk_type.h"
#include "lisk_map.h"
#include <stdio.h>

#ifdef __cplusplus
}
#endif

vd test1(int);
vd test2(vd);
int cmp(vp u, vp v);
vd map_show(vpp map);
vd map_show2(vp map);


int main(int argc, char** argv) {
	int elem_num = 512;
//	if (1 < argc) {
//		elem_num = str2n4(argv[1]);
//	}

	test1(elem_num);
	//test2(elem_num);

	return 0;
}

vd test1(int max_num) {
	int idx = 0;
	key_t key;
	value_t value;
	unsigned long long int ins_size = 0;
	unsigned long long int del_size = 0;
	unsigned long long int free_size = 0;
	vp map = NULL;
	int ret = 0;
	value_p value_ptr = NULL;

	map_init(&map, cmp);
	if (ret < 0) {
		printf("map_init() = %d\n", ret);
		return;
	}

	for (idx = 0; idx < max_num; idx++) {
		key.size = sizeof(idx);
		key.data = (char*)(&idx);
		value.size = sizeof(idx);
		value.data = (char*)(&idx);
		ret = map_insert(map, &key, &value);
		//if (ret != 0) {
		//	printf("insert %d failed. ret = %d\n", idx, ret);
		//}
		//map_display(map);
		//printf("\n");

		//ret = map_find(map, &key, &value_ptr);
		//printf("map_find(key %d, value %d) = %d\n", idx, *(int*)(value_ptr->data), ret);
	}
	map_size(map, &ins_size);

	//show
	printf("after insert:\n");
	map_show(map);
	printf("reverse show:");
	map_show2(map);

	//erase
	for (idx = 0; idx < max_num; idx++) {
		if (idx % 2 == 0) {
			continue;
		}
		key.size = sizeof(idx);
		key.data = (char*)(&idx);
		ret = map_erase(map, &key);
		if (ret != 0) {
			printf("map_erase(key = %d) = %d\n", idx, ret);
		}
		//printf("after delete key %d\n", idx);
		//map_display(map);
	}
	map_size(map, &del_size);

	//show
	printf("after erase:\n");
	map_show(map);
	printf("reverse show:");
	map_show2(map);

	//find
	for (idx = 0; idx < max_num; idx++) {
		key.size = sizeof(idx);
		key.data = (char*)(&idx);
		ret = map_find(map, &key, &value_ptr);
		if (ret == 0) {
			printf("map_find(key %d, value %d) = %d\n", idx, *(int*)(value_ptr->data), ret);
		}
	}

	map_free(&map);
	map_size(map, &free_size);
	printf("map memory size: ins %llu, del %llu, free %llu\n", ins_size, del_size, free_size);
}

int cmp(vp u, vp v) {
	key_p u_key = (key_p)(u);
	key_p v_key = (key_p)(v);
	int* value0 = (int*)(u_key->data);
	int* value1 = (int*)(v_key->data);
	int ret = 0;
	if (*value0 < *value1) {
		ret = -1;
	} else if (*value0 > *value1) {
		ret = 1;
	}

	return ret;
}

vd test2(int max_num) {
	int idx = 0;
	key_t key;
	value_t value;
	int data = 0;
	vp _map = NULL;
	int ret = 0;
	value_p value_ptr = NULL;

	if (max_num <= 0)	{
		printf("max_num = %d\n", max_num);
		return;
	}

	ret = map_init(&_map, cmp);
	if (ret < 0) {
		printf("map_init() = %d\n", ret);
		return;
	}

	for (idx = 0; idx < max_num; idx++) {
		//data = (int)(rand_uint() % max_num);
		//key.size = sizeof(data);
		//key.data = (char*)(&data);
		key.size = sizeof(idx);
		key.data = (char*)(&idx);
		value.size = sizeof(idx);
		value.data = (char*)(&idx);
		ret = map_find(_map, &key, &value_ptr);
		if (ret == 0) {
			printf("key %d has been inserted into map!\n", data);
			continue;
		}

		ret = map_insert(_map, &key, &value);
		if (ret == 0) {
			printf("[i = %d] insert key %d success!\n", idx, idx);
		} else	{
			printf("[i = %d] insert key %d error!\n", idx, idx);
			exit(-1);
		}

		ret = map_find(_map, &key, &value_ptr);
		if (ret == 0) {
			if (idx != *(int*)(value_ptr->data)) {
				printf("elem in map: %d --- %d <-- right: %d --- %d\n", data, idx, *(int*)(key.data), *(int*)(value_ptr->data));
			} else {
				//printf("[i = %d] search key %d success!\n", node->data, node->key);
				//printf("[idx = %d] search key %d success! <-- X\n", *(int*)(value_ptr->data), *(int*)(key.data));
			}
		} else {
			printf("[idx = %d] search key %d error!\n", idx, *(int*)(key.data));
			exit(-1);
		}
		if (0 == (idx % 2)) {
			ret = map_erase(_map, &key);
			if (ret == 0)			{
				//printf("[idx = %d] erase key %d success\n", idx,  *(int*)(key.data));
			} else	{
				printf("[idx = %d] erase key %d error\n", idx,  *(int*)(key.data));
			}
		}
	}
	
	printf("traverse map:\n");
	map_show(_map);
	printf("recverse traverse map:\n");
	map_show2(_map);

	map_free(&_map);
}

vd map_show(vp map)	{
	//show
	int idx = 0;
	int ret = 0;
	iterator_t it;

	ret = map_begin(map, &it);
	while (IS_NZR(map_is_end(map, &it)))	{
		printf("<%d--%d--%d>\n", idx, *(int*)(it.first->data), *(int*)(it.second->data));
		idx++;
		map_next(map, &it);
	}
}

vd map_show2(vp map)	{
	//show
	int idx = 0;
	int ret = 0;
	iterator_t it;

	ret = map_rbegin(map, &it);
	while (IS_NZR(map_is_rend(map, &it)))	{
		printf("<%d--%d--%d>\n", idx, *(int*)(it.first->data), *(int*)(it.second->data));
		idx++;
		map_rnext(map, &it);
	}
}

