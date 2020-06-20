#include "stdafx.h"
#include "python_utils.h"
#include "log.h"

PyObject *py_none(void)
{
	Py_INCREF(Py_None);
	return Py_None;
}

void py_release_none(void)
{
	Py_DECREF(Py_None);
}

PyObject *py_bad_arg(void)
{
	PyErr_BadArgument();
	return NULL;
}

PyObject *py_exception(const char *err, ...)
{
	if (!err)
		PyErr_Clear();
	else {
		char err_buf[512 + 1];
		va_list args;

		va_start(args, err);
		vsnprintf(err_buf, sizeof(err_buf), err, args);
		va_end(args);

		PyErr_SetString(PyExc_RuntimeError, err_buf);
	}

	return py_none();
}

PyObject *py_get_obj_from_tuple(PyObject *args, int pos)
{
	PyObject *p_item = NULL;

	if (pos >= PyTuple_Size(args))
		return NULL;

	if (!(p_item = PyTuple_GetItem(args, pos)))
		return NULL;

	return p_item;
}

double py_get_double(PyObject *obj)
{
	if (!PyFloat_Check(obj))
		return 0;

	return PyFloat_AsDouble(obj);
}

float py_get_float(PyObject *obj)
{
	if (!PyFloat_Check(obj))
		return 0;

	return (float)PyFloat_AsDouble(obj);
}

long py_get_long(PyObject *obj)
{
	if (!PyLong_Check(obj))
		return 0;

	return PyLong_AsLong(obj);
}

//https://docs.python.org/2/howto/cporting.html#long-int-unification
int py_get_int(PyObject *obj)
{
	return (int)py_get_long(obj);
}

unsigned char py_get_byte(PyObject *obj)
{
	return (unsigned char)py_get_int(obj);
}

unsigned long py_get_ulong(PyObject *obj)
{
	if (!PyLong_Check(obj))
		return 0;

	return PyLong_AsUnsignedLong(obj);
}
//https://docs.python.org/2/howto/cporting.html#long-int-unification
unsigned int py_get_uint(PyObject *obj)
{
	return (unsigned int)py_get_ulong(obj);
}

bool py_get_bytes(PyObject *obj, char **ret)
{
	if (!PyBytes_Check(obj))
		return false;

	*ret = PyBytes_AsString(obj);
	return true;
}

bool py_get_str(PyObject *obj, wchar_t **ret)
{
	if (!PyUnicode_Check(obj))
		return false;

	*ret = PyUnicode_AsWideCharString(obj, NULL);
	return true;
}

bool py_get_bool(PyObject *obj)
{
	if (!PyBool_Check(obj))
		return false;

	return PyObject_IsTrue(obj) ? true : false;
}

void py_add_source_path(const char *path)
{
	PyObject *sys = PyImport_ImportModule("sys");
	PyObject *sys_path = PyObject_GetAttrString(sys, "path");
	PyObject *folder_path = PyUnicode_FromString(path);
	PyList_Append(sys_path, folder_path);

	Py_XDECREF(sys);
	Py_XDECREF(sys_path);
	Py_XDECREF(folder_path);
}

/**
* @param module_name: a char string that represent the module name
* @param class_name: a char string that represent the class name
* @param class_args: a stolen reference to a tuple object that represent the class object's initialization parameters
* @return instance: a new reference to a class instance object
*/
PyObject *py_get_class_instance(const char *module_name, const char *class_name, PyObject *class_args)
{
	assert(module_name != NULL && class_name != NULL && class_args != NULL);

	PyObject *name = NULL, *module = NULL, *dict = NULL, *class = NULL, *instance = NULL;

	do {
		name = PyUnicode_FromString(module_name); //new reference
		if (!name) {
			PyErr_Print();
			break;
		}

		module = PyImport_Import(name); //new reference
		if (!module) {
			PyErr_Print();
			break;
		}

		dict = PyModule_GetDict(module); //borrowed reference
		if (!dict) {
			PyErr_Print();
			break;
		}
		class = PyDict_GetItemString(dict, class_name); //borrowed reference
		if (!class) {
			PyErr_Print();
			break;
		}
		if (PyCallable_Check(class)) {
			instance = PyObject_CallObject(class, class_args); //new reference
			if (!instance) {
				PyErr_Print();
				break;
			}
		}
	} while (0);

	Py_XDECREF(name);
	Py_XDECREF(module);
	Py_XDECREF(class_args);

	return instance;
}

/**
* @param instance: a stolen reference to a class instance object
* @param meth_name: a char string that represent the method name
* @param meth_args: a stolen reference to a tuple object that represent the method's parameters
* @return ret: a new reference to the object containing the return value of the method's call
*/
PyObject *py_call_class_method(PyObject *instance, const char *meth_name, PyObject *meth_args)
{
	assert(instance != NULL && meth_name != NULL);

	if (meth_args == NULL) {
		logging(WARNING, L"meth_args is null");
		PyErr_Clear();
		meth_args = Py_BuildValue("(s)", "");
	}

	PyObject *method, *ret = NULL;

	do {
		method = PyObject_GetAttrString(instance, meth_name); //new reference
		if (!method) {
			PyErr_Print();
			break;
		}

		if (!PyCallable_Check(method)) {
			PyErr_Print();
			break;
		}

		ret = PyObject_CallObject(method, meth_args); //new reference

		if (!ret) {
			PyErr_Print();
			break;
		}
	} while (0);

	Py_XDECREF(method);
	Py_XDECREF(meth_args);

	return ret;
}

void py_release_reference(PyObject *ref)
{
	if (ref == NULL)
		return;

	Py_XDECREF(ref);
}

PyObject *py_get_class_instance_th_safe(const char *module_name, const char *class_name, const char *fmt, ...)
{
	PyGILState_STATE g_state;
	PyObject *instance = NULL;
	va_list args;

	g_state = PyGILState_Ensure();

	va_start(args, fmt);
	instance = py_get_class_instance(module_name, class_name, Py_VaBuildValue(fmt, args));
	va_end(args);

	PyGILState_Release(g_state);

	return instance;
}

PyObject *py_call_class_method_th_safe(PyObject *instance, const char *meth_name, const char *fmt, ...)
{
	PyGILState_STATE g_state;
	PyObject *ret = NULL;
	va_list args;

	g_state = PyGILState_Ensure();

	va_start(args, fmt);
	ret = py_call_class_method(instance, meth_name, Py_VaBuildValue(fmt, args));
	va_end(args);

	PyGILState_Release(g_state);

	return ret;
}

void py_release_reference_th_safe(PyObject *ref)
{
	PyGILState_STATE g_state;

	g_state = PyGILState_Ensure();
	py_release_reference(ref);
	PyGILState_Release(g_state);
}
