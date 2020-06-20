#include "stdafx.h"
#include "python_utils.h"
#include "python_log.h"
#include "log.h"

PyObject *pylog_write(PyObject *self, PyObject *args)
{
	int lv = DEBUG;
	wchar_t *msg = NULL;

	if ((lv = py_get_int(py_get_obj_from_tuple(args, 0))) < 0)
		return py_bad_arg();

	if (!py_get_str(py_get_obj_from_tuple(args, 1), &msg))
		return py_bad_arg();

	if (msg == NULL || !wcsncmp(msg, L"", wcslen(msg)))
		return py_bad_arg();

	Py_BEGIN_ALLOW_THREADS;
	logging(lv, msg);
	Py_END_ALLOW_THREADS;

	PyMem_Free(msg);

	return py_none();
}

static int pylog_traverse(PyObject *m, visitproc visit, void *arg) {
	Py_VISIT(GETSTATE(m)->error);
	return 0;
}

static int pylog_clear(PyObject *m) {
	Py_CLEAR(GETSTATE(m)->error);
	return 0;
}

static PyMethodDef pylog_methods[] =
{
	{ "write", pylog_write, METH_VARARGS, NULL },
	{ NULL, NULL }
};

static struct PyModuleDef moduledef = {
	PyModuleDef_HEAD_INIT,
	"log",
	NULL,
	sizeof(struct module_state),
	pylog_methods,
	NULL,
	pylog_traverse,
	pylog_clear,
	NULL
};

PyMODINIT_FUNC PyInit_pylog(void)
{
	PyObject *module = PyModule_Create(&moduledef);

	if (module == NULL)
		return NULL;

	struct module_state *st = GETSTATE(module);

	st->error = PyErr_NewException("log.Error", NULL, NULL);
	if (st->error == NULL) {
		Py_DECREF(module);
		return NULL;
	}

	PyModule_AddIntConstant(module, "DEBUG", DEBUG);
	PyModule_AddIntConstant(module, "INFO", INFO);
	PyModule_AddIntConstant(module, "WARNING", WARNING);
	PyModule_AddIntConstant(module, "ERROR", ERR);
	PyModule_AddIntConstant(module, "CRITICAL", CRITICAL);

	return module;
}
