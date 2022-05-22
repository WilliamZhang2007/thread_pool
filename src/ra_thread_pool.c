#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ra_list.h"
#include "ra_thread_pool.h"

void  *thread_func(void *arg);

int create_factory(Factory **new_factory, int worker_num)
{
    int i;
    int ret;
    Factory *factory;
    Worker *worker;

    if (worker_num < 1)
        worker_num = 1;
    
    /* Create a factory */
    factory = malloc(sizeof(Factory));
    if (factory == NULL)
        return -1;
    memset(factory, 0, sizeof(Factory));
    ret = pthread_mutex_init(&factory->mutex, NULL);
    if (ret != 0)
        goto out;
    ret = pthread_cond_init(&factory->cond, NULL);
    if (ret != 0)
        goto out;
    factory->close = 0;
    
    /* Create workers */
    for (i = 0; i < worker_num; i++)
    {
        worker = malloc(sizeof(Worker));
        if (worker == NULL)
            goto out;
        memset(worker, 0, sizeof(Worker));

        /* Register and start working */
        worker->factory = factory;
        LIST_ADD_HEAD(factory->workers, worker);
        ret = pthread_create(&worker->thread_id, NULL, thread_func, worker);
        if (ret != 0)
            goto out;
    }

    *new_factory = factory;
    return 0;
out:
    close_factory(factory);
    return -1;
}

int close_factory(Factory *factory)
{
    int ret;
    Task *task;

    /* there is 2 places in thread_func determine whether to close factory
         or not. But if a thread is doing a longtime task, we maybe need to 
         wait for a while. */
    factory->close = 1;

    /* broadcast to wakeup sleeping worker */
    while (factory->workers != NULL)
    {
        ret = pthread_mutex_lock(&factory->mutex);
        if (ret != 0)
            return -1;
        pthread_cond_broadcast(&factory->cond);
        ret = pthread_mutex_unlock(&factory->mutex);
        if (ret != 0)
            return -1;
    }

    /* free task */
    while (factory->tasks != NULL)
    {
        task = factory->tasks;
        /* no worker now, so we don't need mutex to protect tasks list now*/
        LIST_REMOVE(factory->tasks, task);
        if (task->free_data)
            task->free_data(task->task_data);
        free(task);
    }

    free(factory);

    return 0;
}

void  *thread_func(void *arg)
{
    int ret;
    Worker *worker = (Worker*)arg;
    Task *task;

    while (1)
    {
        ret = pthread_mutex_lock(&worker->factory->mutex);
        if (ret != 0)
            break;
        /* wait for a task */
        while (worker->factory->tasks == NULL)
        {
            if (worker->factory->close != 0)
                break;
            pthread_cond_wait(&worker->factory->cond, &worker->factory->mutex);  
        } 
        if (worker->factory->close != 0)
        {
            pthread_mutex_unlock(&worker->factory->mutex);
            break;
        }

        /* get a task */
        task = worker->factory->tasks;
        LIST_REMOVE(worker->factory->tasks, task);
        ret = pthread_mutex_unlock(&worker->factory->mutex);
        if (ret != 0)
            break;
        
        /* do a task */
        task->do_task(task->task_data);
        pop_task(worker->factory, task);
    }

    printf("factory closing, thread_id:%lu exit\n", pthread_self());
    LIST_REMOVE(worker->factory->workers, worker);
    free(worker);
    pthread_exit(NULL);
}

int push_task(Factory *factory, Task *task)
{
    int ret;

    if (factory->close != 0)
        return -1;

    ret = pthread_mutex_lock(&factory->mutex);
    if (ret != 0)
        return -1;
    LIST_ADD_TAIL(factory->tasks, task);
    pthread_cond_signal(&factory->cond);
    ret = pthread_mutex_unlock(&factory->mutex);
    if (ret != 0)
        return -1;

    return 0;
}

int pop_task(Factory *factory, Task *task)
{
    int ret;

    printf("pop_task() thread_id:%lu\n", pthread_self());

    if (task->free_data)
        task->free_data(task->task_data);
    free(task);

    return 0;
}