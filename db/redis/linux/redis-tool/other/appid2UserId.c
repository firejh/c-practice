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

#define FATAL(fmt, ...) printf(fmt"\n", ##__VA_ARGS__)
#define DEBUG(fmt, ...) printf(fmt"\n", ##__VA_ARGS__)

redisContext* context = NULL;

char* getUserId(char* dbname);
void parseAppsHash(char* dbList);
void updateAppsHash(char* userId, char* dbName);

int main(int argc, char** argv)
{
    redisReply* reply;
    redisReply* reply1;
    redisReply* reply2;
    redisReply* reply3;
    struct timeval timeout = {1, 0};
    char* meta_ip;
    unsigned short meta_port;
    char command[1024];

    if (argc < 3) {
        fprintf(stderr, "Right usage: meta_tool meta_ip meta_port");
        return -1;
    }
    meta_ip = argv[1];
    meta_port = (unsigned short)strtoul(argv[2], (char**)NULL, 10);
    // meta_ip = "10.209.52.30";
    // meta_port = 6000;

    context = redisConnectWithTimeout(meta_ip, meta_port, timeout);

    if (context == NULL) {
        FATAL("connect to meta server failed");
        freeReplyObject(reply);
	return -1;
    }

    if (context->err) {
        FATAL("connect to meta failed: %s", context->errstr);
        freeReplyObject(reply);
	return -1;
    }

    reply = (redisReply*)redisCommand(context, "hgetall bae_apps_hash");
    if (reply == NULL) {
        FATAL("hgetall bae_apps_hash failed: %s", context->errstr);
        freeReplyObject(reply);
	return -1;
    } else if (reply->type != REDIS_REPLY_ARRAY) {
        FATAL("invalid reply type:%d, REDIS_REPLY_ARRAY %u\n",
		reply->type, REDIS_REPLY_ARRAY);
        freeReplyObject(reply);
	return -1;
    }
    for (unsigned idx = 0; idx < reply->elements; idx += 2) {
        reply1 = reply->element[idx];
        reply2 = reply->element[idx + 1];
        //printf("%s:%s\n", reply1->str, reply2->str);
        sprintf(command, "hdel bae_apps_hash %s", reply1->str);
        reply3 = (redisReply*)redisCommand(context, command);
        freeReplyObject(reply3);
        parseAppsHash(reply2->str);
    }

    freeReplyObject(reply);
    redisFree(context);

    return 0;
}

char* getUserId(char* dbname)
{
    char command[1024];
    sprintf(command, "hget dbname-%s uid", dbname);
    redisReply* reply = (redisReply*)redisCommand(context, command);
    if (reply == NULL) {
        FATAL("hgetall bae_apps_hash failed: %s", context->errstr);
        freeReplyObject(reply);
	return NULL;
    } else if (reply->type != REDIS_REPLY_STRING) {
        FATAL("invalid reply type:%d, REDIS_REPLY_STRING %u\n",
		reply->type, REDIS_REPLY_STRING);
        freeReplyObject(reply);
	return NULL;
    }
 
    return reply->str;
}

void parseAppsHash(char* dbList)
{
    char delim = '-';
    char* next = NULL;
    char* dbName;
    char* userId = NULL;
    char str[1024] = {0};
    char command[1024] = {0};
    redisReply* reply;

    if (dbList[0] == delim) {
        dbList += 1;
    }
    
    while((next = index(dbList, delim)) != NULL) {
        strncpy(str, dbList, next - dbList);
        dbName = str;
        userId = getUserId(dbName);
	// printf("db name:%s, userId:%s\n", dbName, userId);
        updateAppsHash(userId, dbName); 
        dbList = next + 1;
    }

    dbName = dbList;
    userId = getUserId(dbName);
    // printf("db name:%s, userId:%s\n", dbName, userId);
    updateAppsHash(userId, dbName); 
}

void updateAppsHash(char* userId, char* dbName)
{
    char command[1024];
    sprintf(command, "hget bae_apps_hash %s", userId);
    redisReply* reply = (redisReply*)redisCommand(context, command);
    if (reply != NULL && reply->str != NULL) {
        sprintf(command, "hset bae_apps_hash %s %s-%s", userId, reply->str, dbName);
    } else {
        sprintf(command, "hset bae_apps_hash %s -%s", userId, dbName);
    }

    reply = (redisReply*)redisCommand(context, command);
 
    freeReplyObject(reply);
    return;
}

