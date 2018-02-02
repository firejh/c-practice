/**
 * FILE        : lisk_timer.c
 * DESC        : lisk timer (old lisk_clock = base/lisk_clock + ext/lisk_timer)
 * AUTHOR    : v1.0 written by Alex Stocks
 * DATE        : on June 7, 2012
 * LICENCE    : GPL 2.0
 * MOD        :
 **/

#include <lisk_atom.h>
#include <lisk_clock.h>
#include <lisk_list.h>    // timer node list
#include <lisk_log.h>
#include <lisk_ring.h>
#include <lisk_timer.h>


//////////////////////////////////////////
// lisk second
////////////////////////////////////////

static volatile n8    g_cur_lisk_time = 0;

n8 lisk_time_get()
{
    return g_cur_lisk_time;
}

vd lisk_time_update()
{
    lisk_sync_set8(g_cur_lisk_time, get_cur_time_usec());
}

//////////////////////////////////////////
//timer
//////////////////////////////////////////

#define    MAX_MS                    (1000)
#define    MAX_SECOND                (60)
#define    MAX_MINUTE                (60)
#define    MAX_HOUR                (24)
#define    MAX_DAY                    (31)
#define    MS                        (1)
#define    SECOND_MS                (1 * MAX_MS * (MS))
#define    MINUTE_MS                (1 * MAX_SECOND * (SECOND_MS))
#define    HOUR_MS                    (1 * MAX_MINUTE * (MINUTE_MS))
#define    DAY_MS                    (1 * MAX_HOUR * (HOUR_MS))
#define    MS_NUM(expire)            (expire / MS)
#define    SECOND_NUM(expire)        (expire / SECOND_MS)
#define    MINUTE_NUM(expire)        (expire / MINUTE_MS)
#define    HOUR_NUM(expire)        (expire / HOUR_MS)
#define    DAY_NUM(expire)            (expire / DAY_MS)
#define    MINIMUM_DIFF            (10 * MS)

typedef struct lisk_timer_node_tag {
    u8                            trig;
    u4                            type;
    u4                            expire;
    vp                            arg;
    timer_func                    run;
} lisk_timer_node_t, *lisk_timer_node_p, lisk_timer_node_a[1];

typedef enum TIMER_EXPIRE_RANGE_TAG {
    MIN_EXPIRE                    = 1,                //1ms
    MAX_EXPIRE                    = (0XEFFFFFF),        //4026531839ms, 46.6d
} TIMER_EXPIRE_RANGE;

vd timer_list_print(vp timer);

n4 cmp_lisk_timer_node(vp first, vp second)
{
    lisk_timer_node_p node_first;
    lisk_timer_node_p node_second;
    n4 ret = 0;

    node_first    = (lisk_timer_node_p)(first);
    node_second    = (lisk_timer_node_p)(second);
    if (IS_LT(node_first->trig, node_second->trig)) {
        ret = -1;
    } else if (IS_GT(node_first->trig, node_second->trig)) {
        ret = 1;
    } else {
        ret = 0;
    }

    RET_INT(ret, nil_str);
}

n4 timer_list_insert(
                    lisk_timer_p        timer,
                    vp                    list,
                    lisk_timer_node_p    elem,
                    n4                    flag,
                    uw*                    id
                    )
{
    n4                    ret;
    vp                    ptr;
    vp                    elem_ptr;
    lisk_timer_node_p    pos;

    if (IS_NL(timer) || IS_NL(list) || IS_NL(elem)) {
        RET_INT(-1, "@timer = %p, @list = %p, @elem = %p, @flag = %d",
                    timer, list, elem, flag);
    }

    // if list is empty or @elem_ptr is the maximum element
    // in list, insert it at the last of list
    pos = list;
    LIST_FOR_EACH_BEGIN(list, lisk_timer_node_p, cur)
        if (IS_LT(cmp_lisk_timer_node(elem, cur), 0)) {
            pos = cur;
            break;
        }
    LIST_FOR_EACH_END

    if (IS_NZR(flag))    {
        ret = list_move_front(pos, elem);
        if (IS_NZR(ret))    {
            RET_INT(-2, "list_move_front(pos = %p, "
                        "elem = %p) = %d",
                        pos, elem, ret);
        }
    } else {
        elem_ptr = nil;
        ret = ring_pop(timer->ring, (vp)(&elem_ptr));
        if (IS_ZR(ret))    {
            CPY(elem_ptr, elem, sizeof(*elem));
            elem = elem_ptr;
        } else {
            timer->num++;
        }

        ptr = list_insert_front(pos, elem, sizeof(*elem));
        if (IS_NL(ptr))    {
            RET_INT(-2, "list_insert_front(list = %p, "
                        "pos = %p, elem = %p, size = %zu) = %p",
                        list, pos, elem, sizeof(*elem), ptr);
        }
        if (IS_NNL(id))    {
            *id = PTR_2_NUM(ptr);
        }
    }

    RET_INT(0, nil_str);
}

