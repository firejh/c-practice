/**
 * FILE     : tool.c
 * DESC     : common struct & varialbe
 * AUTHOR   : v0.1 written by Alex Stocks
 * DATE     : on April 5, 2015
 * LICENCE  : GPL 2.0
 * MOD      :
 **/

#include "tool.h"

const char* type_name[] = {"string", "list", "set", "hash", "zset", "none"};

int connect_instance(redis_instance* inst)
{
    struct timeval timeout = {1, 0};
    redisContext* context = redisConnectWithTimeout(inst->ip, inst->port, timeout);
    if (context == nil) {
        FATAL("connect to old redis failed");
        return -1;
    }
    if (context->err) {
        FATAL("connect to old redis failed: %s", context->errstr);
        return - 1;
    }

    if (inst->password && strlen(inst->password)) {
        redisReply* reply = (redisReply*)redisCommand(context, "auth %s", inst->password);
        if (reply == nil || reply->type == REDIS_REPLY_ERROR) {
            PRINT_REPLY_ERROR(reply);
            return -1;
        }
        freeReplyObject(reply);
    }

    inst->cxt = context;

    return 0;
}

void disconnect_instance(redis_instance* inst)
{
    if (nil != inst->cxt) {
        redisFree(inst->cxt);
        inst->cxt = nil;
    }
}

int select_db(redis_instance* inst)
{
    long long num = 0;

    redisReply* reply = (redisReply*)redisCommand(inst->cxt, "select %u", inst->db);
    if (reply == nil || !(reply->len) || strcmp(reply->str, "OK")) {
        PRINT_REPLY_ERROR(reply);
        return -1;
    }
    freeReplyObject(reply);

    return 0;
}

long long get_key_num(redis_instance* inst)
{
    long long num = 0;

    redisReply* reply = (redisReply*)redisCommand(inst->cxt, "select %u", inst->db);
    if (reply == nil || !(reply->len) || strcmp(reply->str, "OK")) {
        PRINT_REPLY_ERROR(reply);
        return -1;
    }
    freeReplyObject(reply);

    reply = (redisReply*)redisCommand(inst->cxt, "dbsize");
    assert(reply != nil);
    assert(REDIS_REPLY_INTEGER == reply->type);
    num = reply->integer;
    freeReplyObject(reply);

    return num;
}

long long get_lastsave(redis_instance* inst)
{
    long long time = 0;

    redisReply* reply = (redisReply*)redisCommand(inst->cxt, "lastsave");
    assert(reply != nil);
    assert(REDIS_REPLY_INTEGER == reply->type);
    time = reply->integer;
    freeReplyObject(reply);

    return time;
}

int get_key_type(redis_instance* inst, char* key, size_t len)
{
    assert(nil != inst);
    assert(nil != key);
    assert(0 != len);

    redisReply* reply = (redisReply*)redisCommand(
                inst->cxt, "type %b", key, len);
    assert(reply != nil);
    assert(reply->type == REDIS_REPLY_STATUS);
    assert(0 != reply->len);
    assert(nil != reply->str);

    int flag = 0;
    for (flag = 0; flag < KUNKOWN; flag++) {
        if (0 == strcmp(type_name[flag], reply->str)) {
            break;
        }
    }

    freeReplyObject(reply);

    return flag;
}

long long get_key_pttl(redis_instance* inst, char* key, size_t len)
{
    assert(nil != inst);
    assert(nil != key);
    assert(0 != len);

    redisReply* reply = (redisReply*)redisCommand(
                inst->cxt, "pttl %b", key, len);
    assert(reply != nil);
    assert(reply->type == REDIS_REPLY_INTEGER);

    long long pttl = reply->integer;

    freeReplyObject(reply);

    return pttl;
}

long long get_list_size(redis_instance* inst, char* list, size_t len)
{
    long long num = 0;
    redisReply* reply = (redisReply*)redisCommand(inst->cxt, "llen %b", list, len);
    assert(reply != nil);
    assert(reply->type == REDIS_REPLY_INTEGER);

    num = reply->integer;

    freeReplyObject(reply);

    return num;
}

