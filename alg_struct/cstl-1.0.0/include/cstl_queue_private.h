/*
 * =============================================================================
 *  Copyright (c) 2008 2009 ActiveSys.Wangbo
 *
 *  copyright description ...
 *
 *  Project:    cstl
 *  Module:     queue
 *  Filename:   cstl_queue_private.h
 *
 *  The private struct and static function of queue.
 *
 *  2008-12-14 20:32:44
 * =============================================================================
 */

#ifndef _CSTL_QUEUE_PRIVATE_H
#define _CSTL_QUEUE_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/** include section **/

/** constant declaration and macro section **/
#define _GET_QUEUE_DEQUE_SEQUENCE(_pt_queue)\
    (&((_pt_queue)->_t_sequence._t_deque))
#define _GET_QUEUE_LIST_SEQUENCE(_pt_queue)\
    (&((_pt_queue)->_t_sequence._t_list))

/** data type declaration and struct, union, enum section **/
/* queue struct default use deque_t */
typedef struct _tagqueue
{
    union
    {
        deque_t _t_deque;
        list_t  _t_list;
    }_t_sequence;
}queue_t;

typedef struct _tagpriority_queue
{
    vector_t          _t_vector;
    binary_function_t _t_binary_op;
}priority_queue_t;

/** exported global variable declaration section **/

/** exported function prototype section **/
/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _create_queue
 *        Parameters: in) t_typesize: size_t
 *                          the element type size.
 *                    in) s_typename: const char*
 *                          the element type name.
 *           Returns: queue_t
 *                          new queue.
 *       Description: create the new queue.
 *
 * ----------------------------------------------------------------------------
 */
extern queue_t _create_queue(size_t t_typesize, const char* s_typename);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _queue_push
 *        Parameters: in) pt_queue: queue_t*
 *                          the queue.
 *                    in) ...
 *                          the element.
 *           Returns: void
 *       Description: append a copy of element at the top.
 *
 * ----------------------------------------------------------------------------
 */
extern void _queue_push(queue_t* pt_queue, ...);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _queue_push_varg
 *        Parameters: in) pt_queue: queue_t*
 *                          the queue.
 *                    in) val_elemlist: va_list
 *                          the element list.
 *           Returns: void
 *       Description: append a copy of element at the top.
 *
 * ----------------------------------------------------------------------------
 */
extern void _queue_push_varg(queue_t* pt_queue, va_list val_elemlist);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _create_priority_queue
 *        Parameters: in) t_typesize: size_t
 *                          the element type size.
 *                    in) s_typename: const char*
 *                          the element type name.
 *           Returns: priority_queue_t
 *                          new priority queue.
 *       Description: create the new priority queue.
 *
 * ----------------------------------------------------------------------------
 */
extern priority_queue_t _create_priority_queue(
    size_t t_typesize, const char* s_typename);

/*
 * ----------------------------------------------------------------------------
 *
 *      Functionname: _priority_queue_push
 *        Parameters: in) pt_pqueue: priority_queue_t*
 *                          the priority queue.
 *                    in) ...
 *                          the element.
 *           Returns: void
 *       Description: append a copy of element at the top.
 *
 * ----------------------------------------------------------------------------
 */
extern void _priority_queue_push(priority_queue_t* pt_pqueue, ...);

#ifdef __cplusplus
}
#endif

#endif /* _CSTL_QUEUE_PRIVATE_H */
/** eof **/

