/**
 * FILE		: lisk_list.c
 * DESC		: list
 * AUTHOR	: v0.3.0 written by Alex Stocks
 * DATE		: on Oct 12, 2012
 * MOD		: modified by * on June 2, 2012
 **/

#include <lisk_list.h>
#include <lisk_log.h>

//////////////////////////////////////////
// double linked list
//////////////////////////////////////////
#define	LIST_NEXT(node)			((list_node_p)((node)->next))
#define	LIST_PREV(node)			((list_node_p)((node)->prev))

#define	PREV_MAGIC				0X20131203205112ABUL
#define	NEXT_MAGIC				0XBA21150230213102UL

#pragma pack(1)

typedef struct list_node_tag {
	vp	prev;
	vp	next;
} list_node_t, *list_node_p, list_node_a[1];

#pragma pack()

list_node_p list_node(vp elem)
{
	list_node_p	_node;

	_node = (list_node_p)((n1*)(elem) - sizeof(list_node_t));

	RET_PTR(_node, nil_str);
}

vp list_elem(vp node_ptr)
{
	n1*	_elem;

	_elem = (n1*)(node_ptr) + sizeof(list_node_t);

	RET_PTR(_elem, nil_str);
}

vp list_prev_elem(vp elem)
{
	vp			prev_elem;
	list_node_p	node_ptr;

	prev_elem	= nil;
	node_ptr	= list_node(elem);
	if (IS_NNL(node_ptr)) {
		prev_elem = list_elem(node_ptr->prev);
	}

	RET_PTR(prev_elem, nil_str);
}

vp list_next_elem(vp elem)
{
	vp			next_elem;
	list_node_p	node_ptr;

	next_elem	= nil;
	node_ptr	= list_node(elem);
	if (IS_NNL(node_ptr)) {
		next_elem = list_elem(node_ptr->next);
	}

	RET_PTR(next_elem, nil_str);
}

vp list_rprev_elem(vp elem)
{
	RET_PTR(list_next_elem(elem), nil_str);
}

vp list_rnext_elem(vp elem)
{
	RET_PTR(list_prev_elem(elem), nil_str);
}

vp list_first(vp list)
{
	n4			ret;
	vp			front;
	list_node_p	head;

	ret		= list_empty(list);
	if (IS_ZR(ret)) {
		RET_NL("list_empty(@list %p) = ret %d", list, ret);
	}

	front	= nil;
	head	= list_node(list);
	if (IS_NNL(head)) {
		front = list_elem(head->next);
	}

	RET_PTR(front, nil_str);
}

vp list_last(vp list)
{
	n4			ret;
	vp			last;
	list_node_p	head;

	ret		= list_empty(list);
	if (IS_ZR(ret)) {
		RET_NL("list_empty(@list %p) = ret %d", list, ret);
	}

	last = nil;
	head = list_node(list);
	if (IS_NNL(head)) {
		last = list_elem(head->prev);
	}

	RET_PTR(last, nil_str);
}

/**
 * description	: allocate a node and its content is
 *				  copied from to element @elem<br/><br/>
 *
 * in-@elem		: element pointer
 * in-@len		: element length
 *
 * out-@ret		: if successful, ret is a list elem;
 *				  otherwise nil instead.
 **/
vp list_node_alloc(vp elem, n4 elem_len)
{
	n4	len;
	vp	node;
	vp	elem_ptr;

	// if (IS_ZR(elem_len)) {
	// 	RET_NL("@elem_len = %d", elem_len);
	// }

	len = sizeof(list_node_t) + elem_len;
	if (IS_NNL(ALLOC(vp, node, len))) {
		((list_node_p)node)->prev	= node;
		((list_node_p)node)->next	= node;
		elem_ptr = list_elem(node);
		if (IS_NNL(elem)) {
			CPY(elem_ptr, elem, elem_len);
		}
	}

	return node;
}

vd list_node_dealloc(vp node)
{
	DEALLOC(node);
}

vd list_elem_dealloc(vp elem)
{
	list_node_dealloc(list_node(elem));
}

n4 list_check_magic(vp elem)
{
	n4			ret;
	list_node_p	node;

	ret = -1;
	node = list_node(elem);
	if (IS_EQ(PTR_2_NUM(node->prev), ((uw)(PREV_MAGIC))) &&
		IS_EQ(PTR_2_NUM(node->next), ((uw)(NEXT_MAGIC))))	{
		ret = 0;
	}

	RET_INT(ret, nil_str);
}

