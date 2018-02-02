/**
 * FILE		: lisk_map.c
 * DESC		: map interface
 * AUTHOR	: v1.0 written by Alex Stocks
 * DATE		: on Dec 15, 2012
 * LICENCE	: GPL 2.0
 * MOD		: modified by * on June 2, 2012
 **/

#ifdef __cplusplus
	extern "C" {
#endif

#include <lisk_public.h>
#include <lisk_map.h>
#include <stdio.h>

#if	(defined(__linux__))
#include "lisk_mem_pool.h"			//POOL_WIN_POOL_ALLOC, POOL_WIN_POOL_DEALLOC
#endif

#ifdef __cplusplus
	}
#endif

#if	(defined(__linux__))
#if	(defined(POOL_ALLOC))
#define	WIN_POOL_ALLOC(t, p, s)		POOL_ALLOC(p, s)
#define	WIN_POOL_DEALLOC			POOL_DEALLOC
#else
#define	WIN_POOL_ALLOC(t, p, s)		ALLOC(p, s)
#define	WIN_POOL_DEALLOC			DEALLOC
#endif
#elif	(defined(_WIN32) || defined(_WIN64))
#define	WIN_POOL_ALLOC				ALLOC
#define	WIN_POOL_DEALLOC			DEALLOC
#endif

typedef	key_t						rb_key_t;
typedef	key_p						rb_key_p;
typedef	key_a						rb_key_a;
typedef	value_t						rb_value_t;
typedef	value_p						rb_value_p;
typedef	value_a						rb_value_a;

typedef enum color_tag {
	RED = 0,
	BLACK = 1,
} color_t;


#pragma pack(1)

typedef struct rb_node_tag {
	uw						parent;
	struct rb_node_tag*		left;
	struct rb_node_tag*		right;
	rb_key_t				key;
	rb_value_t				value;
} rb_node_t, *rb_node_p, rb_node_a[1];

typedef struct rb_root_tag {
	rb_node_p					node;
	rb_node_p					min;
	rb_node_p					max;
	compare						cmp;
	u8							size;						//node number
} rb_root_t, *rb_root_p, rb_root_a[1];

#pragma pack()

#define	rb_parent(node)				((rb_node_p)((node)->parent & ~BLACK))
#define rb_color(node)				((node)->parent & BLACK)
#define	color_set_black(node)		((node)->parent |= BLACK)
#define	color_set_red(node)			((node)->parent &= ~BLACK)
#define	color_is_black(node)		(rb_color(node) == BLACK)
#define	color_is_red(node)			(rb_color(node) == RED)

static vd rb_set_parent(rb_node_p node, rb_node_p parent)
{
	node->parent = (node->parent & 3) | *(uw*)(&parent);
}

static vd rb_set_color(rb_node_p node, n4 color)
{
	node->parent = (node->parent & ~BLACK) | color;
}

static n4 rb_init(rb_root_p* root, compare cmp)
{
	rb_root_p rt = nil;
	if (IS_NL(root) || IS_NL(cmp)) {
		return -1;
	}

	WIN_POOL_ALLOC(rb_root_p, rt, sizeof(rb_root_t));
	if (IS_NL(rt)) {
		return -2;
	}
	rt->node = nil;
	rt->cmp = cmp;
	rt->size = 0;
	rt->min = nil;
	rt->max = nil;

	*root = rt;

	return 0;
}

static n4 rb_update_min(rb_root_p root)
{
	rb_node_p node = nil;
	if (IS_NL(root) || IS_NL(root->node))	{
		root->min = nil;
		return -1;
	}

	node = root->node;
	while (IS_NNL(node->left))	{
		node = node->left;
	}
	root->min = node;

	return 0;
}

static n4 rb_update_max(rb_root_p root)
{
	rb_node_p node = nil;
	if (IS_NL(root) || IS_NL(root->node))	{
		root->max = nil;
		return -1;
	}

	node = root->node;
	while (IS_NNL(node->right))	{
		node = node->right;
	}
	root->max = node;

	return 0;
}

/**
 * 		node				right
 *		/ \			==>		/  \
 *     a  right			  node	y
 *        /  \			  / \
 *       b    y			 a  b
 **/
