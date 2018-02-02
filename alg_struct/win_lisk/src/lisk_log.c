/**
 * FILE		: lisk_log.c
 * DESC		: to keep track of the program
 * AUTHOR	: v0.3.0 written by Alex Stocks
 * DATE		: on March 11, 2012
 * LICENCE	: GPL 2.0
 * MOD		: modified by Alex Stocks on Oct 13, 2012
 **/

#include <lisk_log.h>

const n1* color_string(n4 color)
{
	static const n1* color_string_[] = {
		"",
		"\033[0m",
		"\033[1;34;40m",
		"\033[1;31;40m",
		"\033[1;35;40m",
		"\033[1;37;40m",
		"\033[1;32;40m",
		"\033[1;36;40m",
		"\033[1;48;40m",
		"\033[1;33;40m",
		"\033[1;4;40m",
		"\033[1;5;40m",
		"\033[1;7;40m"
	};

	if (color <= 0 || ESCREEN_COLOR_MAX <= color) {
		color = ESCREEN_COLOR_MIN;
	}

	return color_string_[color];
}

