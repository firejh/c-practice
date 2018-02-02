/**
 * FILE		: amazon_test.c
 * DESC		: to complete amazon interview test
 * AUTHOR	: v1.0 written by Alex Stocks
 * DATE		: on Nov 3, 2014
 * LICENCE	:
 * MOD		:
 **/

#include <lisk_log.h>
#include <lisk_map.h>
#include <lisk_string.h>
#include <lisk_vector.h>

#define	WILDCARD				'*'
#define ILLEGAL_CALSS_INDEX		0
#define MAX_CLASS_TREE_DEPTH	8

n4 is_class_same(vp, vp);

typedef struct class_info_tag	{
	lisk_string_a	name;
	vp				sub_class;	// map
} class_info_t, *class_info_p, class_info_a[1];

// just for test, so i do not check the return value here.!!!
vd class_info_init(class_info_p ci, n1* name);
vd class_info_uninit(class_info_p ci);

typedef struct class_tree_tag {
	lisk_vector_a	array;	
} class_tree_t, *class_tree_p, class_tree_a[1];

typedef struct class_routine_tag {
	u4	depth;
	u4	array[MAX_CLASS_TREE_DEPTH];
} class_routine_t, *class_routine_p, class_routine_a[1];

vd class_tree_init(class_tree_p ct);
vd class_tree_uninit(class_tree_p ct);
vd class_tree_insert(class_tree_p ct, n1* name, u4 parent_index);
vd class_tree_find(class_tree_p ct, class_routine_p routine, n1* sub_class_name, lisk_vector_p sub_class_routine);
vd class_tree_find2(class_tree_p ct, n1** sub_class_name, u4 size, lisk_vector_p sub_class_routine);
vd output_class_routine(class_tree_p ct, class_routine_p rouine);

#define		lisk_key_p		key_p
#define		lisk_key_t		key_t
#define		lisk_key_a		key_a

#define		lisk_value_p	value_p
#define		lisk_value_t	value_t
#define		lisk_value_a	value_a

n4 main(n4 argc, n1** argv)
{	
/**
 * 		super  // == java object
 *		/ \
 *   s1_a s1_b
 *        /   \
 *     s2_a    s2_b
 *		/ \	    /  \
 *  s3_a s3_b s3_c s3_d
 **/

	class_tree_a	tree;
	class_tree_init(tree);
	class_tree_insert(tree, "super", ILLEGAL_CALSS_INDEX);	//index, 1
	class_tree_insert(tree, "s1_a", 1);	//index, 2
	class_tree_insert(tree, "s1_b", 1);	//index, 3 
	class_tree_insert(tree, "s2_a", 3); //index, 4
	class_tree_insert(tree, "s2_b", 3); //index, 5
	class_tree_insert(tree, "s3_a", 4); //index, 6
	class_tree_insert(tree, "s3_b", 4); //index, 7
	class_tree_insert(tree, "s3_c", 5); //index, 8
	class_tree_insert(tree, "s3_d", 5); //index, 4

	printf("test case1: super -> s1_a\n");
	do {
		u4  idx;
		n1* sub_class_name[1] = {"s1_a"};
		lisk_vector_a  class_rouine;
		class_routine_a super_routine;
		
		super_routine[0].array[0] = 1;
		super_routine[0].depth = 1;

		lisk_vector_init(class_rouine, 1, sizeof(class_routine_t));
		class_tree_find(tree, super_routine, sub_class_name[0], class_rouine);
		for (idx = 0; IS_LT(idx, class_rouine[0].number); idx++) {
			class_routine_p routine = (class_routine_p)lisk_vector_at((vp)(class_rouine), idx);
			output_class_routine(tree, routine);
		}
		
		lisk_vector_uninit(class_rouine);
	} while (0);

	printf("\ntest case2: super -> *\n");
	do {
		u4  idx;
		n1* sub_class_name[] = {"*"};
		lisk_vector_a  class_rouine;

		lisk_vector_init(class_rouine, 1, sizeof(class_routine_t));
		class_tree_find2(tree, sub_class_name, ARRAY_SIZE(sub_class_name), class_rouine);
		for (idx = 0; IS_LT(idx, class_rouine[0].number); idx++) {
			class_routine_p routine = (class_routine_p)lisk_vector_at((vp)(class_rouine), idx);
			output_class_routine(tree, routine);
		}

		lisk_vector_uninit(class_rouine);
	} while (0);

	printf("\ntest case3: super -> * -> s2_b \n");
	do {
		u4  idx;
		n1* sub_class_name[] = {"*", "s2_b"};
		lisk_vector_a  class_rouine;

		lisk_vector_init(class_rouine, 1, sizeof(class_routine_t));
		class_tree_find2(tree, sub_class_name, ARRAY_SIZE(sub_class_name), class_rouine);
		for (idx = 0; IS_LT(idx, class_rouine[0].number); idx++) {
			class_routine_p routine = (class_routine_p)lisk_vector_at((vp)(class_rouine), idx);
			output_class_routine(tree, routine);
		}

		lisk_vector_uninit(class_rouine);
	} while (0);

	printf("\ntest case4: super -> * -> s2_b -> * \n");
	do {
		u4  idx;
		n1* sub_class_name[] = {"*", "s2_b", "*"};
		lisk_vector_a  class_rouine;

		lisk_vector_init(class_rouine, 1, sizeof(class_routine_t));
		class_tree_find2(tree, sub_class_name, ARRAY_SIZE(sub_class_name), class_rouine);
		for (idx = 0; IS_LT(idx, class_rouine[0].number); idx++) {
			class_routine_p routine = (class_routine_p)lisk_vector_at((vp)(class_rouine), idx);
			output_class_routine(tree, routine);
		}

		lisk_vector_uninit(class_rouine);
	} while (0);

	class_tree_uninit(tree);

	RET_INT(0, nil_str);
}

