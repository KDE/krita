/***************************************************************************
 * eventscript.h
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

#ifndef KROSS_API_EVENTSCRIPT_H
#define KROSS_API_EVENTSCRIPT_H

#include <qstring.h>
#include <qobject.h>

#include "event.h"

namespace Kross { namespace Api {

    /**
     * \todo implement EventScript ?!
     */
    class EventScript : public Event<EventScript>
    {

        public:

            /**
             * Shared pointer to implement reference-counting.
             */
            typedef KSharedPtr<EventScript> Ptr;

            /**
             * Constructor.
             */
            EventScript(const QString& name, Object::Ptr parent);

            /**
             * Destructor.
             */
            virtual ~EventScript();

            virtual const QString getClassName() const;

            virtual Object::Ptr call(const QString& name, KSharedPtr<List> arguments);

        private:
            //ScriptContainer* m_scriptcontainer;
            //Callable* m_callable;
    };

}}

#endif

