/**
 * FILE		: lisk_skiplist.c
 * DESC		: skip list
 * AUTHOR	: v0.3 written by Alex Stocks
 * DATE		: on Oct 15, 2014
 * LICENCE	: GPL 2.0
 * MOD		:
 **/

#include <lisk_log.h>
#include <lisk_clock.h>
#include <lisk_skiplist.h>

#define	SKIP_LIST_MAX_LEVEL	32
#define	SKIP_LIST_PROB		0.25f

//#define	node_alloc(node_, level)	ALLOC(node_, sizeof(node_t) + (level) * sizeof(node_p*))
#define	node_alloc(node_, level)	\
	ALLOC(node_p, node_, sizeof(node_t) + (level) * sizeof(node_p*))

typedef struct node_tag {
	key_a				key;
	value_a				value;
	struct node_tag*	next[1];
} node_t, *node_p;

typedef struct skip_list_tag {
	// Maximum level of the skip list
	// 1 more than the number of levels in the skip list
	n4					level;
	compare				cmp;
	node_p				tail;
	node_p				header;
} skip_list_t, *skip_list_p;

static n4 get_node_level(vd);

n4 skip_list_init(vpp list_pptr, compare cmp)
{
	u4			idx;
	node_p		tail;
	skip_list_p	list;

	if (IS_NL(list_pptr) || IS_NL(cmp))	{
		RET_INT(-1, "@skip_list = nil, @cmp = nil");
	}
	*list_pptr = nil;

	if (IS_NL(node_alloc(tail, 0)))	{
		RET_INT(-2, "node_alloc(list -> tail) = nil");
	}
	SET_ZERO(tail, sizeof(*tail));
	pinfo("alloc list -> tail:%p", tail);

	//if (IS_NL(ALLOC(list, sizeof(*list))))	{
	if (IS_NL(ALLOC(skip_list_p, list, sizeof(*list))))	{
		DEALLOC(tail);
		RET_INT(-2, "ALLOC(size:%zu) = nil", sizeof(*list));
	}
	SET_ZERO(list, sizeof(*list));
	list -> tail = tail;
	pinfo("alloc list:%p", list);

	if (IS_NL(node_alloc(list -> header, SKIP_LIST_MAX_LEVEL)))	{
		DEALLOC(list -> tail);
		DEALLOC(list);
		RET_INT(-2, "node_alloc() = nil");
	}
	pinfo("alloc list -> header:%p", list -> header);
	SET_ZERO(list -> header, sizeof(*(list -> header)));
	for (idx = 0; IS_LT(idx, SKIP_LIST_MAX_LEVEL); idx++)	{
		list -> header -> next[idx] = list -> tail;
	}
	list -> cmp = cmp;

	srand((u4)(get_cur_time_sec()));
	*list_pptr = (vp)(list);

	RET_INT(0, nil_str);
};

vd skip_list_uninit(vpp list_pptr)
{
	node_p		node;
	skip_list_p	list;

	if (IS_NL(list_pptr)) {
		RET_VD("@lisk_pptr = nil");
	}

	if (IS_NL(list = (skip_list_p)(*list_pptr)))	{
		RET_VD("*list_pptr = nil");
	}

	while (node = list -> header, IS_NNL(node)) {
		list -> header = node -> next[0];
		if (IS_NNL(node -> key[0].data)) {
			DEALLOC(node -> key[0].data);
		}
		if (IS_NNL(node -> value[0].data)) {
			DEALLOC(node -> value[0].data);
		}
		pinfo("dealloc node:%p", node);
		DEALLOC(node);
	}
	 
	pinfo("dealloc list:%p", *list_pptr);
	DEALLOC(*list_pptr);
}

n4 get_node_level()
{
	n4			level = 0;
	static u4	top = (u4)(SKIP_LIST_PROB * 0xFFFF);

	while (IS_LT((u4)(rand() & 0xFFFF), top)) {
		level++;
	}

	if (IS_GT(level, SKIP_LIST_MAX_LEVEL))	{
		level = SKIP_LIST_MAX_LEVEL;
	}

	RET_INT(level, nil_str);
}

