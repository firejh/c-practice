/*
 * =============================================================================
 *  Copyright (c) 2008 2009 ActiveSys.Wangbo
 *
 *  copyright description ...
 *
 *  Project:    cstl
 *  Module:     function
 *  Filename:   cstl_function_private.h
 *
 *  The private interface of function.
 *
 *  2009-02-13 15:07:20
 * =============================================================================
 */

#ifndef _CSTL_FUNCTION_PRIVATE_H
#define _CSTL_FUNCTION_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/** include section **/

/** constant declaration and macro section **/

/** data type declaration and struct, union, enum section **/
typedef enum _tagfununarytype
{
    _NEGATE_FUN,
    _LOGICAL_NOT_FUN,
    _RANDOM_NUMBER_FUN
}fun_unary_type_t;

typedef enum _tagfunbinarytype
{
    /* arithmetic */
    _PLUS_FUN,
    _MINUS_FUN,
    _MULTIPLIES_FUN,
    _DIVIDES_FUN,
    _MODULUS_FUN,
    /* comparisons */
    _EQUAL_FUN,
    _NOT_EQUAL_FUN,
    _GREAT_FUN,
    _GREAT_EQUAL_FUN,
    _LESS_FUN,
    _LESS_EQUAL_FUN,
    /* logical */
    _LOGICAL_AND_FUN,
    _LOGICAL_OR_FUN,
    /* increase */
    _INCREASE_FUN
}fun_binary_type_t;

/** exported global variable declaration section **/

/** exported function prototype section **/
/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _fun_get_unary
 *        Parameters: in) s_typename: const char*
 *                          the type name.
 *                    in) t_funtype: fun_unary_type_t
 *                          function type.
 *           Returns: unary_function_t
 *       Description: select unary function.
 *
 * ----------------------------------------------------------------------------
 */
extern unary_function_t _fun_get_unary(
    const char* s_typename, fun_unary_type_t t_funtype);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _fun_get_binary
 *        Parameters: in) s_typename: const char*
 *                          the type name.
 *                    in) t_funtype: fun_binary_type_t
 *                          function type.
 *           Returns: unary_function_t
 *       Description: select binary function.
 *
 * ----------------------------------------------------------------------------
 */
extern binary_function_t _fun_get_binary(
    const char* s_typename, fun_binary_type_t t_funtype);

/* increase */
extern void _fun_increase_char(const void* cpv_input, void* pv_output);
extern void _fun_increase_uchar(const void* cpv_input, void* pv_output);
extern void _fun_increase_short(const void* cpv_input, void* pv_output);
extern void _fun_increase_ushort(const void* cpv_input, void* pv_output);
extern void _fun_increase_int(const void* cpv_input, void* pv_output);
extern void _fun_increase_uint(const void* cpv_input, void* pv_output);
extern void _fun_increase_long(const void* cpv_input, void* pv_output);
extern void _fun_increase_ulong(const void* cpv_input, void* pv_output);
extern void _fun_increase_float(const void* cpv_input, void* pv_output);
extern void _fun_increase_double(const void* cpv_input, void* pv_output);

#ifdef __cplusplus
}
#endif

#endif /* _CSTL_FUNCTION_PRIVATE_H */
/** eof **/

