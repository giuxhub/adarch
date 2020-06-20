#include "stdafx.h"
#include "utils.h"
#include "log.h"
#include "config.h"
#include "connection_task.h"

connection_task_t *connection_task_init(const char *file, const char *_class,
		const char *method, int sock, int port)
{
	assert(
			file != NULL && _class != NULL && method != NULL && sock >= 0 && port > 0);

	connection_task_t *task = NULL;

	utils_create(task, connection_task_t, 1);

	utils_create(task->file, char, CONFIG_MAX_LEN);
	utils_create(task->_class, char, CONFIG_MAX_LEN);
	utils_create(task->method, char, CONFIG_MAX_LEN);

	snprintf(task->file, CONFIG_MAX_LEN, "%s", file);
	snprintf(task->_class, CONFIG_MAX_LEN, "%s", _class);
	snprintf(task->method, CONFIG_MAX_LEN, "%s", method);

	task->main_sock = sock;
	task->port = port;

	return task;
}

void connection_task_destroy(connection_task_t *task)
{
	assert(task != NULL);

	utils_free(task->file);
	utils_free(task->_class);
	utils_free(task->method);
	utils_free(task);
}

PyObject *connection_task_build(connection_task_t *task, int curr_sock,
		const char *ip, uint16_t port)
{
	assert(task != NULL);

	return py_get_class_instance_th_safe(task->file, task->_class, "(iiis)",
			task->main_sock, port, curr_sock, ip);
}

PyObject *connection_task_do(connection_task_t *task, PyObject *instance,
		const char *buf)
{
	assert(task != NULL && instance != NULL && buf != NULL);

	return py_call_class_method_th_safe(instance, task->method, "(s)", buf);
}