n4 skip_list_insert(vp list_ptr, key_p key, value_p value)
{
	n4			level;
	node_p		pre;
	node_p		cur;
	skip_list_p	list;
	node_p		update[SKIP_LIST_MAX_LEVEL];

	list = (skip_list_p)(list_ptr);
	if (IS_NL(list) || IS_NL(key) || IS_NL(value))	{
		RET_INT(-1, "@list = %p, @key = %p, @value = %p", list, key, value);
	}

	pre = list -> header;
	level = list -> level;
	do {
		while (
			cur = pre -> next[level],
			IS_NEQ(cur, list -> tail)
			&& IS_MN(list -> cmp(key, cur -> key))
			)
		{
			pre = cur;
		}

		update[level] = pre;
		level --;
	} while(IS_NMN(level));
	if (IS_NEQ(cur, list -> tail) && IS_ZR(list -> cmp(cur -> key, key))) {
		RET_INT(-3, nil_str);
	}

	level = get_node_level();
	if (IS_GT(level, list -> level)) {
		level = ++(list -> level);
		update[level] = list -> header;
	}

	if (IS_NL(node_alloc(cur, level))) {
		RET_INT(-2, "node_alloc(level:%d) = nil", level);
	}
	pinfo("alloc node:%p", cur);
	cur -> key[0].size = key -> size;
	cur -> value[0].size = value -> size;
	// if (IS_NL(ALLOC(cur -> key[0].data, cur -> key[0].size))) {
	if (IS_NL(ALLOC(n1*, cur -> key[0].data, cur -> key[0].size))) {
		DEALLOC(cur);
		RET_INT(-2, "ALLOC(cur -> key[0].data, size:%u) = nil", key -> size);
	}															
	CPY(cur -> key[0].data, key -> data, cur -> key[0].size); 
	// if (IS_NL(ALLOC(cur -> value[0].data, cur -> value[0].size))) {
	if (IS_NL(ALLOC(n1*, cur -> value[0].data, cur -> value[0].size))) {
		DEALLOC(cur -> key[0].data);
		DEALLOC(cur);
		RET_INT(-2, "ALLOC(cur -> value[0].data, size:%u) = nil", value -> size);
	}														 	
	CPY(cur -> value[0].data, value -> data, cur -> value[0].size);
	do {
		pre = update[level];
		cur -> next[level] = pre -> next[level];
		pre -> next[level] = cur;
		level --;
	} while(IS_NMN(level));

	RET_INT(0, nil_str);
}

