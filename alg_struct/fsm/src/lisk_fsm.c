/**
 * FILE		: lisk_fsm.c
 * DESC		: finite state machine
 * AUTHOR	: v1.0 written by Alex Stocks
 * DATE		: on Nov 28, 2013
 * LICENCE	: GPL 2.0
 * MOD		:
 **/

#include<lisk_fsm.h>
#include<lisk_public.h>
#include <stdlib.h>			// qsort

n4 switcher_cmp(switcher_p node0, switcher_p node1);
n4 fsm_node_cmp(fsm_node_p node0, fsm_node_p node1);

n4 switcher_cmp(switcher_p switcher0, switcher_p switcher1)	{
	n4	ret;

	if (IS_NL(switcher0) || IS_NL(switcher1))	{
		RET_INT(-1, "@switcher1 = %p, @switcher1 = %p", switcher0, switcher1);
	}

	ret = 0;
	if (IS_EQ(INVALID_EVENT, switcher0->event))	{
		ret = 1;
	} else if (IS_EQ(INVALID_EVENT, switcher1->event))	{
		ret = -1;
	} else if (IS_GT(switcher0->event, switcher1->event))	{
		ret = 1;
	} else if (IS_LT(switcher0->event, switcher1->event))	{
		ret = -1;
	}

	RET_INT(ret, nil_str);
}

n4 fsm_node_cmp(fsm_node_p node0, fsm_node_p node1)	{
	n4	ret;

	if (IS_NL(node0) || IS_NL(node1))	{
		RET_INT(-1, "@node1 = %p, @node1 = %p", node0, node1);
	}

	ret = 0;
	if (IS_EQ(INVALID_STATE, node0->state))	{
		ret = 1;
	} else if (IS_EQ(INVALID_STATE, node1->state))	{
		ret = -1;
	} else if (IS_GT(node0->state, node1->state))	{
		ret = 1;
	} else if (IS_LT(node0->state, node1->state))	{
		ret = -1;
	}

	RET_INT(ret, nil_str);
}

vd fsm_node_arr_sort(fsm_node_arr arr, u4 size) {
	n4			idx_i = 0;
	n4			idx_j = 0;
	n4			flag = 0;
	switcher_p	switcher = nil;

	if (IS_NL(arr) || IS_ZR(size))	{
		RET_VD("@arr = %p, @size = %u", arr, size);
	}

	qsort(arr, size, sizeof(arr[0]), (vp)fsm_node_cmp);
	for (idx_i = 0; IS_LT(idx_i, size); idx_i++) {
		flag = -1;
		switcher = arr[idx_i].switcher;
		for (idx_j = 0; IS_LT(idx_j, MAX_SWITCH_NUM); idx_j++) {
			if (IS_ZR(flag))	{
				switcher[idx_j].state = INVALID_STATE;
				switcher[idx_j].event = INVALID_EVENT;
				switcher[idx_j].handler = nil;
				switcher[idx_j].judge = nil;
				continue;
			}
			if (IS_EQ(switcher[idx_j].state, INVALID_STATE)) {
				flag = 0;
			}
		}
		qsort(
				arr[idx_i].switcher,
				MAX_SWITCH_NUM,
				sizeof(switcher_t),
				(vp)switcher_cmp
			);
	}
}

n4 fsm_run(fsm_p fsm, event_param_p param)	{
	n4					ret;
	n4					ret_val;
	fsm_node_t			node;
	fsm_node_p			pnode;
	switcher_t			switcher;
	switcher_p			pswitcher;
	handle_fsm_event	handler;
	handle_fsm_event	judge;

	if (IS_NL(fsm) || IS_NL(param))	{
		RET_INT(-1, "@fsm = %p, @param = %p", fsm, param);
	}

	if (IS_EQ(INVALID_STATE, fsm->state) ||
		IS_EQ(INVALID_EVENT, param->event))	{
		RET_INT(-1, "@fsm->state = %d, @param->event = %d",
					fsm->state, param->event);
	}

	ret_val = 0;
	do {
		node.state = fsm->state;
		pnode = bsearch(
						&node,
						fsm->node,
						fsm->size,
						sizeof(node),
						(vp)fsm_node_cmp
					);
		if (IS_NL(pnode))	{
			ret_val = -2;
			LISK_LOG_ERROR("bsearch(state = %d) = %p",
							fsm->state, pnode);
			break;
		}
		switcher.event = param->event;
		pswitcher = bsearch(
							&switcher,
							pnode->switcher,
							ARR_SIZE(pnode->switcher),
							sizeof(switcher),
							(vp)switcher_cmp
						);
		if (IS_NL(pswitcher))	{
			ret_val = -2;
			LISK_LOG_ERROR("bsearch(event = %d) = %p",
							param->event, pswitcher);
			break;
		}

		// action(handle event)
		handler = pswitcher->handler;
		if (IS_NL(handler))	{
			//LISK_LOG_DEBUG("pswitcher->handler is nil, current state = %d, event = %d",
			//			fsm->state, param->event);
			handler = fsm->handler;
		}
		ret = (*handler)(fsm, param);
		if (IS_NZR(ret))	{
			LISK_LOG_WARN("current state = %d, handler(param{"
						"event = %d, next state = %d}) = %d",
						fsm->state, pswitcher->event, pswitcher->state, ret);
		}

		// switch to next state
		ret = 0;
		judge = pswitcher->judge;
		if (IS_NL(judge))	{
			LISK_LOG_WARN("pswitcher->judge is nil, current state = %d, event = %d",
							fsm->state, param->event);
			judge = fsm->judge;
		}
		ret = (*judge)(fsm, param);
		if (IS_ZR(ret))	{
			fsm->state = pswitcher->state;
		}

		if (IS_EQ(INVALID_STATE, fsm->state))	{
			break;
		}
	} while(1);

	RET_INT(ret_val, nil_str);
}

