#pragma once
#include <threads.h>
#include <stdbool.h>

//Task: to hold the input and the output of a single process line
typedef struct Task_t {
	char expression[120];
	int error_code;
	double result;
	int id;
} Task ;

typedef struct JobQueue_t {
	Task** tasks;
	size_t head, tail, capacity, count;
	mtx_t mutex;
	cnd_t cnd;
	bool shutdown;
} JobQueue;

typedef struct WorkerArgs_t {
	JobQueue* queue;
	Task* results;
} WorkerArgs;

int enqueue_task(JobQueue* queue, Task* task);
Task* dequeue_task(JobQueue* queue);
JobQueue* init_queue();

//Job queue:
//WorkerArgs: to pass stuff necessary for the worker
int worker_loop(void* args);
void thread_pool_init(thrd_t* threads, size_t num_threads, JobQueue* queue, Task* results);
void thread_pool_shutdown(thrd_t* threads, size_t num_threads, JobQueue* queue);