n4 skip_list_update(vp list_ptr, key_p key, value_p value)
{
	n4			level;
	node_p		pre;
	node_p		cur;
	skip_list_p	list;
	node_p		update[SKIP_LIST_MAX_LEVEL];

	list = (skip_list_p)(list_ptr);
	if (IS_NL(list) || IS_NL(key) || IS_NL(value))	{
		RET_INT(-1, "@list = %p, @key = %p, @value = %p", list, key, value);
	}

	pre = list -> header;
	level = list -> level;
	do {
		while (
			cur = pre -> next[level],
			IS_NEQ(cur, list -> tail)
			&& IS_MN(list -> cmp(key, cur -> key))
			)
		{
			pre = cur;
		}

		update[level] = pre;
		level --;
	} while(IS_NMN(level));
	if (IS_NEQ(cur, list -> tail) && IS_ZR(list -> cmp(cur -> key, key))) {
		if (IS_GT(value -> size, cur -> value[0].size)) {
			vp data = NULL;
			// if (IS_NL(ALLOC(data, cur -> value[0].size))) {
			if (IS_NL(ALLOC(vp, data, cur -> value[0].size))) {
				RET_INT(-2, "ALLOC(node -> value -> data, size:%u) = nil", cur -> value[0].size);
			}
			DEALLOC(cur -> value[0].data);
			// cur -> value[0].data = data;
			cur -> value[0].data = (n1*)(data);
		}
		cur -> value[0].size = value -> size;
		CPY(cur -> value[0].data, value -> data, cur -> key[0].size);
		RET_INT(0, nil_str);
	}

	level = get_node_level();
	if (IS_GT(level, list -> level)) {
		level = ++(list -> level);
		update[level] = list -> header;
	}

	if (IS_NL(node_alloc(cur, level))) {
		RET_INT(-2, "node_alloc(level:%d) = nil", level);
	}
	cur -> key[0].size = key -> size;
	cur -> value[0].size = value -> size;
	// if (IS_NL(ALLOC(cur -> key[0].data, cur -> key[0].size))) {
	if (IS_NL(ALLOC(n1*, cur -> key[0].data, cur -> key[0].size))) {
		DEALLOC(cur);
		RET_INT(-2, "ALLOC(cur -> key[0].data, size:%u) = nil", key -> size);
	}															
	CPY(cur -> key[0].data, key -> data, cur -> key[0].size); 
	// if (IS_NL(ALLOC(cur -> value[0].data, cur -> value[0].size))) {
	if (IS_NL(ALLOC(n1*, cur -> value[0].data, cur -> value[0].size))) {
		DEALLOC(cur -> key[0].data);
		DEALLOC(cur);
		RET_INT(-2, "ALLOC(cur -> value[0].data, size:%u) = nil", value -> size);
	}														 	
	CPY(cur -> value[0].data, value -> data, cur -> value[0].size);
	do {
		pre = update[level];
		cur -> next[level] = pre -> next[level];
		pre -> next[level] = cur;
		level --;
	} while(IS_NMN(level));

	RET_INT(1, nil_str);
}

vd skip_list_erase(vp list_ptr, key_p key)
{
	n4			level;
	n4			top;
	node_p		pre;
	node_p		cur;
	skip_list_p	list;
	node_p		update[SKIP_LIST_MAX_LEVEL];

	list = (skip_list_p)(list_ptr);
	if (IS_NL(list) || IS_NL(key))	{
		RET_VD("@list = %p, @key = %p", list, key);
	}

	pre = list -> header;
	level = list -> level;
	do {
		while (
				cur = pre -> next[level],
				IS_NEQ(cur, list -> tail)
				&& IS_MN(list -> cmp(key, cur -> key))
			)
		{
			pre = cur;
		}
		update[level] = pre;
		level --;
	} while(IS_NMN(level));

	if (IS_NEQ(cur, list -> tail) && IS_ZR(list -> cmp(key, cur -> key)))	{
		level = 0;
		top = list -> level;
		while (
				pre = update[level],
				IS_LE(level, top)
				&& IS_EQ(pre -> next[level], cur)
				)
		{
			pre -> next[level] = cur -> next[level];
			level ++;
		}
		DEALLOC(cur -> key[0].data);
		DEALLOC(cur -> value[0].data);
		pinfo("dealloc node:%p", cur);
		DEALLOC(cur);

		while (IS_PN(top)
				&& IS_EQ(list -> header -> next[top],
						list -> tail))
		{
			top --;
		}
		list -> level = top;
	}
}

n4 skip_list_find(vp list_ptr, key_p key, value_p* value)
{
	n4			level;
	node_p		pre;
	node_p		cur;
	skip_list_p	list;

	list = (skip_list_p)(list_ptr);
	if (IS_NL(list) || IS_NL(key) || IS_NL(value))	{
		RET_INT(-1, "@list = %p, @key = %p, @value = %p", list, key, value);
	}
	*value = nil;

	pre = list -> header;
	level = list -> level;
	do {
		while (
				cur = pre -> next[level],
				IS_NEQ(cur, list -> tail)
				&& IS_MN(list -> cmp(key, cur -> key))
			)
		{
			pre = cur;
		}
		level --;
	} while(IS_NMN(level));

	if (IS_EQ(cur, list -> tail) || IS_NZR(list -> cmp(cur -> key, key))) {
		RET_INT(-2, nil_str);
	}
	*value = cur -> value;

	RET_INT(0, nil_str);
}
