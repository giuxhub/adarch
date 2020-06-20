#include "stdafx.h"
#include "threadpool.h"
#include "utils.h"

static void *threadpool_thread(void *threadpool)
{
	threadpool_t *pool = (threadpool_t *)threadpool;
	threadpool_task_t task;
	for (;;) {
		EnterCriticalSection(&pool->lock);

		while ((pool->count == 0) && (!pool->p_shutdown))
			SleepConditionVariableCS(&(pool->notify), &(pool->lock), INFINITE);

		if ((pool->p_shutdown) && (pool->count == 0))
			break;

		task.function = pool->queue[pool->head].function;
		task.argument = pool->queue[pool->head].argument;
		pool->head++;
		pool->head = (pool->head == pool->queue_size) ? 0 : pool->head;
		pool->count--;

		LeaveCriticalSection(&pool->lock);

		(*(task.function))(task.argument);
	}
	pool->started--;

	LeaveCriticalSection(&pool->lock);
	return NULL;
}

static int threadpool_free(threadpool_t *pool)
{
	if (pool == NULL || pool->started > 0)
		return -1;

	if (pool->threads) {
		utils_free(pool->threads);
		utils_free(pool->queue);
		EnterCriticalSection(&pool->lock);
		DeleteCriticalSection(&pool->lock);
	}

	utils_free(pool);
	return 0;
}

int threadpool_add_task(threadpool_t *pool, void *(function)(void *),
	void *argument)
{
	int err = 0, next = 0;

	if (pool == NULL || function == NULL)
		return TP_INVALID;

	EnterCriticalSection(&pool->lock);

	next = pool->tail + 1;
	next = (next == pool->queue_size) ? 0 : next;

	do {
		if (pool->count == pool->queue_size) {
			err = TP_QUEUE_FULL;
			break;
		}

		if (pool->p_shutdown) {
			err = TP_SHUTDOWN;
			break;
		}

		pool->queue[pool->tail].function = function;
		pool->queue[pool->tail].argument = argument;
		pool->tail = next;
		pool->count += 1;

		WakeConditionVariable(&(pool->notify));
	} while (0);

	LeaveCriticalSection(&pool->lock);
	return err;
}

threadpool_t *threadpool_init(int thread_count, int queue_size)
{
	threadpool_t *pool = NULL;
	int i;
	DWORD ret = 0;
	SECURITY_ATTRIBUTES security = { sizeof(security), NULL, TRUE };

	if (thread_count
		<= 0 || thread_count > MAX_THREADS || queue_size <= 0 || queue_size > MAX_QUEUE)
		return NULL;

	utils_create(pool, threadpool_t, 1);

	pool->thread_count = 0;
	pool->queue_size = queue_size;
	pool->head = pool->tail = pool->count = pool->started = 0;
	pool->p_shutdown = false;

	utils_create(pool->queue, threadpool_task_t, queue_size);

	utils_create(pool->threads, HANDLE, thread_count);
	InitializeConditionVariable(&(pool->notify));

	if ((!(InitializeCriticalSectionAndSpinCount(&pool->lock, 0x00000400))
		|| (pool->threads == NULL) || (pool->queue == NULL))) {
		if (pool)
			threadpool_free(pool);

		return NULL;
	}

	for (i = 0; i < thread_count; i++) {
		if ((pool->threads[i] = CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)&threadpool_thread, pool, 0, &ret))
			== NULL) {
			threadpool_destroy(pool);
			return NULL;
		}
		pool->thread_count++;
		pool->started++;
	}

	return pool;
}

int threadpool_destroy(threadpool_t *pool)
{
	int err = 0;

	if (pool == NULL)
		return TP_INVALID;

	EnterCriticalSection(&pool->lock);

	do {
		if (pool->p_shutdown) {
			err = TP_SHUTDOWN;
			break;
		}

		pool->p_shutdown = true;
		WakeAllConditionVariable(&(pool->notify));

		LeaveCriticalSection(&pool->lock);

		WaitForMultipleObjects(pool->thread_count, pool->threads, TRUE, INFINITE);
	} while (0);

	if (!err)
		threadpool_free(pool);

	return err;
}