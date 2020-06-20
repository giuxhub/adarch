#include "threadpool.h"
#include "utils.h"

static void *threadpool_thread(void *threadpool)
{
	threadpool_t *pool = (threadpool_t *) threadpool;
	threadpool_task_t task;
	for (;;) {

		pthread_mutex_lock(&(pool->lock));

		while ((pool->count == 0) && (!pool->p_shutdown))
			pthread_cond_wait(&(pool->notify), &(pool->lock));

		if ((pool->p_shutdown) && (pool->count == 0))
			break;

		task.function = pool->queue[pool->head].function;
		task.argument = pool->queue[pool->head].argument;
		pool->head++;
		pool->head = (pool->head == pool->queue_size) ? 0 : pool->head;
		pool->count--;

		pthread_mutex_unlock(&(pool->lock));

		(*(task.function))(task.argument);
	}
	pool->started--;

	pthread_mutex_unlock(&(pool->lock));
	return NULL;
}

static int threadpool_free(threadpool_t *pool)
{
	if (pool == NULL || pool->started > 0)
		return -1;

	if (pool->threads) {
		utils_free(pool->threads);
		utils_free(pool->queue);

		pthread_mutex_lock(&(pool->lock));
		pthread_mutex_destroy(&(pool->lock));
		pthread_cond_destroy(&(pool->notify));
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

	if (pthread_mutex_lock(&(pool->lock)) != 0)
		return TP_LOCK_FAIL;

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

		if (pthread_cond_signal(&(pool->notify)) != 0) {
			err = TP_LOCK_FAIL;
			break;
		}

	} while (0);

	if (pthread_mutex_unlock(&pool->lock) != 0)
		err = TP_LOCK_FAIL;

	return err;
}

threadpool_t *threadpool_init(int thread_count, int queue_size)
{
	threadpool_t *pool = NULL;
	int i;
	int ret = 0;

	if (thread_count
			<= 0|| thread_count > MAX_THREADS || queue_size <= 0 || queue_size > MAX_QUEUE)
		return NULL;

	utils_create(pool, threadpool_t, 1);

	pool->thread_count = 0;
	pool->queue_size = queue_size;
	pool->head = pool->tail = pool->count = pool->started = 0;
	pool->p_shutdown = false;

	utils_create(pool->queue, threadpool_task_t, queue_size);
	utils_create(pool->threads, pthread_t, thread_count);

	if ((pthread_mutex_init(&(pool->lock), NULL) != 0) || (pthread_cond_init(&(pool->notify), NULL) != 0) ||
			(pool->threads == NULL) || (pool->queue == NULL)) {

		if (pool)
		threadpool_free(pool);

		return NULL;
	}

	for (i = 0; i < thread_count; i++) {
		ret = pthread_create(&pool->threads[i], NULL, &threadpool_thread, (void *)pool);
		if (!ret == 0) {
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
	int err = 0, i = 0;

	if (pool == NULL)
		return TP_INVALID;

	if (pthread_mutex_lock(&(pool->lock)) != 0)
		return TP_LOCK_FAIL;

	do {
		if (pool->p_shutdown) {
			err = TP_SHUTDOWN;
			break;
		}

		pool->p_shutdown = true;

		if ((pthread_cond_broadcast(&(pool->notify)) != 0) || (pthread_mutex_unlock(&(pool->lock)) != 0)) {
			err = TP_LOCK_FAIL;
			break;
		}

		for (i = 0; i < pool->thread_count; i++)
			pthread_join(pool->threads[i], NULL);

	} while (0);

	if (!err)
		threadpool_free(pool);

	return err;
}
