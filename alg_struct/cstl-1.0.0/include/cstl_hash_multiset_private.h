/*
 * =============================================================================
 *  Copyright (c) 2008 2009 ActiveSys.Wangbo
 *
 *  copyright description ...
 *
 *  Project:    cstl
 *  Module:     hash_multiset
 *  Filename:   cstl_hash_multiset_private.h
 *
 *  The private interface of hash_multiset.
 *
 *  2009-01-15 15:05:08
 * =============================================================================
 */

#ifndef _CSTL_HASH_MULTISET_PRIVATE_H
#define _CSTL_HASH_MULTISET_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/** include section **/

/** constant declaration and macro section **/

/** data type declaration and struct, union, enum section **/
typedef struct _taghashmultiset
{
    hashtable_t _t_hashtable;
}hash_multiset_t;

/** exported global variable declaration section **/

/** exported function prototype section **/
/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _create_hash_multiset
 *        Parameters: in) t_typesize: size_t
 *                          the element type size.
 *                    in) s_typename: const char*
 *                          the element type name.
 *           Returns: hash_multiset_t
 *                          new hash_multiset.
 *       Description: create the new hash_multiset.
 *
 * ----------------------------------------------------------------------------
 */
extern hash_multiset_t _create_hash_multiset(
    size_t t_typesize, const char* s_typename);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _hash_multiset_find
 *        Parameters: in) cpt_hash_multiset: const hash_multiset_t*
 *                        the hash_multiset pointer.
 *                    in) ...
 *                        the target value.
 *           Returns: hash_multiset_iterator_t
 *                        the target iterator or end iterator.
 *       Description: find the value in hash_multiset.
 *
 * ----------------------------------------------------------------------------
 */
extern hash_multiset_iterator_t _hash_multiset_find(
    const hash_multiset_t* cpt_hash_multiset, ...);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _hash_multiset_find_varg
 *        Parameters: in) cpt_hash_multiset: const hash_multiset_t*
 *                        the hash_multiset pointer.
 *                    in) val_elemlist: va_list
 *                        the target value.
 *           Returns: hash_multiset_iterator_t
 *                        the target iterator or end iterator.
 *       Description: find the value in hash_multiset.
 *
 * ----------------------------------------------------------------------------
 */
extern hash_multiset_iterator_t _hash_multiset_find_varg(
    const hash_multiset_t* cpt_hash_multiset, va_list val_elemlist);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _hash_multiset_count
 *        Parameters: in) cpt_hash_multiset: const hash_multiset_t*
 *                        the hash_multiset pointer.
 *                    in) ...
 *                        the target value.
 *           Returns: size_t
 *                        count.
 *       Description: counts the numbers of element whose key is value.
 *
 * ----------------------------------------------------------------------------
 */
extern size_t _hash_multiset_count(
    const hash_multiset_t* cpt_hash_multiset, ...);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _hash_multiset_count_varg
 *        Parameters: in) cpt_hash_multiset: const hash_multiset_t*
 *                        the hash_multiset pointer.
 *                    in) val_elemlist: va_list
 *                        the target value.
 *           Returns: size_t
 *                        count.
 *       Description: counts the numbers of element whose key is value.
 *
 * ----------------------------------------------------------------------------
 */
extern size_t _hash_multiset_count_varg(
    const hash_multiset_t* cpt_hash_multiset, va_list val_elemlist);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _hash_multiset_equal_range
 *        Parameters: in) cpt_hash_multiset: const hash_multiset_t*
 *                        the hash_multiset pointer.
 *                    in) ...
 *                        the target value.
 *           Returns: pair_t(hash_multiset_iterator_t, hash_multiset_iterator_t).
 *       Description: find a range that contain all elements whos key is value.
 *
 * ----------------------------------------------------------------------------
 */
extern pair_t _hash_multiset_equal_range(
    const hash_multiset_t* cpt_hash_multiset, ...);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _hash_multiset_equal_range_varg
 *        Parameters: in) cpt_hash_multiset: const hash_multiset_t*
 *                        the hash_multiset pointer.
 *                    in) val_elemlist: va_list
 *                        the target value.
 *           Returns: pair_t(hash_multiset_iterator_t, hash_multiset_iterator_t).
 *       Description: find a range that contain all elements whos key is value.
 *
 * ----------------------------------------------------------------------------
 */
extern pair_t _hash_multiset_equal_range_varg(
    const hash_multiset_t* cpt_hash_multiset, va_list val_elemlist);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _hash_multiset_insert
 *        Parameters: in) pt_hash_multiset: hash_multiset_t*
 *                        the hash_multiset pointer.
 *                    in) ...
 *                        the target value.
 *           Returns: hash_multiset_iterator_t
 *                        the insert result.
 *       Description: insert value into hash_multiset.
 *
 * ----------------------------------------------------------------------------
 */
extern hash_multiset_iterator_t _hash_multiset_insert(
    hash_multiset_t* pt_hash_multiset, ...);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _hash_multiset_insert_varg
 *        Parameters: in) pt_hash_multiset: hash_multiset_t*
 *                        the hash_multiset pointer.
 *                    in) val_elemlist: va_list
 *                        the target value.
 *           Returns: hash_multiset_iterator_t
 *                        the insert result.
 *       Description: insert value into hash_multiset.
 *
 * ----------------------------------------------------------------------------
 */
extern hash_multiset_iterator_t _hash_multiset_insert_varg(
    hash_multiset_t* pt_hash_multiset, va_list val_elemlist);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _hash_multiset_erase
 *        Parameters: in) pt_hash_multiset: hash_multiset_t*
 *                        the rb tree pointer.
 *                    in) ...
 *                        the target value.
 *           Returns: size_t
 *                        the element numbers.
 *       Description: erase all elements whose key is value.
 *
 * ----------------------------------------------------------------------------
 */
extern size_t _hash_multiset_erase(hash_multiset_t* pt_hash_multiset, ...);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _hash_multiset_erase_varg
 *        Parameters: in) pt_hash_multiset: hash_multiset_t*
 *                        the rb tree pointer.
 *                    in) val_elemlist: va_list
 *                        the target value.
 *           Returns: size_t
 *                        the element numbers.
 *       Description: erase all elements whose key is value.
 *
 * ----------------------------------------------------------------------------
 */
extern size_t _hash_multiset_erase_varg(
    hash_multiset_t* pt_hash_multiset, va_list val_elemlist);

#ifdef __cplusplus
}
#endif

#endif /* _CSTL_HASH_MULTISET_PRIVATE_H */
/** eof **/