static rb_root_p rb_rotate_left(rb_root_p root, rb_node_p node)
{
	rb_node_p right = node->right;
	if (IS_NL(right)) {
		return root;
	}

	//node->right, right->left
	node->right = right->left;
	if (IS_NNL(node->right)) {
		rb_set_parent(node->right, node);
	}
	right->left = node;

	//node->parent, right->parent
	rb_set_parent(right, rb_parent(node));
	rb_set_parent(node, right);
	if (IS_NZR(rb_parent(right))) {
		if (IS_EQ(rb_parent(right)->left, node)) {
			rb_parent(right)->left = right;;
		} else {
			rb_parent(right)->right = right;
		}
	}
	else {
		root->node = right;
	}

	return root;
}

/**
 *		node					left
 *		/  \					/  \
 *	  left  c		==>		   a  node
 *	  /  \						  /  \
 *	 a   b						 b    c
 **/
static rb_root_p rb_rotate_right(rb_root_p root, rb_node_p node)
{
	rb_node_p left = node->left;
	if (IS_NL(left)) {
		return root;
	}

	//node->left, left->right
	node->left = left->right;
	if (IS_NNL(node->left)) {
		rb_set_parent(node->left, node);
	}
	left->right = node;

	//node->parent, left->parent
	rb_set_parent(left, rb_parent(node));
	rb_set_parent(node, left);
	if (IS_NZR(rb_parent(left))) {
		if (IS_EQ(rb_parent(left)->left, node)) {
			rb_parent(left)->left = left;
		} else {
			rb_parent(left)->right = left;
		}
	} else {
		root->node = left;
	}

	return root;
}

static rb_node_p rb_create_node(rb_key_p key, rb_value_p value)
{
	rb_node_p node = nil;
	if (IS_NL(key) || IS_NL(value)) {
		return nil;
	}

	WIN_POOL_ALLOC(rb_node_p, node, sizeof(rb_node_t));
	if (IS_NL(node)) {
		return nil;
	}
	//its color is red
	SET_ZERO(node, sizeof(rb_node_t));

	WIN_POOL_ALLOC(n1*, node->key.data, key->size);
	WIN_POOL_ALLOC(n1*, node->value.data, value->size);
	if (IS_NL(node->key.data) || IS_NL(node->value.data)) {
		WIN_POOL_DEALLOC(node->key.data);
		WIN_POOL_DEALLOC(node->key.data);
		WIN_POOL_DEALLOC(node);
		return nil;
	}
	node->key.size = key->size;
	CPY(node->key.data, key->data, node->key.size);
	node->value.size = value->size;
	CPY(node->value.data, value->data, node->value.size);

	return node;
}

vd rb_free_node(rb_node_p node)
{
	if (IS_NNL(node)) {
		WIN_POOL_DEALLOC(node->key.data);
		WIN_POOL_DEALLOC(node->value.data);
		WIN_POOL_DEALLOC(node);
	}
}

vd rb_free_node_recursion(rb_node_p node)
{
	if (IS_NNL(node)) {
		rb_free_node_recursion(node->left);
		rb_free_node_recursion(node->right);
		rb_free_node(node);
	}
}

vd rb_free(rb_root_p root)
{
	if (IS_NNL(root)) {
		rb_free_node_recursion(root->node);
		WIN_POOL_DEALLOC(root);
	}
}

static rb_node_p rb_search(rb_root_p root, rb_key_p key, rb_node_p* parent)
{
	rb_node_p node = root->node;
	rb_node_p pre = nil;
	compare cmp = root->cmp;

	n4 ret = 0;
	while (IS_NNL(node)) {
		ret = cmp((vp)key, (vp)&(node->key));
		if (IS_ZR(ret)) {
			return node;
		}

		pre = node;
		if (IS_MN(ret)) {
			node = node->left;
		} else {
			node = node->right;
		}
	}
	if (IS_NNL(parent)) {
		*parent = pre;
	}

	return nil;
}

rb_value_p rb_find(rb_root_p root, rb_key_p key)
{
	rb_node_p node = nil;
	if (IS_NL(root)) {
		return nil;
	}

	node = rb_search(root, key, nil);
	if (IS_NNL(node)) {
		return &(node->value);
	}

	return nil;
}

