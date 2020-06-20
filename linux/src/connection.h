#ifndef __INC_CONNECTION_H_
#define __INC_CONNECTION_H_

#include "connection_task.h"
#include "connection_active.h"
#include "threadpool.h"

typedef struct connection_t
{
	threadpool_t *pool;
	pthread_mutex_t shutdown_mux;
	pthread_mutex_t accept_mux;
	connection_task_t *task;
	connection_active_t *conn_a;
	int sock;
	bool _started;
	bool _shutdown;
} connection_t;

typedef enum read_socket_error_t
{
	CONN_NONE = 0, CONN_SELECT = -2, CONN_TIMEOUT = -3, CONN_SHUTDOWN = -4,
} read_socket_error_t;

connection_t *connection_init(uint16_t port, int max_conn);
void connection_destroy(connection_t *conn,
		void (*task_free)(connection_task_t *));
void connection_add_task(connection_t *conn, connection_task_t *task);
void connection_start_threadpool(connection_t *conn, u_int max_conn);
bool connection_get_status(connection_t *conn);

#endif
