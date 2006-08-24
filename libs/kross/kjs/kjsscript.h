/***************************************************************************
 * kjsscript.h
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

#ifndef KROSS_KJSSCRIPT_H
#define KROSS_KJSSCRIPT_H

#include "../core/krossconfig.h"
#include "../core/script.h"

namespace Kross {

    // Forward declarations.
    class KjsScriptPrivate;

    /**
     * Handle python scripts. This class implements
     * \a Kross::Script for python.
     */
    class KjsScript : public Kross::Script
    {
        public:

            /**
             * Constructor.
             *
             * \param interpreter The \a Kross::Kjs::KjsInterpreter used
             *       to create this KjsScript instance.
             * \param Action The with this KjsScript associated
             *       \a Kross::Action instance that spends us
             *       e.g. the python scripting code.
             */
            explicit KjsScript(Kross::Interpreter* interpreter, Kross::Action* action);

            /**
             * Destructor.
             */
            virtual ~KjsScript();

            /**
             * Execute the script.
             *
             * \param args The optional arguments passed to the script
             * on excution.
             */
            virtual void execute();

            /**
             * \return the list of functionnames.
             */
            virtual QStringList functionNames();

            /**
             * Call a function in the script.
             *
             * \param name The name of the function which should be called.
             * \param args The optional list of arguments.
             */
            virtual QVariant callFunction(const QString& name, const QVariantList& args = QVariantList());

        private:
            /// Private d-pointer class.
            KjsScriptPrivate* d;

            /// Initialize the script.
            bool initialize();
            /// Finalize and cleanup the script.
            void finalize();
    };

}

#endif