void print_list(redis_instance* inst, char* list, size_t len)
{
    long index = 0;
    do {
        redisReply* reply = (redisReply*)redisCommand(
                    inst->cxt, "lrange %b %lld %lld",
                    list, len, index, index + SCAN_INC - 1);
        assert(reply != nil);
        assert(reply->type == REDIS_REPLY_ARRAY);
        if (0 == reply->elements) {
            break;
        }

        redisReply* reply1;
        for (size_t i = 0; i < reply->elements; i++)  {
            reply1 = reply->element[i];
            assert(reply1 != nil);
            assert(reply1->len != 0);

            printf("\n    [cursor:%ld, key:%s],", index++, reply1->str);
        }

        if (reply->elements < SCAN_INC) {
            freeReplyObject(reply);
            break;
        }

        freeReplyObject(reply);
    } while(1);
}

void migrate_list(redis_instance* src, redis_instance* dst, char* list, size_t len)
{
    long index = 0;
    do {
        redisReply* reply = (redisReply*)redisCommand(
                    src->cxt, "lrange %b %lld %lld",
                    list, len, index, index + SCAN_INC - 1);
        assert(reply != nil);
        assert(reply->type == REDIS_REPLY_ARRAY);
        if (0 == reply->elements) {
            break;
        }

        redisReply* reply1;
        redisReply* reply2;
        for (size_t i = 0; i < reply->elements; i++)  {
            reply1 = reply->element[i];
            assert(reply1 != nil);
            assert(reply1->len != 0);
            reply2 = (redisReply*)redisCommand(dst->cxt, "rpush %b %b",
                        list, len, reply1->str, (size_t)(reply1->len));
            assert(reply2 != nil);
            freeReplyObject(reply2);
        }

        if (reply->elements < SCAN_INC) {
            freeReplyObject(reply);
            break;
        }

        freeReplyObject(reply);
    } while(1);
}

long long get_set_size(redis_instance* inst, char* set, size_t len)
{
    long long num = 0;
    redisReply* reply = (redisReply*)redisCommand(inst->cxt, "scard %b", set, len);
    assert(reply != nil);
    assert(reply->type == REDIS_REPLY_INTEGER);

    num = reply->integer;

    freeReplyObject(reply);

    return num;
}

void print_set(redis_instance* inst, char* set, size_t len)
{
    long index = 0;
    long long cursor = 0;
    redisReply* keys;
    do {
        redisReply* reply = (redisReply*)redisCommand(
                    inst->cxt, "sscan %b %lld count %lld",
                    set, len, cursor, SCAN_INC);
        assert(reply != nil);
        assert(reply->type == REDIS_REPLY_ARRAY);
        assert(2 == reply->elements);
        assert(REDIS_REPLY_STRING == reply->element[0]->type);
        assert(REDIS_REPLY_ARRAY == reply->element[1]->type);

        char* key;
        redisReply *keys = reply->element[1];
        for (size_t i = 0; i < keys->elements; i++)  {
            key = keys->element[i]->str;
            printf("\n    [cursor:%ld, key:%s],", index++, key);
        }

        cursor = strtoll(reply->element[0]->str, (char**)nil, 10);
        freeReplyObject(reply);
    } while(cursor != 0);
}

void print_set2(redis_instance* inst, char* set, size_t len)
{
    redisReply* reply = (redisReply*)redisCommand(
                inst->cxt, "smembers %b", set, len);
    assert(reply != nil);
    assert(reply->type == REDIS_REPLY_ARRAY);

    for (size_t i = 0; i < reply->elements; i++)  {
        printf("\n    [cursor:%lu, key:%s],", i, reply->element[i]->str);
    }

    freeReplyObject(reply);
}

void migrate_set(redis_instance* src, redis_instance* dst, char* set, size_t len)
{
    long long cursor = 0;
    redisReply* keys;
    do {
        redisReply* reply = (redisReply*)redisCommand(
                    src->cxt, "sscan %b %lld count %lld",
                    set, len, cursor, SCAN_INC);
        assert(reply != nil);
        assert(reply->type == REDIS_REPLY_ARRAY);
        assert(2 == reply->elements);
        assert(REDIS_REPLY_STRING == reply->element[0]->type);
        assert(REDIS_REPLY_ARRAY == reply->element[1]->type);

        redisReply* reply1;
        redisReply *keys = reply->element[1];
        for (size_t i = 0; i < keys->elements; i++)  {
            reply1 = (redisReply*)redisCommand(dst->cxt, "sadd %b %b",
                     set, len,
                     keys->element[i]->str,
                     (size_t)(keys->element[i]->len));
            assert(reply1 != nil);
            freeReplyObject(reply1);
        }

        cursor = strtoll(reply->element[0]->str, (char**)nil, 10);
        freeReplyObject(reply);
    } while(cursor != 0);
}

