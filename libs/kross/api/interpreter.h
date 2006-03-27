/***************************************************************************
 * interpreter.h
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

#ifndef KROSS_API_INTERPRETER_H
#define KROSS_API_INTERPRETER_H

#include <qstring.h>
#include <qmap.h>
#include <kdebug.h>

#include "object.h"

namespace Kross { namespace Api {

    // Forward declaration.
    class Manager;
    class ScriptContainer;
    class Script;
    class Interpreter;

    /**
     * While the \a Interpreter is the implemented interpreter this class
     * is used to provide some abstract informations about each interpreter
     * we are able to use within the \a Manager singelton.
     */
    class InterpreterInfo
    {
        public:

            /**
             * Each interpreter is able to define options we could
             * use to manipulate the interpreter behaviour.
             */
            class Option
            {
                public:

                    /**
                    * Map of options.
                    */
                    typedef QMap<QString, Option*> Map;

                    /**
                     * Constructor.
                     * 
                     * \param name The name the option has. This is the
                     *        displayed title and isn't used internaly.
                     * \param comment A comment that describes the option.
                     * \param value The QVariant value this option has.
                     */
                    Option(const QString& name, const QString& comment, const QVariant& value)
                        : name(name), comment(comment), value(value) {}

                    /// The short name of the option.
                    QString name;

                    /// A description of the option.
                    QString comment;

                    /// The value the option has.
                    QVariant value;
            };

            /**
             * Constructor.
             */
            InterpreterInfo(const QString& interpretername, const QString& library, const QString& wildcard, QStringList mimetypes, Option::Map options);

            /**
             * Destructor.
             */
            ~InterpreterInfo();

            /**
             * \return the name of the interpreter. For example "python" or "kjs".
             */
            const QString& getInterpretername();

            /**
             * \return the file-wildcard used to determinate by this interpreter 
             * used scriptingfiles. Those filter will be used e.g. with
             * KGlobal::dirs()->findAllResources() as filtermask. For example
             * python just defines it as "*py".
             */
            const QString& getWildcard();

            /**
             * List of mimetypes this interpreter supports.
             *
             * \return QStringList with mimetypes like
             *         "application/x-javascript".
             */
            const QStringList getMimeTypes();

            /**
             * \return true if an \a Option with that \p key exists else false.
             */
            bool hasOption(const QString& key);

            /**
             * \return the option defined with \p name .
             */
            Option* getOption(const QString name);

            /**
             * \return the value of the option defined with \p name . If there 
             * doesn't exists an option with such a name, the \p defaultvalue 
             * is returned.
             */
            const QVariant& getOptionValue(const QString name, QVariant defaultvalue = QVariant());

            /**
             * \return a map of options.
             */
            Option::Map getOptions();

            /**
             * \return the \a Interpreter instance this \a InterpreterInfo
             * is the describer for.
             */
            Interpreter* getInterpreter();

        private:
            /// The name the interpreter has. Could be something like "python" or "kjs".
            QString m_interpretername;
            /// The name of the library to load for the interpreter.
            QString m_library;
            /// The file wildcard used to determinate extensions.
            QString m_wildcard;
            /// List of mimetypes this interpreter supports.
            QStringList m_mimetypes;
            /// A \a Option::Map with options.
            Option::Map m_options;
            /// The \a Interpreter instance.
            Interpreter* m_interpreter;
    };

    /**
     * Base class for interpreters.
     *
     * Each scripting backend needs to inheritate it's own
     * interpreter from this class and implementate there
     * backend related stuff.
     * The Interpreter will be managed by the \a Kross::Manager
     * class.
     */
    class Interpreter
    {
        public:

            /**
             * Constructor.
             *
             * \param info is the \a InterpreterInfo instance
             *        that describes this interpreter.
             */
            Interpreter(InterpreterInfo* info);

            /**
             * Destructor.
             */
            virtual ~Interpreter();

            /**
             * Create and return a new interpreter dependend
             * \a Script instance.
             *
             * \param scriptcontainer The \a ScriptContainer
             *        to use for the \a Script instance.
             * \return The from \a Script inherited instance.
             */
            virtual Script* createScript(ScriptContainer* scriptcontainer) = 0;

        protected:
            /// The \a InterpreterInfo instance this interpreter belongs to.
            InterpreterInfo* m_interpreterinfo;
    };

}}

#endif

