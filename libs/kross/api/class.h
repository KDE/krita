/***************************************************************************
 * class.h
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

#ifndef KROSS_API_CLASS_H
#define KROSS_API_CLASS_H

#include <QString>
//#include <Q3ValueList>
//#include <QMap>
#include <QObject>

//#include "../main/krossconfig.h"
#include "object.h"
#include "event.h"
#include "list.h"
#include "exception.h"
#include "variant.h"

namespace Kross { namespace Api {

    /**
     * From \a Event inherited template-class to represent
     * class-structures. Classes implemetating this template
     * are able to dynamicly define \a Event methodfunctions
     * accessible from within scripts.
     */
    template<class T>
    class Class : public Event<T>
    {
        public:

            /**
             * Shared pointer to implement reference-counting.
             */
            typedef KSharedPtr<T> Ptr;

            /**
             * Constructor.
             *
             * \param name The name this class has.
             */
            Class(const QString& name)
                : Event<T>(name)
            {
            }

            /**
             * Destructor.
             */
            virtual ~Class()
            {
            }

            template<typename TYPE>
            static Object::Ptr toObject(TYPE t) { return Object::Ptr(t); }

            operator T* () { return (T*)this; }
            //operator Ptr () { return (T*)this; }

            /*
            virtual Object::Ptr call(const QString& name, List::Ptr arguments)
            {
                krossdebug( QString("Class::call(%1)").arg(name) );
                return Event<T>::call(name, arguments);
            }
            */

    };

}}

#endif

