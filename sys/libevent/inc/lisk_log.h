/**
 * FILE		:lisk_log.h
 * DESC		:to keep track of the program
 * AUTHOR	:v0.3.0 written by Alex Stocks
 * DATE		:on Sep 19, 2012
 * MOD		:modified by * on June 2, 2012
 ***/

#ifndef __LISK_LOG_H__
#define __LISK_LOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <lisk_public.h>

typedef enum ESCREEN_COLOR_TAG {
	ESCREEN_COLOR_MIN = 0,
	ENORMAL,
	EBLUE,
	ERED,
	EPRUPLE,
	EWHITE,
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
const n1* color_string(int color);

#define	print			printf
#define	pline(fmt, ...)	printf(fmt"\n", ##__VA_ARGS__)
#define	pinfo(fmt, ...)	printf("%s%s-%s-%d: "fmt"%s\n", color_string(EWHITE),	\
						((n1*)__FILE__), (n1*)__func__,	\
						(n4)__LINE__, ##__VA_ARGS__, color_string(ENORMAL))
#define	perr(fmt, ...)	fprintf(stderr, "%s%s-%s-%d: error: "fmt"\n%s",	\
						color_string(ERED), ((n1*)__FILE__),	\
						(n1*)__func__, (n4)__LINE__, ##__VA_ARGS__,	\
						color_string(ENORMAL))

#define RET_PTR(RET, FMT, ...)		 \
	do {    \
		if (IS_NL(RET) && IS_NZR(strlen(FMT))) {    \
			printf("%s-%s-%d ", __FILE__, __FUNCTION__, __LINE__);	\
			printf("failed, ret is nil, "FMT"\n", ##__VA_ARGS__);    \
		}   \
		return RET; \
	} while (0);

#define RET_NL(FMT, ...)		RET_PTR(nil, FMT, ##__VA_ARGS__)

#define RET_INT(RET, FMT, ...)		\
	do {    \
		if (IS_NZR(RET) && IS_NZR(strlen(FMT))) {   \
			printf("%s-%s-%d ", __FILE__, __FUNCTION__, __LINE__);	\
			printf("ret = %d, "FMT"\n", RET, ##__VA_ARGS__); \
		}   \
		return (RET);   \
	} while (0);

#define	RET_VD(fmt, ...)	\
	do {	\
		if (strlen(fmt))	{ \
			printf(fmt"\n", ##__VA_ARGS__);	\
		}	\
		return;	\
	} while(0);

#ifdef __cplusplus
}
#endif

#endif

