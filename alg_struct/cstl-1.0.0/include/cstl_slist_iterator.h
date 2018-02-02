/*
 * =============================================================================
 *  Copyright (c) 2008 2009 ActiveSys.Wangbo
 *
 *  copyright description ...
 *
 *  Project:    cstl
 *  Module:     slist
 *  Filename:   cstl_slist_iterator.h
 *
 *  The slist iterator struct and interface.
 *
 *  2008-12-15 13:31:15
 * =============================================================================
 */

#ifndef _CSTL_SLIST_ITERATOR_H
#define _CSTL_SLIST_ITERATOR_H

#ifdef __cplusplus
extern "C" {
#endif

/** include section **/

/** constant declaration and macro section **/

/** data type declaration and struct, union, enum section **/
typedef iterator_t slist_iterator_t;
struct _tagslist;

/** exported global variable declaration section **/

/** exported function prototype section **/
/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: create_slist_iterator
 *        Parameters: void
 *           Returns: slist_iterator_t
 *                          new slist iterator.
 *       Description: create new slist iterator.
 *
 * ----------------------------------------------------------------------------
 */
extern slist_iterator_t create_slist_iterator(void);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _slist_iterator_get_value
 *        Parameters: in) cpt_slist: const struct _tagslist*
 *                            the slist pointer.
 *                    in) cpt_iterator: const slist_iterator_t*
 *                            the iterator pointer for getting value.
 *                    modify)pv_value: void*
 *                            the value pointer.
 *           Returns: void
 *       Description: get the value of current slist iterator.
 *
 * ----------------------------------------------------------------------------
 */
extern void _slist_iterator_get_value(
    const struct _tagslist* cpt_slist,
    const slist_iterator_t* cpt_iterator, 
    void* pv_value);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _slist_iterator_set_value
 *        Parameters: in) cpt_slist: const struct _tagslist*
 *                            the slist pointer.
 *                    in) cpt_iterator: const slist_iterator_t*
 *                            the iterator pointer for getting value.
 *                    in) cpv_value: const void*
 *                            the value pointer.
 *           Returns: void
 *       Description: set the value of current slist iterator.
 *
 * ----------------------------------------------------------------------------
 */
extern void _slist_iterator_set_value(
    const struct _tagslist* cpt_slist,
    const slist_iterator_t* cpt_iterator, 
    const void* cpv_value);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _slist_iterator_equal
 *        Parameters: in) cpt_slist: const struct _tagslist*
 *                            the slist pointer.
 *                    in) cpt_iterator: const slist_iterator_t*
 *                            the iterator pointer for getting value.
 *                    in) t_iterator: slist_iterator_t
 *                            the iterator.
 *           Returns: bool_t
 *                            if the first iterator equal the second.
 *       Description: test if the first iterator equal the second.
 *
 * ----------------------------------------------------------------------------
 */
extern bool_t _slist_iterator_equal(
    const struct _tagslist* cpt_slist,
    const slist_iterator_t* cpt_iterator,
    slist_iterator_t t_iterator);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _slist_iterator_get_pointer
 *        Parameters: in) cpt_slist: const struct _tagslist*
 *                            the slist pointer.
 *                    in) cpt_iterator: const slist_iterator_t*
 *                            the iterator pointer for getting value.
 *           Returns: const void*
 *       Description: get the iterator pointer.
 *
 * ----------------------------------------------------------------------------
 */
extern const void* _slist_iterator_get_pointer(
    const struct _tagslist* cpt_slist,
    const slist_iterator_t* cpt_iterator); 

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _slist_iterator_next
 *        Parameters: in) cpt_slist: const struct _tagslist*
 *                            the slist pointer.
 *                    in) pt_iterator: slist_iterator_t*
 *                            the iterator pointer for go to next position.
 *           Returns: void
 *       Description: to the next position of current slist iterator.
 *
 * ----------------------------------------------------------------------------
 */
extern void _slist_iterator_next(
    const struct _tagslist* cpt_slist, slist_iterator_t* pt_iterator);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _slist_iterator_distance
 *        Parameters: in) cpt_begin: const slist_iterator_t*
 *                            the iterator pointer to the first node.
 *                    in) cpt_end: const slist_iterator_t*
 *                            the iterator pointer to the last node.
 *           Returns: int
 *                            the distance of two iterator
 *       Description: get the distance of two iterator.
 *
 * ----------------------------------------------------------------------------
 */
extern int _slist_iterator_distance(
    const slist_iterator_t* cpt_begin, const slist_iterator_t* cpt_end);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _slist_iterator_before
 *        Parameters: in) cpt_iteratorfirst: const slist_iterator_t*
 *                            first slist iterator pointer.
 *                    in) cpt_iteratorsecond: const slist_iterator_t*
 *                            second slist iterator pointer.
 *           Returns: bool_t
 *       Description: test the first iterator before the second one.
 *
 * ----------------------------------------------------------------------------
 */
extern  bool_t _slist_iterator_before(
    const slist_iterator_t* cpt_iteratorfirst, 
    const slist_iterator_t* cpt_iteratorsecond);

#ifdef __cplusplus
}
#endif

#endif /* _CSTL_SLIST_ITERATOR_H */
/** eof **/

