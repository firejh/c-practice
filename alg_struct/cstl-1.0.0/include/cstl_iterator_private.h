/*
 * =============================================================================
 *  Copyright (c) 2008 2009 ActiveSys.Wangbo
 *
 *  copyright description ...
 *
 *  Project:    cstl
 *  Module:     iterator
 *  Filename:   cstl_iterator_private.h
 *
 *  The private interface of iterator.
 *
 *  2009-02-13 10:53:25
 * =============================================================================
 */

#ifndef _CSTL_ITERATOR_PRIVATE_H
#define _CSTL_ITERATOR_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/** include section **/

/** constant declaration and macro section **/

/** data type declaration and struct, union, enum section **/

/** exported global variable declaration section **/

/** exported function prototype section **/
/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _iterator_same_type
 *        Parameters: in) t_iterator: iterator_t
 *                            the first iterator for distance.
 *                    in) t_iteratorsrc: iterator_t
 *                            the second iterator for distance.
 *           Returns: bool_t
 *       Description: check the first iterator is same the second.
 *
 * ----------------------------------------------------------------------------
 */
extern bool_t _iterator_same_type(
    iterator_t t_iteratorfirst, iterator_t t_iteratorsecond);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _iterator_before
 *        Parameters: in) t_iterator: iterator_t
 *                            the first iterator for distance.
 *                    in) t_iteratorsrc: iterator_t
 *                            the second iterator for distance.
 *           Returns: bool_t
 *       Description: check the first iterator is before the second.
 *
 * ----------------------------------------------------------------------------
 */
extern bool_t _iterator_before(
    iterator_t t_iteratorfirst, iterator_t t_iteratorsecond);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _iterator_limit_type
 *        Parameters: in) t_iterator: iterator_t
 *                            the iterator.
 *                    in) t_limittype: iteratortype_t
 *                            the iterator type.
 *           Returns: bool_t
 *       Description: check the iterator type.
 *
 * ----------------------------------------------------------------------------
 */
extern bool_t _iterator_limit_type(
    iterator_t t_iterator, iteratortype_t t_limittype);

#ifdef __cplusplus
}
#endif

#endif /* _CSTL_ITERATOR_PRIVATE_H */
/** eof **/

