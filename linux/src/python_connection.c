#include "stdafx.h"
#include "python_utils.h"
#include "python_connection.h"
#include "connection.h"
#include "connection_active.h"
#include "connection_pool.h"
#include "log.h"

PyObject *pyconnection_send(PyObject *self, PyObject *args)
{
	int id = 0, fd = 0;
	size_t len = 0;
	ssize_t r = 0;
	char *buffer = NULL;
	connection_t *conn = NULL;
	connection_activelist_t *conn_a = NULL;

	if ((id = py_get_int(py_get_obj_from_tuple(args, 0))) < 0)
		return py_bad_arg();

	if ((fd = py_get_int(py_get_obj_from_tuple(args, 1))) < 0)
		return py_bad_arg();

	if (!py_get_bytes(py_get_obj_from_tuple(args, 2), &buffer))
		return py_bad_arg();

	if (buffer == NULL)
		return py_bad_arg();

	if ((len = (size_t)py_get_int(py_get_obj_from_tuple(args, 3))) < 0)
		return py_bad_arg();


	Py_BEGIN_ALLOW_THREADS;
	conn = connection_pool_find(id);
	conn_a = connection_active_find(conn->conn_a, fd);
	r = connection_active_sendbuf(conn->conn_a, conn_a, buffer, len);
	Py_END_ALLOW_THREADS;

	return Py_BuildValue("(i)", r);
}

PyObject *pyconnection_shutdown(PyObject *self, PyObject *args)
{
	int id = 0, fd = 0;
	connection_t *conn = NULL;
	connection_activelist_t *conn_a = NULL;

	if ((id = py_get_int(py_get_obj_from_tuple(args, 0))) < 0)
		return py_bad_arg();

	if ((fd = py_get_int(py_get_obj_from_tuple(args, 1))) < 0)
		return py_bad_arg();

	Py_BEGIN_ALLOW_THREADS;
	conn = connection_pool_find(id);
	conn_a = connection_active_find(conn->conn_a, fd);
	connection_active_shutdown(conn->conn_a, conn_a);
	Py_END_ALLOW_THREADS;

	return py_none();
}

static int pyconnection_traverse(PyObject *m, visitproc visit, void *arg) {
	Py_VISIT(GETSTATE(m)->error);
	return 0;
}

static int pyconnection_clear(PyObject *m) {
	Py_CLEAR(GETSTATE(m)->error);
	return 0;
}

static PyMethodDef pyconnection_methods[] =
{
	{ "send", pyconnection_send, METH_VARARGS, NULL },
	{ "shutdown", pyconnection_shutdown, METH_VARARGS, NULL },
	{ NULL, NULL }
};

static struct PyModuleDef moduledef = {
	PyModuleDef_HEAD_INIT,
	"connection",
	NULL,
	sizeof(struct module_state),
	pyconnection_methods,
	NULL,
	pyconnection_traverse,
	pyconnection_clear,
	NULL
};

PyMODINIT_FUNC PyInit_pyconnection(void)
{
	PyObject *module = PyModule_Create(&moduledef);

	if (module == NULL)
		return NULL;

	struct module_state *st = GETSTATE(module);

	st->error = PyErr_NewException("connection.Error", NULL, NULL);
	if (st->error == NULL) {
		Py_DECREF(module);
		return NULL;
	}

	return module;
}
