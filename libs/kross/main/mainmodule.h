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
#include "../api/event.h"
#include "../api/eventsignal.h"
#include "../api/eventslot.h"
#include "../api/qtobject.h"
#include "../api/eventaction.h"

#include <qstring.h>
#include <qvariant.h>
#include <qobject.h>
//Added by qt3to4:
#include <Q3CString>

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
    class MainModule : public Module
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

            /// \see Kross::Api::Object::getClassName()
            virtual const QString getClassName() const;

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
             * Returns if the defined child is avaible.
             *
             * \return true if child exists else false.
             */
            bool hasChild(const QString& name) const;

            /**
             * Add a Qt signal to the \a Module by creating
             * an \a EventSignal for it.
             *
             * \param name the name the \a EventSignal is
             *       reachable as
             * \param sender the QObject instance which
             *       is the sender of the \p signal
             * \param signal the Qt signal macro the \p sender
             *       emits to call the \a EventSignal
             * \return the newly added \a EventSignal instance
             *       which is now a child of this \a MainModule
             */
//            EventSignal::Ptr addSignal(const QString& name, QObject* sender, Q3CString signal);

            /**
             * Add a Qt slot to the \a Module by creating
             * an \a EventSlot for it.
             *
             * \param name the name the \a EventSlot is
             *       reachable as
             * \param receiver the QObject instance which
             *       is the receiver of the \p signal
             * \param slot the Qt slot macro of the \p receiver
             *       to invoke if the \a EventSlot got called.
             * \return the newly added \a EventSlot instance
             *       which is now a child of this \a MainModule
             */
//            EventSlot::Ptr addSlot(const QString& name, QObject* receiver, Q3CString slot);

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

            /**
             * Add a \a KAction to the eventcollection. The
             * KAction will be wrapped by a \a EventAction
             * and will be added to this collection.
             *
             * \param name name to identify the \a action by
             * \param action the KAction instance that should
             *       be added to this \a MainModule
             * \return the newly added \a EventAction instance
             *       which is now a child of this \a MainModule
             *
             * \todo check \a name dox.
             */
            EventAction::Ptr addKAction(KAction* action, const QString& name = QString::null);

            //typedef QValueList<Callable::Ptr> EventList;
            //EventList getEvents();
            //const QString& serializeToXML();
            //void unserializeFromXML(const QString& xml);

        private:
            /// Private d-pointer class.
            MainModulePrivate* d;
    };

}}

#endif