void migrate_set2(redis_instance* src, redis_instance* dst, char* set, size_t len)
{
    redisReply* reply = (redisReply*)redisCommand(
                src->cxt, "smembers %b", set, len);
    assert(reply != nil);
    assert(reply->type == REDIS_REPLY_ARRAY);

    redisReply* reply1;
    for (size_t i = 0; i < reply->elements; i++)  {
        reply1 = (redisReply*)redisCommand(dst->cxt, "sadd %b %b",
                    set, len,
                    reply->element[i]->str,
                    (size_t)(reply->element[i]->len));
        assert(reply1 != nil);
        freeReplyObject(reply1);
    }

    freeReplyObject(reply);
}

long long get_sset_size(redis_instance* inst, char* sset, size_t len)
{
    long long num = 0;
    redisReply* reply = (redisReply*)redisCommand(inst->cxt, "zcard %b", sset, len);
    assert(reply != nil);
    assert(reply->type == REDIS_REPLY_INTEGER);

    num = reply->integer;

    freeReplyObject(reply);

    return num;
}

void print_sset(redis_instance* inst, char* sset, size_t len)
{
    /*
    kv = (redisReply*)redisCommand(inst->cxt, "zrange %s 0 -1 withscores", key);
    assert (kv != nil);
    assert(kv->type == REDIS_REPLY_ARRAY);
    assert(kv->elements != 0);

    pline("{idx:%lu, sset:%s, member-score num:%lu,", index++, key, kv->elements);
    for (unsigned j = 0; j < kv->elements; j += 2) {
    pline("    {idx:%u, member:%s, score:%s},", j, kv->element[i]->str, kv->element[i + 1]->str);
    }
    pline("}\n");
    freeReplyObject(kv);
    */
    long index = 0;
    long long cursor = 0;
    redisReply* keys;
    do {
        redisReply* reply = (redisReply*)redisCommand(
                    inst->cxt, "zscan %b %lld count %lld",
                    sset, len, cursor, SCAN_INC);
        assert(reply != nil);
        assert(reply->type == REDIS_REPLY_ARRAY);
        assert(2 == reply->elements);
        assert(REDIS_REPLY_STRING == reply->element[0]->type);
        assert(REDIS_REPLY_ARRAY == reply->element[1]->type);

        redisReply *keys = reply->element[1];
        for (size_t i = 0; i < keys->elements; i += 2)  {
            printf("\n    [cursor:%u, key:%s, value:%s],",
                   index++, keys->element[i]->str, keys->element[i + 1]->str);
        }

        cursor = strtoll(reply->element[0]->str, (char**)nil, 10);
        freeReplyObject(reply);
    } while(cursor != 0);
}

void print_sset2(redis_instance* inst, char* sset, size_t len)
{
    redisReply* kv = (redisReply*)redisCommand(inst->cxt,
                "zrange %b 0 -1 withscores", sset, len);
    assert(kv != nil);
    assert(kv != nil);
    assert(kv->elements != 0);

    for (size_t i = 0; i < kv->elements; i += 2) {
        printf("\n    [cursor:%lu, key:%s, value:%s],",
               i / 2, kv->element[i]->str, kv->element[i + 1]->str);
    }

    freeReplyObject(kv);
}

