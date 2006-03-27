/***************************************************************************
 * pythonsecurity.h
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

#ifndef KROSS_PYTHON_SECURITY_H
#define KROSS_PYTHON_SECURITY_H

#include "pythonconfig.h"

#include <qstring.h>

namespace Kross { namespace Python {

    // Forward declaration.
    class PythonInterpreter;

    /**
     * This class handles the used Zope3 RestrictedPython
     * package to spend a restricted sandbox for scripting
     * code.
     *
     * The RestrictedPython code is avaible as Python files.
     * So, this class takes care of loading them and spending
     * the functions we need to access the functionality
     * from within Kross. That way it's easy to update the
     * module with a newer version if some security issues
     * show up.
     *
     * What the RestrictedPython code does is to compile
     * the plain python code (py) into compiled python code (pyc)
     * and manipulate those compiled code by replacing unsafe
     * code with own wrapped code.
     * As example a simple "x = y.z" would be transfered to
     * "x = _getattr_(y, 'z')". The _getattr_ is defined in
     * the RestrictedPython module and will take care of
     * applied restrictions.
     *
     * \see http://www.zope.org
     * \see http://svn.zope.org/Zope3/trunk/src/RestrictedPython/
     */
    class PythonSecurity : public Py::ExtensionModule<PythonSecurity>
    {
        public:

            /**
             * Constructor.
             *
             * \param interpreter The \a PythonInterpreter instance
             *       used to create this Module.
             */
            explicit PythonSecurity(PythonInterpreter* interpreter);

            /**
             * Destructor.
             */
            virtual ~PythonSecurity();

            /**
             * Compile python scripting code and return a restricted
             * code object.
             *
             * \param source The python scripting code.
             * \param filename The filename used on errormessages.
             * \param mode Compilemode, could be 'exec' or 'eval' or 'single'.
             * \return The compiled python code object on success else 
             *         NULL. The caller owns the resulting object and needs
             *         to take care to decrease the ref-counter it not needed
             *         any longer.
             */
            PyObject* compile_restricted(const QString& source, const QString& filename, const QString& mode);

#if 0
            //TODO
            void compile_restricted_function(const Py::Tuple& args, const QString& body, const QString& name, const QString& filename, const Py::Object& globalize = Py::None());
            void compile_restricted_exec(const QString& source, const QString& filename = "<string>");
            void compile_restricted_eval(const QString& source, const QString& filename = "<string>");
#endif

        private:
            /// We keep a pointer to the used \a PythonInterpreter.
            PythonInterpreter* m_interpreter;
            /// The imported external RestrictedPython module.
            Py::Module* m_pymodule;

            /// Initialize the restricted python module.
            inline void initRestrictedPython();

            /// Secure wrapper around the getattr method.
            Py::Object _getattr_(const Py::Tuple&);
    };

}}

#endif
