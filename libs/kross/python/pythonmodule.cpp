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

using namespace Kross::Python;

namespace Kross { namespace Python {

    /// @internal
    class PythonModulePrivate
    {
        public:

            /**
             * The \a PythonInterpreter instance this module is
             * part of.
             */
            PythonInterpreter* m_interpreter;

            /**
             * List of \a PythonExtension instances accessible
             * via this \a PythonModule instance.
             */
            QMap<QString, PythonExtension*> m_modules;

    };

}}

PythonModule::PythonModule(PythonInterpreter* interpreter)
    : Py::ExtensionModule<PythonModule>("__main__")
    , d(new PythonModulePrivate())
{
#ifdef KROSS_PYTHON_MODULE_DEBUG
    krossdebug( QString("Kross::Python::PythonModule::Constructor") );
#endif

    d->m_interpreter = interpreter;

    add_varargs_method("_import", &PythonModule::import, "FIXME: Documentation");

    initialize("The PythonModule is the __main__ python environment used as global object namespace.");
}

PythonModule::~PythonModule()
{
#ifdef KROSS_PYTHON_MODULE_DEBUG
    krossdebug( QString("Kross::Python::PythonModule::Destructor name='%1'").arg(name().c_str()) );
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
        if(modname.startsWith("kross")) {
#ifdef KROSS_PYTHON_MODULE_DEBUG
            krossdebug( QString("Kross::Python::PythonModule::import() module=%1").arg(modname) );
#endif
            if( modname.find( QRegExp("[^a-zA-Z0-9\\_\\-]") ) >= 0 ) {
                krosswarning( QString("Denied import of Kross module '%1' cause of untrusted chars.").arg(modname) );
            }
            else {
                Kross::Api::Module* module = Kross::Api::Manager::scriptManager()->loadModule(modname);
                if(module)
                    return PythonExtension::toPyObject(module);
                krosswarning( QString("Loading of Kross module '%1' failed.").arg(modname) );
            }

        }
    }
    return Py::None();
}
