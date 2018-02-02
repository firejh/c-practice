
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>
#include <unistd.h>
#include <sched.h>

#include <iostream>
#include <atomic>

#include "mpmc-bounded-queue.hpp"

#define MAX_NUM 65500
#define test_num 100000000
// #define test_num 10000000

std::atomic_int push_num(0);
std::atomic_int pop_num(0);
std::atomic_bool flag(false);

uint64_t get_usec()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*1000000 + tv.tv_usec;
}

class Test
{
    public:
        Test() { v_ = 3; }

    private:
        int v_;
};

mpmc_bounded_queue_t<Test*> test_queue(65536);

void* push(void* arg)
{
    int thread_id = *(int*)(arg);
    uint64_t start_time = get_usec();
    while (1) {
        Test* test = new Test();
        while (!test_queue.enqueue(test)) {
            sched_yield();
        }
        push_num ++;
        if (test_num <= push_num.load()) {
            break;
        }
        if (test_queue.size() > MAX_NUM) {
            sched_yield();
            continue;
        }
    }
    uint64_t end_time = get_usec();

    size_t num = push_num;
    printf("push thread %d, push num %zu, use time %f sec\n",
            thread_id, num, (end_time - start_time) / 1000000.0f);
    flag = true;
}

void* pop(void* arg)
{
    int thread_id = *(int*)(arg);
    uint64_t start_time = get_usec();
    while (1) {
        Test* test = NULL;
        if (!test_queue.dequeue(test)) {
            sched_yield();
            continue;
        }
        delete test;
        pop_num++;
        if (test_num <= pop_num.load()) {
            break;
        }
    }
    uint64_t end_time = get_usec();

    size_t num = push_num;
    printf("pop thread %d, pop num %zu, use time %f sec\n",
                thread_id, num, (end_time - start_time) / 1000000.0f);
}

int main()
{
    pthread_t push_id1;
    pthread_t push_id2;

    pthread_t pop_id1;
    pthread_t pop_id2;

    int push_tid1 = 1;
    int  push_tid2 = 2;
    int pop_tid1 = 1;
    int  pop_tid2 = 2;

    pthread_create(&push_id1, NULL, push, &push_tid1);
    pthread_create(&pop_id1, NULL, pop, &pop_tid1);

    pthread_create(&push_id2, NULL, push, &push_tid2);
    pthread_create(&pop_id2, NULL, pop, &pop_tid2);

    pthread_join(pop_id2, NULL);
    pthread_join(push_id2, NULL);

    pthread_join(pop_id1, NULL);
    pthread_join(push_id1, NULL);

    return 1;
}
