/*
 * =============================================================================
 *  Copyright (c) 2008 2009 ActiveSys.Wangbo
 *
 *  copyright description ...
 *
 *  Project:    cstl
 *  Module:     avl_tree
 *  Filename:   cstl_avl_tree_iterator.h
 *
 *  Interface of avl tree iterator.
 *
 *  2008-12-24 19:06:45
 * =============================================================================
 */

#ifndef _CSTL_AVL_TREE_ITERATOR_H
#define _CSTL_AVL_TREE_ITERATOR_H

#ifdef __cplusplus
extern "C" {
#endif

/** include section **/

/** constant declaration and macro section **/

/** data type declaration and struct, union, enum section **/
typedef iterator_t avl_tree_iterator_t;
typedef iterator_t avl_tree_reverse_iterator_t;
struct _tagavltree;

/** exported global variable declaration section **/

/** exported function prototype section **/
/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _create_avl_tree_iterator
 *        Parameters: void
 *           Returns: avl_tree_iterator_t
 *                          new avl_tree iterator.
 *       Description: create new avl_tree iterator.
 *
 * ----------------------------------------------------------------------------
 */
extern avl_tree_iterator_t _create_avl_tree_iterator(void);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _avl_tree_iterator_get_value
 *        Parameters: in) cpt_avl_tree: const struct _tagavltree*
 *                            the avl_tree pointer.
 *                    in) cpt_iterator: const avl_tree_iterator_t*
 *                            the iterator pointer for getting value.
 *                    modify)pv_value: void*
 *                            the value pointer.
 *           Returns: void
 *       Description: get the value of current avl_tree iterator.
 *
 * ----------------------------------------------------------------------------
 */
extern void _avl_tree_iterator_get_value(
    const struct _tagavltree* cpt_avl_tree,
    const avl_tree_iterator_t* cpt_iterator, 
    void* pv_value);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _avl_tree_iterator_get_pointer
 *        Parameters: in) cpt_avl_tree: const struct _tagavltree*
 *                            the avl_tree pointer.
 *                    in) cpt_iterator: const avl_tree_iterator_t*
 *                            the iterator pointer for getting value.
 *           Returns: const void*
 *       Description: get the iterator pointer.
 *
 * ----------------------------------------------------------------------------
 */
extern const void* _avl_tree_iterator_get_pointer(
    const struct _tagavltree* cpt_avl_tree,
    const avl_tree_iterator_t* cpt_iterator); 

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _avl_tree_iterator_next
 *        Parameters: in) cpt_avl_tree: const struct _tagavltree*
 *                            the avl_tree pointer.
 *                    in) pt_iterator: avl_tree_iterator_t*
 *                            the iterator pointer for go to next position.
 *           Returns: void
 *       Description: to the next position of current avl_tree iterator.
 *
 * ----------------------------------------------------------------------------
 */
extern void _avl_tree_iterator_next(
    const struct _tagavltree* cpt_avl_tree, avl_tree_iterator_t* pt_iterator);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _avl_tree_iterator_prev
 *        Parameters: in) cpt_avl_tree: const struct _tagavltree*
 *                            the avl_tree pointer.
 *                    in) pt_iterator: avl_tree_iterator_t*
 *                            the iterator pointer for go to previous position.
 *           Returns: void
 *       Description: to the previous position of current avl_tree iterator.
 *
 * ----------------------------------------------------------------------------
 */
extern void _avl_tree_iterator_prev(
    const struct _tagavltree* cpt_avl_tree, avl_tree_iterator_t* pt_iterator);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _avl_tree_iterator_equal
 *        Parameters: in) cpt_avl_tree: const struct _tagavltree*
 *                            the avl_tree pointer.
 *                    in) cpt_iterator: const avl_tree_iterator_t*
 *                            the iterator pointer for getting value.
 *                    in) t_iterator: avl_tree_iterator_t
 *                            the iterator.
 *           Returns: bool_t
 *                            if the first iterator equal the second.
 *       Description: test if the first iterator equal the second.
 *
 * ----------------------------------------------------------------------------
 */
extern bool_t _avl_tree_iterator_equal(
    const struct _tagavltree* cpt_avl_tree,
    const avl_tree_iterator_t* cpt_iterator,
    avl_tree_iterator_t t_iterator);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _avl_tree_iterator_distance
 *        Parameters: in) cpt_begin: const avl_tree_iterator_t*
 *                            the iterator pointer to the first node.
 *                    in) cpt_end: const avl_tree_iterator_t*
 *                            the iterator pointer to the last node.
 *           Returns: int
 *                            the distance of two iterator
 *       Description: get the distance of two iterator.
 *
 * ----------------------------------------------------------------------------
 */
extern int _avl_tree_iterator_distance(
    const avl_tree_iterator_t* cpt_begin, const avl_tree_iterator_t* cpt_end);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _avl_tree_iterator_before
 *        Parameters: in) cpt_iteratorfirst: const avl_tree_iterator_t*
 *                            first avl_tree iterator pointer.
 *                    in) cpt_iteratorsecond: const avl_tree_iterator_t*
 *                            second avl_tree iterator pointer.
 *           Returns: bool_t
 *       Description: test the first iterator before the second one.
 *
 * ----------------------------------------------------------------------------
 */
extern bool_t _avl_tree_iterator_before(
    const avl_tree_iterator_t* cpt_iteratorfirst, 
    const avl_tree_iterator_t* cpt_iteratorsecond);

#ifdef __cplusplus
}
#endif

#endif /* _CSTL_AVL_TREE_ITERATOR_H */
/** eof **/

