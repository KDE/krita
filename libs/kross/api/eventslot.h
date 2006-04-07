/***************************************************************************
 * eventslot.h
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

#ifndef KROSS_API_EVENTSLOT_H
#define KROSS_API_EVENTSLOT_H

#include <qstring.h>
#include <qobject.h>
//Added by qt3to4:
#include <Q3CString>
#include <ksharedptr.h>

#include "event.h"

namespace Kross { namespace Api {

    /**
     * Each Qt signal and slot connection between a QObject
     * instance and a functionname is represented with
     * a EventSlot and handled by the \a EventManager.
     */
    class EventSlot : public Event<EventSlot>
    {
        public:

            /**
             * Shared pointer to implement reference-counting.
             */
            typedef KSharedPtr<EventSlot> Ptr;

            /**
             * Constructor.
             *
             * \param name The name of the EventSlot. The EventSlot
             *       will be accessible by that unique name via
             *       it's parent.
             * \param parent The parent object this EventSlot is
             *       child of.
             * \param receiver The receiver of the event.
             * \param slot The slot of the receiver which this
             *       EventSlot points to.
             */
            EventSlot(const QString& name, Object* parent, QObject* receiver, Q3CString slot);

            /**
             * Destructor.
             */
            virtual ~EventSlot();

            /// \see Kross::Api::Object::getClassName()
            virtual const QString getClassName() const;

            /// \see Kross::Api::Event::call()
            virtual Object::Ptr call(const QString& name, KSharedPtr<List> arguments);

/*
        private:
            EventManager* m_eventmanager;
            QGuardedPtr<QObject> m_sender;
            QCString m_signal;
            QCString m_slot;
            QString m_function;
            QValueList<EventSlot*> m_slots;
        protected:
            void call(const QVariant&);
        public slots:
            // Stupid signals and slots. To get the passed
            // arguments we need to have all cases of slots
            // avaiable for EventManager::connect() signals.
            void callback();
            void callback(short);
            void callback(int);
            void callback(int, int);
            void callback(int, int, int);
            void callback(int, int, int, int);
            void callback(int, int, int, int, int);
            void callback(int, int, int, int, bool);
            void callback(int, bool);
            void callback(int, int, bool);
            void callback(int, int, const QString&);
            void callback(uint);
            void callback(long);
            void callback(ulong);
            void callback(double);
            void callback(const char*);
            void callback(bool);
            void callback(const QString&);
            void callback(const QString&, int);
            void callback(const QString&, int, int);
            void callback(const QString&, uint);
            void callback(const QString&, bool);
            void callback(const QString&, bool, bool);
            void callback(const QString&, bool, int);
            void callback(const QString&, const QString&);
            void callback(const QString&, const QString&, const QString&);
            void callback(const QStringList&);
            void callback(const QVariant&);
            // The following both slots are more generic to
            // handle Kross::Api::Object instances.
            //void callback(Kross::Api::Object*);
            //void callback(Kross::Api::List*);
*/
        private:
            QObject* m_receiver;
            Q3CString m_slot;
    };

}}

#endif