//////////////////////////////////////////////////////////////////////////
// class info
//////////////////////////////////////////////////////////////////////////

n4 is_class_same(vp ptr1, vp ptr2)
{
	n4			ret;
	lisk_key_p	class1;
	lisk_key_p	class2;

	class1 = (lisk_key_p)(ptr1);
	assert(IS_NNL(class1));
	class2 = (lisk_key_p)(ptr2);
	assert(IS_NNL(class2));

	ret = -1;
	/*
	if (
		(class1->size == 1)
		&&
		(((n1*)(class1->data))[0] == WILDCARD)
	)
	{
		ret = 0;
	}
	else if (
		(class2->size == 1)
		&&
		(((n1*)(class2->data))[0] == WILDCARD)
	)
	{
		ret = 0;
	}
	else 
	*/
	if (
		(class1->size == class2->size)
		&&
		(IS_ZR(CMP(class1->data, class2->data, class1->size)))
	)
	{
		ret = 0;
	}

	RET_INT(ret, nil_str);
}

vd class_info_init(class_info_p ci, n1* name)
{
	assert(IS_NNL(ci));
	lisk_string_init_copy(ci->name, name, strlen(name));
	map_init((vpp)(&(ci->sub_class)), (compare)(is_class_same));
}

vd class_info_uninit(class_info_p ci)
{
	assert(IS_NNL(ci));
	lisk_string_uninit(ci->name);
	map_uninit((vpp)(&(ci->sub_class)));
}

//////////////////////////////////////////////////////////////////////////
// class tree
//////////////////////////////////////////////////////////////////////////

vd class_tree_init(class_tree_p ct)
{
	assert(IS_NNL(ct));
	lisk_vector_init(ct->array, 32, sizeof(class_info_t));
}

vd class_tree_uninit(class_tree_p ct)
{
	u4				idx;
	class_info_p	ci;

	assert(IS_NNL(ct));
	for (idx = 0; IS_LT(idx, ct->array[0].number); idx++) {
		ci = (class_info_p)lisk_vector_at(ct->array, idx);
		class_info_uninit(ci);
	}
	
	lisk_vector_uninit(ct->array);
}

