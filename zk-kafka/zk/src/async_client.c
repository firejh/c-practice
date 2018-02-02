#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zookeeper/zookeeper.h>
#include <zookeeper/zookeeper_log.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>


void watcher(zhandle_t* zk, int type, int state, const char* path, void* cxt)
{
    (void)(zk);
    printf("wather get a event:\n");
    printf("type: %d\n", type);
    printf("state: %d\n", state);
    printf("path: %s\n", path);
    printf("watcher context: %s\n", (char *)cxt);
}

void output_zk_stat(const struct Stat *stat)
{
    time_t ctime;
    char ctime_str[64];
    time_t mtime;
    char mtime_str[64];

    if (!stat) {
        return; 
    }

    ctime = stat->ctime / 1000;
    mtime = stat->mtime / 1000;

    ctime_r((const time_t*)(&mtime), mtime_str);
    ctime_r((const time_t*)(&ctime), ctime_str);

    fprintf(stderr, "zookeeper stat:\nctime = %s, czxid=%#llx, mtime=%s, mzxid=%#llx,"
                "version=%x,aversion=%#x, ephemeralOwner = %#llx\n",
                ctime_str, (unsigned long long)(stat->czxid), mtime_str, (unsigned long long)(stat->mzxid),
                (unsigned)stat->version, (unsigned int)stat->aversion, (unsigned long long)stat->ephemeralOwner);
}

void stat_complete(int ret, const struct Stat *stat, const void *node_data)
{
    printf("stat, node_data:%s, ret:%d\n", (char*)node_data, ret);
    output_zk_stat(stat);
}

void void_complete(int ret, const void *node_data)
{
    printf("void, node_data:%s, ret:%d\n",
           (node_data) ? (char*)node_data : "nil", ret);
}

void string_complete(int ret, const char *node_name, const void *node_data)
{
    printf("node_name:%s, node_data:%s, ret:%d\n",
           (node_name) ? node_name: "nil",
           (node_data) ? (char*)node_data : "nil",
           ret);
}

int main(int argc, const char *argv[])
{
    const char* host = "127.0.0.1:2181";
    int timeout = 1000;
    printf("\nzookeeper_init\n");
    zhandle_t* handle = zookeeper_init(host, watcher, timeout, 0, "hello_zk", 0);
    if (!handle) {
        fprintf(stderr, "failed to init zookeeper\n");
        exit(EXIT_FAILURE);
    }

    printf("\nzoo_acreate\n");
    int ret = zoo_acreate(handle, "/root", "hello_u_zk", 5,
                &ZOO_OPEN_ACL_UNSAFE, 0, string_complete, "acreate");
    if (ret) {
        fprintf(stderr, "zook_create() = %d\n", ret);
        exit(EXIT_FAILURE);
    }
    sleep(1);

    printf("\nzoo_aexists\n");
    ret = zoo_aexists(handle, "/root", 1, stat_complete, "aexists");
    if (ret) {
        fprintf(stderr, "zoo_aexists() = %d\n", ret);
        exit(EXIT_FAILURE);
    }
    sleep(1);

    printf("\nzoo_adelete\n");
    ret = zoo_adelete(handle, "/root", -1, void_complete, "adelete");
    if (ret) {
        fprintf(stderr, "zoo_delete() = %d\n", ret);
        exit(EXIT_FAILURE);
    }
    sleep(1);

    printf("\nzookeeper_close\n");
    zookeeper_close(handle);

    return 0;
}
