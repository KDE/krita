/*
 * This file is part of PyKrita, Krita' Python scripting plugin.
 *
 * Copyright (C) 2013 Alex Turbov <i.zaufi@gmail.com>
 * Copyright (C) 2014-2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only
 */

#ifndef __PYKRITA_MODULE_H__
#define  __PYKRITA_MODULE_H__

#include <Python.h>

#if PY_MAJOR_VERSION >= 3
#ifndef IS_PY3K
#define IS_PY3K
#endif
#endif

/**
 * Initializer for the built-in Python module.
 */
#if defined(IS_PY3K)
PyMODINIT_FUNC PyInit_pykrita();
#else
void initpykrita();
#endif

#endif
