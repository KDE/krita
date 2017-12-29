// This file is part of PyKrita, Krita' Python scripting plugin.
//
// Copyright (C) 2006 Paul Giannaros <paul@giannaros.org>
// Copyright (C) 2012, 2013 Shaheed Haque <srhaque@theiet.org>
// Copyright (C) 2013 Alex Turbov <i.zaufi@gmail.com>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) version 3, or any
// later version accepted by the membership of KDE e.V. (or its
// successor approved by the membership of KDE e.V.), which shall
// act as a proxy defined in Section 6 of version 3 of the license.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library.  If not, see <http://www.gnu.org/licenses/>.
//

#include "PykritaModule.h"

#include "kis_debug.h"

#define PYKRITA_INIT PyInit_pykrita

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
PyMODINIT_FUNC PyInit_pykrita()
{
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
    PyObject *pykritaModule = PyModule_Create(&moduledef);
    PyModule_AddStringConstant(pykritaModule, "__file__", __FILE__);
    return pykritaModule;
}
//END Python module registration

// krita: space-indent on; indent-width 4;
#undef PYKRITA_INIT
