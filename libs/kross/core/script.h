/***************************************************************************
 * script.h
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

#ifndef KROSS_SCRIPT_H
#define KROSS_SCRIPT_H

#include <QString>
#include <QStringList>
#include <QVariant>
#include <QObject>
#include <koffice_export.h>

#include "errorinterface.h"

namespace Kross {

    // Forward declarations.
    class Interpreter;
    class Action;

    /**
     * Base class for interpreter dependend functionality
     * each script provides.
     *
     * Each \a Action holds a pointer to a class
     * that implements the \a Script functionality for the
     * defined \a Interpreter .
     */
    class KROSS_EXPORT Script : public QObject, public ErrorInterface
    {
        public:

            /**
             * Constructor.
             *
             * \param interpreter The \a Interpreter instance
             *       that uses this \a Script instance.
             * \param Action The \a Action instance
             *       this script is associated with.
             */
            Script(Interpreter* const interpreter, Action* const action);

            /**
             * Destructor.
             */
            virtual ~Script();

            /**
             * Execute the script.
             *
             * \param args The optional arguments passed to the script
             * on excution.
             */
            virtual void execute(const QVariant& args = QVariant()) = 0;

            /**
             * \return the list of functionnames.
             */
            virtual QStringList functionNames() = 0;

            /**
             * Call a function in the script.
             *
             * \param name The name of the function which should be called.
             * \param args The optional list of arguments.
             */
            virtual QVariant callFunction(const QString& name, const QVariantList& args = QVariantList()) = 0;

        protected:
            /// The \a Interpreter used to create this Script instance.
            Interpreter* const m_interpreter;
            /// The \a Action associated with this Script.
            Action* const m_action;
    };

}

#endif

