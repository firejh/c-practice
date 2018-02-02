/** 
 * FILE		:lisk_math.h
 * DESC		:math lisk
 * AUTHOR	:v1.0 written by Alex Stocks
 * DATE		:on Aug 15, 2012
 * MOD		:modified by Alex Stocks on Dec 13, 2012. next_prime
 **/

#ifndef __LISK_MATH_H__
#define __LISK_MATH_H__

#ifdef __cplusplus
	extern "C" {
#endif

#include "lisk_type.h"

#ifdef __cplusplus
	}
#endif

/**
 * description	: check whether num is a prime
 * @in-num		: test value
 * @out-ret		: if yes, ret is 0; otherwise -1 instead.
 **/
n4 is_num_prime(n8 num);
/**
 * description	: to get a prime that is only sligtly greater than num <br/><br/>
 *
 * in-@num		: compared value
 *
 * out-@ret		: if successful, ret is that prime; otherwise -1 instead.
 **/
n8 next_prime(n8 num);
/**
 * description		: get a prime array
 * @in-max_num		: those ret primes in prime_arr all are less than max_num
 * @in-prime_num	: wanna gotten prime num
 * @in-prime-arr	: wanna gotten prime array
 * @out-ret			: if yes, ret is value prime num; otherwise -1 instead.
 **/
n8 get_prime_arr(n8 max_num, n8 prime_num, n8* prime_arr);

/**
 * description	: get the power of pwr for num quickly
 * @in-uvalue	: test value
 * @out-ret		: power of pwr for num
 **/
n4 qintpow(n4 num, u4 pwr);

/**
 * description	: get square root of float quickly, form John Carmack
 * @in-fvalue	: square value
 * @out-ret		: square root
 **/
float qsqrt(float fvalue);
/**
 * description	: check if uvalue is power of 2
 * @in-uvalue	: test value
 * @out-ret		: if yes, ret is 0; otherwise -1 instead
 **/
n4 is_pow_of_2(u8 uvalue);
/**
 * description	: get power of 2 that is the minimum of whose value is greater than or equal to uvalue
 * @in-uvalue	: test value
 * @out-ret		: pow of 2
 **/
u8 next_pow_of_2(u8 uvalue);

/**
 * description	: cast the value of d_value from double to 
 if it is less than 2^31
 *				  this func is from John Carmack; magic num 6755399441055744.0 is 1.5*2^52;
 *				  if d_value is less than 2^31 whether it is postive num or minus num, 
 *				  f82int can also work;
 * @in-d_value		: its type is f8
 * @out-ret			: n4 value
 **/
union f8_cast { f8 dv; n4 nv; };
#define f82n4(d_value)	({\
	f8	_dv = 0;	\
	_dv = d_value;	\
	volatile union f8_cast dc; dc.dv = (_dv) + 6755399441055744.0; dc.nv; })
union f8_cast_u4 { f8 dv; u4 uv; };
#define f82u4(d_value)	({\
	f8	_dv = 0;	\
	_dv = d_value;	\
	volatile union f8_cast_u4 dc; dc.dv = (_dv) + 6755399441055744.0; dc.uv; })
/**
 * description	: to swap the two values of the given same type
 * @in-a		: the first value
 * @in-b		: the second value
 * @out-ret		: ret null
 **/
#define SWAP(a, b) \
do {	\
	(vd)(&a == &b); /*check whether types of both of them are the same*/	\
	if (sizeof(a) == sizeof(b)) {	\
		switch (sizeof(a)) {	\
			case 2:	\
					swap2B((u2*)(&a), (u2*)(&b));	\
					break;	\
			case 4:	\
					swap4B((u4*)(&a), (u4*)(&b));	\
					break;	\
			case 8:	\
					swap8B((u8*)(&a), (u8*)(&b));	\
					break;	\
			case 12:	\
					swap12B((un1*)(&a), (un1*)(&b));	\
					break;	\
		}	\
	}	\
} while(0);
vd swap2B(u2* a, u2* b);
vd swap4B(u4* a, u4* b);
vd swap8B(u8* a, u8* b);
vd swap12B(un1* a, un1* b);