void migrate_sset(redis_instance* src, redis_instance* dst, char* sset, size_t len)
{
    /*
    kv = (redisReply*)redisCommand(src->cxt, "zrange %s 0 -1 withscores", key);
    assert (kv != nil);
    assert(kv->type == REDIS_REPLY_ARRAY);
    assert(kv->elements != 0);

    pline("{idx:%lu, sset:%s, member-score num:%lu,", index++, key, kv->elements);
    for (unsigned j = 0; j < kv->elements; j += 2) {
    pline("    {idx:%u, member:%s, score:%s},", j, kv->element[i]->str, kv->element[i + 1]->str);
    }
    pline("}\n");
    freeReplyObject(kv);
    */
    long long cursor = 0;
    redisReply* keys;
    do {
        redisReply* reply = (redisReply*)redisCommand(
                    src->cxt, "zscan %b %lld count %lld",
                    sset, len, cursor, SCAN_INC);
        assert(reply != nil);
        assert(reply->type == REDIS_REPLY_ARRAY);
        assert(2 == reply->elements);
        assert(REDIS_REPLY_STRING == reply->element[0]->type);
        assert(REDIS_REPLY_ARRAY == reply->element[1]->type);

        redisReply* reply1;
        redisReply *keys = reply->element[1];
        for (size_t i = 0; i < keys->elements; i += 2)  {
            reply1 = (redisReply*)redisCommand(dst->cxt, "zadd %b %b %b",
                        sset, len,
                        keys->element[i + 1]->str, (size_t)(keys->element[i + 1]->len),
                        keys->element[i]->str, (size_t)(keys->element[i]->len));
            assert(reply1 != nil);
            freeReplyObject(reply1);
        }

        cursor = strtoll(reply->element[0]->str, (char**)nil, 10);
        freeReplyObject(reply);
    } while(cursor != 0);
}

void migrate_sset2(redis_instance* src, redis_instance* dst, char* sset, size_t len)
{
    redisReply* kv = (redisReply*)redisCommand(src->cxt,
                "zrange %b 0 -1 withscores", sset, len);
    assert(kv != nil);
    assert(kv->type == REDIS_REPLY_ARRAY);
    assert(kv->elements != 0);

    redisReply* reply1;
    for (size_t i = 0; i < kv->elements; i += 2) {
        reply1 = (redisReply*)redisCommand(dst->cxt, "zadd %b %b %b",
                    sset, len,
                    kv->element[i + 1]->str, (size_t)(kv->element[i + 1]->len),
                    kv->element[i]->str, (size_t)(kv->element[i]->len));
        assert(reply1 != nil);
        freeReplyObject(reply1);

    }
    freeReplyObject(kv);
}

long long get_hashtable_size(redis_instance* inst, char* hash, size_t len)
{
    long long num = 0;
    redisReply* reply = (redisReply*)redisCommand(inst->cxt, "hlen %b", hash, len);
    assert(reply != nil);
    assert(reply->type == REDIS_REPLY_INTEGER);

    num = reply->integer;

    freeReplyObject(reply);

    return num;
}

void print_hashtable(redis_instance* inst, char* hash, long long len)
{
    long index = 0;
    long long cursor = 0;
    redisReply* keys;
    do {
        redisReply* reply = (redisReply*)redisCommand(
                    inst->cxt, "hscan %b %lld count %lld",
                    hash, (size_t)(len), cursor, SCAN_INC);
        assert(reply != nil);
        assert(reply->type == REDIS_REPLY_ARRAY);
        assert(2 == reply->elements);
        assert(REDIS_REPLY_STRING == reply->element[0]->type);
        assert(REDIS_REPLY_ARRAY == reply->element[1]->type);

        redisReply *keys = reply->element[1];
        for (size_t i = 0; i < keys->elements; i += 2)  {
            printf("\n    [cursor:%ld, key:%s, value:%s],",
                        index++, keys->element[i]->str, keys->element[i+1]->str);
        }

        cursor = strtoll(reply->element[0]->str, (char**)nil, 10);
        freeReplyObject(reply);
    } while(cursor != 0);
}

void print_hashtable2(redis_instance* inst, char* hash, long long len)
{
    redisReply* kv = (redisReply*)redisCommand(inst->cxt, "hgetall %b", hash, len);
    assert(kv != nil);
    assert(kv->type == REDIS_REPLY_ARRAY);
    assert(kv->elements != 0);

    for (size_t i = 0; i < kv->elements; i += 2)  {
        printf("\n    [cursor:%lu, key:%s, value:%s],",
                    i / 2, kv->element[i]->str, kv->element[i+1]->str);
    }
    freeReplyObject(kv);
}

