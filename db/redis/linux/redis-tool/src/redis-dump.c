/**
 * FILE     : redis-dump.c
 * DESC     : output database
 * AUTHOR   : v1.0 written by Alex Stocks
 * DATE     : on April 5, 2015
 * LICENCE  : GPL 2.0
 * MOD      :
 **/

#include <time.h>

#include "tool.h"

void print_db(redis_instance* inst);
void print_db2(redis_instance* inst);

int main(int argc, char** argv)
{
    if (argc < 5) {
        FATAL("Right usage: ./redis-dump ip port password db_id");
        return -1;
    }

    char* ip = argv[1];
    unsigned short port = (unsigned short)strtoul(argv[2], (char**)NULL, 10);
    char* password = argv[3];
    unsigned db=  (unsigned)strtoul(argv[4], (char**)NULL, 10);

    redis_instance inst = {
        .port = port,
        .db = db,
        .cxt = NULL,
    };
    strncpy(inst.ip, ip, sizeof(inst.ip));
    strncpy(inst.password, password, sizeof(inst.password));
    if (connect_instance(&inst)) {
        FATAL("failed to connect instance{%s-%u}", inst.ip, inst.port);
        return -1;
    }

    print_db(&inst);

    disconnect_instance(&inst);

    return 0;
}

void print_db(redis_instance* inst)
{
    long long num = get_key_num(inst);
    if (num <= 0) {
        FATAL("get_key_num() = %lld", num);
        return;
    }
    long long last = get_lastsave(inst);
    struct tm tm_res = {0};
    localtime_r((time_t*)&last, &tm_res);
    char buf[1024];
    snprintf(buf, sizeof(buf),
             "%04d-%02d-%02d_%02d-%02d-%02d",
             tm_res.tm_year + 1900, tm_res.tm_mon + 1,
             tm_res.tm_mday, tm_res.tm_hour,
             tm_res.tm_min, tm_res.tm_sec);
    pline("{db:%lu, size:%lld, lastsave:%s}", inst->db, num, buf);

    long index = 0;
    long long idx = 0;
    redisReply* reply;
    do {
        reply = (redisReply*)redisCommand(inst->cxt, "scan %lld count %lld", idx, SCAN_INC);
        assert(reply != NULL);
        if (reply->type == REDIS_REPLY_ERROR
            && !strncmp(reply->str, "ERR unknown command 'scan'", reply->len)) {
            // redis server's version is too old
            print_db2(inst);
            freeReplyObject(reply);
            break;
        }
        assert(reply->type == REDIS_REPLY_ARRAY);
        assert(2 == reply->elements);
        assert(REDIS_REPLY_STRING == reply->element[0]->type);
        assert(REDIS_REPLY_ARRAY == reply->element[1]->type);

        idx = strtoll(reply->element[0]->str, (char**)NULL, 10);
        redisReply* keys = reply->element[1];
        for (size_t i = 0; i < keys->elements; i++)  {
            print_key(inst, keys->element[i]->str, (size_t)(keys->element[i]->len));
        }

        freeReplyObject(reply);
    } while(idx != 0);
}

void print_db2(redis_instance* inst)
{
    redisReply* reply = (redisReply*)redisCommand(inst->cxt, "keys *");
    assert(reply != NULL);
    for (size_t idx = 0; idx < reply->elements; idx++) {
        print_key2(inst, reply->element[idx]->str, (size_t)(reply->element[idx]->len));
    }
    freeReplyObject(reply);
}