n4 list_init(vpp list_pptr)
{
	n4				len;
	list_node_p		list;

	if (IS_NL(list_pptr))	{
		RET_INT(-1, "@list = %p", list_pptr);
	}

	len = sizeof(uw);
	list = (list_node_p)list_node_alloc(nil, len);
	if (IS_NL(list)) {
		RET_INT(-2, "list_node_alloc(nil, %d) = %p", len, list);
	}
	list -> prev	= list;
	list -> next	= list;

	*list_pptr	= list_elem(list);

	RET_INT(0, nil_str);
}

vp list_insert_front(vp cur, vp elem, n4 elem_len)
{
	n4			flag;
	n4			ret;
	list_node_p	node;
	list_node_p	cur_node;
	list_node_p	prev_node;

	if (IS_NL(cur) || IS_NL(elem) || IS_NPN(elem_len)) {
		RET_NL("@cur = %p, @elem = %p, @elem_len = %d.",
				cur, elem, elem_len);
	}

	flag = -1;
	ret = list_check_magic(elem);
	if (IS_ZR(ret))	{
		node = list_node(elem);
		if (IS_NL(node))	{
			RET_NL("list_node(elem = %p) = %p", elem, node);
		}
		flag = 0;
	} else {
		node = list_node_alloc(elem, elem_len);
		if (IS_NL(node)) {
			RET_NL("list_node_alloc(elem = %p, len = %d) = %p",
					elem, elem_len, node);
		}
	}

	cur_node = (list_node_p)(list_node(cur));
	if (IS_NL(cur_node)) {
		if (IS_ZR(flag))	{
			list_node_dealloc(node);
		}
		RET_NL("list_node(%p) = %p", cur, cur_node);
	}

	prev_node		= (list_node_p)(cur_node->prev);
	node->next		= cur_node;
	node->prev		= prev_node;
	prev_node->next	= node;
	cur_node->prev	= node;

	RET_PTR(list_elem(node), nil_str);
}

vp list_insert_back(vp cur, vp elem, n4 elem_len)
{
	n4			flag;
	n4			ret;
	list_node_p	node;
	list_node_p	cur_node;
	list_node_p	next_node;

	if (IS_NL(cur) || IS_NL(elem) || IS_ZR(elem_len)) {
		RET_NL("@cur = %p, @elem = %p, @elem_len = %d.",
				cur, elem, elem_len);
	}

	flag = -1;
	ret = list_check_magic(elem);
	if (IS_ZR(ret))	{
		node = list_node(elem);
		if (IS_NL(node))	{
			RET_NL("list_node(elem = %p) = %p", elem, node);
		}
		flag = 0;
	} else {
		node = list_node_alloc(elem, elem_len);
		if (IS_NL(node)) {
			RET_NL("list_node_alloc(elem = %p, len = %d) = %p",
					elem, elem_len, node);
		}
	}

	cur_node = (list_node_p)(list_node(cur));
	if (IS_NL(cur_node)) {
		if (IS_ZR(flag))	{
			list_node_dealloc(node);
		}
		RET_NL("list_node(%p) = %p", cur, cur_node);
	}

	next_node		= (list_node_p)(cur_node->next);
	node->next		= next_node;
	node->prev		= cur_node;
	next_node->prev	= node;
	cur_node->next	= node;

	RET_PTR(list_elem(node), nil_str);
}

n4 list_move_front(vp dst_elem, vp src_elem)
{
	list_node_p		src_node;
	list_node_p		dst_node;

	if (IS_NL(src_elem) || IS_NL(dst_elem))	{
		RET_INT(-1, "@src_elem = %p, @dst_elem = %p",
					src_elem, dst_elem);
	}

	src_node = list_node(src_elem);
	dst_node = list_node(dst_elem);
	if (IS_NL(src_node) || IS_NL(dst_node))	{
		RET_INT(-1, "@src_node = %p, @dst_node = %p",
					src_node, dst_node);
	}

	//delete source node from its current list
	LIST_PREV(src_node)->next	= src_node->next;
	LIST_NEXT(src_node)->prev	= src_node->prev;
	src_node->prev				= nil;
	src_node->next				= nil;

	//insert source node before the destination node
	src_node->next				= dst_node;
	src_node->prev				= dst_node->prev;
	LIST_PREV(dst_node)->next	= src_node;
	dst_node->prev				= src_node;

	RET_INT(0, nil_str);
}

