/*
 * Copyright (c) 2009-2012, Pieter Noordhuis <pcnoordhuis at gmail dot com>
 * Copyright (c) 2009-2012, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "hiredis.h"
#include "sds.h"
#include "tool.h"

#define MAX_LONG_LEN 128
#define REDIS_AOF_COMMAND_THRESHOLD 1000000
#define REPLAY_DEFAULT_PIPE_CMDS 100

#define CMD_LONG 1 // return immediately on eof
#define ARG_LONG 0 // retry on eof

/*
 * replay_ctx: contain the replay context
 * @file buffer needed suppose to be not, we use cached IO,
 * @if needed we can increase the cache size
 */
typedef struct replay_ctx {
    off_t size;             // current file size
    off_t pos;              // current file seek position
    off_t diff;             // current file diff

    long processed_cmd;     // processed commands number
    long skipped_cmd;       // unrecoginized commands number

    FILE* fp;               // aof file fp
    sds filename;           // aof file name
    void* buf;              // buffer for file I/O

    sds ip;                 // target ip
    int port;               // target port

    int pipe_cmds;          // commands sent per time
    redisContext*  redis;   // context for replay the commands

    sds filter;             // key filter;
    sds prefix;             // prefix to be replaced
    sds replace_prefix;     // replace prefix to be added for keys
} replay_ctx;

/* confuraton for replay */
typedef struct config {
    int pipe_cmds;
    char*  filename;
    char*  host;
    char*  prefix;
    char*  replace_prefix;
    char*  filter;
    int verbose;
    int only_check;
} config;

static const char*  CMD_SUPPORTED[] = {
    "MSET",
    "MSETNX",
    "DEL",
    "SET",
    "HMSET",
    "EXPIRE",
    "EXPIREAT",
    "PERSIST",
    "PEXPIRE",
    "PEXPIREAT",
    "DUMP",
    "EXISTS",
    "PTTL",
    "RESTORE",
    "TTL",
    "TYPE",
    "APPEND",
    "BITCOUNT",
    "DECR",
    "DECRBY",
    "GET",
    "GETBIT",
    "GETRANGE",
    "GETSET",
    "INCR",
    "INCRBY",
    "INCRBYFLOAT",
    "MGET",
    "PSETEX",
    "SETBIT",
    "SETEX",
    "SETNX",
    "SETRANGE",
    "STRLEN",
    "HDEL",
    "HEXISTS",
    "HGET",
    "HGETALL",
    "HINCRBY",
    "HINCRBYFLOAT",
    "HKEYS",
    "HLEN",
    "HMGET",
    "HSET",
    "HSETNX",
    "HVALS",
    "LINDEX",
    "LINSERT",
    "LLEN",
    "LPOP",
    "LPUSH",
    "LPUSHX",
    "LRANGE",
    "LREM",
    "LSET",
    "LTRIM",
    "RPOP",
    "RPUSH",
    "RPUSHX",
    "SADD",
    "SCARD",
    "SISMEMBER",
    "SMEMBERS",
    "SPOP",
    "SRANDMEMBER",
    "SREM",
    "ZADD",
    "ZCARD",
    "ZCOUNT",
    "ZINCRBY",
    "ZRANGE",
    "ZRANGEBYSCORE",
    "ZRANK",
    "ZREM",
    "ZREMRANGEBYRANK",
    "ZREMRANGEBYSCORE",
    "ZREVRANGE",
    "ZREVRANGEBYSCORE",
    "ZREVRANK",
    "ZSCORE",
};

/*
 * util section
 *
 */

/* Return the UNIX time in microseconds */
long long ustime(void) {
    struct timeval tv;
    long long ust;

    gettimeofday(&tv, NULL);
    ust = ((long long)tv.tv_sec)*1000000;
    ust += tv.tv_usec;
    return ust;
}

int consume_new_line(char* buf)
{
    if (strncmp(buf,"\r\n",2) != 0) {
        return REDIS_ERR;
    }
    return REDIS_OK;
}

void safe_free_args(int argc, sds* argvs)
{
    if (argvs == NULL) {
        return;
    }

    for (int i = 0; i < argc; i++) {
        sdsfree(argvs[i]);
    }

    free(argvs);
}

