/***************************************************************************
 * script.h
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

#ifndef KROSS_API_SCRIPT_H
#define KROSS_API_SCRIPT_H

#include <qstring.h>
#include <qstringlist.h>

#include "class.h"

namespace Kross { namespace Api {

    // Forward declarations.
    class Object;
    class Interpreter;
    class ScriptContainer;
    class List;
    class Exception;

    /**
     * Base class for interpreter dependend functionality
     * each script provides.
     *
     * Each \a ScriptContainer holds a pointer to a class
     * that implements the \a Script functionality for the
     * defined \a Interpreter .
     */
    class Script
    {
        public:

            /**
             * Constructor.
             *
             * \param interpreter The \a Interpreter instance
             *       that uses this \a Script instance.
             * \param scriptcontainer The \a ScriptContainer instance
             *       this script is associated with.
             */
            Script(Interpreter* const interpreter, ScriptContainer* const scriptcontainer);

            /**
             * Destructor.
             */
            virtual ~Script();

            /**
             * \return true if the script throwed an exception
             *        else false.
             */
            bool hadException();

            /**
             * \return the \a Exception the script throwed.
             */
            Exception::Ptr getException();

            /**
             * Set a new exception this script throwed.
             *
             * \param e The \a Exception .
             */
            void setException(Exception::Ptr e);

            /**
             * Clear previous exceptions. If called \a hadException()
             * will return false again.
             */
            void clearException();

            /**
             * Execute the script.
             *
             * \throws Exception on error.
             * \return The execution result. Could be NULL too.
             */
            virtual Kross::Api::Object::Ptr execute() = 0;

            /**
             * \return a list of callable functionnames this
             * script spends.
             */
            virtual const QStringList& getFunctionNames() = 0;

            /**
             * Call a function.
             *
             * \throws Exception on error.
             * \param name The name of the function to execute.
             * \param args Optional arguments passed to the function.
             * \return The result of the called function. Could be NULL.
             */
            virtual Kross::Api::Object::Ptr callFunction(const QString& name, Kross::Api::List::Ptr args) = 0;

            /**
             * \return a list of classnames.
             */
            virtual const QStringList& getClassNames() = 0;

            /**
             * Create and return a new class instance.
             *
             * \throws Exception on error.
             * \param name The name of the class to create a instance of.
             * \return The new classinstance.
             */
            virtual Kross::Api::Object::Ptr classInstance(const QString& name) = 0;

        protected:
            /// The \a Interpreter used to create this Script instance.
            Interpreter* const m_interpreter;
            /// The \a ScriptContainer associated with this Script.
            ScriptContainer* const m_scriptcontainer;

        private:
            /// The \a Exception this script throwed.
            Exception::Ptr m_exception;
    };

}}

#endif

