#include "stdafx.h"
#include "utils.h"
#include "log.h"
#include "connection_active.h"

static void connection_active_free(connection_activelist_t *conn_list)
{
	assert(conn_list != NULL);

	close(conn_list->fd);
	utils_free(conn_list);
}

connection_active_t *connection_active_init(void)
{
	connection_active_t *conn = NULL;

	utils_create(conn, connection_active_t, 1);
	utils_create(conn->conn_list, connection_activelist_t, 1);

	list_init(&(conn->conn_list->list));

	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

	if (pthread_mutex_init(&(conn->mux), &attr) != 0) {
		utils_free(conn->conn_list);
		utils_free(conn);
		return NULL;
	}

	return conn;
}

void connection_active_destroy(connection_active_t *conn)
{
	assert(conn != NULL);

	pthread_mutex_lock(&(conn->mux));
	connection_activelist_t *curr, *nn = NULL;
	list_destroy(curr, nn, (conn->conn_list), list, connection_active_free);
	pthread_mutex_destroy(&(conn->mux));

	utils_free(conn);
}

connection_activelist_t *connection_active_add(connection_active_t *conn,
		int fd)
{
	assert(conn != NULL);

	connection_activelist_t *tmp_conn_list = NULL;

	utils_create(tmp_conn_list, connection_activelist_t, 1);

	tmp_conn_list->fd = fd;
	tmp_conn_list->stop = false;

	pthread_mutex_lock(&(conn->mux));
	list_add(&(tmp_conn_list->list), &(conn->conn_list->list));
	pthread_mutex_unlock(&(conn->mux));


	return tmp_conn_list;
}

connection_activelist_t *connection_active_find(connection_active_t *conn,
		int fd)
{
	assert(conn != NULL && fd >= 0);

	connection_activelist_t *tmp = NULL;
	list_t *cur, *n;

	pthread_mutex_lock(&(conn->mux));
	list_for_each(cur, n, &(conn->conn_list->list))
	{
		tmp = list_entry(cur, connection_activelist_t, list);
		if (tmp->fd == fd) {
			pthread_mutex_unlock(&(conn->mux));
			return tmp;
		}
	}
	pthread_mutex_unlock(&(conn->mux));

	return NULL;
}

void connection_active_shutdown(connection_active_t *conn,
		connection_activelist_t *conn_list)
{
	assert(conn != NULL && conn_list != NULL);

	pthread_mutex_lock(&(conn->mux));
	conn_list->stop = true;
	pthread_mutex_unlock(&(conn->mux));
}

void connection_active_close(connection_active_t *conn,
		connection_activelist_t *conn_list)
{
	assert(conn != NULL && conn_list != NULL);

	py_release_reference_th_safe(conn_list->instance);

	pthread_mutex_lock(&(conn->mux));
	list_del(&(conn_list->list));
	connection_active_free(conn_list);
	pthread_mutex_unlock(&(conn->mux));
}

bool connection_active_getstatus(connection_active_t *conn,
		connection_activelist_t *conn_list)
{
	assert(conn != NULL && conn_list != NULL);

	pthread_mutex_lock(&(conn->mux));
	bool ret = conn_list->stop;
	pthread_mutex_unlock(&(conn->mux));

	return ret;
}

void connection_active_setinstance(connection_active_t *conn,
		connection_activelist_t *conn_list, PyObject *instance)
{
	assert(conn != NULL && conn_list != NULL);

	pthread_mutex_lock(&(conn->mux));
	conn_list->instance = instance;
	pthread_mutex_unlock(&(conn->mux));
}

ssize_t connection_active_sendbuf(connection_active_t *conn,
		connection_activelist_t *conn_list, const char *buf, size_t len)
{
	assert(conn != NULL && conn_list != NULL && buf != NULL && 0 < len);

	pthread_mutex_lock(&(conn->mux));
	ssize_t s = 0;
	if ((s = send(conn_list->fd, buf, len, 0)) < 0)
		logging(WARNING, L"unable to send on socket %d", conn_list->fd);
	pthread_mutex_unlock(&(conn->mux));

	return s;
}
