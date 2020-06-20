#ifndef __INC_CONNECTION_TASK_H_
#define __INC_CONNECTION_TASK_H_

#include "python_utils.h"

typedef struct connection_task_t
{
	char *file;
	char *_class;
	char *method;
	int main_sock;
	int port;
} connection_task_t;

connection_task_t *connection_task_init(const char *file, const char *_class,
		const char *method, int sock, int port);
void connection_task_destroy(connection_task_t *task);
PyObject *connection_task_build(connection_task_t *task, int curr_sock,
		const char *ip, uint16_t port);
PyObject *connection_task_do(connection_task_t *task, PyObject *instance,
		const char *buf);

#endif