static rb_root_p rb_insert_rebalance(rb_root_p root, rb_node_p node)
{
	rb_node_p parent = nil;

	rb_node_p gparent = nil;
	rb_node_p uncle = nil;
	rb_node_p tmp = nil;
	while (IS_NNL(parent = rb_parent(node)) && color_is_red(parent)) {
		gparent = rb_parent(parent);
		if (IS_EQ(parent, gparent->left)) {
			uncle = gparent->right;
			if (IS_NNL(uncle) && color_is_red(uncle)) {
				color_set_black(uncle);
				color_set_black(parent);
				color_set_red(gparent);
				node = gparent;
			} else {
				if (IS_EQ(node, parent->right)) {
					rb_rotate_left(root, parent);
					tmp = parent;
					parent = node;
					node = tmp;
				}

				color_set_black(parent);
				color_set_red(gparent);
				root = rb_rotate_right(root, gparent);
			}
		} else {
			uncle = gparent->left;
			if (IS_NNL(uncle) && color_is_red(uncle)) {
				color_set_black(uncle);
				color_set_black(parent);
				color_set_red(gparent);
				node = gparent;
			} else {
				if (IS_EQ(node, parent->left)) {
					rb_rotate_right(root, parent);
					tmp = parent;
					parent = node;
					node = tmp;
				}

				color_set_black(parent);
				color_set_red(gparent);
				root = rb_rotate_left(root, gparent);
			}
		}
	}

	color_set_black(root->node);
	return root;
}

static rb_root_p rb_insert(rb_root_p root, rb_key_p key, rb_value_p value)
{
	rb_node_p parent = nil;
	rb_node_p node = nil;
	n4 ret = 0;

	if (IS_NL(root) || IS_NL(key) || IS_NL(value)) {
		return nil;
	}

	if (IS_LT(root->size + 1, root->size)) {
		return nil;
	}

	node = rb_search(root, key, &parent);
	if (IS_NNL(node)) {
		return root;
	}

	node = rb_create_node(key, value);
	if (IS_NL(node)) {
		return nil;
	}

	if (IS_NNL(parent)) {
		ret = root->cmp((vp)key, (vp)&(parent->key));
		if (IS_MN(ret)) {
			parent->left = node;
		} else {
			parent->right = node;
		}
		rb_set_parent(node, parent);
	} else {
		root->node = node;
	}
	root->size++;

	root = rb_insert_rebalance(root, node);
	//rb_update_min(root);
	//rb_update_max(root);
	if (IS_NL(root->min))	{
		root->min = node;
	} else	{
		ret = root->cmp((vp)key, (vp)&(root->min->key));
		if (IS_MN(ret)) {
			root->min = node;
		}
	}

	if (IS_NL(root->max))	{
		root->max = node;
	} else {
		ret = root->cmp((vp)key, (vp)&(root->max->key));
		if (IS_PN(ret)) {
			root->max = node;
		}
	}

	return root;
}

static rb_value_p rb_update(rb_root_p root, rb_key_p key, rb_value_p value)
{
	rb_node_p node = nil;
	if (IS_NL(root) || IS_NL(key) || IS_NL(value)) {
		return nil;
	}

	node = rb_search(root, key, nil);
	if (IS_NL(node)) {
		return nil;
	}

	if (IS_GT(value->size, node->value.size)) {
		n1* data = nil;
		WIN_POOL_ALLOC(n1*, data, value->size);
		if (IS_NL(data)) {
			return nil;
		}
		WIN_POOL_DEALLOC(node->value.data);
		node->value.data = data;
	}
	CPY(node->value.data, value->data, value->size);
	node->value.size = value->size;

	return &(node->value);
}

