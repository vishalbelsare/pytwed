/*
Filename: twed_wrap.c
Python wrapper for twed
Author: JZ
Licence: MIT
******************************************************************
This software and description is free delivered "AS IS" with no 
guaranties for work at all. Its up to you testing it modify it as 
you like, but no help could be expected from me due to lag of time 
at the moment. I will answer short relevant questions and help as 
my time allow it. I have tested it played with it and found no 
problems in stability or malfunctions so far. 
Have fun.
*****************************************************************
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <Python.h>
#include <numpy/arrayobject.h>
#include "twed.h"

static PyObject* twed_ (PyObject* dummy, PyObject* args, PyObject* kw) {
    PyObject* input1 = NULL;
    PyObject* input2 = NULL;
    PyObject* input3 = NULL;
    PyObject* input4 = NULL;

    double nu = 0.001;
    double lmbda = 1.0;
    int degree = 2;

    static char* keywords[] = { "arr1", "arr2", "arr1_spec", "arr2_spec", "nu", "lmbda", "degree", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kw, "OOOO|ddi", keywords, &input1, &input2, &input3, &input4, &nu, &lmbda, &degree)) {
        Py_RETURN_NONE;
    }

    int ndims = PyArray_NDIM(input1);
    uint64_t* arr1_dims = PyArray_DIMS(input1);
    uint64_t* arr2_dims = PyArray_DIMS(input2);

    PyArrayObject* arr1 = NULL;
    PyArrayObject* arr2 = NULL;
    PyArrayObject* ts_specs1 = NULL;
    PyArrayObject* ts_specs2 = NULL;
	
    int n_feats = 1;
    if(ndims > 1) { 
		n_feats = arr1_dims[1];
        arr1 = input1;
        arr2 = input2;
	} else {
        uint64_t dims1[] = { arr1_dims[0], 1 };
        PyArray_Dims newshape1 = { dims1, 2 };

        uint64_t dims2[] = { arr2_dims[0], 1 };
        PyArray_Dims newshape2 = { dims2, 2 };

        arr1 = (PyArrayObject*)PyArray_Newshape(input1, &newshape1, NPY_CORDER);
        arr2 = (PyArrayObject*)PyArray_Newshape(input2, &newshape2, NPY_CORDER);
    }

    arr1 = (PyArrayObject*)PyArray_FromAny(arr1, PyArray_DescrFromType(NPY_DOUBLE), 1, 2, NPY_ARRAY_CARRAY_RO, NULL);
    if(arr1 == NULL) goto fail;
    arr2 = (PyArrayObject*)PyArray_FromAny(arr2, PyArray_DescrFromType(NPY_DOUBLE), 1, 2, NPY_ARRAY_CARRAY_RO, NULL);
    if(arr2 == NULL) goto fail;
    ts_specs1 = (PyArrayObject*)PyArray_FromAny(input3, PyArray_DescrFromType(NPY_DOUBLE), 1, 1, NPY_ARRAY_CARRAY_RO, NULL);
    if(ts_specs1 == NULL) goto fail;
    ts_specs2 = (PyArrayObject*)PyArray_FromAny(input4, PyArray_DescrFromType(NPY_DOUBLE), 1, 1, NPY_ARRAY_CARRAY_RO, NULL);
    if(ts_specs2 == NULL) goto fail;

    double* arr1_data = (double*)PyArray_DATA(arr1);
    double* arr2_data = (double*)PyArray_DATA(arr2);
    double* arr1_specs_data = (double*)PyArray_DATA(ts_specs1);
    double* arr2_specs_data = (double*)PyArray_DATA(ts_specs2);

    double ret = DTWEDL1d(n_feats, arr1_data, (int)arr1_dims[0], arr1_specs_data, arr2_data, (int)arr2_dims[0], arr2_specs_data, nu, lmbda, degree);

    Py_DECREF(arr1);
    Py_DECREF(arr2);
    Py_DECREF(ts_specs1);
    Py_DECREF(ts_specs2);

    return PyFloat_FromDouble(ret);

fail:
    Py_XDECREF(arr1);
    Py_XDECREF(arr2);
    Py_XDECREF(ts_specs1);
    Py_XDECREF(ts_specs2);

    Py_RETURN_NONE;
}

static PyMethodDef twedmethods[] = {
    { "twed", (PyCFunctionWithKeywords)twed_,
      METH_VARARGS|METH_KEYWORDS,
      "Computes the Time Warp Edit Distance (Marteau, 2009).\n"
      "Inputs:\n"
      "\tFirst input (N x D)\n"
      "\tSecond input (M x D)\n"
      "\tTimepoint indices for the first input (N x 1)\n"
      "\tTimepoint indices for the second input (M x 1)\n"
      "\tKeywords nu (double), lmbda (double), degree (integer)\n"
      "\n"
      "Output:\n"
      "\tThe distance between the two inputs." },
    {NULL, NULL, 0, NULL} /* Sentinel */
};
   
PyMODINIT_FUNC PyInit_twed(void)
{
    PyObject *module;
    static struct PyModuleDef twed = {
    PyModuleDef_HEAD_INIT,
    "twed", /* name of module */
    "",          /* module documentation, may be NULL */
    -1,          /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
    twedmethods,
	NULL,
    NULL,
    NULL,
    NULL
	};
    module = PyModule_Create(&twed);
    if (!module) return NULL;

    /* Load `numpy` functionality. */
    import_array();

    return module;
}
