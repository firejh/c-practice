/**
 * FILE		: color.h
 * DESC		: to output string with color on 8bit-depth screen
 * AUTHOR	: v1.0 written by Alex Stocks
 * DATE		: on June 5, 2014
 * LICENCE	: GPL 2.0
 * MOD		:
 **/

#ifndef __COLOR_H__
#define __COLOR_H__

#if  (defined(__cplusplus))
	extern "C" {
#endif

typedef enum ESCREEN_COLOR_TAG {
	ESCREEN_COLOR_MIN = 0,
	ENORMAL,
	EBLUE,
	ERED,
	EPRUPLE,
	EWHILTE,
	EGREEN,
	EAQUA,
	EHIGH_LIGHT,
	EYELLOW,
	EUNDER_LINE,
	EUNDER_LINE_TWINKLE,
	EUNDER_LINE_TWINKLE_HIGH_LIGHT,
	ESCREEN_COLOR_MAX,
} ESCREEN_COLOR;

/**
 * description	: get color string <br/><br/>
 *
 * in-@color	: ESCREEN_COLOR
 *
 * out-@ret		: color string
 **/
const char* color_string(int color);

#if  (defined(__cplusplus))
	}
#endif

#endif