vd class_tree_insert(class_tree_p ct, n1* name, u4 parent_index)
{
	u4				index;
	class_info_t	ci;
	class_info_p	parent;
	lisk_key_a		key;
	lisk_value_a	value;

	assert(IS_NNL(ct));
	assert(IS_NNL(name));
	assert(IS_NMN(parent_index));
	assert(IS_GE(ct->array[0].number, parent_index));

    class_info_init(&ci, name);
	lisk_vector_push_back(ct->array, &ci, 1);
	
	if (IS_EQ(parent_index, ILLEGAL_CALSS_INDEX)) {
		RET_VD(nil_str);
	}
	
	index = ct->array[0].number;
	key[0].size = strlen(name);
	key[0].data = name;
	value[0].size = sizeof(index);
	value[0].data = (n1*)(&index);
	parent = (class_info_p)lisk_vector_at(ct->array, parent_index - 1);
	map_insert(parent->sub_class, key, value);
}

vd class_tree_find(class_tree_p ct, class_routine_p routine, n1* sub_class_name, lisk_vector_p sub_class_array)
{
	n4				ret;
	u4				index;
	class_info_p	ci;
	class_routine_a sub_class_routine;

	assert(IS_NNL(ct));
	assert(IS_NNL(routine));
	assert(IS_LT(routine->depth, MAX_CLASS_TREE_DEPTH));
	assert(IS_NNL(sub_class_name));
	assert(IS_NMN(sub_class_array));

	CPY(sub_class_routine, routine, sizeof(sub_class_routine));
	sub_class_routine[0].depth++;
	index = routine->array[routine->depth - 1];
	ci = (class_info_p)lisk_vector_at(ct->array, index - 1);
	if (IS_EQ(1, strlen(sub_class_name)) && IS_EQ(WILDCARD, sub_class_name[0]))	 {
		MAP_FOR_EACH_BEGIN(ci->sub_class, it)
			// index = *(u4*)(((lisk_value_p)(it.second->data))->data);
			index = *(u4*)(it.second->data);
			sub_class_routine[0].array[sub_class_routine[0].depth - 1] = index;
			lisk_vector_push_back(sub_class_array, sub_class_routine, 1);
		MAP_FOR_EACH_END
	} else {
		lisk_key_a		key;
		lisk_value_p	value;
		key[0].size = strlen(sub_class_name);
		key[0].data = sub_class_name;
		ret = map_find(ci->sub_class, key, &value);
		if (IS_ZR(ret)){
			index = *(u4*)(value->data);
			sub_class_routine[0].array[sub_class_routine[0].depth - 1] = index;
			lisk_vector_push_back(sub_class_array, sub_class_routine, 1);
		}
	}
}

vd class_tree_find2(class_tree_p ct, n1** sub_class_name, u4 size, lisk_vector_p sub_class_routine)
{
	u4	idx_i;
	u4	idx_j;
	class_routine_a super_routine;

	assert(IS_NNL(ct));
	assert(IS_NNL(sub_class_name));
	assert(IS_NZR(size));
	assert(IS_NNL(sub_class_routine));

	super_routine[0].array[0] = 1;
	super_routine[0].depth = 1;

	class_tree_find(ct, super_routine, sub_class_name[0], sub_class_routine);
	for (idx_i = 1; IS_LT(idx_i, size); idx_i++) {
		lisk_vector_a  class_rouine;
		lisk_vector_init_copy(class_rouine, sub_class_routine->data, sub_class_routine->number, sub_class_routine->length);
		lisk_vector_clear(sub_class_routine);

		for (idx_j = 0; IS_LT(idx_j, class_rouine->number); idx_j++) {
			class_tree_find(ct, (class_routine_p)lisk_vector_at((vp)(class_rouine), idx_j), sub_class_name[idx_i], sub_class_routine);
		}

		lisk_vector_uninit(class_rouine);
	}
}

vd output_class_routine(class_tree_p ct, class_routine_p rouine)
{
	u4				idx;
	u4				index;
	class_info_p	ci;

	assert(IS_NNL(ct));
	assert(IS_NNL(rouine));

	for (idx = 0; IS_LT(idx, rouine->depth); idx++) {
		index = rouine->array[idx];
		ci = (class_info_p)lisk_vector_at(ct->array, index - 1);
		printf("%s -> ", ci->name->data);
	}
	printf("nil\n");
}