/***************************************************************************
 * pythonconfig.h
 * This file is part of the KDE project
 * copyright (C)2004-2005 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef KROSS_PYTHON_CONFIG_H
#define KROSS_PYTHON_CONFIG_H

#include "../core/krossconfig.h"

// Prevent warnings
#if defined(_XOPEN_SOURCE)
  #undef _XOPEN_SOURCE
#endif
#if defined(_POSIX_C_SOURCE)
  #undef _POSIX_C_SOURCE
#endif

// The Python.h needs to be included first.
#include <Python.h>
#include <object.h>
#include <compile.h>
#include <eval.h>
#include <frameobject.h>

// Include the PyCXX stuff.
#include "cxx/Config.hxx"
#include "cxx/Objects.hxx"
#include "cxx/Extensions.hxx"

namespace Kross {

/**
 * The Python plugin for the \a Kross scripting framework.
 *
 * The code in this namespace manage the embedded python
 * interpreter and python-scripts.
 *
 * There is no dependency to e.g. the \a Kross::KexiDB
 * wrapper. Everything is handled through the common
 * \a Kross::Api bridge. Therefore this interpreter-
 * implementation should be able to make all defined
 * wrappers accessible by the python scripting
 * language.
 *
 * Internaly we use PyCXX - a set of classes to help
 * create extensions of python in the C++ language - to
 * access the python c api. Any python version since
 * 2.0 is supported.
 *
 * \author Sebastian Sauer
 * \sa http://www.python.org
 * \sa http://cxx.sourceforge.net
 */
namespace Python {

    // The version of this python plugin. This will be exported
    // to the scripting code. That way we're able to write
    // scripting code for different incompatible Kross python
    // bindings by checking the version. You should increment
    // this number only if you really know what you're doing.
    #define KROSS_PYTHON_VERSION 1

    // Enable debugging for Kross::Python::PythonScript
    //#define KROSS_PYTHON_SCRIPT_CTOR_DEBUG
    //#define KROSS_PYTHON_SCRIPT_DTOR_DEBUG
    //#define KROSS_PYTHON_SCRIPT_INIT_DEBUG
    //#define KROSS_PYTHON_SCRIPT_FINALIZE_DEBUG
    //#define KROSS_PYTHON_SCRIPT_EXEC_DEBUG
    //#define KROSS_PYTHON_SCRIPT_CALLFUNC_DEBUG
    //#define KROSS_PYTHON_SCRIPT_CLASSINSTANCE_DEBUG

    // Enable debugging for Kross::Python::PythonModule
    //#define KROSS_PYTHON_MODULE_DEBUG

    // Enable debugging for Kross::Python::PythonExtension
    //#define KROSS_PYTHON_EXTENSION_CTOR_DEBUG
    //#define KROSS_PYTHON_EXTENSION_DTOR_DEBUG

    //#define KROSS_PYTHON_EXTENSION_TOPYOBJECT_DEBUG

    //#define KROSS_PYTHON_EXTENSION_GETATTR_DEBUG
    //#define KROSS_PYTHON_EXTENSION_GETATTRMETHOD_DEBUG
    //#define KROSS_PYTHON_EXTENSION_SETATTR_DEBUG

    //#define KROSS_PYTHON_EXTENSION_CALL_DEBUG

    //#define KROSS_PYTHON_VARIANT_DEBUG

}}

#endif
