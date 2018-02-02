/**
* FILE     : timer_test.c
* DESC     : to test lisk_timer
* AUTHOR   : v0.3.0 written by Alex Stocks
* DATE     : on Oct 27, 2012
* LICENCE  : GPL 2.0
* MOD      :
**/

#include <lisk_clock.h>
#include <lisk_log.h>
#include <lisk_timer.h>

typedef struct config_tag {
    uw           tid0;
    uw           tid1;
    uw           tid2;
    u4           loop;
    lisk_timer_t timer;
} config_t, *config_p, config_a[1];

n4 config_init(vd);
n4 config_uninit(vd);
vd terminate(n4 sig);
n4 timer_once(vp timer, uw id, vp arg);
n4 timer_cycle(vp timer, uw id, vp arg);
vd timer_test(vd);
vd run(vd);

config_t g_config = { 0 };

n4 main(n4 argc, n1** argv)
{
    n4 ret;

    ret = config_init();
    if (IS_NZR(ret)) {
        RET_INT(-1, "config_init() = %d", ret);
    }

    timer_test();

    run();

    config_uninit();

    RET_INT(0, nil_str);
}

vd terminate(n4 sig)
{
    switch (sig) {
    default:
        config_uninit();
        exit(1);
        break;
    }
}

n4 config_init()
{
    n4 ret;
    u8 num;
    u8 size;

    ret = timer_init(&(g_config.timer));
    if (IS_NZR(ret)) {
        config_uninit();
        RET_INT(-2, "timer_init(timer = %p) = %d",
            &(g_config.timer), ret);
    }

    timer_mem_size(&(g_config.timer), &num, &size);
    pinfo("init timer node number = %llu, "
        "memory size = %llu", num, size);

    RET_INT(0, nil_str);
}

n4 config_uninit()
{
    timer_uninit(&(g_config.timer));

    RET_INT(0, nil_str);
}

n4 timer_once(vp timer, uw id, vp arg)
{
    uw time;

    time = PTR_2_NUM(arg);
    pline("id = %lu, loop time = %lums", id, time);

    RET_INT(0, nil_str);
}

n4 timer_cycle(vp timer, uw id, vp arg)
{
    uw time;

    time = PTR_2_NUM(arg);
    printf("id = %lu, loop time = %lums\n", id, time);

    g_config.loop++;

    RET_INT(0, nil_str);
}

vd timer_test()
{
    n4          ret;
    u8          num;
    uw          expire;
    u8          size;
    timer_key_t key;

    expire = EXPIRE_MS(500);
    key.type = ETIMER_ONCE;
    key.expire = expire;
    key.func = (timer_func)(timer_once);
    key.arg = NUM_2_PTR(expire);
    ret = timer_add(&(g_config.timer), &key, nil);
    if (IS_NZR(ret)) {
        perr("timer_add(timer = %p, &key = %p, nil) = %d",
            &(g_config.timer), &key, ret);
    }

    timer_mem_size(&(g_config.timer), &num, &size);
    pinfo("after add %lums clock, timer node "
        "number = %llu, memory size = %llu",
        expire, num, size);

    expire = EXPIRE_S(2);
    key.type = ETIMER_LOOP;
    key.expire = expire;
    key.func = (timer_func)(timer_cycle);
    key.arg = NUM_2_PTR(expire);
    ret = timer_add(&(g_config.timer), &key, &(g_config.tid0));
    if (IS_NZR(ret)) {
        perr("timer_add(timer = %p, &key = %p, &id = %p) = %d",
            &(g_config.timer), &key, &(g_config.tid0), ret);
    }

    timer_mem_size(&(g_config.timer), &num, &size);
    pinfo("after add %lums clock, timer node "
        "number = %llu, memory size = %llu",
        expire, num, size);

    expire = EXPIRE_S(4);
    key.type = ETIMER_LOOP;
    key.expire = expire;
    key.func = (timer_func)(timer_cycle);
    key.arg = NUM_2_PTR(expire);
    ret = timer_add(&(g_config.timer), &key, &(g_config.tid1));
    if (IS_NZR(ret)) {
        perr("timer_add(timer = %p, &key = %p, &id = %p) = %d",
            &(g_config.timer), &key, &(g_config.tid1), ret);
    }

    timer_mem_size(&(g_config.timer), &num, &size);
    pinfo("after add %lums clock, timer node "
        "number = %llu, memory size = %llu",
        expire, num, size);

    expire = EXPIRE_S(10);
    key.type = ETIMER_LOOP;
    key.expire = expire;
    key.func = (timer_func)(timer_cycle);
    key.arg = NUM_2_PTR(expire);
    ret = timer_add(&(g_config.timer), &key, &(g_config.tid2));
    if (IS_NZR(ret)) {
        perr("timer_add(timer = %p, &key = %p, &id = %p) = %d",
            &(g_config.timer), &key, &(g_config.tid2), ret);
    }

    timer_mem_size(&(g_config.timer), &num, &size);
    pinfo("after add %lums clock, timer node "
        "number = %llu, memory size = %llu",
        expire, num, size);
}

vd run()
{
    n4 ret;
    u8 num;
    u8 size;

    for (;;) {
        if (IS_GE(g_config.loop, 3) && IS_NZR(g_config.tid0)) {
            ret = timer_del(&(g_config.timer), g_config.tid0);
            pinfo("timer id = %lu, loop = %d, timer_del() = %d",
                g_config.tid0, g_config.loop, ret);
            g_config.tid0 = 0;
        }
        else if (IS_GE(g_config.loop, 7) && IS_NZR(g_config.tid1)) {
            ret = timer_del(&(g_config.timer), g_config.tid1);
            pinfo("timer id = %lu, loop = %d, timer_del() = %d",
                g_config.tid1, g_config.loop, ret);
            g_config.tid1 = 0;
        }
        else if (IS_GE(g_config.loop, 20) && IS_NZR(g_config.tid2)) {
            ret = timer_del(&(g_config.timer), g_config.tid2);
            pinfo("timer id = %lu, loop = %d, timer_del() = %d",
                g_config.tid2, g_config.loop, ret);
            g_config.tid2 = 0;
            break;
        }

        lisk_time_update();
        timer_loop(&(g_config.timer));
    }

    timer_mem_size(&(g_config.timer), &num, &size);
    pinfo("timer node number = %llu, memory size = %llu", num, size);
}
