#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <hiredis.h>
#include <async.h>
#include <adapters/ae.h>

/* Put event loop in the global scope, so it can be explicitly stopped */
static aeEventLoop *loop = NULL;

void getCallback(redisAsyncContext *c, void *r, void *privdata) {
    redisReply *reply = r;
    if (reply == NULL) return;
    printf("argv[%s]: %s\n", (char*)privdata, reply->str);

    /* Disconnect after receiving the reply to GET */
    // redisAsyncDisconnect(c);
}

void rpopCallback(redisAsyncContext *c, void *r, void *privdata) {
    redisReply *reply = r;
    if (reply == NULL) return;
    printf("argv[%s]: element number:%zu\n", (char*)privdata, reply->elements);
    for (size_t i = 0; i < reply->elements; i++) {
        printf("idx:%zu, result:%s\n", i, reply->element[i]->str);
    }

    /* Disconnect after receiving the reply to GET */
    redisAsyncDisconnect(c);
}

void connectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        aeStop(loop);
        return;
    }

    printf("Connected...\n");
}

void disconnectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        aeStop(loop);
        return;
    }

    printf("Disconnected...\n");
    aeStop(loop);
}

// redis.conf: tcp-keepalive 300 seconds
int testOnBorrow(redisAsyncContext* context)
{
    redisContext *c = &(context->c);
    c->flags |= REDIS_BLOCK;
    redisReader *reader = c->reader;
    c->reader = redisReaderCreate();
	redisReply *reply = (redisReply*)redisCommand(&c, "PING");
    int ret = -1;
    if (reply != NULL && reply->type == REDIS_REPLY_STATUS && strcmp(reply->str, "PONG") == 0) {
		ret = 0;
	}
    freeReplyObject(reply);

    redisReaderFree(c->reader);
    c->reader = reader;
    context->c.flags &= ~REDIS_BLOCK;

	return ret;
}

int main (int argc, char **argv) {
    signal(SIGPIPE, SIG_IGN);

    // redisAsyncContext *c = redisAsyncConnect("127.0.0.1", 6379);
    redisAsyncContext *c = redisAsyncConnect("192.168.11.100", 6000);
    if (c->err) {
        /* Let *c leak for now... */
        printf("Error: %s\n", c->errstr);
        return 1;
    }

    printf("testOnBorrow result:%d\n", testOnBorrow(c));

    loop = aeCreateEventLoop(64);
    redisAeAttach(loop, c);
    redisAsyncSetConnectCallback(c,connectCallback);
    redisAsyncSetDisconnectCallback(c,disconnectCallback);
    // redisAsyncCommand(c, NULL, NULL, "SET key %b", argv[argc-1], strlen(argv[argc-1]));
    redisAsyncCommand(c, NULL, NULL, "SET hello world2");
    redisAsyncCommand(c, getCallback, (char*)"end-1", "GET hello");
    redisAsyncCommand(c, NULL, NULL, "lpush languages python cpp csharp java");
    redisAsyncCommand(c, rpopCallback, (char*)"end-2", "lrange languages 0 -1");
    aeMain(loop);
    return 0;
}

