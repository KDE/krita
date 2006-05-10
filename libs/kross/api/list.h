/***************************************************************************
 * list.h
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

#ifndef KROSS_API_LIST_H
#define KROSS_API_LIST_H

#include <QString>
#include <q3valuelist.h>
#include <q3intdict.h>
//Added by qt3to4:
#include <Q3PtrList>
#include <koffice_export.h>
#include "object.h"
#include "value.h"

namespace Kross { namespace Api {

    /**
     * The List class implementates \a Value to handle
     * lists and collections.
     */
    class KROSS_EXPORT List : public Value< List, Q3ValueList<Object::Ptr> >
    {
            friend class Value< List, Q3ValueList<Object::Ptr> >;
        public:

            /**
             * Shared pointer to implement reference-counting.
             */
            typedef KSharedPtr<List> Ptr;

            operator QStringList () {
                //QValueList<Object::Ptr> getValue()
                krossdebug("999999999999 ...........................");
                return QStringList();
            }

            /**
             * Constructor.
             *
             * \param value The list of \a Object instances this
             *        list has initialy.
             * \param name A name this list has.
             */
            List(Q3ValueList<Object::Ptr> value = Q3ValueList<Object::Ptr>(), const QString& name = "list");

            /**
             * Destructor.
             */
            virtual ~List();

            /**
             * See \see Kross::Api::Object::getClassName()
             */
            virtual const QString getClassName() const;

            /**
             * \return a string representation of the whole list.
             *
             * \see Kross::Api::Object::toString()
             */
            virtual const QString toString();

            /**
             * Return the \a Object with defined index from the
             * QValueList this list holds.
             *
             * \throw TypeException If index is out of bounds.
             * \param idx The QValueList-index.
             * \param defaultobject The default \a Object which should
             *        be used if there exists no item with such an
             *        index. This \a Object instance will be returned
             *        if not NULL and if the index is out of bounds. If
             *        its NULL a \a TypeException will be thrown.
             * \return The \a Object instance.
             */
            Object* item(int idx, Object* defaultobject = 0);

            /**
             * Return the number of items in the QValueList this
             * list holds.
             *
             * \return The number of items.
             */
            uint count();

            /**
             * Append an \a Kross::Api::Object to the list.
             *
             * \param object The \a Kross::Api::Object instance to
             *       append to this list.
             */
            void append(Object::Ptr object);

            template<typename TYPE>
            static Object::Ptr toObject(TYPE t) { return t; }
    };

    /**
     * This template class extends the \a List class with
     * generic functionality to deal with lists.
     */
    template< class OBJECT >
    class ListT : public List
    {
        public:
            ListT() : List() {}
            ListT(Q3ValueList<OBJECT> values) : List(values) {}

            template< typename TYPE >
            ListT(Q3IntDict<TYPE> values) : List()
            {
                Q3IntDictIterator<TYPE> it( values );
                TYPE *t;
                while( (t = it.current()) != 0 ) {
                    this->append( new OBJECT(t) );
                    ++it;
                }
            }

            template< typename TYPE >
            ListT(const Q3PtrList<TYPE> values) : List()
            {
                Q3PtrListIterator<TYPE> it(values);
                TYPE *t;
                while( (t = it.current()) != 0 ) {
                    this->append( new OBJECT(t) );
                    ++it;
                }
            }

            virtual ~ListT() {}

            template<typename TYPE>
            static Object::Ptr toObject(TYPE t)
            {
                return new ListT(t);
            }
    };

}}

#endif

