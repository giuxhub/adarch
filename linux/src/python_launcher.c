#include "stdafx.h"
#include "python_utils.h"
#include "python_launcher.h"
#include "python_log.h"
#include "python_connection.h"

#include "log.h"

static bool init = false;
static PyThreadState *main_thread;

static int py_trace_func(PyObject *obj, PyFrameObject *f, int what, PyObject *arg)
{
	Py_ssize_t size;

	switch (what) {
	case PyTrace_CALL:
		if (Py_OptimizeFlag)
			f->f_lineno = PyCode_Addr2Line(f->f_code, f->f_lasti);

		logging(DEBUG, L"call: file[%S], line[%d], func[%S]",
			PyUnicode_AsUTF8AndSize(f->f_code->co_filename, &size),
			f->f_lineno,
			PyUnicode_AsUTF8AndSize(f->f_code->co_name, &size));
		break;

	case PyTrace_RETURN:
		break;

	case PyTrace_EXCEPTION:
		logging(WARNING, L"exception: file[%S], line[%d], func[%S]",
			PyUnicode_AsUTF8AndSize(f->f_code->co_filename, &size),
			f->f_lineno,
			PyUnicode_AsUTF8AndSize(f->f_code->co_name, &size));

		break;
	}
	return 0;
}


void py_init(const char *arg, size_t *size)
{
	if (init)
		return;

	Py_SetProgramName(Py_DecodeLocale(arg, size));

	PyImport_AppendInittab("log", &PyInit_pylog);
	PyImport_AppendInittab("connection", &PyInit_pyconnection);

	Py_Initialize();

	py_add_source_path(".");
	PyEval_SetTrace(py_trace_func, NULL);

	if (!PyEval_ThreadsInitialized()) {;
		PyEval_InitThreads();
		main_thread = PyEval_SaveThread();
	}

	init = true;
}

void py_destroy(void)
{
	if (!init)
		return;

	PyEval_RestoreThread(main_thread);

	Py_Finalize();
	init = false;
}
