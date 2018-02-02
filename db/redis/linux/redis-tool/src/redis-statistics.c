/**
 * FILE     : redis-statistics.c
 * DESC     : statistics redis instance
 * AUTHOR   : v0.1 written by Alex Stocks
 * DATE     : on April 23, 2015
 * LICENCE  : copyright @baidu inc
 * MOD      :
 **/

#include "tool.h"

#define MAX_INST_NUM 16
#define MAX_DB_NUM 400

typedef struct db_quota_tag   {
    int inst;
    int db;
    int quota;
    int used;
} db_quota_t, *db_quota_p, db_quota_a[1];

size_t g_db_quota_size = 0;
db_quota_t g_db_quota[10000] = {0};

void statistics_db(redis_instance* inst)
{
    // select db
    select_db(inst);

    redisReply* reply = (redisReply*)redisCommand(inst->cxt, "keys dbname-*");
    assert(reply != NULL);
    char* dbname;
    size_t dbname_len;
    redisReply* kv;
    char tmp[64];
    for (size_t idx = 0; idx < reply->elements; idx++) {
        dbname = reply->element[idx]->str;
        dbname_len = (size_t)(reply->element[idx]->len);
        kv = (redisReply*)redisCommand(inst->cxt,
                    "hget %b cluster_id", dbname, dbname_len);
        if (kv == NULL) {
            memset(g_db_quota + g_db_quota_size, 0, sizeof(g_db_quota[g_db_quota_size]));
            freeReplyObject(kv);
            continue;
        }
        if (kv->type != REDIS_REPLY_STRING) {
            memset(g_db_quota + g_db_quota_size, 0, sizeof(g_db_quota[g_db_quota_size]));
            freeReplyObject(kv);
            continue;
        }
        int i = 0;
        for (i = 0; i < kv->len; i++) {
            if (kv->str[i] == ':') {
                break;
            }
        }
        if (kv->len < i) {
            continue;
        }
        strncpy(tmp, kv->str, i);
        tmp[i] = '\0';
        g_db_quota[g_db_quota_size].inst = (int)strtol(tmp, (char**)NULL, 10);
        g_db_quota[g_db_quota_size].db = (int)strtol(kv->str + i + 1, (char**)NULL, 10);
        if (MAX_INST_NUM <= g_db_quota[g_db_quota_size].inst
                    || MAX_DB_NUM <= g_db_quota[g_db_quota_size].db) {
            FATAL("illegal inst:db %s", kv->str);
        }
        freeReplyObject(kv);

        kv = (redisReply*)redisCommand(inst->cxt,
                    "hget %b disk_quota", dbname, dbname_len);
        if (kv == NULL) {
            memset(g_db_quota + g_db_quota_size, 0, sizeof(g_db_quota[g_db_quota_size]));
            freeReplyObject(kv);
            continue;
        }
        if (kv->type != REDIS_REPLY_STRING) {
            memset(g_db_quota + g_db_quota_size, 0, sizeof(g_db_quota[g_db_quota_size]));
            freeReplyObject(kv);
            continue;
        }
        g_db_quota[g_db_quota_size].quota = (int)strtol(kv->str, (char**)NULL, 10);
        freeReplyObject(kv);

        kv = (redisReply*)redisCommand(inst->cxt,
                    "hget %b used_disk_quota", dbname, dbname_len);
        if (kv == NULL) {
            memset(g_db_quota + g_db_quota_size, 0, sizeof(g_db_quota[g_db_quota_size]));
            freeReplyObject(kv);
            continue;
        }
        if (kv->type != REDIS_REPLY_STRING) {
            memset(g_db_quota + g_db_quota_size, 0, sizeof(g_db_quota[g_db_quota_size]));
            freeReplyObject(kv);
            continue;
        }
        g_db_quota[g_db_quota_size].used = (int)strtol(kv->str, (char**)NULL, 10);
        freeReplyObject(kv);

        g_db_quota_size ++;
    }
    freeReplyObject(reply);
}

int main(int argc, char** argv)
{
    if (argc < 3) {
        FATAL("Right usage: ./redis-statistic redis-meta-ip redis-meta-port");
        return -1;
    }

    char* ip = argv[1];
    unsigned short port = (unsigned short)strtoul(argv[2], (char**)NULL, 10);

    redis_instance meta = {
        .port = port,
        .db = 0,
        .cxt = NULL,
    };
    strncpy(meta.ip, ip, sizeof(meta.ip));
    strcpy(meta.password, "");
    if (connect_instance(&meta)) {
        FATAL("failed to connect instance{%s-%u}", meta.ip, meta.port);
        return -1;
    }

    statistics_db(&meta);

    disconnect_instance(&meta);

    if (!g_db_quota_size) {
        return -1; 
    }

    int dbnum_total = 0;
    int dbnum[MAX_INST_NUM] = {0};
    int quota[MAX_INST_NUM] = {0};
    int used[MAX_INST_NUM] = {0};
    for (size_t idx = 0; idx < g_db_quota_size; idx++) {
        if (MAX_INST_NUM <= g_db_quota[idx].inst) {
            FATAL("idx:%zu, db_quota{inst:%d, db:%d, quota:%d, used:%d}",
                        idx, g_db_quota[idx].inst, g_db_quota[idx].db, g_db_quota[idx].quota, g_db_quota[idx].used);
            continue;
        }
        dbnum_total++;
        dbnum[g_db_quota[idx].inst] ++;
        quota[g_db_quota[idx].inst] += g_db_quota[idx].quota;
        used[g_db_quota[idx].inst] += g_db_quota[idx].used;
    }

    for (int i = 0; i < MAX_INST_NUM; i++) {
        pline("instance{id:%d, dbnum:%d, quota:%d, used:%d}",
                    i, dbnum[i], quota[i], used[i]);
    }
    pline("total db number:%d", dbnum_total);

    return 0;
}

