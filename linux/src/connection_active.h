#ifndef __INC_CONNECTION_ACTIVE_H_
#define __INC_CONNECTION_ACTIVE_H_

#include "python_utils.h"
#include "list.h"

typedef struct connection_activelist_t
{
	int fd;
	bool stop;
	PyObject *instance;
	list_t list;
} connection_activelist_t;

typedef struct connection_active_t
{
	connection_activelist_t *conn_list;
	pthread_mutex_t mux;
} connection_active_t;

connection_active_t *connection_active_init(void);
void connection_active_destroy(connection_active_t *conn);
connection_activelist_t *connection_active_add(connection_active_t *conn,
		int fd);
connection_activelist_t *connection_active_find(connection_active_t *conn,
		int fd);
void connection_active_shutdown(connection_active_t *conn,
		connection_activelist_t *conn_list);
void connection_active_close(connection_active_t *conn,
		connection_activelist_t *conn_list);
bool connection_active_getstatus(connection_active_t *conn,
		connection_activelist_t *conn_list);
void connection_active_setinstance(connection_active_t *conn,
		connection_activelist_t *conn_list, PyObject *instance);
ssize_t connection_active_sendbuf(connection_active_t *conn,
		connection_activelist_t *conn_list, const char *buf, size_t len);

#endif