/**
 * description	: to swap the two values @v0 and @v1 <br/><br/>
 *
 * in-out@v0		: the first value
 * in-out@v1		: the second value
 * in-@len			: value len
 *
 * @out-ret			: if successful, ret is 0; or if the param are illegal, ret is -1
 **/
n4 _swap(vp v0, vp v1, n4 len);

/**
 * description	: to get the minimum value of the two value a and b of the same type
 * @in-a		: the first value
 * @in-b		: the second value
 * @out-ret		: the minimum value
 **/
#define MIN(a, b) ({	\
	typeof(a) _a = (a);	\
	typeof(b) _b = (b);	\
	(vd)(&_a == &_b);	\
	(_a < _b ? _a : _b); })
#define	MIN3(a, b, c)			(MIN(a, MIN(b, c)))
#define	MIN4(a, b, c, d)		(MIN(MIN3(a, b c), d))
#define	MIN5(a, b, c, d, e)		(MIN(MIN4(a, b, c, d), e))

/**
 * description	: to get the mid value of @a and @b and @c <br/><br/>
 *
 * in-@a		: the first value
 * in-@b		: the second value
 * in-@c		: the third value
 *
 * out-@ret		: the mid value
 **/
#define MID(a, b, c) ({	\
	typeof(a) _a = a;	\
	typeof(b) _b = b;	\
	typeof(c) _c = c;	\
	(vd)(&_a == &_b);	\
	(vd)(&_a == &_c);	\
	typeof(a) _mid = a;	\
	if (IS_LE(a, b)) { /*a <= b*/	\
		if (IS_GE(a, c)) { /*a <= b, a >= c*/	\
			_mid = a;	\
		} else if (IS_GE(b, c)) { /*a <= b, a <= c,b >= c*/	\
			_mid = c;	\
		} else { /*a <= b,b <= c*/	\
			_mid = b;	\
		}	\
	} else { /*a >= b*/	\
		if (IS_LE(a, c)) { /*a >= b, a <= c*/	\
			_mid = a;	\
		} else if (IS_GE(b, c)) { /* a >= b, a >= c, b >= c*/.	\
			_mid = b;	\
		} else {	/* a >= b, a >= c, b <= c*/	\
			_mid = c;	\
		}	\
	}	\
	_mid;	\
})

/**
 * description	: to get the maximum value of the two value a and b of the same type
 * in-@a		: the first value
 * in-@b		: the second value
 * out-@ret		: the maximum value
 **/
#define MAX(a, b) ({	\
	typeof(a) _a = (a);	\
	typeof(b) _b = (b);	\
	(vd)(&_a == &_b);	\
	(_a < _b ? _b : _a); })
#define	MAX3(a, b, c)			(MAX(a, MAX(b, c)))
#define	MAX4(a, b, c, d)		(MAX(MAX3(a, b c), d))
#define	MAX5(a, b, c, d, e)		(MAX(MAX4(a, b, c, d), e))

#include "lisk_type.h"
#include "lisk_public.h"

#define VAL_MAX(v) \
({ \
	typeof(v) _max = 0; \
	if (SAME_TYPE(v, n1)) {	\
		_max = N1_MAX;	\
	}	\
	if (SAME_TYPE(v, n2)) {	\
		_max = N2_MAX;	\
	}	\
	if (SAME_TYPE(v, n4)) {	\
		_max = N4_MAX;	\
	}	\
	if (SAME_TYPE(v, n8)) {	\
		_max = N8_MAX;	\
	}	\
	if (SAME_TYPE(v, un1)) {	\
		_max = UN1_MAX;	\
	}	\
	if (SAME_TYPE(v, u2)) {	\
		_max = UN2_MAX;	\
	}	\
	if (SAME_TYPE(v, u4)) {	\
		_max = UN4_MAX;	\
	}	\
	if (SAME_TYPE(v, u8)) {	\
		_max = UN8_MAX;	\
	}	\
	if (SAME_TYPE(v, f4)) {	\
		_max = F4_MAX;	\
	}	\
	if (SAME_TYPE(v, f8)) {	\
		_max = F8_MAX;	\
	}	\
    _max; \
})

