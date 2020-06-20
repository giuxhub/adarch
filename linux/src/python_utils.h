#ifndef __INC_PYTHON_UTILS_H_
#define __INC_PYTHON_UTILS_H_

#include <frameobject.h>

#define SET_EXCEPTION(x) PyErr_SetString(PyExc_RuntimeError, #x)

struct module_state {
	PyObject *error;
};

#define GETSTATE(m) ((struct module_state *)PyModule_GetState(m))

PyObject *py_none(void);
void py_release_none(void);
PyObject *py_bad_arg(void);
PyObject *py_exception(const char *err, ...);

PyObject *py_get_obj_from_tuple(PyObject *args, int pos);
double py_get_double(PyObject *obj);
float py_get_float(PyObject *obj);
long py_get_long(PyObject *obj);
int py_get_int(PyObject *obj);
unsigned char py_get_byte(PyObject *obj);
unsigned long py_get_ulong(PyObject *obj);
unsigned int py_get_uint(PyObject *obj);
bool py_get_bytes(PyObject *obj, char **ret);
bool py_get_str(PyObject *obj, wchar_t **ret);
bool py_get_bool(PyObject *obj);

void py_add_source_path(const char *path);

PyObject *py_get_class_instance(const char *module_name, const char *class_name, PyObject *class_args);
PyObject *py_call_class_method(PyObject *instance, const char *meth_name, PyObject *meth_args);
void py_release_reference(PyObject *ref);

PyObject *py_get_class_instance_th_safe(const char *module_name, const char *class_name, const char *fmt, ...);
PyObject *py_call_class_method_th_safe(PyObject *instance, const char *meth_name, const char *fmt, ...);
void py_release_reference_th_safe(PyObject *ref);


#endif
