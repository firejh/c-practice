/**
 * FILE     : tool.h
 * DESC     : common struct & varialbe
 * AUTHOR   : v0.1 written by Alex Stocks
 * DATE     : on April 5, 2015
 * LICENCE  : GPL 2.0
 * MOD      :
 **/

#ifndef __TOOL_H__
#define __TOOL_H__

#if  (defined(__cplusplus))
    extern "C" {
#endif

#include "fmacros.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include "hiredis.h"

#if  (defined(__cplusplus))
    }
#endif

#define SCAN_INC            10

#define PRINT_REPLY_ERROR(reply) do { \
    if (reply && reply->type == REDIS_REPLY_ERROR && reply->len && reply->str) { \
        FATAL("reply error:%s", reply->str); \
    } \
} while (0); \
freeReplyObject(reply);

typedef struct redis_instanceTag {
    char  ip[64];
    unsigned short port;
    char password[1024];
    unsigned long db;
    redisContext* cxt;
} redis_instance;

enum KV_TYPE
{
    KSTRING = 0,
    KLIST,
    KSET,
    KHASH,
    KSSET,
    KNONE,
    KUNKOWN,
};

extern const char* type_name[];

int connect_instance(redis_instance* inst);
void disconnect_instance(redis_instance* inst);
int select_db(redis_instance* inst);

long long get_key_num(redis_instance* inst);
long long get_lastsave(redis_instance* inst);

int get_key_type(redis_instance* inst, char* key, size_t len);
long long get_key_pttl(redis_instance* inst, char* key, size_t len);

long long get_list_size(redis_instance* inst, char* list, size_t len);
long long get_set_size(redis_instance* inst, char* set, size_t len);
long long get_sset_size(redis_instance* inst, char* sset, size_t len);
long long get_hashtable_size(redis_instance* inst, char* hash, size_t len);

void print_key(redis_instance* inst, char* key, size_t len);
void print_key2(redis_instance* inst, char* key, size_t len);
void migrate_key(redis_instance* src, redis_instance* dst, char* key, size_t len);
void migrate_key2(redis_instance* src, redis_instance* dst, char* key, size_t len);

#endif

