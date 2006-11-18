/***************************************************************************
 * interpreter.h
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

#ifndef KROSS_INTERPRETER_H
#define KROSS_INTERPRETER_H

#include <QString>
#include <QStringList>
#include <QVariant>
#include <QMap>

#include "errorinterface.h"
#include <koffice_export.h>

namespace Kross {

    // Forward declaration.
    class Manager;
    class Action;
    class Script;
    class Interpreter;

    /**
     * The InterpreterInfo class provides abstract information about
     * a \a Interpreter before the interpreter-backend itself is
     * loaded.
     */
    class KROSSCORE_EXPORT InterpreterInfo
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
                    typedef QMap<QString, Option* > Map;

                    /**
                     * Constructor.
                     *
                     * \param comment A comment that describes the option.
                     * \param value The QVariant value this option has.
                     */
                    Option(const QString& comment, const QVariant& value)
                        : comment(comment), value(value) {}

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
            const QString interpreterName() const;

            /**
             * \return the file-wildcard used to determinate by this interpreter
             * used scriptingfiles. Those filter will be used e.g. with
             * KGlobal::dirs()->findAllResources() as filtermask. For example
             * python just defines it as "*py".
             */
            const QString wildcard() const;

            /**
             * List of mimetypes this interpreter supports.
             *
             * \return QStringList with mimetypes like
             *         "application/x-javascript".
             */
            const QStringList mimeTypes() const;

            /**
             * \return true if an \a Option with that \p key exists else false.
             */
            bool hasOption(const QString& key) const;

            /**
             * \return the option defined with \p name .
             */
            Option* option(const QString name) const;

            /**
             * \return the value of the option defined with \p name . If there
             * doesn't exists an option with such a name, the \p defaultvalue
             * is returned.
             */
            const QVariant optionValue(const QString name, QVariant defaultvalue = QVariant()) const;

            /**
             * \return a map of options.
             */
            Option::Map options();

            /**
             * \return the \a Interpreter instance this \a InterpreterInfo
             * is the describer for.
             */
            Interpreter* interpreter();

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };

    /**
     * Base class for interpreter implementations.
     *
     * Each scripting backend needs to inheritate it's own
     * interpreter and implement it.
     *
     * The Interpreter will be managed by the \a Kross::Manager
     * class.
     */
    class KROSSCORE_EXPORT Interpreter : public ErrorInterface
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
             * \return the \a InterpreterInfo that represents
             * this \a Interpreter .
             */
            InterpreterInfo* interpreterInfo() const;

            /**
             * Create and return a new interpreter dependent
             * \a Script instance.
             *
             * \param Action The \a Action
             *        to use for the \a Script instance.
             * \return The from \a Script inherited instance.
             */
            virtual Script* createScript(Action* Action) = 0;

            /// \internal hook to keep easier binary compatibility.
            virtual void virtual_hook(int id, void* data);

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };

}

#endif

