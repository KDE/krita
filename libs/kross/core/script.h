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
#include <koffice_export.h>

#include "errorinterface.h"

namespace Kross {

    // Forward declarations.
    class Interpreter;
    class Action;

    /**
     * Base class for interpreter dependent functionality
     * each script provides.
     *
     * Each \a Action holds a pointer to a class
     * that implements the \a Script functionality for the
     * defined \a Interpreter .
     */
    class KROSSCORE_EXPORT Script : public ErrorInterface
    {
        public:

            /**
             * Constructor.
             *
             * \param interpreter The \a Interpreter instance that
             *        was used to created this \a Script instance.
             * \param Action The \a Action instance this script is
             *        associated with.
             */
            Script(Interpreter* interpreter, Action* action);

            /**
             * Destructor.
             */
            virtual ~Script();

            /**
             * \return the \a Interpreter instance that was used to created
             * this \a Script .
             */
            Interpreter* interpreter() const;

            /**
             * \return the \a Action instance associated with this \a Script .
             */
            Action* action() const;

            /**
             * Execute the script.
             */
            virtual void execute() = 0;

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