void refresh_aof_stat(replay_ctx* repctx)
{
    struct stat sb;
    FILE* fp = repctx->fp;
    if (fstat(fileno(fp),&sb) == -1) {
        perr("failed to get stat of file %s\n", repctx->filename);
        return;
    }
    repctx->size = sb.st_size;
    repctx->pos = ftello(fp);
    repctx->diff = repctx->size - repctx->pos;
}

int safe_read_long(replay_ctx* ctx, char prefix, long* target, int return_immediately)
{
    char buf[MAX_LONG_LEN],* eptr;
    FILE* fp = ctx->fp;
    long len = 0, remain = MAX_LONG_LEN, bytes = 0;
    int retry = 1;

    eptr = buf;

    memset(buf, '\0', sizeof(buf));

    do {
        if (fgets(eptr, remain, fp) == NULL) {
            /* return 0 if eof is read at the 1st try, and this is an expected way */
            if (retry == 1 && return_immediately) {
                *target = 0;
                return REDIS_OK;
            } else {
                ++retry;
                usleep(1000000);
                continue;
            }
        }

        if (buf[0] != prefix) {
            return REDIS_ERR;
        }

        len = strtol(buf + 1, &eptr, 10);

        if (consume_new_line(eptr) == REDIS_OK) {
            *target = len;
            return REDIS_OK;
        }

        bytes = (long) (eptr - buf);
        remain -= bytes;
        ++retry;
    } while(remain > 0);

    return REDIS_ERR;
}

int safe_read_string(replay_ctx* ctx, sds* target, int key)
{
    sds argsds;
    long len, remain, bytes;
    char* buf = NULL, *eptr;
    long prelen = 0, replen = 0;
    int retry = 1;

    int should = key && ctx->prefix != NULL && ctx->replace_prefix != NULL;

    if (should) {
        prelen = sdslen(ctx->prefix);
        replen = sdslen(ctx->replace_prefix);
    }

    if (safe_read_long(ctx, '$', &len, ARG_LONG) == REDIS_ERR) {
        perr("failed to read argument length, bad format file");
        goto error;
    }

    buf = (char*)malloc(len + 2);
    if (buf == NULL) {
        perr("failed to malloc memory for command argument");
        goto error;
    }

    eptr = buf;
    remain = len + 2;
    do {
        /* do the file position check */
        if (retry % 2 == 0) {
            usleep(1000000);
        }

        bytes = fread(eptr, 1, remain, ctx->fp);
        remain -= bytes;
        eptr = eptr + bytes;
        ++retry;
    } while(remain > 0);

    eptr = buf + len;
    if (consume_new_line(eptr) == REDIS_ERR) {
        perr("no new line subffix found");
        goto error;
    }

    long real = len;

    if (should) {
        if (!strncmp(ctx->prefix, buf, sdslen(ctx->prefix))){
            real = len + replen - prelen;
        } else {
            should = 0;
        }
    }

    buf[len] = '\0';
    argsds = sdsnewlen(NULL, real);

    if (should) {
        sdscpylen(argsds, ctx->replace_prefix, replen);
        sdscatlen(argsds, buf + prelen, len - prelen);
    } else {
        sdscpylen(argsds, buf, len);
    }

   * target = argsds;
    free(buf);
    return REDIS_OK;

error:
   * target = NULL;

    if (buf != NULL) {
        free(buf);
    }

    return REDIS_ERR;
}

int supported(sds cmd)
{
    int numcommands = sizeof(CMD_SUPPORTED) / sizeof(char*);

    for (int i = 0; i < numcommands; i++) {
        if (!strcasecmp(CMD_SUPPORTED[i], cmd)) {
            return 1;
        }
    }

    return 0;
}

