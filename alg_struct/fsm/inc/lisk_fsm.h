/**
 * FILE		: lisk_fsm.h
 * DESC		: finite state machine
 * AUTHOR	: v1.0 written by Alex Stocks
 * DATE		: on Nov 28, 2013
 * LICENCE	: GPL 2.0
 * MOD		:
 **/

#ifndef __LISK_FSM_H__
#define __LISK_FSM_H__

#if  (defined(__cplusplus))
	extern "C" {
#endif

#include <lisk_type.h>

#if  (defined(__cplusplus))
	}
#endif

#define	MAX_SWITCH_NUM		13

#define	FSM_EVENT_ID		n4
#define	FSM_STATE_ID		n4

#define	INVALID_EVENT		((n4)(-1))
#define	INVALID_STATE		((n4)(-1))

typedef struct event_param_tag	{
	FSM_EVENT_ID			event;						// event id
	lisk_data_t				param;						// event parameter
} event_param_t, *event_param_p, event_param_a[1];

typedef struct switcher_tag	{
	FSM_STATE_ID			state;						// next state
	FSM_EVENT_ID			event;						// event
	vp						handler;					// event handler
	// if its return value of @judge is non-zero, fsm will fail to swtich its state.
	vp						judge;						// state condition handler.
} switcher_t, *switcher_p, switcher_a[1];

typedef struct fsm_node_tag	{
	FSM_STATE_ID			state;						// state id
	n1*						name;						// state name
	switcher_t				switcher[MAX_SWITCH_NUM];	// state switch array
} fsm_node_t, *fsm_node_p, fsm_node_a[1];

typedef	fsm_node_t			fsm_node_arr[];

typedef struct fsm_tag	{
	FSM_STATE_ID			state;						// current state id
	vp						handler;					// default event handler
	vp						judge;						// default condition handler
	n4						size;						// @node size
	fsm_node_p				node;						// fsm node array
} fsm_t, *fsm_p, fsm_a[1];

typedef	n4	(*handle_fsm_event)(fsm_p fsm, event_param_p event);

vd fsm_node_arr_sort(fsm_node_arr arr, u4 size);

#define	FSM_NODE_ARR_BEGIN(arr)							\
	static fsm_node_arr arr = {

#define	FSM_NODE_ARR_END								\
	{INVALID_STATE, nil, {{0}}}};

#define	FSM_NODE_BEGIN(state, name)						\
	{state, name, {
#define	FSM_NODE_END									\
	{INVALID_STATE, INVALID_EVENT, nil, 0}}},

#define	FSM_SWITCH(state, event, handler, judge)		\
	{state, event, (vp)(handler), (vp)(judge)},

#define	FSM_INIT(fsm, _state, _handler, _judge, arr)	\
	fsm_t	fsm;										\
	do {												\
		fsm.state = _state;								\
		fsm.handler = _handler;							\
		fsm.judge = _judge;								\
		fsm.size = ARR_SIZE(arr);						\
		fsm.node = ((vp)(arr));							\
	} while (0);

/**
 * description	: write <br/><br/>
 *
 * in-@fd		: file decription
 * in-out@buf	: buffer
 * in-@buf_len	: buffer len
 *
 * out-@ret		: if successful, ret is 0; otherwise -1 instead.
 **/
n4 fsm_run(fsm_p fsm, event_param_p event);

#include <stdio.h>
#define	LISK_LOG_DEBUG(fmt, ...)	pinfo(fmt, ##__VA_ARGS__);
#define	LISK_LOG_WARN(fmt, ...)		perr(fmt, ##__VA_ARGS__);
#define	LISK_LOG_ERROR(fmt, ...)	perr(fmt, ##__VA_ARGS__);
#define	RET_INT(ret, fmt, ...)				\
	if (ret && strlen(fmt))	{							\
		LISK_LOG_ERROR(fmt, ##__VA_ARGS__);	\
	}										\
	return ret;

#define	RET_VD(fmt, ...)					\
	if (strlen(fmt))	{					\
	LISK_LOG_DEBUG(fmt, ##__VA_ARGS__);		\
	}										\
	return;

#define	print						printf
#define	pline(fmt, ...)				printf(fmt"\n", ##__VA_ARGS__)
#define	pinfo(fmt, ...)				printf("%s-%s-%d: "fmt"\n", ((n1*)__FILE__),	\
											(n1*)__func__, (n4)__LINE__, ##__VA_ARGS__)
#define	perr(fmt, ...)				fprintf(stderr, "\033[1;31;40m%s-%s-%d: error: "fmt"\n",	\
									((n1*)__FILE__), (n1*)__func__, (n4)__LINE__, ##__VA_ARGS__)

#endif

