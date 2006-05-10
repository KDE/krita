/***************************************************************************
 * eventsignal.h
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

#ifndef KROSS_API_EVENTSIGNAL_H
#define KROSS_API_EVENTSIGNAL_H

//#include <QString>
//#include <qvaluelist.h>
//#include <QMap>
//#include <QVariant>
//#include <qsignalmapper.h>
//#include <qguardedptr.h>
#include <qobject.h>
//Added by qt3to4:
#include <Q3CString>
#include <ksharedptr.h>

#include "event.h"

namespace Kross { namespace Api {

    /**
     * Each Qt signal and slot connection between a QObject
     * instance and a functionname is represented with
     * a EventSignal and handled by \a EventManager.
     */
    class EventSignal : public Event<EventSignal>
    {
        public:

            /**
             * Shared pointer to implement reference-counting.
             */
            typedef KSharedPtr<EventSignal> Ptr;

            /**
             * Constructor.
             */
            EventSignal(const QString& name, Object* parent, QObject* sender, Q3CString signal);

            /**
             * Destructor.
             */
            virtual ~EventSignal();

            virtual const QString getClassName() const;

            virtual Object::Ptr call(const QString& name, KSharedPtr<List> arguments);

/*
        signals:
            void callback();
            void callback(const QString&);
            void callback(int);
            void callback(bool);
*/
        private:
            QObject* m_sender;
            Q3CString m_signal;
    };

}}

#endif

