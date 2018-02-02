/*
 * =============================================================================
 *  Copyright (c) 2008 2009 ActiveSys.Wangbo
 *
 *  copyright description ...
 *
 *  Project:    cstl
 *  Module:     hashtable
 *  Filename:   cstl_hashtable_iterator.h
 *
 *  The iterator interface of hashtable.
 *
 *  2009-01-12 15:34:56
 * =============================================================================
 */

#ifndef _CSTL_HASHTABLE_ITERATOR_H
#define _CSTL_HASHTABLE_ITERATOR_H

#ifdef __cplusplus
extern "C" {
#endif

/** include section **/

/** constant declaration and macro section **/

/** data type declaration and struct, union, enum section **/
typedef iterator_t hashtable_iterator_t;
struct _taghashtable;

/** exported global variable declaration section **/

/** exported function prototype section **/
/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _create_hashtable_iterator
 *        Parameters: void
 *           Returns: hashtable_iterator_t
 *                          new hashtable iterator.
 *       Description: create new hashtable iterator.
 *
 * ----------------------------------------------------------------------------
 */
extern hashtable_iterator_t _create_hashtable_iterator(void);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _hashtable_iterator_get_value
 *        Parameters: in) cpt_hashtable: const struct _taghashtable*
 *                            the hashtable pointer.
 *                    in) cpt_iterator: const hashtable_iterator_t*
 *                            the iterator pointer for getting value.
 *                    modify)pv_value: void*
 *                            the value pointer.
 *           Returns: void
 *       Description: get the value of current hashtable iterator.
 *
 * ----------------------------------------------------------------------------
 */
extern void _hashtable_iterator_get_value(
    const struct _taghashtable* cpt_hashtable,
    const hashtable_iterator_t* cpt_iterator, 
    void* pv_value);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _hashtable_iterator_get_pointer
 *        Parameters: in) cpt_hashtable: const struct _taghashtable*
 *                            the hashtable pointer.
 *                    in) cpt_iterator: const hashtable_iterator_t*
 *                            the iterator pointer for getting value.
 *           Returns: const void*
 *       Description: get the iterator pointer.
 *
 * ----------------------------------------------------------------------------
 */
extern const void* _hashtable_iterator_get_pointer(
    const struct _taghashtable* cpt_hashtable,
    const hashtable_iterator_t* cpt_iterator); 

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _hashtable_iterator_next
 *        Parameters: in) cpt_hashtable: const struct _taghashtable*
 *                            the hashtable pointer.
 *                    in) pt_iterator: hashtable_iterator_t*
 *                            the iterator pointer for go to next position.
 *           Returns: void
 *       Description: to the next position of current hashtable iterator.
 *
 * ----------------------------------------------------------------------------
 */
extern void _hashtable_iterator_next(
    const struct _taghashtable* cpt_hashtable, hashtable_iterator_t* pt_iterator);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _hashtable_iterator_equal
 *        Parameters: in) cpt_hashtable: const struct _taghashtable*
 *                            the hashtable pointer.
 *                    in) cpt_iterator: const hashtable_iterator_t*
 *                            the iterator pointer for getting value.
 *                    in) t_iterator: hashtable_iterator_t
 *                            the iterator.
 *           Returns: bool_t
 *                            if the first iterator equal the second.
 *       Description: test if the first iterator equal the second.
 *
 * ----------------------------------------------------------------------------
 */
extern bool_t _hashtable_iterator_equal(
    const struct _taghashtable* cpt_hashtable,
    const hashtable_iterator_t* cpt_iterator,
    hashtable_iterator_t t_iterator);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _hashtable_iterator_distance
 *        Parameters: in) cpt_begin: const hashtable_iterator_t*
 *                            the iterator pointer to the first node.
 *                    in) cpt_end: const hashtable_iterator_t*
 *                            the iterator pointer to the last node.
 *           Returns: int
 *                            the distance of two iterator
 *       Description: get the distance of two iterator.
 *
 * ----------------------------------------------------------------------------
 */
extern int _hashtable_iterator_distance(
    const hashtable_iterator_t* cpt_begin, const hashtable_iterator_t* cpt_end);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _hashtable_iterator_before
 *        Parameters: in) cpt_iteratorfirst: const hashtable_iterator_t*
 *                            first hashtable iterator pointer.
 *                    in) cpt_iteratorsecond: const hashtable_iterator_t*
 *                            second hashtable iterator pointer.
 *           Returns: bool_t
 *       Description: test the first iterator before the second one.
 *
 * ----------------------------------------------------------------------------
 */
extern bool_t _hashtable_iterator_before(
    const hashtable_iterator_t* cpt_iteratorfirst, 
    const hashtable_iterator_t* cpt_iteratorsecond);

#ifdef __cplusplus
}
#endif

#endif /* _CSTL_HASHTABLE_ITERATOR_H */
/** eof **/

