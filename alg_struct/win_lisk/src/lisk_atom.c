/**
 * FILE		:lisk_atom.c
 * DESC		:to supply atom
 * AUTHOR	:v0.3.0 written by Alex Stocks
 * DATE		:on Sep 17, 2012
 * MOD		:modified by * on June 2, 2012
 ***/

#include <lisk_atom.h>
#include <lisk_public.h>

n4 get_bit(n1* ptr, n4 pos)
{
	n1	res;

	if (IS_NL(ptr) || IS_MN(pos))	{
		return -1;
	}

	ptr += pos / (sizeof(n1) * 8);
	res = (*ptr) & (0x1 << (pos % (sizeof(n1) * 8)));

	return res;
}

n4 set_bit1(n1* ptr, n4 pos)
{
	static const un1	mask[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

	if (IS_NL(ptr) || IS_MN(pos))	{
		return -1;
	}

	ptr += pos / (sizeof(n1) * 8);
	*ptr |= mask[pos % (sizeof(n1) * 8)];

	return 0;
}

n4 set_bit0(n1* ptr, n4 pos)
{
	static const un1	mask[8] = {0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F};

	if (IS_NL(ptr) || IS_MN(pos))	{
		return -1;
	}

	ptr += pos / (sizeof(n1) * 8);
	*ptr &= mask[pos % (sizeof(n1) * 8)];

	return 0;
}

