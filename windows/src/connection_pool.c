#include "stdafx.h"
#include "config.h"
#include "connection_pool.h"
#include "connection_task.h"
#include "utils.h"
#include "jsmn.h"
#include "log.h"

#define MAX_CONN 8

static connection_pool_t *conn_p = NULL;
static HANDLE mux = NULL;

static void connection_pool_free(connection_pool_t *conn_list)
{
	assert(conn_list != NULL);

	connection_destroy(conn_list->conn, connection_task_destroy);
	utils_free(conn_list);
}

bool connection_pool_init(list_t *config)
{
	assert(config != NULL);

	config_t *tmp;
	config_port_list_t *entry_tmp;
	connection_pool_t *tmp_conn;
	SECURITY_ATTRIBUTES security = { sizeof(security), NULL, true };

	if ((mux = CreateMutex(&security, FALSE, NULL)) == NULL)
		return false;

	utils_create(conn_p, connection_pool_t, 1);
	list_init(&(conn_p->list));

	list_t *cur, *n;
	list_for_each(cur, n, config) {
		tmp = list_entry(cur, config_t, list);
		list_t *curr, *nn = NULL;
		list_for_each(curr, nn, &(tmp->entry->ports->list)){
			entry_tmp = list_entry(curr, config_port_list_t, list);
			utils_create(tmp_conn, connection_pool_t, 1);
			if ((tmp_conn->conn = connection_init((entry_tmp->port), MAX_CONN)) == NULL) {
				utils_free(tmp_conn);
				logging(DEBUG, L"connection init");
				return false;
			}
			connection_task_t *task = NULL;
			if ((task = connection_task_init(tmp->entry->file, tmp->entry->_class,
				tmp->entry->method, tmp_conn->conn->sock, entry_tmp->port)) == NULL) {
				connection_task_destroy(task);
				logging(DEBUG, L"task setup");
				return false;
			}
			connection_add_task(tmp_conn->conn, task);
			connection_start_threadpool(tmp_conn->conn, MAX_CONN);
			list_add(&(tmp_conn->list), &(conn_p->list));
		}
	}
	return true;
}

void connection_pool_destroy(void)
{
	if (conn_p == NULL)
		return;

	WaitForSingleObject(mux, INFINITE);
	connection_pool_t *curr, *nn = NULL;

	list_destroy(connection_pool_t, curr, connection_pool_t, nn, conn_p, list, connection_pool_free);
	CloseHandle(mux);
}

connection_t *connection_pool_find(int fd)
{
	assert(conn_p != NULL && fd >= 0);

	connection_pool_t *tmp = NULL;
	list_t *cur, *n;

	WaitForSingleObject(mux, INFINITE);
	list_for_each(cur, n, &(conn_p->list)) {
		tmp = list_entry(cur, connection_pool_t, list);
		if (tmp->conn->sock == fd) {
			ReleaseMutex(mux);
			return tmp->conn;
		}
	}
	ReleaseMutex(mux);
	return NULL;
}

