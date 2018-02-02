#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zookeeper/zookeeper.h>
#include <zookeeper/zookeeper_log.h>
#include <time.h>

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

    fprintf(stderr,"zookeeper stat:\nctime = %s, czxid=%#llx, mtime=%s, mzxid=%#llx,"
                "version=%x,aversion=%#x, ephemeralOwner = %#llx\n",
                ctime_str, (unsigned long long)(stat->czxid), mtime_str, (unsigned long long)(stat->mzxid),
                (unsigned)stat->version, (unsigned int)stat->aversion, (unsigned long long)stat->ephemeralOwner);
}

int main(int argc, const char *argv[])
{
    // const char* host = "127.0.0.1:2181";
    const char* host = "192.168.202.221:2181";
    int timeout = 1000; // 1s
    printf("\nzookeeper_init\n");
    zhandle_t* handle = zookeeper_init(host, watcher, timeout, 0, "hello_zk", 0);
    if (!handle) {
        fprintf(stderr, "failed to init zookeeper\n");
        exit(EXIT_FAILURE);
    }

    char path[1024] = {0};
    struct ACL acl[] = {{ZOO_PERM_ALL, ZOO_ANYONE_ID_UNSAFE}};
    struct ACL_vector acls = {1, acl};
    printf("\nzoo_create\n");
    char temp_path[] = {"/root"};
    char temp_path_value[] = {"hello_u_zk"};
    // int ret = zoo_create(handle, "/root", "hello_u_zk", 5, &acls, 0, path, sizeof(path));
    int flags = ZOO_SEQUENCE | ZOO_EPHEMERAL;
    int ret = zoo_create(handle, "/root", "hello_u_zk", 5, &acls, flags, path, sizeof(path));
    if (ret) {
        fprintf(stderr, "zook_create() = %d\n", ret);
        exit(EXIT_FAILURE);
    }
    printf("create tmp path %s, value %s, zk path:%s\n", temp_path, temp_path_value, path);

    struct Stat stat;
    printf("\nzoo_exists\n");
    ret = zoo_exists(handle, "/root", 1, &stat);
    if (ret) {
        fprintf(stderr, "zoo_exists() = %d\n", ret);
        exit(EXIT_FAILURE);
    }
    output_zk_stat(&stat);

    printf("\nzoo_delete\n");
    ret = zoo_delete(handle, "/root", -1);
    if (ret) {
        fprintf(stderr, "zoo_delete() = %d\n", ret);
        exit(EXIT_FAILURE);
    }

    printf("\nzookeeper_close\n");
    zookeeper_close(handle);

    return 0;
}