n4 list_move_back(vp dst_elem, vp src_elem)
{
	list_node_p		src_node;
	list_node_p		dst_node;

	if (IS_NL(src_elem) || IS_NL(dst_elem))	{
		RET_INT(-1, "@src_elem = %p, @dst_elem = %p",
					src_elem, dst_elem);
	}

	src_node	= list_node(src_elem);
	dst_node	= list_node(dst_elem);

	//delete source node from its current list
	LIST_PREV(src_node)->next	= src_node->next;
	LIST_NEXT(src_node)->prev	= src_node->prev;
	src_node->prev				= nil;
	src_node->next				= nil;

	//insert source node after the destination node
	src_node->next				= dst_node->next;
	src_node->prev				= dst_node;
	LIST_NEXT(dst_node)->prev	= src_node;
	dst_node->next				= src_node;

	RET_INT(0, nil_str);
}

vp list_push_front(vp list, vp elem, n4 elem_len)
{
	RET_PTR(list_insert_back(list, elem, elem_len), nil_str);
}

vp list_push_back(vp list, vp elem, n4 elem_len)
{
	RET_PTR(list_insert_front(list, elem, elem_len), nil_str);
}

vp list_erase(vp list, vp elem, n4 flag)
{
	n4			ret;
	vp			pret;
	list_node_p	node;
	list_node_p	prev;
	list_node_p	next;

	if (IS_NL(elem)) {
		RET_NL("@elem = %p", elem);
	}

	node = list_node(elem);
	if (IS_NL(node)) {
		RET_NL("list_node(&elem = %p) = %p", elem, node);
	}
	if (IS_EQ(list, elem)) {
		list_node_dealloc(node);
		RET_NL(nil_str);
	}

	pret = nil;
	ret = list_check_magic(elem);
	if (IS_NZR(ret))	{
		// @node is not a magic node
		prev = node->prev;
		next = node->next;

		if (IS_NNL(prev))	{
			prev->next = next;
			pret = list_elem(prev);
		}
		if (IS_NNL(next))	{
			next->prev = prev;
		}
		node->next = NUM_2_PTR(NEXT_MAGIC);
		node->prev = NUM_2_PTR(PREV_MAGIC);
	}

	if (IS_EQ(flag, ELIST_ERASE_FULL))	{
		list_elem_dealloc(elem);
	}

	RET_PTR(pret, nil_str);
}

vd list_clear(vp list)
{
	n4	flag;
	vp	head;
	vp	node;
	vp	prev;

	if (IS_NL(head = list)) {
		RET_VD("@list = %p", list);
	}

	flag = ELIST_ERASE_FULL;
	node = list_next_elem(head);
	while (IS_NNL(node) && IS_NEQ(head, node)) {
		prev = list_erase(head, node, flag);
		node = list_next_elem(prev);
	}
}

vd list_uninit(vpp list)
{
	n4	flag;
	vp	head;

	if (IS_NL(list) || IS_NL(head = *list)) {
		RET_VD("@list = %p, *@list = %p", list, head);
	}

	list_clear(head);

	flag = ELIST_ERASE_FULL;
	list_erase(head, head, flag);

	*list= nil;
}

n4 list_size(vp list)
{
	n4			size;
	list_node_p	head;

	size = 0;
	if (IS_NL(list)) {
		RET_INT(size, "@list = %p", list);
	}

	head = list_node(list);
	if (IS_NL(head)) {
		RET_INT(size, "list_node(%p) = %p", list, head);
	}
	size = 0;
	LIST_FOR_EACH_BEGIN(list, n1*, _node)
		size++;
	LIST_FOR_EACH_END

	RET_INT(size, nil_str);
}

n4 list_empty(vp list)
{
	list_node_p	head;
	vp			node;

	if (IS_NL(list)) {
		RET_INT(0, "@list = %p", list);
	}

	head = list_node(list);
	if (IS_NL(head)) {
		RET_INT(0, "list_node(%p) = %p",list, head);
	}

	node = head->next;
	if (IS_EQ(node, head)) {
		RET_INT(0, nil_str);
	}

	RET_INT(-1, nil_str);
}

vd list_print(vp list)
{
	list_node_p	head;
	list_node_p	node;

	head = list_node(list);
	node = head;
	do {
		printf("%p -> ", node);
		node = node->next;
	} while(IS_NEQ(node, head));
	printf("\n");
}