int get_redis_command(replay_ctx* ctx, int* argc, sds** argv)
{
    long len = 0, j, step = 1;
    int normal = 0, key = 0;
    sds* tmp;

    if (safe_read_long(ctx, '*', &len, CMD_LONG) != REDIS_OK) {
        goto error;
    }

    if (len == 0) {
        *argv = NULL;
        *argc = 0;
        return REDIS_OK;
    }

    *argc = len;
    tmp = (sds*)malloc(len * sizeof(sds));
    if (tmp == NULL) {
        goto error;
    }

    if (safe_read_string(ctx, &tmp[0], 0) != REDIS_OK) {
        goto error;
    }

    if (!strcasecmp(tmp[0], "SELECT")) {
        normal = 0;
        step = 0;
    } else if (!strcasecmp(tmp[0], "MSET")) {
        normal = 0;
        step = 2;
    } else if (!strcasecmp(tmp[0], "MSETNX")) {
        normal = 0;
        step = 2;
    } else if (!strcasecmp(tmp[0], "DEL")) {
        normal = 0;
        step = 1;
    } else {
        normal = 1;
        step = 0;
    }

    for (j = 1; j < len; j++) {
        if (normal) {
            if ( j == 1) {
                key = 1;
            } else {
                key = 0;
            }
        } else {
            if ( step == 0 ) {
                key = 0;
            } else if ( (j + 1) % step == 0 ) {
                key = 1;
            } else {
                key = 0;
            }
        }
        if (safe_read_string(ctx, &tmp[j], key) != REDIS_OK) {
            goto error;
        }
    }

    *argv = tmp;
    return REDIS_OK;

error:
    safe_free_args(len, *argv);
    *argv = NULL;
    return REDIS_ERR;
}

