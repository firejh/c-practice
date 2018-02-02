/**
 * FILE		: fsm_test.c
 * DESC		: to test lisk_fsm
 * AUTHOR	: v1.0 written by Alex Stocks
 * DATE		: on Nov 30, 2013
 * LICENCE	: GPL 2.0
 * MOD		: 
 **/

#if  (defined(__cplusplus))
	extern "C" {
#endif

#include <lisk_public.h>
#include <lisk_fsm.h>
#include <stdio.h>
#include <stdlib.h>

#if  (defined(__cplusplus))
	}
#endif

typedef enum EWATCH_EVENT_TAG {
	EWATCH_POWERON	= 0X01 << 0,
	EWATCH_SPLIT	= 0X01 << 1,
	EWATCH_UNSPLIT	= 0X01 << 2,
	EWATCH_STOP		= 0X01 << 3,
	EWATCH_POWEROFF	= 0X01 << 4
} EWATCH_EVENT;

typedef enum EWATCH_STATE_TAG {
	EWATCH_START	= 0X01 << 0,
	EWATCH_RUNNING	= 0X01 << 1,
	EWATCH_SUSPEND	= 0X01 << 2,
	EWATCH_STOPPED	= 0X01 << 3,
	EWATCH_CLOSED	= INVALID_STATE
} EWATCH_STATE;

n1* get_event_str(FSM_EVENT_ID id);
n1* get_state_str(FSM_STATE_ID id);

n4 watch_poweron(fsm_p fsm, event_param_p param);
n4 watch_split(fsm_p fsm, event_param_p param);
n4 watch_unsplit(fsm_p fsm, event_param_p param);
n4 watch_stop(fsm_p fsm, event_param_p param);
n4 watch_poweroff(fsm_p fsm, event_param_p param);
n4 watch_default_handle(fsm_p fsm, event_param_p param);

n4 watch_running_judge(fsm_p fsm, event_param_p param);
n4 watch_suspend_judge(fsm_p fsm, event_param_p param);
n4 watch_stopped_judge(fsm_p fsm, event_param_p param);
n4 watch_closed_judge(fsm_p fsm, event_param_p param);
n4 watch_default_judge(fsm_p fsm, event_param_p param);

FSM_NODE_ARR_BEGIN(watch_arr)
	FSM_NODE_BEGIN(EWATCH_START, TO_STR(EWATCH_START))
		FSM_SWITCH(EWATCH_RUNNING, EWATCH_POWERON, watch_poweron, watch_running_judge)
	FSM_NODE_END

	FSM_NODE_BEGIN(EWATCH_RUNNING, TO_STR(EWATCH_RUNNING))
		FSM_SWITCH(EWATCH_SUSPEND, EWATCH_SPLIT, watch_split, watch_suspend_judge)
		FSM_SWITCH(EWATCH_STOPPED, EWATCH_STOP, watch_stop, watch_stopped_judge)
	FSM_NODE_END

	FSM_NODE_BEGIN(EWATCH_SUSPEND, TO_STR(EWATCH_SUSPEND))
		FSM_SWITCH(EWATCH_RUNNING, EWATCH_UNSPLIT, watch_unsplit, watch_running_judge)
		FSM_SWITCH(EWATCH_STOPPED, EWATCH_STOP, watch_stop, watch_stopped_judge)
	FSM_NODE_END

	FSM_NODE_BEGIN(EWATCH_STOPPED, TO_STR(EWATCH_STOPPED))
		FSM_SWITCH(EWATCH_CLOSED, EWATCH_POWEROFF, watch_poweroff, watch_closed_judge)
	FSM_NODE_END
FSM_NODE_ARR_END

n4 main(n4 argc, n1** argv)	{
	event_param_a	param;

	fsm_node_arr_sort(watch_arr, ARR_SIZE(watch_arr));

	FSM_INIT(
				watch_fsm,
				EWATCH_START,
				watch_default_handle,
				watch_default_judge,
				watch_arr
			);

	param[0].event = EWATCH_POWERON;
	param[0].param.data = nil;
	param[0].param.size = 0;
	fsm_run(&watch_fsm, param);

	return 0;
}

n1* get_event_str(FSM_EVENT_ID event)	{
	n1*	ret;
	switch (event)	{
	case EWATCH_POWERON:	{
		ret = "power on";
		break;
	}

	case EWATCH_SPLIT:	{
		ret = "split";
		break;
	}

	case EWATCH_UNSPLIT:	{
		ret = "unsplit";
		break;
	}

	case EWATCH_STOP:	{
		ret = "stop";
		break;
	}

	case EWATCH_POWEROFF:	{
		ret = "power off";
		break;
	}

	default:	{
		ret = "error event";
		break;
	}
	}

	return ret;
}

n1* get_state_str(FSM_STATE_ID state)	{
	n1*	ret;

	switch (state)	{
	case EWATCH_START:	{
		ret = "start";
		break;
	}

	case EWATCH_RUNNING:	{
		ret = "running";
		break;
	}

	case EWATCH_SUSPEND:	{
		ret = "suspend";
		break;
	}

	case EWATCH_STOPPED:	{
		ret = "stopped";
		break;
	}

	case EWATCH_CLOSED:	{
		ret = "closed";
		break;
	}

	default:	{
		ret = "error state";
		break;
	}
	}

	return ret;
}

