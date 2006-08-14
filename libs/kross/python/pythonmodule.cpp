/***************************************************************************
 * pythonmodule.cpp
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

#include "pythonmodule.h"
#include "pythoninterpreter.h"

#include <QRegExp>

using namespace Kross;

namespace Kross {

    /// @internal
    class PythonModulePrivate
    {
        public:

            /**
             * The \a PythonInterpreter instance this module is
             * part of.
             */
            PythonInterpreter* const m_interpreter;

            /**
             * List of \a PythonExtension instances accessible
             * via this \a PythonModule instance.
             */
            //QMap<QString, PythonExtension*> m_modules;

            PythonModulePrivate(PythonInterpreter* const interpreter)
                : m_interpreter(interpreter) {}
    };

}

PythonModule::PythonModule(PythonInterpreter* interpreter)
    : Py::ExtensionModule<PythonModule>("__main__")
    , d(new PythonModulePrivate(interpreter))
{
    #ifdef KROSS_PYTHON_MODULE_DEBUG
        krossdebug( QString("PythonModule::Constructor name='%1'").arg(name().c_str()) );
    #endif

    add_varargs_method("_import", &PythonModule::import, "Kross import hook.");
    initialize("The PythonModule is the __main__ python environment used as global object namespace.");
}

PythonModule::~PythonModule()
{
    #ifdef KROSS_PYTHON_MODULE_DEBUG
        krossdebug( QString("PythonModule::Destructor name='%1'").arg(name().c_str()) );
    #endif

    delete d;
}

Py::Dict PythonModule::getDict()
{
    return moduleDictionary();
}

Py::Object PythonModule::import(const Py::Tuple& args)
{
    if(args.size() > 0) {
        QString modname = args[0].as_string().c_str();
        if(Kross::Manager::self().hasObject(modname)) {
            #ifdef KROSS_PYTHON_MODULE_DEBUG
                krossdebug( QString("PythonModule::import() module=%1 is internal").arg(modname) );
            #endif
            QObject* obj = Kross::Manager::self().object(modname);
            Q_ASSERT(obj);
            return Py::asObject( new PythonExtension(obj) );
        }
    }
    return Py::None();
}