n4 timer_ins_node(
                lisk_timer_p        timer,
                lisk_timer_node_p    node,
                n4                    flag,
                uw*                    id
                )
{
    n4        idx;
    n4        ret;
    n8        diff;

    if (IS_NL(timer) || IS_NL(node)) {
        RET_INT(-1, "@timer = %p, @node = %p, @flag = %d",
                    timer, node, flag);
    }

    diff = PTR_OFFSET(node->trig, timer->clock);
    idx = 0;
    if (IS_LE(node->trig, timer->clock))        {
        idx = 0;
    } else if (IS_NZR(DAY_NUM(diff)))        {
        idx = 4;
    } else if (IS_NZR(HOUR_NUM(diff)))        {
        idx = 3;
    } else if (IS_NZR(MINUTE_NUM(diff)))    {
        idx = 2;
    } else if (IS_NZR(SECOND_NUM(diff)))    {
        idx = 1;
    } else {
        idx = 0;
    }

    ret = timer_list_insert(timer, timer->list[idx], node, flag, id);
    if (IS_NZR(ret))    {
        RET_INT(-2, "timer_list_insert(timer = %p, list[idx = %d] "
                    "= %p, node = %p, flag = %d) = %d",
                    timer, idx, timer->list[idx], node, flag, ret);
    }

    RET_INT(0, nil_str);
}

vd timer_del_node(vp timer_ptr, vp list, vp node)
{
    n4                ret;
    n4                flag;
    lisk_timer_p    timer;

    timer = timer_ptr;
    if (IS_NL(timer))    {
        RET_VD("@timer = %p", timer);
    }

    flag = ELIST_ERASE_FULL;
    ret = ring_push(timer->ring, (vp)(&node));
    if (IS_ZR(ret))    {
        flag = ELIST_ERASE_PART;
    } else {
        timer->num--;
    }

    list_erase(list, node, flag);

    RET_VD(nil_str);
}

n4 timer_cascade(lisk_timer_p timer, u4 level)
{
    n4                flag;
    n4                guard;
    n8                diff;

    if (IS_NL(timer) || IS_NBT(level, 0, MAX_TIMER_LEVEL)) {
        RET_INT(-1, "@timer = %p, level = %u", timer, level);
    }

    guard = 0;
    flag = 1;
    LIST_FOR_EACH_BEGIN(timer->list[level], lisk_timer_node_p, cur)
        if (IS_LE(cur->trig, timer->clock))    {
            guard = 0;
        } else    {
            diff = cur->trig - timer->clock;
            switch (level) {
            case 1:
                guard = IS_GE(SECOND_NUM(diff), 1);
                break;

            case 2:
                guard = IS_GE(MINUTE_NUM(diff), 1);
                break;

            case 3:
                guard = IS_GE(HOUR_NUM(diff), 1);
                break;

            case 4:
                guard = IS_GE(DAY_NUM(diff), 1);
                break;
            }
        }

        if (IS_NZR(guard)) {
            break;
        }

        timer_ins_node(timer, cur, flag, nil);
        cur = timer->list[level];
    LIST_FOR_EACH_END

    RET_INT(0, nil_str);
}