void migrate_hashtable(redis_instance* src, redis_instance* dst, char* hash, long long len)
{
    long long cursor = 0;
    redisReply* keys;
    do {
        redisReply* reply = (redisReply*)redisCommand(
                    src->cxt, "hscan %b %lld count %lld",
                    hash, (size_t)(len), cursor, SCAN_INC);
        assert(reply != nil);
        assert(reply->type == REDIS_REPLY_ARRAY);
        assert(2 == reply->elements);
        assert(REDIS_REPLY_STRING == reply->element[0]->type);
        assert(REDIS_REPLY_ARRAY == reply->element[1]->type);

        redisReply* reply1;
        redisReply* keys = reply->element[1];
        for (size_t i = 0; i < keys->elements; i += 2)  {
            reply1 = (redisReply*)redisCommand(dst->cxt, "hset %b %b %b",
                        hash, len,
                        keys->element[i]->str, (size_t)(keys->element[i]->len),
                        keys->element[i + 1]->str, (size_t)(keys->element[i + 1]->len));
            assert(reply1 != nil);
            freeReplyObject(reply1);
        }

        cursor = strtoll(reply->element[0]->str, (char**)nil, 10);
        freeReplyObject(reply);
    } while(cursor != 0);
}

void migrate_hashtable2(redis_instance* src, redis_instance* dst, char* hash, long long len)
{
    redisReply* kv = (redisReply*)redisCommand(src->cxt, "hgetall %b", hash, len);
    assert(kv != nil);
    assert(kv->type == REDIS_REPLY_ARRAY);
    assert(kv->elements != 0);

    redisReply* reply = nil;
    for (size_t i = 0; i < kv->elements; i += 2)  {
        reply = (redisReply*)redisCommand(dst->cxt, "hset %b %b %b",
                    hash, len,
                    kv->element[i]->str, (size_t)(kv->element[i]->len),
                    kv->element[i + 1]->str, (size_t)(kv->element[i + 1]->len));
        assert(reply != nil);
        freeReplyObject(reply);
    }
    freeReplyObject(kv);
}

void print_key(redis_instance* inst, char* key, size_t len)
{
    long long size = 0;
    int flag = get_key_type(inst, key, len);
    long long pttl = get_key_pttl(inst, key, len);
    printf("{key:%s, type:%s, pttl:%lldms, db:%lu,",
            key, type_name[flag], pttl, inst->db);
    switch (flag) {
        case KSTRING: {
            redisReply* kv = (redisReply*)redisCommand(inst->cxt, "get %b", key, len);
            assert(kv != nil);
            assert(kv->type == REDIS_REPLY_STRING);
            assert(kv->len != 0);
            pline(" value:%s}", kv->str);
            freeReplyObject(kv);
        }
        break;

        case KHASH: {
            size = get_hashtable_size(inst, key, len);
            printf(" size:%lld", size);
            if (size) {
                printf(",\n  kvs:[");
                print_hashtable(inst, key, len);
                pline("\b \n  ]");
            }
            pline("}");
        }
        break;

        case KLIST: {
            size = get_list_size(inst, key, len);
            printf(" size:%lld", size);
            if (size) {
                printf(",\n  values:[");
                print_list(inst, key, len);
                pline("\b \n  ]");
            }
            pline("}");
        }
        break;

        case KSET: {
            size = get_set_size(inst, key, len);
            printf(" size:%lld", size);
            if (size) {
                printf(",\n  values:[");
                print_set(inst, key, len);
                pline("\b \n  ]");
            }
            pline("}");
        }
        break;

        case KSSET: {
            size = get_sset_size(inst, key, len);
            printf(" size:%lld", size);
            if (size) {
                printf(",\n  values:[");
                print_sset(inst, key, len);
                pline("\b \n  ]");
            }
            pline("}");
        }
        break;

        case KNONE: {
            FATAL("none type of key:%s", key);
        }
        break;

        case KUNKOWN: {
            FATAL("unknown type of key:%s", key);
        }
    }
}