n4 watch_poweron(fsm_p fsm, event_param_p param)	{
	if (IS_NL(fsm) || IS_NL(param))	{
		RET_INT(-1, "@fsm = %p, @param = %p", fsm, param);
	}

	pinfo("currrent state = %s, event = %s",
			get_state_str(fsm->state), get_event_str(param->event));

	if (IS_NEQ(fsm->state, EWATCH_START))	{
		RET_INT(-2, "current state of fsm is %d which is not equal to EWATCH_START = %d",
					fsm->state, EWATCH_START);
	}

	RET_INT(0, nil_str);
}

n4 watch_split(fsm_p fsm, event_param_p param)	{
	if (IS_NL(fsm) || IS_NL(param))	{
		RET_INT(-1, "@fsm = %p, @param = %p", fsm, param);
	}

	pinfo("currrent state = %s, event = %s",
			get_state_str(fsm->state), get_event_str(param->event));

	RET_INT(0, nil_str);
}

n4 watch_unsplit(fsm_p fsm, event_param_p param)	{
	if (IS_NL(fsm) || IS_NL(param))	{
		RET_INT(-1, "@fsm = %p, @param = %p", fsm, param);
	}

	pinfo("currrent state = %s, event = %s",
			get_state_str(fsm->state), get_event_str(param->event));

	RET_INT(0, nil_str);
}

n4 watch_stop(fsm_p fsm, event_param_p param)	{
	if (IS_NL(fsm) || IS_NL(param))	{
		RET_INT(-1, "@fsm = %p, @param = %p", fsm, param);
	}

	pinfo("currrent state = %s, event = %s",
			get_state_str(fsm->state), get_event_str(param->event));

	RET_INT(0, nil_str);
}

n4 watch_poweroff(fsm_p fsm, event_param_p param)	{
	if (IS_NL(fsm) || IS_NL(param))	{
		RET_INT(-1, "@fsm = %p, @param = %p", fsm, param);
	}

	pinfo("currrent state = %s, event = %s",
			get_state_str(fsm->state), get_event_str(param->event));

	RET_INT(0, nil_str);
}

n4 watch_default_handle(fsm_p fsm, event_param_p param)	{
	if (IS_NL(fsm) || IS_NL(param))	{
		RET_INT(-1, "@fsm = %p, @param = %p", fsm, param);
	}

	pinfo("currrent state = %s, event = %s",
			get_state_str(fsm->state), get_event_str(param->event));

	RET_INT(0, nil_str);
}

n4 watch_running_judge(fsm_p fsm, event_param_p param)	{
	n4	flag0;
	n4	flag1;
	n4	ret;

	if (IS_NL(fsm) || IS_NL(param))	{
		RET_INT(-1, "@fsm = %p, @param = %p", fsm, param);
	}

	flag0 = IS_EQ(fsm->state, EWATCH_START) && IS_EQ(param->event, EWATCH_POWERON);
	flag1 = IS_EQ(fsm->state, EWATCH_SUSPEND) && IS_EQ(param->event, EWATCH_UNSPLIT);

	ret = -2;
	if (IS_NZR(flag0))	{
		ret = 0;
		param->event = EWATCH_SPLIT;	// start(power on)->running(split)->suspend
	} else if (IS_NZR(flag1))	{
		ret = 0;
		param->event = EWATCH_STOP;		// suspend(unsplit)->running(stop)->stopped
	}

	RET_INT(ret, nil_str);
}

n4 watch_suspend_judge(fsm_p fsm, event_param_p param)	{
	n4	flag;
	n4	ret;

	if (IS_NL(fsm) || IS_NL(param))	{
		RET_INT(-1, "@fsm = %p, @param = %p", fsm, param);
	}

	flag = IS_EQ(fsm->state, EWATCH_RUNNING) && IS_EQ(param->event, EWATCH_SPLIT);

	ret = -2;
	if (IS_NZR(flag))	{
		ret = 0;
		param->event = EWATCH_STOP;		// running(split)->suspend(stop)->stopped
	}

	RET_INT(ret, nil_str);
}

n4 watch_stopped_judge(fsm_p fsm, event_param_p param)	{
	n4	flag0;
	n4	flag1;
	n4	ret;

	if (IS_NL(fsm) || IS_NL(param))	{
		RET_INT(-1, "@fsm = %p, @param = %p", fsm, param);
	}

	flag0 = IS_EQ(fsm->state, EWATCH_RUNNING) && IS_EQ(param->event, EWATCH_STOP);
	flag1 = IS_EQ(fsm->state, EWATCH_SUSPEND) && IS_EQ(param->event, EWATCH_STOP);

	ret = -2;
	if (IS_NZR(flag0) || IS_NZR(flag1))	{
		ret = 0;
		param->event = EWATCH_POWEROFF;
	}

	RET_INT(ret, nil_str);
}

n4 watch_closed_judge(fsm_p fsm, event_param_p param)	{
	n4	flag;
	n4	ret;

	if (IS_NL(fsm) || IS_NL(param))	{
		RET_INT(-1, "@fsm = %p, @param = %p", fsm, param);
	}

	flag = IS_EQ(fsm->state, EWATCH_STOPPED) && IS_EQ(param->event, EWATCH_POWEROFF);

	ret = -2;
	if (IS_NZR(flag))	{
		ret = 0;
	}

	RET_INT(ret, nil_str);
}

n4 watch_default_judge(fsm_p fsm, event_param_p param)	{
	if (IS_NL(fsm) || IS_NL(param))	{
		RET_INT(-1, "@fsm = %p, @param = %p", fsm, param);
	}

	pinfo("currrent state = %s, event = %s, its judge is nil",
			get_state_str(fsm->state), get_event_str(param->event));

	RET_INT(0, nil_str);
}