static rb_root_p rb_erase_rebalance(rb_root_p root, rb_node_p parent, rb_node_p node)
{
	rb_node_p sibling = nil;
	while ((IS_NL(node) || color_is_black(node)) && IS_NEQ(root->node, node)) {
		if (IS_EQ(node, parent->left)) {
			sibling = parent->right;
			//sibling is red
			if (color_is_red(sibling)) {
				color_set_black(sibling);
				color_set_red(parent);
				root = rb_rotate_left(root, parent);
				sibling = parent->right;
			}
			//sibling is black, sb->left is black, sb->right is black
			if ((IS_NL(sibling->left) || color_is_black(sibling->left)) &&
				(IS_NL(sibling->right) || color_is_black(sibling->right))) {
				color_set_red(sibling);
				node = parent;
				parent = rb_parent(node);
			} else {
				//sibling is black, sb->left is red, sb->right is black
				if (IS_NL(sibling->right) || color_is_black(sibling->right)) {
					if (IS_NNL(sibling->left)) {
						color_set_black(sibling->left);
					}
					color_set_red(sibling);
					root = rb_rotate_right(root, sibling);
					sibling = parent->right;
				}
				//sibling is black, sb->left is red, sb->right is red
				//sibling is black, sb->left is black, sb->right is red
				rb_set_color(sibling, rb_color(parent));
				color_set_black(parent);
				if (IS_NNL(sibling->right)) {
					color_set_black(sibling->right);
				}
				root = rb_rotate_left(root, parent);
				node = root->node;
				break;
			}
		} else {
			sibling = parent->left;
			if (color_is_red(sibling)) {
				color_set_black(sibling);
				color_set_red(parent);
				root = rb_rotate_right(root, parent);
				sibling = parent->left;
			}
			if ((IS_NL(sibling->left) || color_is_black(sibling->left)) &&
				(IS_NL(sibling->right) || color_is_black(sibling->right))) {
				color_set_red(sibling);
				node = parent;
				parent = rb_parent(node);
			} else {
				if (IS_NL(sibling->left) || color_is_black(sibling->left)) {
					if (IS_NNL(sibling->right)) {
						color_set_black(sibling->right);
					}
					color_set_red(sibling);
					root = rb_rotate_left(root, sibling);
					sibling = parent->left;
				}
				rb_set_color(sibling, rb_color(parent));
				color_set_black(parent);
				if (IS_NNL(sibling->left)) {
					color_set_black(sibling->left);
				}
				root = rb_rotate_right(root, parent);
				node = root->node;
				break;
			}
		}
	}

	if (IS_NNL(node)) {
 		color_set_black(node);
	}

	return root;
}

/**
 * description	: delete a element whose value is @value from map @map.
 *				  the difference between @map_erase and @map_erase_ex is the latter don't
 *				  need to search element @value by its key. so it is more quickly. <br/><br/>
 *
 * in-out@map	: map object
 * in-@key		: the value of erasing element
 *
 * out-@ret		: if delete successful, ret is 0; otherwise -1 when the params are illegal
 * 				  or -2 when fail to find that element.
 **/
static rb_root_p rb_erase_ex(rb_root_p root, rb_value_p value)
{
	rb_node_p node = nil;
	rb_node_p parent = nil;
	rb_node_p child = nil;
	rb_node_p scapedog = nil;
	rb_node_p tmp = nil;
	n4 black_flag = 0;
	n4 min_update_flag = 0;
	n4 max_update_flag = 0;

	if (IS_NL(root) || IS_NL(root->node) || IS_NL(value)) {
		return root;
	}

	if (IS_ZR(root->size)) {
		return nil;
	}

#if (defined(__linux__))
	node = mem_of_st(value, rb_node_t, value);
#elif (defined(_WIN32) || defined(__WIN64))
	node = (rb_node_p)((n1*)(value) - (sizeof(rb_node_t) - sizeof(rb_value_t)));
#endif
	if (IS_NNL(node->left) && IS_NNL(node->right)) {
		tmp = node->right;
		while (IS_NNL(tmp->left)) {
			tmp = tmp->left;
		}
		scapedog = tmp;
		parent = rb_parent(node);
		if (IS_NNL(parent)) {
			if (IS_EQ(parent->left, node)) {
				parent->left = scapedog;
			} else {
				parent->right = scapedog;
			}
		} else {
			root->node = scapedog;
		}

 		child = scapedog->right;
 		if (color_is_black(scapedog)) {
 			black_flag = 1;
 		}
 		parent = rb_parent(scapedog);
		if (IS_EQ(parent, node)) {
			parent = scapedog;
		} else {
			if (IS_NNL(child)) {
				rb_set_parent(child, parent);
			}
			parent->left = child;
			scapedog->right = node->right;
			rb_set_parent(node->right, scapedog);
		}
		scapedog->parent = node->parent;
		scapedog->left = node->left;
		rb_set_parent(node->left, scapedog);
	} else {
		if (IS_NNL(node->left)) {
			child = node->left;
		} else if (IS_NNL(node->right)) {
			child = node->right;
		}

		if (color_is_black(node)) {
			black_flag = 1;
		}
		parent = rb_parent(node);

		if (IS_NNL(child)) {
			rb_set_parent(child, parent);
		}
		if (IS_NNL(parent)) {
			if (IS_EQ(parent->left, node)) {
				parent->left = child;
			} else {
				parent->right = child;
			}
		} else {
			root->node = child;
		}
	}

	min_update_flag = 0;
	if (IS_EQ(root->min, node))	{
		min_update_flag = 1;
	}

	max_update_flag = 0;
	if (IS_EQ(root->max, node))	{
		max_update_flag = 1;
	}

	rb_free_node(node);
	root->size--;

	if (IS_NZR(black_flag)) {
		root = rb_erase_rebalance(root, parent, child);
	}
	if (IS_NZR(min_update_flag))	{
		rb_update_min(root);
	}
	if (IS_NZR(max_update_flag))	{
		rb_update_max(root);
	}

	return root;
}

