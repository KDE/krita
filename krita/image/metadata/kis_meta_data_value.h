/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#ifndef _KIS_META_DATA_VALUE_H_
#define _KIS_META_DATA_VALUE_H_

#include <QList>
#include <QMap>

#include <krita_export.h>

class QVariant;

namespace KisMetaData {
    /**
     * Value is build on top of QVariant to extend it to support the various type,
     * and extension throught properties qualifier.
     */
    class KRITAIMAGE_EXPORT Value {
        struct Private;
        public:
            /// Define the possible value type
            enum ValueType {
                Invalid,
                Variant,
                OrderedArray,
                UnorderedArray,
                AlternativeArray,
                LangArray,
                Structure
            };
        public:
            Value();
            Value(const QVariant& value);
            /**
             * @param type is one of OrderedArray, UnorderedArray, AlternativeArray
             * or LangArray
             */
            Value(const QList<Value>& array, ValueType type = OrderedArray);
            Value(const QMap<QString, Value>& structure);
            Value(const Value& v);
            Value& operator=(const Value& v);
            ~Value();
        public:
            void setPropertyQualifier(const Value&);
            bool hasPropertyQualifier() const;
        public:
            ValueType type() const;
            QVariant asVariant() const;
            void setVariant(const QVariant& variant);
        private:
            Private* const d;
    };
}


#endif
