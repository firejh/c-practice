/**
 * FILE     : conf.c
 * DESC     : [string]key-[string]value dictonary
 * AUTHOR   : v0.3.0 written by Alex Stocks
 * DATE     : on Oct 25, 2012
 * LICENCE  : GPL 2.0
 * MOD      :
 **/

#include "redis.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "log.h"
#include "dict.h"
#include "sds.h"

//////////////////////////////////////////
//get_opt
//////////////////////////////////////////

#define PTR_OFFSET(ptr0, ptr1) ({                  \
    (((char*)(ptr0) < (char*)(ptr1)) ?             \
    ((char*)(ptr1) - (char*)(ptr0)) :              \
    ((char*)(ptr0) - (char*)(ptr1)));              \
})

static int getLineKey(char* line, char** key, size_t* size);
static int getLineValue(char* line, char** value, size_t* size);
static int isComment(const char* buf, size_t buf_len);
static int parseLine(dict* dt, sds line);
static int parseArray(dict* dt, sds key, sds value);

#define COMMENT_CHAR0       '#'
#define KEY_CHAR1           '['
#define KEY_CHAR2           ']'
#define CR_CHAR             '\n'
#define EQUAL_CHAR0         '='
#define EQUAL_CHAR1         ':'
#define ARRAY_DELIM         ','
#define DEFAULT_FIRST_KEY   "default"

static const char* suffix = "__lisk_array_size_suffix__";

int getLineKey(char* line, char** key, size_t* size)
{
    char  equal0;
    char  equal1;
    char* ret;

    equal0 = EQUAL_CHAR0;
    equal1 = EQUAL_CHAR1;

    ret = strchr(line, equal0);
    if (!ret) {
        ret = strchr(line, equal1);
        if (!ret) {
            perr("strchr(line = %s, equal1 = %c) = %p", line, equal1, ret);
            return -1;
        }
    }

    *key = line;
    *size = (size_t)PTR_OFFSET(ret, line);

    return 0;
}

int getLineValue(char* line, char** value, size_t* value_size)
{
    char        equal0;
    char        equal1;
    char*       ret;
    unsigned    size;

    size = strlen(line);
    if (!size) {
        return -1;
    }

    equal0 = EQUAL_CHAR0;
    equal1 = EQUAL_CHAR1;

    ret = strchr(line, equal0);
    if (!ret) {
        ret = strchr(line, equal1);
        if (!ret) {
            perr("strchr(line = %p, equal1 = %c) = %p", line, equal1, ret);
            return -2;
        }
    }

    *value = ret + sizeof(char);
    *value_size = (size_t)PTR_OFFSET(line + size, *value);

    return 0;
}

int isComment(const char* buf, size_t buf_len)
{
    int ret = 0;

    ret = -2;
    if (0 < buf_len && buf[0] == COMMENT_CHAR0) {
        ret = 0;
    }

    return ret;
}

int isFirstKey(const char* buf, unsigned buf_len)
{
    int ret = 0;

    ret = -2;
    if (buf[0] == KEY_CHAR1 && buf[buf_len-1] == KEY_CHAR2) {
        ret = 0;
    }

    return ret;
}

int parseLine(dict* dt, sds line)
{
    static char first_key[1024] = {DEFAULT_FIRST_KEY};
    do {
        if (!sdslen(line)) {
            break;
        }

        int ret = isComment((char*)line, sdslen(line));
        if (!ret) {
            break;
        }

        sdstrimspace(line);
        if (!sdslen(line)) {
            break;
        }

        // get first key
        ret = isFirstKey((char*)(line), sdslen(line));
        if (!ret) {
            ((char*)line)[sdslen(line)-1] = '\0';
            snprintf(first_key, sizeof(first_key), "%s", (char*)(line) + 1);
            continue;
        }

        // key
        sds key = sdsnewlen(first_key, strlen(first_key));
        char* key_str;
        size_t length;
        ret = getLineKey((char*)(line), &key_str, &length);
        if (ret || !length) {
            sdsfree(key);
            break;
        }
        sdscatlen(key, (void*)(key_str), length);

        // value
        char* value_str;
        ret = getLineValue((char*)(line), &value_str, &length);
        if (ret || !length) {
            sdsfree(key);
            continue;
        }
        sds value = sdsnewlen(value_str, length);

        dictAdd(dt, (void*)(key), (void*)(value));
        parseArray(dt, key, value);
    } while(0);

    return 0;
}

