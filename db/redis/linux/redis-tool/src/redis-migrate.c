/**
 * FILE     : redis-migrate.c
 * DESC     : migrate database
 * AUTHOR   : v1.0 written by Alex Stocks
 * DATE     : on April 7, 2015
 * LICENCE  : GPL 2.0
 * MOD      :
 **/

#include <time.h>

#include "tool.h"

void migrate_db(redis_instance* src, redis_instance* dst);
void migrate_db2(redis_instance* src, redis_instance* dst);

int main(int argc, char** argv)
{
    if (argc < 9) {
        FATAL("Right usage: ./redis-migrate src-ip src-port src-password src-db-id "
                                            "dst-ip dst-port dst-password dst-db-id");
        return -1;
    }

    char* ip = argv[1];
    unsigned short port = (unsigned short)strtoul(argv[2], (char**)NULL, 10);
    char* password = argv[3];
    unsigned db=  (unsigned)strtoul(argv[4], (char**)NULL, 10);
    /*
    char* ip = "127.0.0.1";
    unsigned short port = 10000;
    char* password = "redis-instance-password";
    unsigned db = 0;
    */

    redis_instance src = {
        .port = port,
        .db = db,
        .cxt = NULL,
    };
    strncpy(src.ip, ip, sizeof(src.ip));
    strncpy(src.password, password, sizeof(src.password));
    if (connect_instance(&src)) {
        FATAL("failed to connect instance{%s-%u}", src.ip, src.port);
        return -1;
    }

    ip = argv[5];
    port = (unsigned short)strtoul(argv[6], (char**)NULL, 10);
    password = argv[7];
    db=  (unsigned)strtoul(argv[8], (char**)NULL, 10);

    redis_instance dst = {
        .port = port,
        .db = db,
        .cxt = NULL,
    };
    strncpy(dst.ip, ip, sizeof(dst.ip));
    strncpy(dst.password, password, sizeof(dst.password));
    if (connect_instance(&dst)) {
        FATAL("failed to connect instance{%s-%u}", dst.ip, dst.port);
        return -1;
    }

    migrate_db(&src, &dst);

    disconnect_instance(&dst);
    disconnect_instance(&src);

    return 0;
}

void migrate_db(redis_instance* src, redis_instance* dst)
{
    long long num = get_key_num(src);
    if (num <= 0) {
        FATAL("src key num %lld", num);
        return;
    }

    // select db
    select_db(dst);

    long index = 0;
    long long idx = 0;
    redisReply* reply;
    do {
        reply = (redisReply*)redisCommand(src->cxt,
                    "scan %lld count %lld", idx, SCAN_INC);
        assert(reply != NULL);
        if (reply->type == REDIS_REPLY_ERROR
            && !strncmp(reply->str, "ERR unknown command 'scan'", reply->len)) {
            // redis server's version is too old
            migrate_db2(src, dst);
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
            migrate_key(src, dst, keys->element[i]->str, (size_t)(keys->element[i]->len));
        }

        freeReplyObject(reply);
    } while(idx != 0);
}

void migrate_db2(redis_instance* src, redis_instance* dst)
{
    long long num = get_key_num(src);
    if (num <= 0) {
        FATAL("src key num %lld", num);
        return;
    }

    // select db
    select_db(dst);

    redisReply* reply = (redisReply*)redisCommand(src->cxt, "keys *");
    assert(reply != NULL);
    for (size_t idx = 0; idx < reply->elements; idx++) {
        migrate_key2(src, dst, reply->element[idx]->str, (size_t)(reply->element[idx]->len));
    }
    freeReplyObject(reply);
}

