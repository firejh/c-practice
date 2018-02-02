
#include <signal.h>

#include <async.h>
#include <hiredis.h>
#include <adapter/hiredis_ae.h>

#include "log.h"
#include "tool.h"

typedef struct config_tag {
    int stop;
    aeEventLoop *event_loop;
    redisAsyncContext *clt;

    char server_ip[64];
    unsigned server_port;
    char password[128];
} config_t, *config_p, config_a[1];

config_a g_config = {0};

void connect_callback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }
    printf("Connected...\n");
}

void disconnect_callback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }

    printf("Disconnected...\n");

    config_p conf = (config_p)(c->data);
    if (!conf) {
        return;
    }

    aeStop(conf->event_loop);
}

static void auth_callback(redisAsyncContext *c, void *r, void *privdata) {
    (void)(c);
    (void)(privdata);

    config_p conf = (config_p)(c->data);
    redisReply *reply = r;
    if (!conf || !reply) {
        return;
    }

    if (reply->type == REDIS_REPLY_ERROR) {
        FATAL("we will exit, auth reply error:%s", reply->str);
        conf->stop = 1;
    } else {
        DEBUG("auth reply:%s", reply->str);
    }
}

void get_callback(redisAsyncContext *c, void *r, void *privdata) {
    config_p conf = (config_p)(c->data);
    redisReply *reply = r;
    if (!conf || !reply) {
        return;
    }

    pline("argv[%s]: %s", (char*)privdata, reply->str);

    if (conf->stop) {
        redisAsyncDisconnect(c);
    }
}

static void signal_handle(int sig) {
    const char *msg;
    config_p conf = g_config;

    switch (sig) {
    case SIGINT:
        msg = "Received SIGINT scheduling shutdown...";
        break;
    case SIGTERM:
        msg = "Received SIGTERM scheduling shutdown...";
        break;
    default:
        msg = "Received shutdown signal, scheduling shutdown...";
    };

    pinfo("%s", msg);
    // do it as redis.c:sigShutdownHandler
    if (conf->stop) {
        perr("Good bye bye...");
        exit(1);
    }

    conf->stop = 1;
    aeStop(conf->event_loop); // stop loop, aeMain will check this flag
    aeDeleteEventLoop(conf->event_loop); // free event loop
    redisAsyncFree(conf->clt); // free async context
    exit(1);
}

int main (int argc, char **argv) {
    config_p conf = g_config;

    strncpy(conf->server_ip, "127.0.0.1", sizeof(conf->server_ip));
    conf->server_port = 6379;
    if (1 < argc) {
        strncpy(conf->server_ip, argv[1], sizeof(conf->server_ip));
    }
    if (2 < argc) {
        conf->server_port = strtoul(argv[2], (char**)nil, 10);
    }
    if (3 < argc) {
        strncpy(conf->password, argv[3], sizeof(conf->password));
    }

    // signal
    // signal(SIGHUP, SIG_IGN);
    // signal(SIGPIPE, SIG_IGN);

    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = signal_handle;
    sigaction(SIGTERM, &act, nil);
    sigaction(SIGINT, &act, nil);

    conf->event_loop = aeCreateEventLoop(1024);

    redisAsyncContext *clt = redisAsyncConnect(conf->server_ip, conf->server_port);
    if (clt->err) {
        // Let *c leak for now..
        FATAL("%s", clt->errstr);
        return 1;
    }
    redisAeAttach(conf->event_loop, clt);
    conf->clt = clt;
    conf->clt->data = (void*)(conf);

    redisAsyncSetConnectCallback(clt, connect_callback);
    redisAsyncSetDisconnectCallback(clt, disconnect_callback);
    if (conf->password && strlen(conf->password)) {
        redisAsyncCommand(clt, auth_callback, nil, "AUTH %s", conf->password);
    }
    redisAsyncCommand(clt, nil, nil, "SET key_test value_test");
    redisAsyncCommand(clt, get_callback, (char*)"game-over!", "GET key_test");

    aeMain(conf->event_loop);
    aeDeleteEventLoop(conf->event_loop);

    return 0;
}