rb_root_p rb_erase(rb_root_p root, rb_key_p key)
{
	rb_node_p node = nil;
	if (IS_NL(root) || IS_NL(root->node) || IS_NL(key)) {
		return nil;
	}

	if (IS_ZR(root->size)) {
		return nil;
	}

	node = rb_search(root, key, nil);
	if (IS_NL(node)) {
		return root;
	}

	return rb_erase_ex(root, &(node->value));
}

rb_root_p rb_begin(rb_root_p root, rb_node_p* begin)
{
	if (IS_NL(root) || IS_NL(root->node) || IS_NL(begin)) {
		return nil;
	}

	*begin = root->min;
	//CPY(begin, root->min, SIZE(*begin));

	return root;
}

rb_root_p rb_end(rb_root_p root, rb_node_p end)
{
	if (IS_NL(root) || IS_NL(root->node) || IS_NL(end)) {
		return nil;
	}

	SET_ZERO(end, sizeof(*end));

	return root;
}

rb_root_p rb_node_next(rb_root_p root, rb_node_p node, rb_node_p* next_node)
{
	rb_node_p parent = rb_parent(node);
	rb_node_p next = node->right;
	if (IS_NL(root) || IS_NL(root->node) || IS_NL(node) || IS_NL(next_node)) {
		return nil;
	}

	if (IS_EQ(node, root->max))	{
		*next_node = nil;
		return root;
	}

	if (IS_NNL(next)) {
		while (IS_NNL(next->left)) {
			next = next->left;
		}
	}	else if (IS_EQ(parent->left, node)) {
		next = parent;
	}	else	{
		while (IS_NNL(parent) && IS_EQ(node, parent->right))	{
			node = parent;
			parent = rb_parent(node);
		}
		if (IS_NL(parent))	{
			return nil;
		}
		next = parent;
	}
	*next_node = next;

	return root;
}

rb_root_p rb_node_next_ex(rb_root_p root, rb_value_p value, rb_node_p* next_node)
{
	rb_node_p node = nil;
	if (IS_NL(root) || IS_NL(root->node) || IS_NL(value) || IS_NL(next_node)) {
		return nil;
	}

#if (defined(__linux__))
	node = mem_of_st(value, rb_node_t, value);
#elif (defined(_WIN32) || defined(__WIN64))
	node = (rb_node_p)((n1*)(value) - (sizeof(rb_node_t) - sizeof(rb_value_t)));
#endif

	return rb_node_next(root, node, next_node);
}

rb_root_p rb_rbegin(rb_root_p root, rb_node_p* rbegin)
{
	if (IS_NL(root) || IS_NL(root->node) || IS_NL(rbegin)) {
		return nil;
	}

	*rbegin = root->max;

	return root;
}

rb_root_p rb_rend(rb_root_p root, rb_node_p rend)
{
	if (IS_NL(root) || IS_NL(root->node) || IS_NL(rend)) {
		return nil;
	}

	SET_ZERO(rend, sizeof(*rend));

	return root;
}

rb_root_p rb_node_rnext(rb_root_p root, rb_node_p node, rb_node_p* next_node)
{
	rb_node_p parent = rb_parent(node);
	rb_node_p rnext = node->left;
	if (IS_NL(root) || IS_NL(root->node) || IS_NL(node) || IS_NL(next_node)) {
		return nil;
	}

	if (IS_EQ(node, root->min))	{
		*next_node = nil;
		return root;
	}

	if (IS_NNL(rnext)) {
		while (IS_NNL(rnext->right)) {
			rnext = rnext->right;
		}
	}	else if (IS_EQ(parent->right, node)) {
		rnext = parent;
	}	else	{
		while (IS_NNL(parent) && IS_EQ(node, parent->left))	{
			node = parent;
			parent = rb_parent(node);
		}
		if (IS_NL(parent))	{
			return nil;
		}
		rnext = parent;
	}
	*next_node = rnext;

	return root;
}

