//thread pool initialized
//worker loop defined
//worker loops indefinitely
//if nothing to do but not shutdown
//cnd_wait

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "thread_pool.h"
#include "calculator.h"
#define QUEUE_SIZE 100

int enqueue_task(JobQueue * queue, Task * task)
{
    mtx_lock(&queue->mutex);
    while (queue->count == queue->capacity && !queue->shutdown)
        cnd_wait(&queue->not_full, &queue->mutex);

    if (queue->shutdown) {
        mtx_unlock(&queue->mutex);
        return 0;
    }
    queue->tasks[queue->tail] = task;
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->count++;
    mtx_unlock(&queue->mutex);
    cnd_signal(&queue->not_empty);
    return 1;
}

int dequeue_task(JobQueue* queue, Task** out_task) {
    *out_task = NULL;
    if (queue->count == 0)
        return 0;
    Task* task = queue->tasks[queue->head];
    queue->tasks[queue->head] = NULL;
    queue->head = (queue->head + 1) % queue->capacity;
    queue->count--;
    cnd_signal(&queue->not_full);
    *out_task = task;
    return 1;
}

JobQueue* init_queue()
{
    JobQueue* queue = (JobQueue*)malloc(sizeof(JobQueue));
    if (queue == NULL)
        return NULL;
    queue->capacity = QUEUE_SIZE;
    queue->tasks = (Task**)malloc(queue->capacity * sizeof(Task*));
    if (queue->tasks == NULL) {
        free(queue);
        return NULL;
    }
    queue->count = 0;
    queue->head = 0;
    queue->tail = 0;
    queue->shutdown = false;
    if (mtx_init(&queue->mutex, mtx_plain) != thrd_success)
        goto fail_mutex;
    if (cnd_init(&queue->not_empty) != thrd_success)
        goto fail_not_empty;
    if (cnd_init(&queue->not_full) != thrd_success)
        goto fail_not_full;

    return queue;

fail_not_full:
    cnd_destroy(&queue->not_empty);
fail_not_empty:
    mtx_destroy(&queue->mutex);
fail_mutex:
    free(queue->tasks);
    free(queue);
    return NULL;
}

int worker_loop(void* args)
{
    WorkerArgs* w_args = (WorkerArgs*)args;
    JobQueue* queue = w_args->queue;
    while (true) {
        mtx_lock(&queue->mutex);
        
        while (queue->count == 0 && !queue->shutdown)
            cnd_wait(&queue->not_empty, &queue->mutex);

        if (queue->count == 0) {
            mtx_unlock(&queue->mutex);
            break;
        }

        Task* task = NULL;
        if (!dequeue_task(queue, &task)) {
            mtx_unlock(&queue->mutex);
            break;
        }
        mtx_unlock(&queue->mutex);

        int has_previous_answer = 0;
        task->error_code = process_line(task->expression, strlen(task->expression), &(task->result), &has_previous_answer);
        w_args->results[task->id].result = task->result;
        w_args->results[task->id].error_code = task->error_code;
        free(task);
    }
    free(w_args);
    return 0;
}

void thread_pool_init(thrd_t* threads, size_t num_threads, JobQueue* queue, Task* results) {
    if (queue == NULL)
        return;
    
    for (size_t i = 0; i < num_threads; i++) {
        WorkerArgs* args = malloc(sizeof(WorkerArgs));
        if (args == NULL) {
            continue;
        }
        args->queue = queue;
        args->results = results;
        thrd_create(&threads[i], worker_loop, args);
    }
}

void thread_pool_shutdown(thrd_t* threads, size_t num_threads, JobQueue* queue) {
    mtx_lock(&queue->mutex);
    queue->shutdown = true;
    mtx_unlock(&queue->mutex);

    cnd_broadcast(&queue->not_empty);
    cnd_broadcast(&queue->not_full);

    for (size_t i = 0; i < num_threads; i++) {
        thrd_join(threads[i], NULL);
    }

    cnd_destroy(&queue->not_empty);
    cnd_destroy(&queue->not_full);
    mtx_destroy(&queue->mutex);
}
