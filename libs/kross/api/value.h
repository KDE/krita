/***************************************************************************
 * value.h
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

#ifndef KROSS_API_VALUE_H
#define KROSS_API_VALUE_H

#include <qstring.h>
#include <qvariant.h>
//#include <kdebug.h>

#include "object.h"

namespace Kross { namespace Api {

    /**
     * Template class to represent values.
     *
     * Classes like \a Variant or \a List are implementing this
     * class. That way we have a common base for all kind of
     * values.
     */
    template<class T, class V>
    class Value : public Object
    {
        public:

            /**
             * Constructor.
             *
             * \param value The initial value this
             *        Value has.
             * \param name The name this Value has.
             */
            Value(V value, const QString& name)
                : Object(name)
                , m_value(value) {}

            /**
             * Destructor.
             */
            virtual ~Value() {}

            //operator V&() const { return m_value; }

            /**
             * Return the value.
             *
             * \return The value this Value-class holds.
             */
            V& getValue() { return m_value; }
            //operator V& () { return m_value; }

#if 0
//do we need it anyway?
            /**
             * Set the value.
             * The value is call-by-value cause it may
             * contain some KShared and therefore
             * we need to keep a local copy to keep
             * it from disappearing.
             *
             * \param value The value to set.
             */
            void setValue(V& value) { m_value = value; }
#endif

        private:
            V m_value;
    };

}}

#endif

