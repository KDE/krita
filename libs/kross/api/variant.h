/***************************************************************************
 * variant.h
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

#ifndef KROSS_API_VARIANT_H
#define KROSS_API_VARIANT_H

#include <QString>
#include <QVariant>
#include <koffice_export.h>

#include "object.h"
#include "value.h"
#include "exception.h"

namespace Kross { namespace Api {

    class List;

    /**
     * Variant value to wrap a QVariant into a \a Kross::Api::Value
     * to enable primitive types like strings or numerics.
     */
    class KROSS_EXPORT Variant : public Value<Variant, QVariant>
    {
            friend class Value<Variant, QVariant>;
        public:

            /**
             * Constructor.
             *
             * \param value The initial QVariant-value
             *        this Variant-Object has.
             */
            Variant(const QVariant& value);

            inline operator bool () { return getValue().toBool(); }
            inline operator int () { return getValue().toInt(); }
            inline operator uint () { return getValue().toUInt(); }
            inline operator double () { return getValue().toDouble(); }
            inline operator const char* () { return getValue().toString().toLatin1().data(); }

            inline operator QString () { return getValue().toString(); }
            inline operator const QString () { return getValue().toString(); }
            //inline operator const QString& () { return getValue().toString(); }

            //inline operator Q3CString () { return getValue().toCString(); }
            //inline operator const Q3CString () { return getValue().toCString(); }
            //inline operator const Q3CString& () { return getValue().asCString(); }

            inline operator QVariant () { return getValue(); }
            inline operator const QVariant& () { return getValue(); }

            /**
             * Operator to return a QStringList.
             *
             * We can not just use getValue().toStringList() here cause maybe
             * this Kross::Api::Variant is a Kross::Api::List which could be
             * internaly used for list of strings as well. So, we use the
             * toStringList() function which will take care of translating a
             * Kross::Api::List to a QStringList if possible or to throw an
             * exception if the Kross::Api::List isn't a QStringList.
             */
            inline operator QStringList () {
                return toStringList(this);
            }
            inline operator QList<QVariant> () {
                return toList(this);
            }

            /**
             * Destructor.
             */
            virtual ~Variant();

            /// \see Kross::Api::Object::getClassName()
            virtual const QString getClassName() const;

            /**
             * \return a string representation of the variant.
             *
             * \see Kross::Api::Object::toString()
             */
            virtual const QString toString();

            /**
             * Try to convert the given \a Object into
             * a QVariant.
             *
             * \throw TypeException If the convert failed.
             * \param object The object to convert.
             * \return The to a QVariant converted object.
             */
            static const QVariant& toVariant(Object* object);

            /**
             * Try to convert the given \a Object into
             * a QString.
             *
             * \throw TypeException If the convert failed.
             * \param object The object to convert.
             * \return The to a QString converted object.
             */
            static const QString toString(Object* object);

            /**
             * Try to convert the given \a Object into
             * a int.
             *
             * \throw TypeException If the convert failed.
             * \param object The object to convert.
             * \return The to a int converted object.
             */
            static int toInt(Object* object);

            /**
             * Try to convert the given \a Object into
             * a uint.
             *
             * \throw TypeException If the convert failed.
             * \param object The object to convert.
             * \return The to a uint converted object.
             */
            static uint toUInt(Object* object);

            /**
             * Try to convert the given \a Object into
             * a uint.
             *
             * \throw TypeException If the convert failed.
             * \param object The object to convert.
             * \return The to a uint converted object.
             */
            static double toDouble(Object* object);

            /**
             * Try to convert the given \a Object into
             * a Q_LLONG.
             *
             * \throw TypeException If the convert failed.
             * \param object The object to convert.
             * \return The to a Q_LLONG converted object.
             */
            static qlonglong toLLONG(Object* object);

            /**
             * Try to convert the given \a Object into
             * a Q_ULLONG.
             *
             * \throw TypeException If the convert failed.
             * \param object The object to convert.
             * \return The to a Q_ULLONG converted object.
             */
            static qulonglong toULLONG(Object* object);

            /**
             * Try to convert the given \a Object into
             * a boolean value.
             *
             * \throw TypeException If the convert failed.
             * \param object The object to convert.
             * \return The to a bool converted object.
             */
            static bool toBool(Object* object);

            /**
             * Try to convert the given \a Object into
             * a QStringList.
             *
             * \throw TypeException If the convert failed.
             * \param object The object to convert.
             * \return The to a QList converted object.
             */
            static QStringList toStringList(Object* object);

            /**
             * Try to convert the given \a Object into
             * a QList of QVariant's.
             *
             * \throw TypeException If the convert failed.
             * \param object The object to convert.
             * \return The to a QList converted object.
             */
            static QList<QVariant> toList(Object* object);

    };

}}

#endif

