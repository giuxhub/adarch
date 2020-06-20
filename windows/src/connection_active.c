#include "stdafx.h"
#include "connection_active.h"
#include "utils.h"
#include "log.h"

static void connection_active_free(connection_activelist_t *conn_list)
{
	assert(conn_list != NULL);

	closesocket(conn_list->fd);
	utils_free(conn_list);
}

connection_active_t *connection_active_init(void)
{
	connection_active_t *conn = NULL;

	utils_create(conn, connection_active_t, 1);
	utils_create(conn->conn_list, connection_activelist_t, 1);

	list_init(&(conn->conn_list->list));

	SECURITY_ATTRIBUTES security = { sizeof(security), NULL, true };
	if ((conn->mux = CreateMutex(&security, FALSE, NULL)) == NULL) {
		utils_free(conn->conn_list);
		utils_free(conn);
		return NULL;
	}

	return conn;
}

void connection_active_destroy(connection_active_t *conn)
{
	assert(conn != NULL);

	WaitForSingleObject(conn->mux, INFINITE);
	connection_activelist_t *curr, *nn = NULL;
	list_destroy(connection_activelist_t, curr, connection_activelist_t, nn, (conn->conn_list), list, connection_active_free);

	CloseHandle(conn->mux);
	utils_free(conn);
}

connection_activelist_t *connection_active_add(connection_active_t *conn, int fd)
{
	assert(conn != NULL);

	connection_activelist_t *tmp_conn_list = NULL;

	utils_create(tmp_conn_list, connection_activelist_t, 1);

	tmp_conn_list->fd = fd;
	tmp_conn_list->stop = false;

	WaitForSingleObject(conn->mux, INFINITE);
	list_add(&(tmp_conn_list->list), &(conn->conn_list->list));
	ReleaseMutex(conn->mux);

	return tmp_conn_list;
}

connection_activelist_t *connection_active_find(connection_active_t *conn, int fd)
{
	assert(conn != NULL && fd >= 0);

	connection_activelist_t *tmp = NULL;
	list_t *cur, *n;

	WaitForSingleObject(conn->mux, INFINITE);
	list_for_each(cur, n, &(conn->conn_list->list)) {
		tmp = list_entry(cur, connection_activelist_t, list);
		if (tmp->fd == fd) {
			ReleaseMutex(conn->mux);
			return tmp;
		}
	}
	ReleaseMutex(conn->mux);

	return NULL;
}

void connection_active_shutdown(connection_active_t *conn, connection_activelist_t *conn_list)
{
	assert(conn != NULL && conn_list != NULL);

	WaitForSingleObject(conn->mux, INFINITE);
	conn_list->stop = true;
	ReleaseMutex(conn->mux);
}

void connection_active_close(connection_active_t *conn, connection_activelist_t *conn_list)
{
	assert(conn != NULL && conn_list != NULL);

	py_release_reference_th_safe(conn_list->instance);

	WaitForSingleObject(conn->mux, INFINITE);
	list_del(&(conn_list->list));
	connection_active_free(conn_list);
	ReleaseMutex(conn->mux);
}

bool connection_active_getstatus(connection_active_t *conn, connection_activelist_t *conn_list)
{
	assert(conn != NULL && conn_list != NULL);

	WaitForSingleObject(conn->mux, INFINITE);
	bool ret = conn_list->stop;
	ReleaseMutex(conn->mux);

	return ret;
}

void connection_active_setinstance(connection_active_t *conn, connection_activelist_t *conn_list, PyObject *instance)
{
	assert(conn != NULL && conn_list != NULL);

	WaitForSingleObject(conn->mux, INFINITE);
	conn_list->instance = instance;
	ReleaseMutex(conn->mux);
}

ssize_t connection_active_sendbuf(connection_active_t *conn, connection_activelist_t *conn_list, const char *buf, size_t len)
{
	assert(conn != NULL && conn_list != NULL && buf != NULL && 0 < len);

	WaitForSingleObject(conn->mux, INFINITE);
	ssize_t s = 0;
	if ((s = send(conn_list->fd, buf, len, 0)) < 0)
		logging(WARNING, L"unable to send on socket %d", conn_list->fd);
	ReleaseMutex(conn->mux);

	return s;
}