void cmd_to_string(int argc, const char** argv)
{
    for(int i = 0; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");
}

int get_pipe_reply(replay_ctx* ctx)
{
    redisContext* c = ctx->redis;
    void* reply = NULL;
    int ret = REDIS_OK;

    ret = redisGetReply(c, &reply);
    if (c->err & (REDIS_ERR_IO | REDIS_ERR_EOF)) {
        perr("connection error, failed to get reply");
    } else if (c->err) {
        perr("command execution error");
    }
    if (reply != NULL) {
        freeReplyObject(reply);
    }
    return ret;
}

void check_aof(replay_ctx* ctx)
{
    sds* argv = NULL;
    int argc;
    long loops = 0;

    while(1) {
        if (get_redis_command(ctx, &argc, &argv) != REDIS_OK) {
            perr("wrong format command at position %ld", (long)ctx->pos);
            goto end;
        }

        if (argv == NULL) {
            perr("reach end of file");
            goto end;
        }

        if (!supported(argv[0])) {
            sds cmd = sdsjoin(argv, argc, (char*)(" "));
            perr("unsupported cmd found: %s", cmd);
            sdsfree(cmd);
            ctx->skipped_cmd += 1;
        } else {
            ctx->processed_cmd += 1;
        }

        loops += 1;
        safe_free_args(argc, argv);

        refresh_aof_stat(ctx);
        if ( loops >= 10000 ) {
             perr("progress: [processed:%ld] [skipped:%ld] [filesize:%ld] [postion:%ld] [diff:%ld]", \
                ctx->processed_cmd, ctx->skipped_cmd, (long)ctx->size, (long)ctx->pos, (long)ctx->diff);
             loops = 0;
        }
    }

end:
    refresh_aof_stat(ctx);
    perr("progress: [processed:%ld] [skipped:%ld] [filesize:%ld] [postion:%ld] [diff:%ld]", \
        ctx->processed_cmd, ctx->skipped_cmd, (long)ctx->size, (long)ctx->pos, (long)ctx->diff);
    return;
}

void process_aof(replay_ctx* ctx)
{
    unsigned long loops = 0;
    int j, argc;
    int cmds = 0;
    sds*  argv = NULL;

    //long long getcommand = 0;
    //long long runcommand = 0;
    //long long aftercommand = 0;

    perr("start to process aof file: %s", ctx->filename);
    while(1) {
        cmds = 0;
        while(cmds < ctx->pipe_cmds) {
            if (get_redis_command(ctx, &argc, &argv) != REDIS_OK) {
                perr("wrong format command at position %ld", (long)ctx->pos);
                //printf("wrong format command at %x\n", ctx->pos);
                goto error;
            }

            if (argv == NULL) {
                usleep(1000000);
                break;
            }

            if (argc > 1 && ctx->filter != NULL) {
                if (strncasecmp(ctx->filter, argv[1], sdslen(ctx->filter))) {
                    ctx->skipped_cmd += 1;
                    safe_free_args(argc, argv);
                    continue;
                }
            }

            if (!supported(argv[0])) {
                sds cmd = sdsjoin(argv, argc, (char*)(" "));
                perr("unsupported cmd found: %s", cmd);
                sdsfree(cmd);
                ctx->skipped_cmd += 1;
            } else if (!strcasecmp(argv[0], "MSET") || !strcasecmp(argv[0], "MSETNX")) {
                char** subargv = NULL;
                size_t* subargvlen = NULL;
                for(j = 1; j < argc; j++) {
                    subargv = (char**)malloc(3 * sizeof(char*));
                    if (subargv == NULL) {
                        goto error;
                    }

                    subargvlen = malloc(3 * sizeof(size_t));
                    if (subargvlen == NULL) {
                        free(subargv);
                        goto error;
                    }

                    subargv[0] = argv[0] + 1;
                    subargv[1] = argv[j++];
                    subargv[2] = argv[j];
                    subargvlen[0] = sdslen(argv[0]) - 1;
                    subargvlen[1] = sdslen(subargv[1]);
                    subargvlen[2] = sdslen(subargv[2]);

                    if (redisAppendCommandArgv(ctx->redis, 3, (const char**)subargv, subargvlen) != REDIS_OK) {
                        perr("failed to append sub command to redis output buffer");
                        goto error;
                    }

                    free(subargvlen);
                    free(subargv);
                    ++cmds;
                }
            } else {
                size_t* argvlen = malloc(argc*  sizeof(size_t));
                if (argvlen == NULL) {
                    goto error;
                }

                for(j = 0; j < argc; j++) {
                    argvlen[j] = sdslen(argv[j]);
                }

                if (redisAppendCommandArgv(ctx->redis, argc, (const char**)argv, argvlen) != REDIS_OK) {
                    perr("failed to append command to redis output buffer");
                    goto error;
                }
                free(argvlen);
                argvlen = NULL;
                ++cmds;
            }
            safe_free_args(argc, argv);
        }

        pinfo("%d commands sent in this loop", cmds);

        for(int i = 0; i < cmds; i++) {
            if (get_pipe_reply(ctx) != REDIS_OK) {
                goto error;
            }
        }

        loops += (cmds + 1);
        ctx->processed_cmd += cmds;

        refresh_aof_stat(ctx);
        if ((loops >= 10000) || (ctx->diff < REDIS_AOF_COMMAND_THRESHOLD)) {
            perr("progress: [processed:%ld] [skipped:%ld] [filesize:%ld] [postion:%ld] [diff:%ld]", \
                ctx->processed_cmd, ctx->skipped_cmd, (long)ctx->size, (long)ctx->pos, (long)ctx->diff);
            loops = 0;
        }
    }

error:
    return;
}

int ping_redis(replay_ctx* ctx)
{
    redisContext* rc = ctx->redis;
    redisCommand(rc, "INFO");
    if (rc->err) {
        return REDIS_ERR;
    }
    return REDIS_OK;
}

void free_reply_ctx(replay_ctx* ctx)
{
    if (ctx->fp != NULL) {
        fclose(ctx->fp);
    }

    if (ctx->buf != NULL) {
        free(ctx->buf);
    }
    if (ctx->filename != NULL) {
        sdsfree(ctx->filename);
    }

    if (ctx->ip != NULL) {
        sdsfree(ctx->ip);
    }

    if (ctx->filter != NULL) {
        sdsfree(ctx->filter);
    }

    if (ctx->prefix != NULL) {
        sdsfree(ctx->prefix);
    }

    if (ctx->replace_prefix != NULL) {
        sdsfree(ctx->replace_prefix);
    }

    if (ctx->redis != NULL) {
        redisFree(ctx->redis);
    }
}

replay_ctx* init_replay_ctx(config* conf)
{
    replay_ctx* ctx = (replay_ctx*)malloc(sizeof(replay_ctx));
    if (NULL == ctx) {
        perr("Failed to malloc replay_ctx");
        return NULL;
    }

    ctx->filename = sdsnew(conf->filename);
    ctx->fp = fopen(ctx->filename, "r+");
    if (ctx->fp == NULL) {
        perr("Cannot open file: %s", conf->filename);
        goto error;
    }

    /* set large buffer for file IO */
    ctx->buf = (void*)malloc(1024 * 1024);
    setbuf(ctx->fp, ctx->buf);

    char* p = conf->host;
    char* s = strchr(p, ':');
    *s = '\0';
    ctx->ip = sdsnew(p);
    ctx->port = atoi(s+1);

    ctx->pos = 0;
    ctx->processed_cmd = 0;
    ctx->skipped_cmd = 0;

    ctx->pipe_cmds = conf->pipe_cmds;

    ctx->prefix = NULL;
    ctx->replace_prefix = NULL;
    if (conf->prefix != NULL && conf->replace_prefix != NULL) {
        ctx->prefix = sdsnew(conf->prefix);
        ctx->replace_prefix = sdsnew(conf->replace_prefix);
    }

    ctx->filter = NULL;
    if (conf->filter != NULL) {
        ctx->filter = sdsnew(conf->filter);
    }

    refresh_aof_stat(ctx);

    ctx->redis = redisConnect(ctx->ip, ctx->port);
    if (ctx->redis->err) {
        perr("failed to connecto to %s:%d\n", ctx->ip, ctx->port);
        goto error;
    }
    return ctx;

error:
    free_reply_ctx(ctx);
    return NULL;
}

void parse_args(int argc, char** argv, config* conf)
{
    conf->filename = NULL;
    conf->host = NULL;
    conf->filter = NULL;
    conf->prefix = NULL;
    conf->replace_prefix = NULL;
    conf->pipe_cmds = REPLAY_DEFAULT_PIPE_CMDS;
    conf->verbose = 0;
    conf->only_check = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--file") == 0) {
            conf->filename = argv[i+1];
            ++i;
        }

        if (strcmp(argv[i], "--dest") == 0) {
            conf->host = argv[i+1];
            ++i;
        }

        if (strcmp(argv[i], "--prefix") == 0) {
            conf->prefix = argv[i+1];
            ++i;
        }

        if (strcmp(argv[i], "--replace_prefix") == 0) {
            conf->replace_prefix = argv[i+1];
            ++i;
        }

        if (strcmp(argv[i], "--pipe_cmds") == 0) {
            conf->pipe_cmds = atoi(argv[i+1]);
            ++i;
        }
        if (strcmp(argv[i], "--filter") == 0) {
            conf->filter = argv[i+1];
            ++i;
        }
        if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0) {
            conf->verbose = 1;
        }
        if (strcmp(argv[i], "--only_check") == 0) {
            conf->only_check = 1;
        }
    }
}

