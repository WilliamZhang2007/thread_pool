#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "ra_thread_pool.h"

typedef struct CounterData {
    int i;
    struct timeval *now;
}CounterData;

void counter(void *task_data)
{
    CounterData *counter_data = (CounterData*)task_data;

    gettimeofday(counter_data->now, NULL);
    printf("now.tv_sec:%ld\n", counter_data->now->tv_sec);
    printf("now.tv_usec:%ld\n", counter_data->now->tv_usec);
    printf("counter:%d, thread_id:%lu\n", counter_data->i, pthread_self());
    sleep(10);
}

void release_counter(void *task_data)
{
    CounterData *counter_data = (CounterData*)task_data;
    printf("release_counter() counter:%d, thread_id:%lu\n", counter_data->i,
        pthread_self());
    free(counter_data->now);
}

int main(int argc, char * argv[])
{
    Factory *test_factory = NULL;      /* not a good way to use memory resource :( */
    int worker_num = 4;
    int task_num = 8;
    Task *task;
    CounterData *counter_data;
    int i;

    create_factory(&test_factory, worker_num);
    for (i = 0; i < task_num; i++)
    {
        task = (Task*)malloc(sizeof(Task));
        counter_data = (CounterData*)malloc(sizeof(CounterData));
        counter_data->i = i + 1;
        counter_data->now = (struct timeval*)malloc(sizeof(struct timeval));
        task->task_data = counter_data;
        task->do_task = counter;
        task->free_data = release_counter;

        push_task(test_factory, task);
        sleep(2);
    }

    close_factory(test_factory);
    return 0;
}