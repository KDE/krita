/*
 Copyright 1998 The Regents of the University of California. 
 All rights reserved. See Legal.htm for full text and disclaimer.
*/

/* Prevent warnings */
#if defined(_XOPEN_SOURCE)
  #undef _XOPEN_SOURCE
#endif
#if defined(_POSIX_C_SOURCE)
  #undef _POSIX_C_SOURCE
#endif

#include "Python.h"
#ifdef __cplusplus
extern "C" {
#endif
PyObject py_object_initializer = {PyObject_HEAD_INIT(0)};
#ifdef __cplusplus
}
#endif





       
