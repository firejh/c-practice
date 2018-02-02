/*
 * =============================================================================
 *  Copyright (c) 2008 2009 ActiveSys.Wangbo
 *
 *  copyright description ...
 *
 *  Project:    cstl
 *  Module:     algobase
 *  Filename:   cstl_algobase_private.h
 *
 *  The private interface of base algorithm.
 *
 *  2009-02-18 15:23:48
 * =============================================================================
 */

#ifndef _CSTL_ALGOBASE_PRIVATE_H
#define _CSTL_ALGOBASE_PRIVATE_H

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
 *      Functionname: _algo_fill
 *        Parameters: in) t_first: forward_iterator_t
 *                        the first iterator.
 *                    in) t_last: forward_iterator_t
 *                        the last iterator.
 *                    in) ...
 *                        element list.
 *           Returns: void
 *       Description: assign the value to every elements in range 
 *                    [t_first, t_last).
 *
 * ----------------------------------------------------------------------------
 */
extern void _algo_fill(
    forward_iterator_t t_first, forward_iterator_t t_last, ...);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _algo_fill_varg
 *        Parameters: in) t_first: forward_iterator_t
 *                        the first iterator.
 *                    in) t_last: forward_iterator_t
 *                        the last iterator.
 *                    in) val_elemlist: va_list
 *                        element list.
 *           Returns: void
 *       Description: assign the value to every elements in range 
 *                    [t_first, t_last).
 *
 * ----------------------------------------------------------------------------
 */
extern void _algo_fill_varg(
    forward_iterator_t t_first, forward_iterator_t t_last, va_list val_elemlist);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _algo_fill_n
 *        Parameters: in) t_first: output_iterator_t
 *                        the first iterator.
 *                    in) t_fillsize: size_t
 *                        the last iterator.
 *                    in) ...
 *                        element list.
 *           Returns: output_iterator_t
 *       Description: assign the value to every elements in range 
 *                    [t_first, t_first + t_fillsize).
 *
 * ----------------------------------------------------------------------------
 */
extern output_iterator_t _algo_fill_n(
    forward_iterator_t t_first, size_t t_fillsize, ...);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _algo_fill_n_varg
 *        Parameters: in) t_first: output_iterator_t
 *                        the first iterator.
 *                    in) t_fillsize: size_t
 *                        the fill size
 *                    in) val_elemlist: va_list
 *                        element list.
 *           Returns: output_iterator_t
 *       Description: assign the value to every elements in range 
 *                    [t_first, t_first + t_fillsize).
 *
 * ----------------------------------------------------------------------------
 */
extern output_iterator_t _algo_fill_n_varg(
    forward_iterator_t t_first, size_t t_fillsize, va_list val_elemlist);

#ifdef __cplusplus
}
#endif

#endif /* _CSTL_ALGOBASE_PRIVATE_H */
/** eof **/

