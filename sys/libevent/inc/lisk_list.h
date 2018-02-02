/**
 * FILE		: lisk_list.h
 * DESC		: list
 * AUTHOR	: v0.3.0 written by Alex Stocks
 * DATE		: on Oct 12, 2012
 * MOD		: modified by * on June 2, 2012
 **/

#ifndef __LISK_LIST_H__
#define __LISK_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <lisk_public.h>	//prefetch

#define	prefetch(x)		__builtin_prefetch(x)

#define	LIST_FOR_EACH_BEGIN(list, ptr_type, __list_ptr__)	\
do {	\
	ptr_type __list_ptr__ = (ptr_type)list_next_elem(list);	\
	if (IS_EQ(__list_ptr__, list))	{	\
		break;	\
	}	\
	for (; prefetch(list_next_elem(__list_ptr__)),	\
			IS_NNL(__list_ptr__) && IS_NEQ(__list_ptr__, list);	\
		__list_ptr__ = (ptr_type)list_next_elem(__list_ptr__)) {

#define	LIST_FOR_EACH_END	}	\
} while (0);

#define	LIST_FOR_EACH_RBEGIN(list, ptr_type, __list_ptr__)	\
do {	\
	ptr_type __list_ptr__ = (ptr_type)list_prev_elem(list);	\
	if (IS_EQ(__list_ptr__, list))	{	\
		break;	\
	}	\
	for (; prefetch(list_prev_elem(__list_ptr__)),	\
			IS_NNL(__list_ptr__) && IS_NEQ(__list_ptr__, list);	\
		__list_ptr__ = (ptr_type)list_prev_elem(__list_ptr__)) {

#define	LIST_FOR_EACH_REND	}	\
} while (0);

/**
 * description	: define list head <br/><br/>
 *
 * in-out@list	: list. pay attention that its definition should be like
 *				  "vp list;
 *				  n4 ret;
 *				  ret = list_init(&list);
 *				  if (ret != 0)	{
 *					return clause;
 *				  }"
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 or -2 instead.
 **/
n4  list_init(vpp list);
/**
 * description	: free list <br/><br/>
 *
 * in-out@list	: list
 *
 * out-@ret		: the return value is vd.
 **/
vd list_uninit(vpp list);
/**
 * description	: allocate a node and its content is copied from element @elem
 *				  and insert it before current element @cur_ptr <br/><br/>
 *
 * in-@cur_elem	: current element
 * in-@elem		: element pointer
 * in-@len		: element length
 *
 * out-@ret		: if successful, ret is the list elem equivalent to @elem;
 *				  otherwise nil instead.
 **/
vp list_insert_front(vp cur_elem, vp elem, n4 elem_len);
/**
 * description	: allocate a node and its content is copied from element @elem
 *				  and insert it after current element @cur_ptr <br/><br/>
 *
 * in-@cur_ptr	: current element
 * in-@elem		: element pointer
 * in-@len		: element length
 *
 * out-@ret		: if successful, ret is the list elem equivalent to @elem;
 *				  otherwise nil instead.
 **/
vp list_insert_back(vp cur_elem, vp elem, n4 elem_len);
/**
 * description	: allocate a node and its content is copied from element @elem
 *				  and insert it at the beginning of the list <br/><br/>
 *
 * in-@list		: list head
 * in-@elem		: element pointer
 * in-@len		: element length
 *
 * out-@ret		: if successful, ret is the list elem equivalent to @elem;
 *				  otherwise nil instead.
 **/
vp list_push_front(vp list, vp elem, n4 elem_len);
/**
 * description	: allocate a node and its content is copied from element @elem
 *				  and insert it at the end of the list <br/><br/>
 *
 * in-@list		: list head
 * in-@elem		: element pointer
 * in-@len		: element length
 *
 * out-@ret		: if successful, ret is the list elem equivalent to @elem;
 *				  otherwise nil instead.
 **/
vp list_push_back(vp list, vp elem, n4 elem_len);
/**
 * description	: delete @src_elem from its list and insert it into
 *				  another list before the @dst_elem <br/><br/>
 *
 * in-@dst_elem	: destination element
 * in-@src_elem	: source element
 *
 * out-@ret		: if successful, the return value is 0; otherwise
 *				  the return value is -1 if the input parameter
 *				  @src_elem or @dst_elem is nil.
 **/
n4 list_move_front(vp dst_elem, vp src_elem);
/**
 * description	: delete @src_elem from its list and insert it into
 *				  another list after the @dst_elem <br/><br/>
 *
 * in-@dst_elem	: destination element
 * in-@src_elem	: source element
 *
 * out-@ret		: if successful, the return value is 0; otherwise
 *				  the return value is -1 if the input parameter
 *				  @src_elem or @dst_elem is nil.
 **/
n4 list_move_back(vp dst_elem, vp src_elem);
typedef enum ELIST_ERASE_FLAG_TAG {
	ELIST_ERASE_PART	= 0X01 << 0,
	ELIST_ERASE_FULL	= 0X01 << 1,
} ELIST_ERASE_FLAG;
/**
 * description	: erase the element @elem in the @list and the return
 *				  value is the element after @elem. if the @flag is
 *				  ELIST_ERASE_FULL, @list will not deallocate @elem's
 *				  memory space. <br/><br/>
 *
 * in-@list		: the pointer of list head
 * in-@elem		: element
 * in-@flag		: deallocation flag
 *
 * out-@ret		: if successful, ret is @elem's previous element;
 *				  otherwise nil instead.
 **/
vp list_erase(vp list, vp elem, n4 flag);
/**
 * description	: delete all elements in the list @list <br/><br/>
 *
 * in-@list		: the pointer of list
 *
 * out-@ret		: the return value is vd.
 **/
vd list_clear(vp list);
/**
 * description	: return the number of elements in list
 *				  whose head is @list <br/><br/>
 *
 * in-@list		: list head
 *
 * out-@ret		: return is the number of elements.
 **/
n4 list_size(vp list);
/**
 * description	: return if the list has element or not <br/><br/>
 *
 * in-@list		: list head
 *
 * out-@ret		: if it has element, ret is 0; otherwise -1 instead.
 **/
n4 list_empty(vp list);
/**
 * description	: get a element before the element @elem <br/><br/>
 *
 * in-@elem		: element pointer
 *
 * out-@ret		: if successful, ret is the element before
 *				  given elem @elem; otherwise nil instead.
 **/
vp list_prev_elem(vp elem);
/**
 * description	: get a element after the element @elem <br/><br/>
 *
 * in-@elem		: element pointer
 *
 * out-@ret		: if successful, ret is the element after given
 *				  elem @elem; otherwise nil instead.
 **/
vp list_next_elem(vp elem);
/**
 * description	: get a element reverse before the element @elem <br/><br/>
 *
 * in-@elem		: element pointer
 *
 * out-@ret		: if successful, ret is the pointer of that
 *				  element; otherwise nil instead.
 **/
vp list_rprev_elem(vp elem);
/**
 * description	: get a element reverse after the element @elem <br/><br/>
 *
 * in-@elem		: element pointer
 *
 * out-@ret		: if successful, ret is the pointer of that element;
 *				  otherwise nil instead.
 **/
vp list_rnext_elem(vp elem);
/**
 * description	: get the first element of list whose head is @list <br/><br/>
 *
 * in-@list		: list head pointer
 *
 * out-@ret		: if successful, ret is the list's first element;
 *				  otherwise nil instead.
 **/
vp list_first(vp list);
/**
 * description	: get the last element of list whose head is @list <br/><br/>
 *
 * in-@list		: list head pointer
 *
 * out-@ret		: if successful, ret is the list's last element;
 *				  otherwise nil instead.
 **/
vp list_last(vp list);

#ifdef __cplusplus
}
#endif

#endif

