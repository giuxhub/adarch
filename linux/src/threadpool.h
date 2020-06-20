#ifndef __INC_THREADPOOL_H_
#define __INC_THREADPOOL_H_

#include "stdafx.h"

#define MAX_THREADS 64
#define MAX_QUEUE 256

typedef enum
{
	TP_INVALID = -1, TP_LOCK_FAIL = -2, TP_QUEUE_FULL = -3, TP_SHUTDOWN = -4,
} threadpool_err;

typedef struct threadpool_task_t
{
	void *(*function)(void *);
	void *argument;
} threadpool_task_t;

typedef struct threadpool_t
{
	pthread_mutex_t lock;
	pthread_t *threads;
	pthread_cond_t notify;
	threadpool_task_t *queue;
	bool p_shutdown;
	int thread_count;
	int queue_size;
	int head;
	int tail;
	int count;
	int started;
} threadpool_t;

threadpool_t *threadpool_init(int thread_count, int queue_size);
int threadpool_destroy(threadpool_t *pool);
int threadpool_add_task(threadpool_t *pool, void *(function)(void *),
		void *argument);

#endif
