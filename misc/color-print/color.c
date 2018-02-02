/**
 * FILE		: color.c
 * DESC		: to output string with color on 8bit-depth screen
 * AUTHOR	: v1.0 written by Alex Stocks
 * DATE		: on June 5, 2014
 * LICENCE	: GPL 2.0
 * MOD		:
 **/

#include "color.h"

const char* color_string(int color)
{
	static const char* color_string_[] = {
		[ESCREEN_COLOR_MIN] = "",
		[ENORMAL] = "\033[0m",
		[EBLUE] = "\033[1;34;40m",
		[ERED] = "\033[1;31;40m",
		[EPRUPLE] = "\033[1;35;40m",
		[EWHILTE] = "\033[1;37;40m",
		[EGREEN] = "\033[1;32;40m",
		[EAQUA] = "\033[1;36;40m",
		[EHIGH_LIGHT] = "\033[1;48;40m",
		[EYELLOW] = "\033[1;33;40m",
		[EUNDER_LINE] = "\033[1;4;40m",
		[EUNDER_LINE_TWINKLE] = "\033[1;5;40m",
		[EUNDER_LINE_TWINKLE_HIGH_LIGHT] = "\033[1;7;40m"
	};

	if (color <= 0 || ESCREEN_COLOR_MAX <= color) {
		color = ESCREEN_COLOR_MIN;
	}

	return color_string_[color];
}

#if defined(COLOR_TEST)

#include <stdio.h>

int main(int argc, char** argv)
{
	int idx;

	for (idx = ESCREEN_COLOR_MIN; idx < ESCREEN_COLOR_MAX; idx++)	{
		printf("%s""hello\n""%s", color_string(idx), color_string(ENORMAL));
	}

	return 0;
}

#endif

