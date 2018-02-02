/**
 * FILE     : redis-max-kv.cc
 * DESC     : test redis's max value length
 * AUTHOR   : v1.0 written by Alex Stocks
 * DATE     : on May 30, 2015
 * LICENCE  : GPL 2.0
 * MOD      :
 **/

#if  (defined(__cplusplus))
    extern "C" {
#endif

#include "tool.h"


#if  (defined(__cplusplus))
    }
#endif

#include <time.h>
#include <string>
using namespace std;

void insert(redis_instance& inst, string& key, string& value);
void get(redis_instance& inst, string& key, string& value);
void hgetall(redis_instance& inst, string& key);

int main(int argc, char** argv)
{
    if (argc < 4) {
        FATAL("Right usage: ./redis-max-kv ip port password");
        return -1;
    }

    char* ip = argv[1];
    unsigned short port = (unsigned short)strtoul(argv[2], (char**)NULL, 10);
    char* password = argv[4];
    unsigned db= 0;

    redis_instance inst = {0};
    inst.port = port,
    inst.db = db,
    strncpy(inst.ip, ip, sizeof(inst.ip));
    if (password) {
        strncpy(inst.password, password, sizeof(inst.password));
    }
    if (connect_instance(&inst)) {
        FATAL("failed to connect instance{%s-%u}", inst.ip, inst.port);
        return -1;
    }

    string str("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890");
    string key;
    std::string value;

    key = "fruit";
    hgetall(inst, key);

    // 16k
    key = "16K";
    value = "";
    for (int i = 0; i < 265; i++) {
        value += str;
    }
    insert(inst, key, value);

    // 16k
    key = "1M";
    value = "";
    for (int i = 0; i < 16913; i++) {
        value += str;
    }
    insert(inst, key, value);

    // 16M
    key = "16M";
    value = "";
    for (int i = 0; i < 276900; i++) {
        value += str;
    }
    insert(inst, key, value);
    get(inst, key, value);

    // 64M
    key = "64M";
    value = "";
    for (int i = 0; i < 1107600; i++) {
        value += str;
    }
    insert(inst, key, value);

    // 128M
    key = "128M";
    value = "";
    for (int i = 0; i < 1107600*2; i++) {
        value += str;
    }
    insert(inst, key, value);

    disconnect_instance(&inst);

    return 0;
}

void insert(redis_instance& inst, string& key, string& value)
{
    redisReply* reply;
    reply = (redisReply*)redisCommand(inst.cxt, "set %s %s", key.c_str(), value.c_str());
    assert(reply != NULL);
    assert(reply->type == REDIS_REPLY_STATUS);
    pline("set %s result %s", key.c_str(), reply->str);
    freeReplyObject(reply);
}

void get(redis_instance& inst, string& key, string& value)
{
    redisReply* reply;
    reply = (redisReply*)redisCommand(inst.cxt, "get %s", key.c_str());
    assert(reply != NULL);
    assert(reply->type == REDIS_REPLY_STRING);
    value = reply->str;
    pline("get %s result %zu", key.c_str(), value.size());
    freeReplyObject(reply);
}

void hgetall(redis_instance& inst, string& key)
{
    redisReply* reply;
    reply = (redisReply*)redisCommand(inst.cxt, "hgetall %s", key.c_str());
    assert(reply != NULL);
    freeReplyObject(reply);
}

