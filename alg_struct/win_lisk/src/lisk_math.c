/** 
 * FILE     :lisk_math.c
 * DESC     :math lib
 * AUTHOR   :v1.0 written by Alex Stocks
 * DATE     :on Aug 15, 2012
 * MOD      :modified by * on June 2, 2012
 **/
// #ifdef __cplusplus
// extern "C" {
// #endif

#include <lisk_math.h>
#include <lisk_public.h>
#include <math.h>
#include <stdlib.h>		//strtoul, strtoull

// #ifdef __cplusplus
// 	}
// #endif

///////////////////////////////////////////////////
//prime
///////////////////////////////////////////////////
n4 is_num_prime(n8 num) {
	n4 flag = 0;
	n8 idx = 0;
	if (num < 2 || num % 2 == 0 || num % 3 == 0) {
		return -1;
	}

	for (idx = 3; idx < num; idx++) {
		if (num < idx * idx) {
			break;
		}

		if (num % idx == 0) {
			flag = -1;
			break;
		}
		if (6 * idx + 1 < num && num % (6 * idx + 1) == 0) {
			flag = -1;
			break;
		}
		if (6 * idx - 1 < num && num % (6 * idx - 1) == 0) {
			flag = -1;
			break;
		}
	}

	return flag;
}

n8 next_prime(n8 num) {
	n4 ret = 0;
	n8 prime = 0;	
	if (num < 0) {
		num = 3;
	}

	for (prime = num + 1; num < prime; prime++) {
		ret = is_num_prime(prime);
		if (ret == 0) {
			break;
		}
	}

	if (prime <= num) {
		prime = -1;
	}

	return prime;
}

n8 get_prime_arr(n8 max_num, n8 prime_num, n8* prime_arr) {
	n4 ret = 0;
	n8 cur_prime_num = 0;
	n8 num = 0;
	if (max_num < 1 || prime_num <1 || NULL == prime_arr) {
		return -1;
	}

//	for (num = max_num; 1 < num && cur_prime_num < prime_num; num--) {
//		ret = is_num_prime(num);
//		if (ret == 0) {
//			prime_arr[cur_prime_num++] = num;
//		}
//	}
	cur_prime_num = max_num;
	for (num = 0; num < prime_num; num++)	{
		cur_prime_num = next_prime(cur_prime_num);
		if (IS_EQ(cur_prime_num, -1))	{
			return -1;
		}
		prime_arr[num] = cur_prime_num;
	}

	return cur_prime_num;
}

////////////////////////////////////////////////////////////
//sqrt, pow
////////////////////////////////////////////////////////////
//from John Carmack, very quickly, but not so absolutely
f4 qsqrt( f4 number ) {
	n4 i = 0;
	f4 x2 = 0;
	f4 y = 0;
	const f4 threehalfs = 1.5f;
	union {
		n4 i;
		f4 f;
	}i_y;

	if (IS_LT(number, 0)) {
		return -1.0f;
	}

	x2 = number * 0.5f;
	i_y.f = y;
	i   = *(n4*)(&i_y.i);   // evil floating point bit level hacking
//	i   = 0x5f3759df - ( i >> 1 ); // what the fuck? Carmack
	i   = 0x5f375a86 - ( i >> 1 );  //Lomont
	i_y.i = i;
	y   = *(f4*)(&(i_y.f));
	y   = y * (threehalfs - (x2*y*y)); // 1st iteration
	y   = y * (threehalfs - (x2*y*y)); // 2nd iteration, this can be removed
	y   = y * (threehalfs - (x2*y*y)); // 3nd iteration
	if (IS_FZ(y)) {
		y = LESS;
	}
	
	return 1.0f / y;
}

n4 is_pow_of_2(u8 x) {
	if (IS_ZR((x) & (x-1))) {
		return 0;
	}

	return 1;
}

u8 next_pow_of_2(u8 x) {
	if (IS_ZR(is_pow_of_2(x))) {
	  return x;
	}

	x |= x>>1;
	x |= x>>2;
	x |= x>>4;
	x |= x>>8;
	x |= x>>16;

	return x+1;
}

n4 qintpow(n4 x, u4 n) {
	n4 result = 0;
	if (n == 0) {
		return 1;
	} else {
		while ((n & 1) == 0) {
			n >>= 1;
			x *= x;
		}
	}
	result = x;
	n >>= 1;
	while (n != 0) {
		x *= x;
		if ((n & 1) != 0) {
		  result *= x;
		}
		n >>= 1;
	}
	return result;
}

////////////////////////////////////////////////////////////
//swap
////////////////////////////////////////////////////////////
vd swap2B(u2* a, u2* b) {
	*a = *a ^ *b;
	*b = *a ^ *b;
	*a = *a ^ *b;
}

vd swap4B(u4* a, u4* b) {
	*a = *a ^ *b;
	*b = *a ^ *b;
	*a = *a ^ *b;
}

vd swap8B(u8* a, u8* b) {
	*a = *a ^ *b;
	*b = *a ^ *b;
	*a = *a ^ *b;
}

vd swap12B(un1* a, un1* b) {
	swap8B((u8*)a, (u8*)b);
	swap4B((u4*)(a + 8), (u4*)(b + 8));
}

n4 _swap(vp v0, vp v1, n4 len) {
	un1* c0 = NULL;
	un1* c1 = NULL;
	n4 idx = 0;
	if (IS_NL(v0) || IS_NL(v1) || IS_LE(len, 0)) {
		return -1;
	}

	if (IS_BE((n1*)(v1), (n1*)(v0), ((n1*)(v0) + len - 1)) ||
		IS_BE((n1*)(v0), (n1*)(v1), ((n1*)(v1) + len-1))) {
		return -1;
	}

	for (idx = 0; idx < len; idx++) {
		c0 = (un1*)(v0) + idx;
		c1 = (un1*)(v1) + idx;
		*c0 = *c0 ^ *c1;
		*c1 = *c0 ^ *c1;
		*c0 = *c0 ^ *c1;
	}

	return 0;
}

//////////////////////////////////////////
//ntohll htonll
//////////////////////////////////////////

u2 hton2(u2 val) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
	un1* val_ptr = (un1*)(&val);
	SWAP(val_ptr[0], val_ptr[1]);
#endif
	return val;
}

u2 ntoh2(u2 val) {
	return hton2(val);
}

u4 hton4(u4 val) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
	un1* val_ptr = (un1*)(&val);
	SWAP(val_ptr[0], val_ptr[3]);
	SWAP(val_ptr[1], val_ptr[2]);
#endif
	return val;
}

u4 ntoh4(u4 val) {
	return hton4(val);
}

u8 hton8(u8 val) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
	un1* val_ptr = (un1*)(&val);
	SWAP(val_ptr[0], val_ptr[7]);
	SWAP(val_ptr[1], val_ptr[6]);
	SWAP(val_ptr[2], val_ptr[5]);
	SWAP(val_ptr[3], val_ptr[4]);
#endif
	return val;
}

u8 ntoh8(u8 val) {
	return hton8(val);
}