n4 timer_update(lisk_timer_p timer)
{
    n4    ret;
    n8    cur_clock;
    n4    idx;
    u4    max_idx;
    u4    diff;
    u4    inc[MAX_TIMER_LEVEL + 1];

    static const u4 limit[MAX_TIMER_LEVEL + 1] ={
                                                MAX_MS,
                                                MAX_SECOND,
                                                MAX_MINUTE,
                                                MAX_HOUR,
                                                MAX_DAY
                                                };

    static const u4 ms_limit[MAX_TIMER_LEVEL + 1] = {
                                                    MS,
                                                    SECOND_MS,
                                                    MINUTE_MS,
                                                    HOUR_MS,
                                                    DAY_MS
                                                    };

    if (IS_NL(timer)) {
        RET_INT(-1, "@timer = %p", timer);
    }

    //update clock
    cur_clock = lisk_time_get() / 1000LL;
    diff = PTR_OFFSET(cur_clock, timer->clock);
    if (IS_LT(diff, MINIMUM_DIFF))    {
        sched_yield();
        RET_INT(-2, nil_str);
    }
    timer->clock = cur_clock;

    SET_ZERO(inc, sizeof(inc));
    idx = MAX_TIMER_LEVEL - 1;
    while (IS_NMN(idx))    {
        inc[idx] = (diff) / (ms_limit[idx]);
        diff %= ms_limit[idx];
        idx--;
    }

    max_idx = 0;
    for (idx = 0; IS_LT(idx, MAX_TIMER_LEVEL); idx++)    {
        if (IS_NZR(inc[idx]))    {
            timer->hand[idx] += inc[idx];
            inc[idx + 1] += (timer->hand[idx] / limit[idx]);
            timer->hand[idx] %= limit[idx];
            max_idx = idx + 1;
        }
    }

    for (idx = 1; IS_LT(idx, max_idx); idx++)    {
        ret = timer_cascade(timer, idx);
        if (IS_NZR(ret))    {
            LISK_LOG_ERROR("timer_cascade(timer = %p, idx = %d) = %d",
                            timer, idx, ret);
        }
    }

    RET_INT(0, nil_str);
}

n4 timer_init(vp timer_ptr)
{
    n4                ret;
    u4                idx;
    u4                len;
    u4                num;
    vp                list;
    lisk_timer_p    timer;

    timer = timer_ptr;
    if (IS_NL(timer)) {
        RET_INT(-1, "@timer_ptr = %p", timer);
    }
    SET_ZERO(timer, sizeof(lisk_timer_t));

    num = SIZE_K(1);
    len = sizeof(vp);
    ret = ring_init(&(timer->ring), num, len);
    if (IS_NZR(ret))    {
        timer->ring = nil;
    }
    ring_set_type(timer->ring, RING_SP_SC);

    for (idx = 0; IS_LT(idx, MAX_TIMER_LEVEL); idx++) {
        ret = list_init(&list);
        if (IS_NZR(ret)) {
            for (idx = 0; IS_LT(idx, timer->size); idx++)    {
                list_uninit(&(timer->list[idx]));
            }
            ring_uninit(&(timer->ring));
            RET_INT(-2, "idx %u: list_init(&list = %p) = %d",
                        idx, &list, ret);
        }
        timer->list[timer->size++] = list;
    }
    lisk_time_update();
    timer->clock = lisk_time_get() / 1000L;

    RET_INT(0, nil_str);
}

vd timer_uninit(vp timer_ptr)
{
    n4                flag;
    n4                ret;
    u4                idx;
    vp                node;
    vp                list;
    lisk_timer_p    timer;

    timer = timer_ptr;
    if (IS_NL(timer))    {
        RET_VD(nil_str);
    }

    list = nil;
    flag = ELIST_ERASE_FULL;
    if (IS_NNL(timer->ring))    {
        do {
            ret = ring_pop(timer->ring, (vp)(&node));
            if (IS_NZR(ret))    {
                break;
            }

            list_erase(list, node, flag);
        } while (1);

        ring_uninit(&(timer->ring));
    }

    for (idx = 0; IS_LT(idx, timer->size); idx++) {
        list = timer->list[idx];
        LIST_FOR_EACH_BEGIN(list, lisk_timer_node_p, timer_node)
            timer_node->run(nil, PTR_2_NUM(timer_node), timer_node->arg);
        LIST_FOR_EACH_END

        list_uninit(&list);
    }
}