void show_usage(char** argv)
{
    pline("Usage: %s", argv[0]);
    pline("\t\t--file <file.aof>: aof file");
    pline("\t\t--dest <ip:port>: the target ip & port");
    pline("\t\t[--filter <filter>]: key prefix filter, only the keys match this filter would be processed");
    pline("\t\t[--prefix <prefix>]: old key prefix to be replaced");
    pline("\t\t[--replace_prefix <replace>]: new key prefix to be used");
    pline("\t\t[--pipe_cmds <cmds>]: cmds number for pipeline");
    pline("\t\t[--only_check]: only do check");
    pline("\t\t[-v|--verbose]");
}

int main(int argc, char** argv)
{
    replay_ctx* ctx = NULL;
    if (argc < 4) {
        show_usage(argv);
        exit(1);
    }

    config* conf = (config*)malloc(sizeof(config));
    if (conf == NULL) {
        exit(1);
    }

    parse_args(argc, argv, conf);
    if (conf->filename == NULL || conf->host == NULL) {
        exit(1);
    }

    ctx = init_replay_ctx(conf);
    if (ctx == NULL) {
        perr("failed to init replay context");
        exit(1);
    }

    int only_check = conf->only_check;
    free(conf);

    if (only_check) {
        check_aof(ctx);
    } else {
        process_aof(ctx);
    }

    return 0;
}
