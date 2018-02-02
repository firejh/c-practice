/******************************************************
# DESC    :
# AUTHOR  : Alex Stocks
# VERSION : 1.0
# LICENCE : Apache License 2.0
# EMAIL   : alexstocks@foxmail.com
# MOD     : 2018-02-02 11:48
# FILE    : a.c
******************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <time.h>

#include "time.h"

#define TIME_USEC   ({                                     \
  struct timeval cur_tv;                                   \
  gettimeofday(&cur_tv, NULL);                             \
  ((unsigned long)(cur_tv.tv_sec) << 32) + cur_tv.tv_usec; \
  })

void output_time_string(unsigned long t)
{
    struct tm tm_res = {0};
    time_t sec = t >> 32;
    unsigned int usec = (unsigned int)t;
    localtime_r((time_t*)&(sec), &tm_res);

    char buf[256];
    // strftime(buf, 256, "%Y:%m:%dT%H:%M:%S:%06d", &tm_res, (int)(t));
    strftime(buf, 256, "%Y:%m:%dT%H:%M:%S:%06d", &tm_res);

    printf("t.sec:%ld, t.usec:%ld, %s\n", sec, usec,  buf);
}

int main(int argc, char** argv)
{
    struct timeval cur_tv;
    gettimeofday(&cur_tv, NULL);
    printf("sec:%ld, usec:%ld\n", cur_tv.tv_sec, cur_tv.tv_usec);

    struct tm tm_res = {0};
    localtime_r((time_t*)&(cur_tv.tv_sec), &tm_res);

    char buf[256];
    strftime(buf, 256, "%Y:%m:%dT%H:%M:%S:%06d", &tm_res);
    printf("%s\n", buf);

    unsigned long t = TIME_USEC;
    output_time_string(t);

    return 0;
}
