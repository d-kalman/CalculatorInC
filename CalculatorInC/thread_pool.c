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
    if (queue->count >= queue->capacity) {
        mtx_unlock(&queue->mutex);
        return 0;
    }
    queue->tasks[queue->tail] = task;
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->count++;
    mtx_unlock(&queue->mutex);
    cnd_signal(&queue->cnd);
    return 1;
}

Task* dequeue_task(JobQueue* queue) {
    Task* task = queue->tasks[queue->head];
    queue->head = (queue->head + 1) % queue->capacity;
    queue->count--;
    return task;
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

    if (mtx_init(&queue->mutex, mtx_plain) != thrd_success) {
        free(queue->tasks);
        free(queue);
        return NULL;
    }
    if (cnd_init(&queue->cnd) != thrd_success) {
        mtx_destroy(&queue->mutex);
        free(queue->tasks);
        free(queue);
        return NULL;
    }

    return queue;
}

int worker_loop(void* args)
{
    WorkerArgs* w_args = (WorkerArgs*)args;
    JobQueue* queue = w_args->queue;
    while (true) {
        mtx_lock(&queue->mutex);
        if (queue->shutdown && queue->count == 0) {
            mtx_unlock(&queue->mutex);
            break;
        }
        while (queue->count == 0 && !queue->shutdown)
            cnd_wait(&queue->cnd, &queue->mutex);
        Task* task = dequeue_task(queue);
        mtx_unlock(&queue->mutex);
        int has_previous_answer = 0;
        task->error_code = process_line(task->expression, strlen(task->expression), &(task->result), &has_previous_answer);
        w_args->results[task->id].result = task->result;
        w_args->results[task->id].error_code = task->error_code;
        free(task);
    }
    return 0;
}

void thread_pool_init(thrd_t* threads, size_t num_threads, JobQueue* queue, Task* results) {
    WorkerArgs* args = malloc(sizeof(WorkerArgs));
    if (args == NULL || queue == NULL)
        return;
    for (size_t i = 0; i < num_threads; i++) {
        args->queue = queue;
        args->results = results;
        thrd_create(&threads[i], worker_loop, args);
    }
}

void thread_pool_shutdown(thrd_t* threads, size_t num_threads, JobQueue* queue) {
    mtx_lock(&queue->mutex);
    queue->shutdown = true;
    mtx_unlock(&queue->mutex);

    cnd_broadcast(&queue->cnd); // Wake up all workers

    for (size_t i = 0; i < num_threads; i++) {
        thrd_join(threads[i], NULL);
    }

    cnd_destroy(&queue->cnd);
    mtx_destroy(&queue->mutex);
}
