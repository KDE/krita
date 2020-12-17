// This file is part of PyKrita, Krita' Python scripting plugin.
//
// SPDX-FileCopyrightText: 2006 Paul Giannaros <paul@giannaros.org>
// SPDX-FileCopyrightText: 2012, 2013 Shaheed Haque <srhaque@theiet.org>
// SPDX-FileCopyrightText: 2013 Alex Turbov <i.zaufi@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
//

#include "PykritaModule.h"

#include "kis_debug.h"

#define PYKRITA_INIT PyInit_pykrita

struct module_state {
    PyObject *error;
};

#if defined(IS_PY3K)
#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))
#else
#define GETSTATE(m) (&_state)
static struct module_state _state;
#endif

/// \note Namespace name written in uppercase intentionally!
/// It will appear in debug output from Python plugins...
namespace PYKRITA
{
    PyObject* debug(PyObject* /*self*/, PyObject* args)
    {
        const char* text;

        if (PyArg_ParseTuple(args, "s", &text))
            dbgScript << text;
        Py_INCREF(Py_None);
        return Py_None;
    }
}                                                           // namespace PYKRITA

namespace
{
    PyMethodDef pykritaMethods[] = {
        {
            "qDebug"
            , &PYKRITA::debug
            , METH_VARARGS
            , "True KDE way to show debug info"
        }
        , { 0, 0, 0, 0 }
    };
}                                                           // anonymous namespace

//BEGIN Python module registration
#if defined(IS_PY3K)
// Python 3 initializes modules differently from Python 2
//
static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT
    , "pykrita"
    , "The pykrita module"
    , -1
    , pykritaMethods
    , 0
    , 0
    , 0
    , 0
};

#define INITERROR return NULL

PyMODINIT_FUNC PyInit_pykrita()

#else
#define INITERROR return

void
initpykrita(void)
#endif
{
#if defined(IS_PY3K)
    PyObject *pykritaModule = PyModule_Create(&moduledef);
#else
    PyObject *pykritaModule = Py_InitModule("pykrita", pykritaMethods);
#endif

    if (pykritaModule == NULL)
        INITERROR;

    PyModule_AddStringConstant(pykritaModule, "__file__", __FILE__);

#if defined(IS_PY3K)
    return pykritaModule;
#endif
}

//END Python module registration

// krita: space-indent on; indent-width 4;
#undef PYKRITA_INIT
