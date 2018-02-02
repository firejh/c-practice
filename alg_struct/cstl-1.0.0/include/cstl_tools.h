/*
 * =============================================================================
 *  Copyright (c) 2008 2009 ActiveSys.Wangbo
 *
 *  copyright description ...
 *
 *  Project:    cstl
 *  Module:     tools
 *  Filename:   cstl_tools.h
 *
 *  The interface of tools.
 *
 *  2009-02-16 16:36:25
 * =============================================================================
 */

#ifndef _CSTL_TOOLS_H
#define _CSTL_TOOLS_H

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
 *      Functionname: _tools_valid_iterator_range
 *        Parameters: in) t_first: iterator_t
 *                          first iterator.
 *                    in) t_last: iterator_t
 *                          last iterator.
 *                    in) t_limittype: iteratortype_t
 *                          the lower limit iterator type.
 *           Returns: bool_t
 *       Description: test the iterator is valid or not.
 *
 * ----------------------------------------------------------------------------
 */
extern bool_t _tools_valid_iterator_range(
    iterator_t t_first, iterator_t t_last, iteratortype_t t_limittype);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _tools_get_typename
 *        Parameters: in) t_iterator: iterator_t
 *                          iterator.
 *           Returns: char*
 *       Description: get the type name.
 *
 * ----------------------------------------------------------------------------
 */
extern char* _tools_get_typename(iterator_t t_iterator);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _tools_get_typesize
 *        Parameters: in) t_iterator: iterator_t
 *                          iterator.
 *           Returns: size_t
 *       Description: get the type size.
 *
 * ----------------------------------------------------------------------------
 */
extern size_t _tools_get_typesize(iterator_t t_iterator);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _tools_same_elem_type
 *        Parameters: in) t_first: iterator_t
 *                          first iterator.
 *                    in) t_second: iterator_t
 *                          second iterator.
 *           Returns: bool_t
 *       Description: test the element type.
 *
 * ----------------------------------------------------------------------------
 */
extern bool_t _tools_same_elem_type(iterator_t t_first, iterator_t t_second);

#ifdef __cplusplus
}
#endif

#endif /* _CSTL_TOOLS_H */
/** eof **/