n4 timer_add(vp timer_ptr, timer_key_p key, uw* id)
{
    n4                    ret;
    n4                    flag;
    lisk_timer_p        timer;
    lisk_timer_node_a    node;

    timer = (lisk_timer_p)(timer_ptr);
    if (IS_NL(timer) ||
        IS_NL(key) ||
        IS_NBE(key->expire, MIN_EXPIRE, MAX_EXPIRE)) {
        RET_INT(-1, "@timer = %p, @key = %p, @key->expire = %u",
                    timer, key, key->expire);
    }

    if (IS_NL(key->func))    {
        RET_INT(-1, "@key->func = %p", key->func);
    }

    if (IS_ZR(timer->size)) {
        RET_INT(-1, "timer->size = %u", timer->size);
    }

    node[0].type    = key->type;
    node[0].expire    = key->expire;
    node[0].run        = key->func;
    node[0].arg        = key->arg;
    node[0].trig    = timer->clock + key->expire;
    flag = 0;
    ret = timer_ins_node(timer, node, flag, id);
    if (IS_NZR(ret))    {
        RET_INT(-2, "timer_ins_node(timer = %p, node = %p, "
                    "flag = %d, id = %p) = %d",
                    timer, node, flag, id, ret);
    }

    RET_INT(0, nil_str);
}

n4 timer_del(vp timer_ptr, uw id)
{
    vp                node;
    lisk_timer_p    timer;

    timer = timer_ptr;
    if (IS_NL(timer) || IS_ZR(id))    {
        RET_INT(-1, "@timer = %p, @id = %lu", timer, id);
    }

    node = NUM_2_PTR(id);
    timer_del_node(timer, nil, node);

    RET_INT(0, nil_str);
}

n4 timer_loop(vp timer_ptr)
{
    n4                    flag;
    n4                    ret;
    vp                    list;
    lisk_timer_node_p    prev;
    lisk_timer_p        timer;

    timer = (lisk_timer_p)(timer_ptr);
    if (IS_NL(timer)) {
        RET_INT(-1, "timer = %p", timer);
    }

    if (IS_ZR(timer->clock) || IS_ZR(timer->size)) {
        RET_INT(-1, "timer->clock = %lld, timer->size = %u", timer->clock, timer->size);
    }

    ret = timer_update(timer);
    if (IS_NZR(ret))    {
        RET_INT(0, nil_str);
    }

    flag = 1;
    prev = nil;
    list = timer->list[0];
    LIST_FOR_EACH_BEGIN(list, lisk_timer_node_p, node)
        if (IS_GT(node->trig, timer->clock)) {
            break;
        }
        prev = (lisk_timer_node_p)(list_prev_elem(node));
        ret = node->run(timer, PTR_2_NUM(node), node->arg);
        if (IS_EQ(ret, -3) || IS_EQ(node->type, ETIMER_ONCE)) {
            timer_del_node(timer, list, node);
        } else {
            node->trig = timer->clock + node->expire;
            ret = timer_ins_node(timer, node, flag, nil);
            if (IS_NZR(ret))    {
                // it will not happen if lisk fails
                // to insert this timer node.
                // DEALLOC(node);
                LISK_LOG_ERROR("timer_ins_node(timer = %p, node = %p) = %d",
                            timer, node, ret);
            }
        }

        node = prev;
    LIST_FOR_EACH_END

    RET_INT(0, nil_str);
}

vd timer_list_print(vp timer_ptr)
{
    u4                idx;
    lisk_timer_p    timer;

    if (IS_NL(timer = timer_ptr))    {
        RET_VD(nil_str);
    }

    for (idx = 0; IS_LT(idx, MAX_TIMER_LEVEL); idx++)    {
        pline("print %d timer list", idx);
        //list_print(timer->list[idx]);
    }
}

n4 timer_mem_size(vp timer_ptr, u8* num, u8* size)
{
    lisk_timer_p    timer;

    if (IS_NL(timer = timer_ptr))    {
        RET_INT(-1, "@timer = %p", timer);
    }

    if (IS_NNL(num))    {
        *num = timer->num;
    }
    if (IS_NNL(size))    {
        *size = 0;

        // timer head 76B
        *size += sizeof(lisk_timer_t);                                    // 88B
        // timer list head {prev, next, uw} 24B
        *size += timer->size * (sizeof(vp) + sizeof(vp) + sizeof(uw));    // 120B
        // timer ring { ring_t + timer->ring->data }
        *size += (sizeof(u4) * 6 + SIZE_K(8) * sizeof(vp));                // 65560B
        // timer node {prev, next, lisk_timer_node_t} 48B
        *size += timer->num * (sizeof(vp) + sizeof(vp) + sizeof(lisk_timer_node_t));
    }

    RET_INT(0, nil_str);
}