void print_key2(redis_instance* inst, char* key, size_t len)
{
    long long size = 0;
    int flag = get_key_type(inst, key, len);
    long long pttl = get_key_pttl(inst, key, len);
    printf("{key:%s, type:%s, pttl:%lldms, db:%lu,",
            key, type_name[flag], pttl, inst->db);
    switch (flag) {
        case KSTRING: {
            redisReply* kv = (redisReply*)redisCommand(inst->cxt, "get %b", key, len);
            assert(kv != nil);
            assert(kv->type == REDIS_REPLY_STRING);
            assert(kv->len != 0);
            pline(" value:%s}", kv->str);
            freeReplyObject(kv);
        }
        break;

        case KHASH: {
            size = get_hashtable_size(inst, key, len);
            printf(" size:%lld", size);
            if (size) {
                printf(",\n  kvs:[");
                print_hashtable2(inst, key, len);
                pline("\b \n  ]");
            }
            pline("}");
        }
        break;

        case KLIST: {
            size = get_list_size(inst, key, len);
            printf(" size:%lld", size);
            if (size) {
                printf(",\n  values:[");
                print_list(inst, key, len);
                pline("\b \n  ]");
            }
            pline("}");
        }
        break;

        case KSET: {
            size = get_set_size(inst, key, len);
            printf(" size:%lld", size);
            if (size) {
                printf(",\n  values:[");
                print_set2(inst, key, len);
                pline("\b \n  ]");
            }
            pline("}");
        }
        break;

        case KSSET: {
            size = get_sset_size(inst, key, len);
            printf(" size:%lld", size);
            if (size) {
                printf(",\n  values:[");
                print_sset2(inst, key, len);
                pline("\b \n  ]");
            }
            pline("}");
        }
        break;

        case KNONE: {
            FATAL("none type of key:%s", key);
        }
        break;

        case KUNKOWN: {
            FATAL("unknown type of key:%s", key);
        }
    }
}

void migrate_key(redis_instance* src, redis_instance* dst, char* key, size_t len)
{
    long long size = 0;
    int flag = get_key_type(src, key, len);
    switch (flag) {
        case KSTRING: {
            redisReply* kv = (redisReply*)redisCommand(src->cxt, "get %b", key, len);
            assert(kv != nil);
            assert(kv->type == REDIS_REPLY_STRING);
            assert(kv->len != 0);

            redisReply* reply = (redisReply*)redisCommand(
                        dst->cxt, "set %b %b", key, len, kv->str, (size_t)(kv->len));
            freeReplyObject(reply);

            freeReplyObject(kv);
        }
        break;

        case KHASH: {
            migrate_hashtable(src, dst, key, len);
        }
        break;

        case KLIST: {
            migrate_list(src, dst, key, len);
        }
        break;

        case KSET: {
            migrate_set(src, dst, key, len);
        }
        break;

        case KSSET: {
            migrate_sset(src, dst, key, len);
        }
        break;

        case KNONE: {
            FATAL("none type of key:%s", key);
        }
        break;

        case KUNKOWN: {
            FATAL("unknown type of key:%s", key);
        }
    }
}

void migrate_key2(redis_instance* src, redis_instance* dst, char* key, size_t len)
{
    long long size = 0;
    int flag = get_key_type(src, key, len);
    switch (flag) {
        case KSTRING: {
            redisReply* kv = (redisReply*)redisCommand(src->cxt, "get %b", key, len);
            assert(kv != nil);
            assert(kv->type == REDIS_REPLY_STRING);
            assert(kv->len != 0);

            redisReply* reply = (redisReply*)redisCommand(
                        dst->cxt, "set %b %b", key, len, kv->str, (size_t)(kv->len));
            freeReplyObject(reply);

            freeReplyObject(kv);
        }
        break;

        case KHASH: {
            migrate_hashtable2(src, dst, key, len);
        }
        break;

        case KLIST: {
            migrate_list(src, dst, key, len);
        }
        break;

        case KSET: {
            migrate_set2(src, dst, key, len);
        }
        break;

        case KSSET: {
            migrate_sset2(src, dst, key, len);
        }
        break;

        case KNONE: {
            FATAL("none type of key:%s", key);
        }
        break;

        case KUNKOWN: {
            FATAL("unknown type of key:%s", key);
        }
    }
}