int parseArray(dict* dt, sds key, sds value)
{
    char* key_str = key;
    char* value_str = value;
    char line_key[strlen(key_str) + 32];

    int idx = 0;
    char limit = ARRAY_DELIM;
    char* start = value_str;
    char* end = value_str + strlen(value_str);
    sds key_;
    sds value_;
    for (;;) {
        if (start >= end) {
            break;
        }
        char* pos = strchr(start, limit);
        if (!pos) {
            break;
        }
        if (pos <= start) {
            start += sizeof(char);
            continue;
        }
        sprintf(line_key, "%s%d", key_str, idx);
        key_ = sdsnew(line_key);
        value_ = sdsnewlen((void*)(start), PTR_OFFSET(start, pos));
        dictAdd(dt, key_, value_);

        start = pos + sizeof(char);
        idx++;
    }

    if (idx) {
        if (start < end) {
            sprintf(line_key, "%s%d", key_str, idx);
            key_ = sdsnew(line_key);
            value_ = sdsnewlen(start, PTR_OFFSET(start, end));
            dictAdd(dt, key_, value_);
            idx++;
        }

        sprintf(line_key, "%s%s", key_str, suffix);
        key_ = sdsnew(line_key);
        char size_str[64];
        sprintf(size_str, "%d", idx);
        value_ = sdsnew(size_str);
        dictAdd(dt, key_, value_);
    }

    return 0;
}

int fileGetLine(int fd, sds line)
{
    off_t cursor = lseek(fd, 0, SEEK_CUR);
    if (cursor < 0) {
        cursor = 0;
    }
    struct stat    fst;
    fstat(fd, &fst);
    size_t file_len = fst.st_size;
    if ((size_t)(cursor + 1) >= file_len) {
        return -1;
    }

    int flag = 0;
    char new_line = '\n';
    char same_line = '\\';
    size_t sub_str_len = 0;
    char buf[1024];
    char* new_line_buf;
    ssize_t offset = 0;
    while (0 < (offset = pread(fd, buf, sizeof(buf)-1, cursor))) {
        buf[offset] = '\0';
        // check '\n' or '\\'
        new_line_buf = strchr(buf, new_line);
        if (new_line_buf) {
            new_line_buf[0] = '\0';
            sub_str_len = strlen(buf);
            flag = 0;
            if (sub_str_len && buf[sub_str_len-1] == same_line) {
                flag = 1;
                sub_str_len --;
            }
            sdscatlen(line, buf, sub_str_len);
            cursor += (sub_str_len + 1 + flag);
            lseek(fd, cursor, SEEK_SET);
            if (!flag) {
                break;
            }
        } else {
            sdscatlen(line, buf, strlen(buf));
            cursor += strlen(buf);
            lseek(fd, cursor, SEEK_SET);
        }
    }

    if (offset <= 0) {
        return -2;
    }

    return 0;
}

unsigned int confSdsHash(const void *key) {
    return dictGenHashFunction((unsigned char*)key, sdslen((char*)key));
}

static int confSdsKeyCompare(
	void *privdata, const void *key1, const void *key2)
{
    int l1,l2;
    DICT_NOTUSED(privdata);

    l1 = sdslen((sds)key1);
    l2 = sdslen((sds)key2);
    if (l1 != l2) return 0;
    return memcmp(key1, key2, l1) == 0;
}

static void confSdsDestructor(void *privdata, void *val)
{
    DICT_NOTUSED(privdata);

    sdsfree(val);
}

/* Db->dict, keys are sds strings, vals are Redis objects. */
static dictType confDictType = {
    confSdsHash,                /* hash function */
    NULL,                       /* key dup */
    NULL,                       /* val dup */
    confSdsKeyCompare,          /* key compare */
    confSdsDestructor,          /* key destructor */
    confSdsDestructor           /* val destructor */
};

