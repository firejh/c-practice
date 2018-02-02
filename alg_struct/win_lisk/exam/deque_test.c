/**
 * FILE		: deque_test.c
 * DESC		: test deque
 * AUTHOR	: v1.0 written by Alex Stocks
 * DATE		: on April 22, 2014
 * LICENCE	: GPL 2.0
 * MOD		:
 **/

#include "lisk_public.h"
#include "lisk_deque.h"

vd deque_output(vp deque);
vd queue_test(vp deque);
vd queue_test2(vp deque);
vd stack_test(vp deque);
vd stack_test2(vp deque);

n4 main(n4 argc, n1 **argv)
{
	n4	ret;
	u4	size;
	vp	deque;

	size = sizeof(n4);
	// ret =  deque_init(deque, size);
	ret =  _deque_init(&deque, size, 2, 8);
	if (IS_NZR(ret))	{
		RET_INT(-1, "DEQUE_INIT(@deque = %p, size = %u) = %d", deque, size, ret);
	}

	// queue_test(deque);
	// queue_test2(deque);
	stack_test(deque);
	// stack_test2(deque);

	deque_uninit(deque);

	return 0;
}

vd deque_output(vp deque)
{
	n4	ret;
	u4	idx;
	u4	size;
	u4	top;
	vp	ptr;

	if (IS_NL(deque)) {
		RET_VD("@deque = %p", deque);
	}

	ret = deque_size(deque, &size);
	pinfo("deque_size(@deque = %p) = %d, size = %u", deque, ret, size);
	ptr = deque_front(deque);
	if (IS_NNL(ptr)) {
		pinfo("front value: %d", *(n4*)(ptr));
	}
	ptr = deque_back(deque);
	if (IS_NNL(ptr)) {
		pinfo("back value: %d", *(n4*)(ptr));
	}

	top = size;
	for (idx = 0; IS_LT(idx, top); idx++) {
		ptr = deque_at(deque, idx);
		if (IS_NNL(ptr)) {
			pinfo("%d: %d", idx, *(n4*)(ptr));
		}
	}
}

vd queue_test(vp deque)
{
	n4	idx;
	n4	ret;
	n4	top;
	n4	value;

	if (IS_NL(deque)) {
		RET_VD("@deque = %p", deque);
	}

	top = 7;
	for (idx = 0; IS_LT(idx, top); idx++) {
		ret = deque_push_back(deque, &idx);
		if (IS_NZR(ret)) {
			perr("deque_push_back(@deque = %p, idx = %d) = %d", deque, idx, ret);
		}
	}

	deque_output(deque);

	top = idx;
	for (idx = 0; IS_LT(idx, top); idx++) {
		ret = deque_pop_front(deque, &value);
		if (IS_ZR(ret)) {
			pinfo("%d: %d", idx, value);
		}
	}

	ret = deque_empty(deque);
	pinfo("deque_empty(deque) = %d", ret);
}

vd queue_test2(vp deque)
{
	n4	idx;
	n4	ret;
	n4	top;
	n4	value;

	if (IS_NL(deque)) {
		RET_VD("@deque = %p", deque);
	}

	top = 7;
	for (idx = 0; IS_LT(idx, top); idx++) {
		ret = deque_push_front(deque, &idx);
		if (IS_NZR(ret)) {
			perr("deque_push_back(@deque = %p, idx = %d) = %d", deque, idx, ret);
		}
	}

	deque_output(deque);

	top = idx;
	for (idx = 0; IS_LT(idx, top); idx++) {
		ret = deque_pop_back(deque, &value);
		if (IS_ZR(ret)) {
			pinfo("%d: %d", idx, value);
		}
	}

	ret = deque_empty(deque);
	pinfo("deque_empty(deque) = %d", ret);
}

vd stack_test(vp deque)
{
	n4	idx;
	n4	ret;
	n4	top;
	n4	value;

	if (IS_NL(deque)) {
		RET_VD("@deque = %p", deque);
	}

	top = 7000;
	for (idx = 0; IS_LT(idx, top); idx++) {
		ret = deque_push_front(deque, &idx);
		if (IS_NZR(ret)) {
			perr("deque_push_back(@deque = %p, idx = %d) = %d", deque, idx, ret);
		}
	}

	deque_output(deque);

	top = idx;
	for (idx = 0; IS_LT(idx, top); idx++) {
		ret = deque_pop_front(deque, &value);
		if (IS_ZR(ret)) {
			pinfo("%d: %d", idx, value);
		}
	}

	ret = deque_empty(deque);
	pinfo("deque_empty(deque) = %d", ret);
}

vd stack_test2(vp deque)
{
	n4	idx;
	n4	ret;
	n4	top;
	n4	value;

	if (IS_NL(deque)) {
		RET_VD("@deque = %p", deque);
	}

	top = 7;
	for (idx = 0; IS_LT(idx, top); idx++) {
		ret = deque_push_back(deque, &idx);
		if (IS_NZR(ret)) {
			perr("deque_push_back(@deque = %p, idx = %d) = %d", deque, idx, ret);
		}
	}

	deque_output(deque);

	top = idx;
	for (idx = 0; IS_LT(idx, top); idx++) {
		ret = deque_pop_back(deque, &value);
		if (IS_ZR(ret)) {
			pinfo("%d: %d", idx, value);
		}
	}

	ret = deque_empty(deque);
	pinfo("deque_empty(deque) = %d", ret);
}
