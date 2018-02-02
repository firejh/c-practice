/*
 * =============================================================================
 *  Copyright (c) 2008 2009 ActiveSys.Wangbo
 *
 *  copyright description ...
 *
 *  Project:    cstl
 *  Module:     numeric
 *  Filename:   cstl_numeric_private.h
 *
 *  The private interface of numeric.
 *
 *  2009-02-17 15:52:33
 * =============================================================================
 */

#ifndef _CSTL_NUMERIC_PRIVATE_H
#define _CSTL_NUMERIC_PRIVATE_H

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
 *      Functionname: _algo_iota
 *        Parameters: in) t_first: forward_iterator_t.
 *                        the first iterator.
 *                    in) t_last: forward_iterator_t
 *                        the last iterator.
 *                    in) ...
 *                        element list.
 *           Returns: void
 *       Description: assign sequentially increasing values to range.
 *
 * ----------------------------------------------------------------------------
 */
extern void _algo_iota(
    forward_iterator_t t_first, forward_iterator_t t_last, ...);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _algo_iota_varg
 *        Parameters: in) t_first: forward_iterator_t.
 *                        the first iterator.
 *                    in) t_last: forward_iterator_t
 *                        the last iterator.
 *                    in) val_elemlist: va_list
 *                        element list.
 *           Returns: void
 *       Description: assign sequentially increasing values to range.
 *
 * ----------------------------------------------------------------------------
 */
extern void _algo_iota_varg(
    forward_iterator_t t_first,
    forward_iterator_t t_last,
    va_list val_elemlist);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _algo_accumulate
 *        Parameters: in) t_first: input_iterator_t
 *                        the first iterator.
 *                    in) t_last: input_iterator_t
 *                        the last iterator.
 *                    out) pv_output: void*
 *                        the output value.
 *                    in) ...
 *           Returns: void
 *       Description: it computes the sum of init and all of the elements in 
 *                    the range [t_first, t_last).
 *
 * ----------------------------------------------------------------------------
 */
extern void _algo_accumulate(
    input_iterator_t t_first, 
    input_iterator_t t_last, 
    void* pv_output,
    ...);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _algo_accumulate_if
 *        Parameters: in) t_first: input_iterator_t
 *                        the first iterator.
 *                    in) t_last: input_iterator_t
 *                        the last iterator.
 *                    in) t_binary_op: binary_function_t
 *                        the binary operation.
 *                    out) pv_output: void*
 *                        the output value.
 *                    in) ...
 *           Returns: void
 *       Description: it computes the sum of init and all of the elements in 
 *                    the range [t_first, t_last) use the binary operation.
 *
 * ----------------------------------------------------------------------------
 */
extern void _algo_accumulate_if(
    input_iterator_t t_first,
    input_iterator_t t_last,
    binary_function_t t_binary_op,
    void* pv_output,
    ...);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _algo_accumulate_if_varg
 *        Parameters: in) t_first: input_iterator_t
 *                        the first iterator.
 *                    in) t_last: input_iterator_t
 *                        the last iterator.
 *                    in) t_binary_op: binary_function_t
 *                        the binary operation.
 *                    out) pv_output: void*
 *                        the output value.
 *                    in) val_elemlist: va_list
 *           Returns: void
 *       Description: it computes the sum of init and all of the elements in 
 *                    the range [t_first, t_last) use the binary operation.
 *
 * ----------------------------------------------------------------------------
 */
extern void _algo_accumulate_if_varg(
    input_iterator_t t_first,
    input_iterator_t t_last,
    binary_function_t t_binary_op,
    void* pv_output,
    va_list val_elemlist);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _algo_inner_product
 *        Parameters: in) t_first1: input_iterator_t
 *                        the first iterator.
 *                    in) t_last1: input_iterator_t
 *                        the last iterator.
 *                    in) t_first2: input_iterator_t
 *                        the second iterator.
 *                    out) pv_output: void*
 *                        the output value.
 *                    in) ...
 *           Returns: void
 *       Description: calculate the inner product of range [t_first1, t_last1)
 *                    and [t_first2, t_last2).
 *
 * ----------------------------------------------------------------------------
 */
extern void _algo_inner_product(
    input_iterator_t t_first1,
    input_iterator_t t_last1,
    input_iterator_t t_first2,
    void* pv_output,
    ...);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _algo_inner_product_if
 *        Parameters: in) t_first1: input_iterator_t
 *                        the first iterator.
 *                    in) t_last1: input_iterator_t
 *                        the last iterator.
 *                    in) t_first2: input_iterator_t
 *                        the second iterator.
 *                    in) t_binary_op1: binary_function_t
 *                        binary function 1.
 *                    in) t_binary_op2: binary_function_t
 *                        binary function2
 *                    out) pv_output: void*
 *                        the output value.
 *                    in) ...
 *           Returns: void
 *       Description: calculate the inner product of range [t_first1, t_last1)
 *                    and [t_first2, t_last2) use t_binary_op1 and t_binary_op2.
 *
 * ----------------------------------------------------------------------------
 */
extern void _algo_inner_product_if(
    input_iterator_t t_first1,
    input_iterator_t t_last1,
    input_iterator_t t_first2,
    binary_function_t t_binary_op1,
    binary_function_t t_binary_op2,
    void* pv_output,
    ...);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _algo_inner_product_if_varg
 *        Parameters: in) t_first1: input_iterator_t
 *                        the first iterator.
 *                    in) t_last1: input_iterator_t
 *                        the last iterator.
 *                    in) t_first2: input_iterator_t
 *                        the second iterator.
 *                    in) t_binary_op1: binary_function_t
 *                        binary function 1.
 *                    in) t_binary_op2: binary_function_t
 *                        binary function2
 *                    out) pv_output: void*
 *                        the output value.
 *                    in) val_elemlist: va_list
 *           Returns: void
 *       Description: calculate the inner product of range [t_first1, t_last1)
 *                    and [t_first2, t_last2) use t_binary_op1 and t_binary_op2.
 *
 * ----------------------------------------------------------------------------
 */
extern void _algo_inner_product_if_varg(
    input_iterator_t t_first1,
    input_iterator_t t_last1,
    input_iterator_t t_first2,
    binary_function_t t_binary_op1,
    binary_function_t t_binary_op2,
    void* pv_output,
    va_list val_elemlist);

#ifdef __cplusplus
}
#endif

#endif /* _CSTL_NUMERIC_PRIVATE_H */
/** eof **/