rb_root_p rb_node_rnext_ex(rb_root_p root, rb_value_p value, rb_node_p* next_node)
{
	rb_node_p node = nil;
	if (IS_NL(root) || IS_NL(root->node) || IS_NL(value) || IS_NL(next_node)) {
		return nil;
	}

#if (defined(__linux__))
	node = mem_of_st(value, rb_node_t, value);
#elif (defined(_WIN32) || defined(__WIN64))
	node = (rb_node_p)((n1*)(value) - (sizeof(rb_node_t) - sizeof(rb_value_t)));
#endif

	return rb_node_rnext(root, node, next_node);
}

static vd rb_node_display(rb_node_p node)
{
	if (IS_NL(node)) {
		printf("<nil>");
		return;
	}

	//printf("parent %p, left %p, right %p, self %p,", rb_parent(node), node->left, node->right, node);
	if (color_is_black(node)) {
		//printf("color: black,");
		printf("<b,");
	} else {
		//printf("color: red,");
		printf("<r,");
	}
	//printf("key %d, value %d\n", *(n4*)(node->key.data), *(n4*)(node->value.data));
	printf(" %d, %d>", *(n4*)(node->key.data), *(n4*)(node->value.data));
	//rb_node_display(node->left);
	//rb_node_display(node->right);
}

static vd rb_display_all(rb_node_p node)
{
	if (IS_NL(node)) {
		return;
	}

	printf("parent: ");
	rb_node_display(rb_parent(node));
	printf(", self: ");
	rb_node_display(node);
	printf(", left: ");
	rb_node_display(node->left);
	printf(", right: ");
	rb_node_display(node->right);
	printf("\n");
	rb_display_all(node->left);
	rb_display_all(node->right);
}

static vd rb_display(rb_root_p root)
{
	if (IS_NL(root)) {
		return;
	}

	//rb_node_display(root->node);
	rb_display_all(root->node);
}

n4 map_init(vpp map, compare cmp)
{
	return rb_init((rb_root_p*)map, cmp);
}

vd map_uninit(vpp map)
{
	if (IS_NNL(map))	{
		rb_free((rb_root_p)*map);
		*map = nil;
	}
}

n4 map_insert(vp map, key_p key, value_p value)
{
	rb_root_p root = rb_insert((rb_root_p)map, (rb_key_p)key, (rb_value_p)value);
	if (IS_NL(root)) {
		return -1;
	}

	return 0;
}

n4 map_update(vp map, key_p key, value_p value)
{
	rb_value_p new_value = rb_update((rb_root_p)map, key, (rb_value_p)value);
	if (IS_NL(new_value)) {
		return -1;
	}

	return 0;
}

n4 map_erase(vp map, key_p key)
{
	rb_root_p root = rb_erase((rb_root_p)map, (rb_key_p)key);
	if (IS_NL(root)) {
		return -1;
	}

	return 0;
}

n4 map_erase_ex(vp map, value_p value)
{
	rb_root_p root = rb_erase_ex((rb_root_p)map, (rb_value_p)value);
	if (IS_NL(root)) {
		return -1;
	}

	return 0;
}

n4 map_find(vp map, key_p key, value_p* value)
{
	rb_value_p vl = nil;
	if (IS_NL(value)) {
		return -1;
	}

	vl = rb_find((rb_root_p)map, (rb_key_p)key);
	if (IS_NNL(vl)) {
		*value = (value_p)vl;
		return 0;
	}

	return -2;
}

vd map_display(vp map)
{
	rb_display((rb_root_p)(map));
}

n4 map_node_size(rb_node_p node, u4* size)
{
	if (IS_NL(node) || IS_NL(size)) {
		return -1;
	}

	*size += sizeof(rb_node_t) + node->key.size + node->value.size;
	map_node_size(node->left, size);
	map_node_size(node->right, size);

	return 0;
}

