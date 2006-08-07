/***************************************************************************
 * mainmodule.h
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

#ifndef KROSS_API_MAINMODULE_H
#define KROSS_API_MAINMODULE_H

#include "../api/object.h"
#include "../api/variant.h"
#include "../api/module.h"
#include "../api/qtobject.h"

#include <QString>
#include <QVariant>
#include <QObject>
#include <koffice_export.h>
#include <ksharedptr.h>
#include <kaction.h>

namespace Kross { namespace Api {

    // Forward declarations.
    class MainModulePrivate;

    /**
     * This class implements \a Module for the global
     * \a Manager singleton and local \a ScriptContainer
     * instances.
     *
     * The MainModule class provides base functionality
     * for a root node in a tree of \a Kross::Api::Object
     * instances.
     */
    class KROSS_EXPORT MainModule : public Module
    {
        public:

            /// Shared pointer to implement reference-counting.
            typedef KSharedPtr<MainModule> Ptr;

            /**
             * Constructor.
             *
             * \param name the name of the \a Module . While the
             *       global manager module has the name "Kross"
             *       the \a ScriptContainer instances are accessible
             *       by there \a ScriptContainer::getName() name.
             */
            explicit MainModule(const QString& name);

            /**
             * Destructor.
             */
            virtual ~MainModule();

            /**
             * \return true if the script throwed an exception
             *        else false.
             */
            bool hadException();

            /**
             * \return the \a Exception this module throwed.
             */
            Exception* getException();

            /**
             * Set the \a Exception this module throwed.
             *
             * \param exception The \a Exception this module throws or
             *       NULL if you like to clear exception and to let
             *       \a hadException() return false.
             */
            void setException(Exception* exception);

            /**
             * Add a \a QObject to the eventcollection. All
             * signals and slots the QObject has will be
             * added to a new \a EventCollection instance
             * which is child of this \a EventCollection
             * instance.
             *
             * \param object the QObject instance that should
             *       be added to this \a MainModule
             * \param name the name under which this QObject instance
             *       should be registered as
             * \return the newly added \a QtObject instance
             *       which is now a child of this \a MainModule
             */
            QtObject::Ptr addQObject(QObject* object, const QString& name = QString::null);

        private:
            /// Private d-pointer class.
            MainModulePrivate* d;
    };

}}

#endif

