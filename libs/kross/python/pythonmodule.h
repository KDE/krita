/***************************************************************************
 * pythonmodule.h
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

#ifndef KROSS_PYTHONMODULE_H
#define KROSS_PYTHONMODULE_H

#include "pythonconfig.h"
#include "pythonextension.h"

#include <QString>

namespace Kross {

    // Forward declaration.
    class PythonInterpreter;
    class PythonModulePrivate;

    /**
     * The PythonModule is the __main__ python environment
     * used as global object namespace.
     *
     * The module also spends access to the whole Kross
     * functionality and manages all the PythonExtension
     * modules.
     */
    class PythonModule : public Py::ExtensionModule<PythonModule>
    {
        public:

            /**
             * Constructor.
             *
             * \param interpreter The \a PythonInterpreter instance
             *        used to create this PythonModule.
             */
            PythonModule(PythonInterpreter* interpreter);

            /**
             * Destructor.
             */
            virtual ~PythonModule();

            /**
             * \return the dictonary this PythonModule has.
             */
            Py::Dict getDict();

        private:
            /// Internal d-pointer class.
            PythonModulePrivate* d;

            /// Import hook used to load external kross libs on demand.
            Py::Object import(const Py::Tuple&);
    };

}

#endif