n4 map_mem_size(vp map, u4* size)
{
	rb_root_p root = nil;
	if (IS_NL(map) || IS_NL(size)) {
		return -1;
	}

	root = (rb_root_p)(map);
	*size = sizeof(*root);
	map_node_size(root->node, size);

	return 0;
}

n4 map_size(vp map, u8* size)
{
	rb_root_p root = nil;
	if (IS_NL(map) || IS_NL(size)) {
		return -1;
	}

	root = (rb_root_p)(map);
	*size = root->size;

	return 0;
}

n4 map_begin(vp map, iterator_p it)
{
	rb_root_p root = nil;
	rb_node_p node = nil;
	if (IS_NL(map) || IS_NL(it)) {
		return -1;
	}

	root = (rb_root_p)(map);
	//SET_ZERO(&node, SIZE(node));
	root = rb_begin(root, &node);
	if (IS_NL(root)) {
		return -2;
	}
	it->first = (key_p)(&(node->key));
	it->second = (value_p)(&(node->value));

	return 0;
}

n4 map_end(vp map, iterator_p it)
{
	rb_root_p root = nil;
	rb_node_t node;
	if (IS_NL(map) || IS_NL(it)) {
		return -1;
	}

	root = (rb_root_p)(map);
	SET_ZERO(&node, sizeof(node));
	root = rb_end(root, &node);
	if (IS_NL(root)) {
		return -2;
	}
	it->first = (key_p)(&(node.key));
	it->second = (value_p)(&(node.value));

	return 0;
}

n4 map_is_end(vp map, iterator_p it)
{
	if (IS_NL(map) || IS_NL(it)) {
		return -1;
	}

	if (IS_NZR(it->first) || IS_NZR(it->second)) {
		return -2;
	}

	return 0;
}

n4 map_next(vp map, iterator_p it)
{
	rb_root_p root = nil;
	rb_value_p node = nil;
	rb_node_p next = nil;
	if (IS_NL(map) || IS_NL(it)) {
		return -1;
	}

	root = (rb_root_p)(map);
	node = (rb_value_p)(it->second);
	//SET_ZERO(&next, SIZE(next));
	root = rb_node_next_ex(root, node, &next);
	if (IS_NL(root)) {
		return -2;
	}
	if (IS_NL(next))	{
		SET_ZERO(it, sizeof(*it));
	} else	{
		it->first = (key_p)(&(next->key));
		it->second = (value_p)(&(next->value));
	}

	return 0;
}

n4 map_rbegin(vp map, iterator_p it)
{
	rb_root_p root = nil;
	rb_node_p node = nil;
	if (IS_NL(map) || IS_NL(it)) {
		return -1;
	}

	root = (rb_root_p)(map);
	//SET_ZERO(&node, SIZE(node));
	root = rb_rbegin(root, &node);
	if (IS_NL(root)) {
		return -2;
	}
	it->first = (key_p)(&(node->key));
	it->second = (value_p)(&(node->value));

	return 0;
}

n4 map_rend(vp map, iterator_p it)
{
	rb_root_p root = nil;
	rb_node_t node;
	if (IS_NL(map) || IS_NL(it)) {
		return -1;
	}

	root = (rb_root_p)(map);
	SET_ZERO(&node, sizeof(node));
	root = rb_rend(root, &node);
	if (IS_NL(root)) {
		return -2;
	}
	it->first = (key_p)(&(node.key));
	it->second = (value_p)(&(node.value));

	return 0;
}

n4 map_is_rend(vp map, iterator_p it)
{
	if (IS_NL(map) || IS_NL(it)) {
		return -1;
	}

	if (IS_NZR(it->first) || IS_NZR(it->second)) {
		return -2;
	}

	return 0;
}

n4 map_rnext(vp map, iterator_p it)
{
	rb_root_p root = nil;
	rb_value_p node = nil;
	rb_node_p next = nil;
	if (IS_NL(map) || IS_NL(it)) {
		return -1;
	}

	root = (rb_root_p)(map);
	node = (rb_value_p)(it->second);
	SET_ZERO(&next, sizeof(next));
	root = rb_node_rnext_ex(root, node, &next);
	if (IS_NL(root)) {
		return -2;
	}
	if (IS_NL(next))	{
		SET_ZERO(it, sizeof(*it));
	} else	{
		it->first = (key_p)(&(next->key));
		it->second = (value_p)(&(next->value));
	}

	return 0;
}