int confInit(void** conf_pptr, char* file)
{
    if (!(file) && !(conf_pptr)) {
        perr("@file = %p, @dict = %p", (void*)file, (void*)conf_pptr);
        return -1;
    }

    dict* dt = dictCreate(&confDictType, NULL);
    int fd = open(file, O_RDONLY);
    sds line = sdsnewlen(NULL, 96);
    do {
        sdsclear(line);
        int ret = fileGetLine(fd, line);
        if (ret) {
            break;
        }

        parseLine(dt, line);
    } while (1);
    int ret = 0;
    if (!dictSize(dt)) {
        dt = NULL;
        ret = -1;
    }
    sdsfree(line);
    close(fd);

    *conf_pptr = dt;

    return ret;
}

void confUninit(void** conf_pptr)
{
    if (conf_pptr && *conf_pptr) {
        // dictEntry* de;
        // dictIterator* it = dictGetIterator(*conf_pptr);
        // while ((de = dictNext(it))) { 
        //     sdsfree(dictGetKey(de));
        //     sdsfree(dictGetVal(de));
        // }
        // dictReleaseIterator(it); 
        dictRelease((dict*)(*conf_pptr));
        *conf_pptr = NULL;
    }
}

const char* confGetValue(void* dict, const char* key_str0, const char* key_str1)
{
    if (!dict || !key_str0 || !key_str1) {
        perr("@dict = %p, @key_str0 = %p, @key_str1 = %p", dict, key_str0, key_str1);
        return NULL;
    }

    if (!(strlen(key_str1))) {
        key_str1 = key_str0;
        key_str0 = DEFAULT_FIRST_KEY;
    }

    char key_string[strlen(key_str0) + strlen(key_str1) + 32];
    sprintf(key_string, "%s%s", key_str0, key_str1);
    sds key = sdsnew(key_string);
    dictEntry* de = dictFind(dict, key);
    if (!de) {
        perr("dictFind(dict = %p, key = %s) = nil", dict, key_string);
        return NULL;
    }

    return dictGetVal(de);
}

const char* confGetArrayValue(void* dict, const char* key_str0, const char* key_str1, int idx)
{
    if (!(dict) || !(key_str0) || !(key_str1)) {
        perr("@dict = %p, @key0 = %p, @key1 = %p", dict, key_str0, key_str1);
        return NULL;
    }

    if (!strlen(key_str1)) {
        key_str1 = key_str0;
        key_str0 = DEFAULT_FIRST_KEY;
    }

    char key_string[strlen(key_str0) + strlen(key_str1) + 32];
    sprintf(key_string, "%s%s%d", key_str0, key_str1, idx);
    sds key = sdsnew(key_string);
    dictEntry* de = dictFind(dict, key);
    sdsfree(key);
    if (!de) {
        perr("dictFind(dict = %p, key = %s) = nil", dict, key_string);
        return NULL;
    }

    return dictGetVal(de);
}

int confGetArraySize(void* dict, const char* key_str0, const char* key_str1, size_t* size)
{
    if (!(dict) || !(key_str0) || !(key_str1) ||!(size)) {
        perr("@dict = %p, @key0 = %p, @key1 = %p, @size = %p",
                dict, key_str0, key_str1, (void*)(size));
        return -1;
    }

    if (!strlen(key_str1)) {
        key_str1 = key_str0;
        key_str0 = DEFAULT_FIRST_KEY;
    }

    char line_key[strlen(key_str0) + strlen(key_str1) + 32];
    sprintf(line_key, "%s%s%s", key_str0, key_str1, suffix);
    sds key = sdsnew(line_key);
    dictEntry* de = dictFind(dict, key);
    sdsfree(key);
    if (!de) {
        perr("dictFind(dict = %p, key = %s) = nil", (void*)(dict), line_key);
        return -2;
    }

    *size = strtoul(dictGetVal(de), (char**)NULL, 10);

    return 0;
}