#define VAL_MIN(v) \
({ \
	typeof(v) _min = 0; \
	if (SAME_TYPE(v, n1)) {	\
		_min = N1_MIN;	\
	}	\
	if (SAME_TYPE(v, n2)) {	\
		_min = N2_MIN;	\
	}	\
	if (SAME_TYPE(v, n4)) {	\
		_min = N4_MIN;	\
	}	\
	if (SAME_TYPE(v, n8)) {	\
		_min = N8_MIN;	\
	}	\
	if (SAME_TYPE(v, un1)) {	\
		_min = UN1_MIN;	\
	}	\
	if (SAME_TYPE(v, u2)) {	\
		_min = UN2_MIN;	\
	}	\
	if (SAME_TYPE(v, u4)) {	\
		_min = UN4_MIN;	\
	}	\
	if (SAME_TYPE(v, u8)) {	\
		_min = UN8_MIN;	\
	}	\
	if (SAME_TYPE(v, f4)) {	\
		_min = F4_MIN;	\
	}	\
	if (SAME_TYPE(v, f8)) {	\
		_min = F8_MIN;	\
	}	\
    _min; \
})

/**
 * description	: get absolute value <br/><br/>
 *
 * in-@a		: the test value
 *
 * out-@ret		: the absolute value
 **/
#define	ABS(a) ({	\
	typeof(a) _a = a;	\
	if (_a < 0) {	\
	_a = (-1 * _a + 1 - 1);	\
	}	\
	_a;	\
})

/**
 * description	: get absolute diff <br/><br/>
 *
 * in-@a		: the first value
 * in-@b		: the second value
 *
 * out-@ret		: the absolute difference between a and b
 **/
#define	ABS_DIFF(a, b) ({	\
	typeof(a) _a = a;	\
	typeof(b) _b = b;	\
	(vd)(&_a == &b);	\
	((_a < _b) ? (_b - _a) : (_a - _b));	\
})


u2 hton2(u2 val);
u2 ntoh2(u2 val);
u4 hton4(u4 val);
u4 ntoh4(u4 val);
u8 hton8(u8 val);
u8 ntoh8(u8 val);
/**
 * description	: convert the sequence of a num @x from native to seq <br/><br/>
 *
 * in-@x		: input param
 *
 * out-@ret		: net value of @x
 **/
#define NET_VALUE(x) ({	\
	 (sizeof((x)) == 2) ? hton2((x)) : ((sizeof((x)) == 4) ? hton4((x)) : ((sizeof((x)) == 8) ? hton8((x)) : (x)));		\
	 })
/**
 * description	: convert the sequence of a num @x from net to native <br/><br/>
 *
 * in-@x		: input param
 *
 * out-@ret		: native value of @x
 **/
#define HOST_VALUE(x) ({	\
	 (sizeof((x)) == 2) ? ntoh2((x)) : ((sizeof((x)) == 4) ? ntoh4((x)) : ((sizeof((x)) == 8) ? ntoh8((x)) : (x)));		\
	 })

/**
 * description	: cyet another host value. onvert the sequence of a num @x with no type to another host type <br/><br/>
 *
 * in-@x		: input param
 *
 * out-@ret		: another type value of @x
 **/
#define	YAHV2(x)	NET_VALUE((u2)(x))
#define	YAHV4(x)	NET_VALUE((u4)(x))
#define	YAHV8(x)	NET_VALUE((u8)(x))

#ifdef __cplusplus
	}
#endif

#endif
