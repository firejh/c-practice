/**
 * FILE     : conf_test.c
 * DESC     : to test lisk_dict
 * AUTHOR   : v0.3.0 written by Alex Stocks
 * DATE     : on Oct 20, 2012
 * LICENCE  : GPL 2.0
 * MOD      :
 **/

#include "log.h"
#include "conf.h"

void confTest(char* file);

int main(int argc, char** argv)
{
    char* file;

    if (argc < 2) {
        perr("Legal command: ./bin/conf_test ./exam/lisk.conf");
        return -1;
    }
    file = argv[1];

    pinfo("print values of the defined key:");
    confTest(file);

    return 0;
}

void confTest(char* file)
{
    int         idx;
    int         ret;
    size_t      size;
    void*       conf;
    char*       key;
    char*       key1;
    const char* value;

    ret = confInit(&conf, file);
    if (ret) {
        perr("confInit(file = %s, conf = %p) = %d", file, &conf, ret);
        return;
    }

    // not exist
    key = "im.max.uid";
    value = confGetValue(conf, key, "");
    pinfo("key %s, value %s", key, value);

    // default
    key = "hello";
    value = confGetValue(conf, key, "");
    pinfo("key %s, value %s", key, value);
    key = "homedir";
    value = confGetValue(conf, key, "");
    pinfo("key %s, value %s", key, value);

    // over
    key = "over";
    key1 = "path";
    value = confGetValue(conf, key, key1);
    pinfo("key %s-%s, value %s", key, key1, value);

    idx = 0;
    key1 = "array";
    ret = confGetArraySize(conf, key, key1, &size);
    if (!ret) {
        pinfo("key %s-%s, array size: %d", key, key1, size);
    }
    do {
        value = confGetArrayValue(conf, key, key1, idx);
        if (!value) {
            break;
        }

        pinfo("key %s-%s, idx = %d, value %s", key, key1, idx, value);
        idx++;
    } while(1);

    confUninit(&conf);
}

