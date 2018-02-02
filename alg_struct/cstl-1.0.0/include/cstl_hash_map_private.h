/*
 * =============================================================================
 *  Copyright (c) 2008 2009 ActiveSys.Wangbo
 *
 *  copyright description ...
 *
 *  Project:    cstl
 *  Module:     hash_map
 *  Filename:   cstl_hash_map_private.h
 *
 *  The iterator interface of hash_map.
 *
 *  2009-01-15 19:45:45
 * =============================================================================
 */

#ifndef _CSTL_HASH_MAP_PRIVATE_H
#define _CSTL_HASH_MAP_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/** include section **/

/** constant declaration and macro section **/

/** data type declaration and struct, union, enum section **/
typedef struct _taghashmap
{
    pair_t      _t_pair;
    hashtable_t _t_hashtable;
}hash_map_t;

/** exported global variable declaration section **/

/** exported function prototype section **/
/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _create_hash_map
 *        Parameters: in) t_keytypesize: size_t
 *                          the key element type size.
 *                    in) s_keytypename: const char*
 *                          the key element type name.
 *                    in) t_valuetypesize: size_t
 *                          the value element type size.
 *                    in) s_valuetypename: const char*
 *                          the value element type name.
 *           Returns: hash_map_t
 *                          new hash_map.
 *       Description: create the new hash_map.
 *
 * ----------------------------------------------------------------------------
 */
extern hash_map_t _create_hash_map(
    size_t t_keytypesize, const char* s_keytypename,
    size_t t_valuetypesize, const char* s_valuetypename);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _hash_map_find
 *        Parameters: in) cpt_hash_map: const hash_map_t*
 *                        the hash_map pointer.
 *                    in) ...
 *                        the target value.
 *           Returns: hash_map_iterator_t
 *                        the target iterator or end iterator.
 *       Description: find the value in hash_map.
 *
 * ----------------------------------------------------------------------------
 */
extern hash_map_iterator_t _hash_map_find(const hash_map_t* cpt_hash_map, ...);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _hash_map_find_varg
 *        Parameters: in) cpt_hash_map: const hash_map_t*
 *                        the hash_map pointer.
 *                    in) val_elemlist: va_list
 *                        the target value.
 *           Returns: hash_map_iterator_t
 *                        the target iterator or end iterator.
 *       Description: find the value in hash_map.
 *
 * ----------------------------------------------------------------------------
 */
extern hash_map_iterator_t _hash_map_find_varg(
    const hash_map_t* cpt_hash_map, va_list val_elemlist);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _hash_map_count
 *        Parameters: in) cpt_hash_map: const hash_map_t*
 *                        the hash_map pointer.
 *                    in) ...
 *                        the target value.
 *           Returns: size_t
 *                        count.
 *       Description: counts the numbers of element whose key is value.
 *
 * ----------------------------------------------------------------------------
 */
extern size_t _hash_map_count(const hash_map_t* cpt_hash_map, ...);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _hash_map_count_varg
 *        Parameters: in) cpt_hash_map: const hash_map_t*
 *                        the hash_map pointer.
 *                    in) val_elemlist: va_list
 *                        the target value.
 *           Returns: size_t
 *                        count.
 *       Description: counts the numbers of element whose key is value.
 *
 * ----------------------------------------------------------------------------
 */
extern size_t _hash_map_count_varg(
    const hash_map_t* cpt_hash_map, va_list val_elemlist);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _hash_map_equal_range
 *        Parameters: in) cpt_hash_map: const hash_map_t*
 *                        the hash_map pointer.
 *                    in) ...
 *                        the target value.
 *           Returns: pair_t(hash_map_iterator_t, hash_map_iterator_t).
 *       Description: find a range that contain all elements whos key is value.
 *
 * ----------------------------------------------------------------------------
 */
extern pair_t _hash_map_equal_range(const hash_map_t* cpt_hash_map, ...);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _hash_map_equal_range_varg
 *        Parameters: in) cpt_hash_map: const hash_map_t*
 *                        the hash_map pointer.
 *                    in) val_elemlist: va_list
 *                        the target value.
 *           Returns: pair_t(hash_map_iterator_t, hash_map_iterator_t).
 *       Description: find a range that contain all elements whos key is value.
 *
 * ----------------------------------------------------------------------------
 */
extern pair_t _hash_map_equal_range_varg(
    const hash_map_t* cpt_hash_map, va_list val_elemlist);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _hash_map_at
 *        Parameters: in) pt_hash_map: hash_map_t*
 *                        the rb tree pointer.
 *                    in) ...
 *                        the target value.
 *           Returns: void*
 *                        the value pointer
 *       Description: return the value pointer that is associate with the key.
 *                    if the element is not already in the hash_map, this function 
 *                    will insert the element.
 *
 * ----------------------------------------------------------------------------
 */
extern void* _hash_map_at(hash_map_t* pt_hash_map, ...);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _hash_map_at_varg
 *        Parameters: in) pt_hash_map: hash_map_t*
 *                        the rb tree pointer.
 *                    in) val_elemlist: va_list
 *                        the target value.
 *           Returns: void*
 *                        the value pointer
 *       Description: return the value pointer that is associate with the key.
 *                    if the element is not already in the hash_map, this function 
 *                    will insert the element.
 *
 * ----------------------------------------------------------------------------
 */
extern void* _hash_map_at_varg(hash_map_t* pt_hash_map, va_list val_elemlist);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _hash_map_erase
 *        Parameters: in) pt_hash_map: hash_map_t*
 *                        the rb tree pointer.
 *                    in) ...
 *                        the target value.
 *           Returns: size_t
 *                        the element numbers.
 *       Description: erase all elements whose key is value.
 *
 * ----------------------------------------------------------------------------
 */
extern size_t _hash_map_erase(hash_map_t* pt_hash_map, ...);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _hash_map_erase_varg
 *        Parameters: in) pt_hash_map: hash_map_t*
 *                        the rb tree pointer.
 *                    in) val_elemlist: va_list
 *                        the target value.
 *           Returns: size_t
 *                        the element numbers.
 *       Description: erase all elements whose key is value.
 *
 * ----------------------------------------------------------------------------
 */
extern size_t _hash_map_erase_varg(hash_map_t* pt_hash_map, va_list val_elemlist);

#ifdef __cplusplus
}
#endif

#endif /* _CSTL_HASH_MAP_PRIVATE_H */
/** eof **/

