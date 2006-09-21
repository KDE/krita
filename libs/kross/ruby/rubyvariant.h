/***************************************************************************
 * rubyvariant.h
 * This file is part of the KDE project
 * copyright (C)2005 by Cyrille Berger (cberger@cberger.net)
 * copyright (C)2006 by Sebastian Sauer (mail@dipe.org)
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

#ifndef KROSS_RUBYVARIANT_H
#define KROSS_RUBYVARIANT_H

#include <ruby.h>
#include <st.h>
//#include <typeinfo>

#include "rubyconfig.h"
#include "../core/metatype.h"

#include <QString>
#include <QStringList>
#include <QVariant>
#include <QMetaType>

#include <QDate>
#include <QTime>
#include <QDateTime>

namespace Kross {

    /**
     * The RubyType helper classes used to cast between QVariant
     * and VALUE values.
     *
     * Following QVariant::Type's are implemented;
     *   \li QVariant::Invalid
     *   \li QVariant::Int
     *   \li QVariant::UInt
     *   \li QVariant::Double
     *   \li QVariant::Bool
     *   \li QVariant::LongLong
     *   \li QVariant::ULongLong
     *   \li QVariant::ByteArray
     *   \li QVariant::String
     *   \li QVariant::StringList
     *   \li QVariant::List
     *   \li QVariant::Map
     *
     * Following QVariant::Type's are unimplemented yet (do we need them anyways?);
     *   \li QVariant::BitArray
     *   \li QVariant::Date
     *   \li QVariant::Time
     *   \li QVariant::DateTime
     *   \li QVariant::Bitmap
     *   \li QVariant::Brush
     *   \li QVariant::Char
     *   \li QVariant::Color
     *   \li QVariant::Cursor
     *   \li QVariant::Font
     *   \li QVariant::Icon
     *   \li QVariant::Image
     *   \li QVariant::KeySequence
     *   \li QVariant::Line
     *   \li QVariant::LineF
     *   \li QVariant::Locale
     *   \li QVariant::Palette
     *   \li QVariant::Pen
     *   \li QVariant::Pixmap
     *   \li QVariant::Point
     *   \li QVariant::PointArray
     *   \li QVariant::PointF
     *   \li QVariant::Polygon
     *   \li QVariant::Rect
     *   \li QVariant::RectF
     *   \li QVariant::RegExp
     *   \li QVariant::Region
     *   \li QVariant::Size
     *   \li QVariant::SizeF
     *   \li QVariant::SizePolicy
     *   \li QVariant::TextFormat
     *   \li QVariant::TextLength
     *   \li QVariant::Url
     */
    template<typename VARIANTTYPE, typename RBTYPE = VALUE>
    struct RubyType
    {
        // template-specialisations need to implement following both static
        // functions to translate between QVariant and VALUE valueects.

        //inline static RBTYPE toVALUE(const VARIANTTYPE&) { return Py::None(); }
        //inline static QVARIANTTYPE toVariant(const VARIANTTYPE&) { return QVariant(); }
    };

    /// \internal
    template<>
    struct RubyType<QVariant>
    {
        static VALUE toVALUE(const QVariant& v);
        static QVariant toVariant(VALUE value);
    };

    /// \internal
    template<>
    struct RubyType<int>
    {
        inline static VALUE toVALUE(int i) {
            return INT2FIX(i);
        }
        inline static int toVariant(VALUE value) {
            if(TYPE(value) == T_FLOAT) // ruby is not able to convert double into int :-/
                return int(NUM2DBL(value));
            return FIX2INT(value);
        }
    };

    /// \internal
    template<>
    struct RubyType<uint>
    {
        inline static VALUE toVALUE(uint i) {
            return UINT2NUM(i);
        }
        inline static uint toVariant(VALUE value) {
            if(TYPE(value) == T_FLOAT) // ruby is not able to convert double into uint :-/
                return uint(NUM2DBL(value));
            return FIX2UINT(value);
        }
    };

    /// \internal
    template<>
    struct RubyType<double>
    {
        inline static VALUE toVALUE(double d) {
            return rb_float_new(d);
        }
        inline static double toVariant(VALUE value) {
            return NUM2DBL(value);
        }
    };

    /// \internal
    template<>
    struct RubyType<bool>
    {
        inline static VALUE toVALUE(bool b) {
            return b ? Qtrue : Qfalse;
        }
        inline static double toVariant(VALUE value) {
            return TYPE(value) == T_TRUE;
        }
    };

    /// \internal
    template<>
    struct RubyType<qlonglong>
    {
        inline static VALUE toVALUE(qlonglong l) {
            return /*INT2NUM*/ LONG2NUM((long)l);
        }
        inline static qlonglong toVariant(VALUE value) {
            return NUM2LONG(value);
        }
    };

    /// \internal
    template<>
    struct RubyType<qulonglong>
    {
        inline static VALUE toVALUE(qulonglong l) {
            return UINT2NUM((unsigned long)l);
        }
        inline static qulonglong toVariant(VALUE value) {
            return NUM2UINT(value);
        }
    };

    /// \internal
    template<>
    struct RubyType<QByteArray>
    {
        inline static VALUE toVALUE(const QByteArray& ba) {
            return rb_str_new2(ba.constData());
        }
        inline static QByteArray toVariant(VALUE value) {
            return STR2CSTR(value);
        }
    };

    /// \internal
    template<>
    struct RubyType<QString>
    {
        inline static VALUE toVALUE(const QString& s) {
            return s.isNull() ? rb_str_new2("") : rb_str_new2(s.toLatin1().data());
        }
        inline static QString toVariant(VALUE value) {
            return STR2CSTR(value);
        }
    };

    /// \internal
    template<>
    struct RubyType<QStringList>
    {
        inline static VALUE toVALUE(const QStringList& list) {
            VALUE l = rb_ary_new();
            foreach(QString s, list)
                rb_ary_push(l, RubyType<QString>::toVALUE(s));
            return l;
        }
        inline static QStringList toVariant(VALUE value) {
            QStringList l;
            for(int i = 0; i < RARRAY(value)->len; i++)
                l.append( RubyType<QString>::toVariant( rb_ary_entry(value, i) ) );
            return l;
        }
    };

    /// \internal
    template<>
    struct RubyType<QVariantList>
    {
        inline static VALUE toVALUE(const QVariantList& list) {
            VALUE l = rb_ary_new();
            foreach(QVariant v, list)
                rb_ary_push(l, RubyType<QVariant>::toVALUE(v));
            return l;
        }
        inline static QVariantList toVariant(VALUE value) {
            QVariantList l;
            for(int i = 0; i < RARRAY(value)->len; i++)
                l.append( RubyType<QVariant>::toVariant( rb_ary_entry(value, i) ) );
            return l;
        }
    };

    /// \internal
    template<>
    struct RubyType<QVariantMap>
    {
        inline static VALUE toVALUE(const QVariantMap& map) {
            VALUE h = rb_hash_new();
            QMap<QString, QVariant>::ConstIterator it(map.constBegin()), end(map.end());
            for(; it != end; ++it)
                rb_hash_aset(h, RubyType<QString>::toVALUE(it.key()), RubyType<QVariant>::toVALUE(it.value()) );
            return h;
        }
        inline static int convertHash(VALUE key, VALUE value, VALUE  vmap) {
            QVariantMap* map; 
            Data_Get_Struct(vmap, QVariantMap, map);
            if (key != Qundef)
                map->insert(STR2CSTR(key), RubyType<QVariant>::toVariant(value));
            return ST_CONTINUE;
        }
        inline static QVariantMap toVariant(VALUE value) {
            QVariantMap map;
            VALUE vmap = Data_Wrap_Struct(rb_cObject, 0,0, &map);
            rb_hash_foreach(value, (int (*)(...))convertHash, vmap);
            return map;
        }
    };

    /**
     * The RubyMetaTypeFactory helper class us used as factory within
     * \a RubyExtension to translate an argument into a \a MetaType
     * needed for QGenericArgument's data pointer.
     */
    class RubyMetaTypeFactory
    {
        public:
            static MetaType* create(const char* typeName, VALUE valueect);
    };

    /// \internal
    template<typename VARIANTTYPE>
    class RubyMetaTypeVariant : public MetaTypeVariant<VARIANTTYPE>
    {
        public:
            RubyMetaTypeVariant(VALUE value)
                : MetaTypeVariant<VARIANTTYPE>( RubyType<VARIANTTYPE>::toVariant(value) ) {}
            virtual ~RubyMetaTypeVariant() {}
    };

}

#endif
