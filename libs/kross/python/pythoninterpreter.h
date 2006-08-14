/***************************************************************************
 * pythoninterpreter.h
 * This file is part of the KDE project
 * copyright (C)2004-2006 by Sebastian Sauer (mail@dipe.org)
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

#ifndef KROSS_PYTHONINTERPRETER_H
#define KROSS_PYTHONINTERPRETER_H

#include "pythonconfig.h"
#include "../core/krossconfig.h"
#include "../core/interpreter.h"
#include "../core/action.h"
#include "../core/manager.h"

#include <QString>

namespace Kross {

    // Forward declarations.
    class PythonScript;
    class PythonModule;
    class PythonInterpreterPrivate;

    /**
     * Python interpreter bridge.
     *
     * Implements an \a Kross::Interpreter for the python
     * interpreter.
     */
    class PythonInterpreter : public Kross::Interpreter
    {
            friend class PythonScript;
        public:

            /**
             * Constructor.
             *
             * \param info The \a Kross::InterpreterInfo instance
             *        which describes the \a PythonInterpreter for
             *        applications using Kross.
             */
            PythonInterpreter(Kross::InterpreterInfo* info);

            /**
             * Destructor.
             */
            virtual ~PythonInterpreter();

            /**
             * \return a \a PythonScript instance.
             */
            virtual Kross::Script* createScript(Kross::Action* Action);

        private:
            /// \internal d-pointer instance.
            PythonInterpreterPrivate* d;

            /// Initialize the python interpreter.
            inline void initialize();
            /// Finalize the python interpreter.
            inline void finalize();

            /**
             * \return the \a PythonModule instance.
             */
            PythonModule* mainModule() const;
    };

}

#endif
