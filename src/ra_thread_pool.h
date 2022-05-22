/* Imagine thread pool as a factory, the threads would be workers. */
#ifndef __RA_THREAD_POOL_H__
#define __RA_THREAD_POOL_H__

#include <pthread.h>

typedef struct __Worker {
    pthread_t thread_id;
    struct __Factory *factory;
    struct __Worker *prev;
    struct __Worker *next;
}Worker;

typedef struct __Task {
    void *task_data;
    void (*do_task)(void *task_data);
    void (*free_data)(void *task_data);
    struct __Task *prev;
    struct __Task *next;
}Task;

typedef struct __Factory {
    struct __Worker *workers;
    struct __Task *tasks;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int close;
}Factory;

int create_factory(Factory **new_factory, int worker_num);
int close_factory(Factory *factory);
int push_task(Factory *factory, Task *task);
int pop_task(Factory *factory, Task *task);

#endif