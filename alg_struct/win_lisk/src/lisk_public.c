//filename		: lisk_public.h
//func			: global macros, global variables, global functions
//version		: v1.0
//author		: Alex Stocks
//date			: 2012-07
//verify		:
//license		: GPL 2.0
//declaration	: Long live C!


#include <lisk_public.h>

n4 copy_data(vp dst, vp src, u4 len)	{
	n4 __cpy_flag__ = -1;
	n1* __cpy_dst__ = (n1*)(dst);
	n1* __cpy_src__ = (n1*)(src);
	u4 __cpy_len__ = (u4)(len);
	if (IS_LE(__cpy_len__, 0)) {
		__cpy_flag__ = 0;
	} else if (IS_NEQ(__cpy_dst__, __cpy_src__)) {
		memmove(__cpy_dst__, __cpy_src__, __cpy_len__);
		__cpy_flag__ = 0;
	}
	return __cpy_flag__;
}

typedef union epoll_data {
	vp			ptr;
	uw			u64;
} epoll_data_t;

uw PTR_2_NUM(vp _ptr_)	
{
	epoll_data_t __epoll_data__;
	__epoll_data__.ptr = _ptr_;
	return (uw)(__epoll_data__.u64);
}

vp NUM_2_PTR(uw num)	
{
	epoll_data_t _epoll_data_;
	_epoll_data_.u64 = num;
	return _epoll_data_.ptr;
}

