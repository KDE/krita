/***************************************************************************
 * variant.cpp
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

#include "variant.h"
#include "list.h"

#include <klocale.h>

using namespace Kross::Api;

Variant::Variant(const QVariant& value)
    : Value<Variant, QVariant>(value)
{
}

Variant::~Variant()
{
}

const QString Variant::getClassName() const
{
    return "Kross::Api::Variant";
}

const QString Variant::toString()
{
    return getValue().toString();
}

/*
const QString Variant::getVariantType(Object::Ptr object)
{
    switch( toVariant(object).type() ) {

        case QVariant::CString:
        case QVariant::String:
            return "Kross::Api::Variant::String";
        case QVariant::Map:
            return "Kross::Api::Variant::Dict";
        case QVariant::StringList:
        case QVariant::List:
            return "Kross::Api::Variant::List";
        case QVariant::Double:
            //return "Kross::Api::Variant::Double";
        case QVariant::UInt: 
            //return "Kross::Api::Variant::UInt"; // python isn't able to differ between int and uint :-(
        case QVariant::LongLong:
        case QVariant::ULongLong:
        case QVariant::Int:
            return "Kross::Api::Variant::Integer";
        case QVariant::Bool:
            return "Kross::Api::Variant::Bool";
        default: //Date, Time, DateTime, ByteArray, BitArray, Rect, Size, Color, Invalid, etc.
            return "Kross::Api::Variant";
    }
}
*/

const QVariant& Variant::toVariant(Object* object)
{
    return Object::fromObject<Variant>( object )->getValue();
}

const QString Variant::toString(Object* object)
{
    const QVariant& variant = toVariant(object);
    if(! variant.canConvert(QVariant::String))
        throw Exception::Ptr( new Exception(i18n("Kross::Api::Variant::String expected, but got %1.",variant.typeName()).toLatin1().data()) );
    return variant.toString();
}

int Variant::toInt(Object* object)
{
    const QVariant& variant = toVariant(object);
    if(! variant.canConvert(QVariant::Int))
        throw Exception::Ptr( new Exception(i18n("Kross::Api::Variant::Int expected, but got %1.",variant.typeName()).toLatin1().data()) );
    return variant.toInt();
}

uint Variant::toUInt(Object* object)
{
    const QVariant& variant = toVariant(object);
    if(! variant.canConvert(QVariant::UInt))
        throw Exception::Ptr( new Exception(i18n("Kross::Api::Variant::UInt expected, but got %1.",variant.typeName()).toLatin1().data()) );
    return variant.toUInt();
}

double Variant::toDouble(Object* object)
{
    const QVariant& variant = toVariant(object);
    if(! variant.canConvert(QVariant::Double))
        throw Exception::Ptr( new Exception(i18n("Kross::Api::Variant::Double expected, but got %1.",variant.typeName()).toLatin1().data()) );
    return variant.toDouble();
}

qlonglong Variant::toLLONG(Object* object)
{
    const QVariant& variant = toVariant(object);
    if(! variant.canConvert(QVariant::LongLong))
        throw Exception::Ptr( new Exception(i18n("Kross::Api::Variant::LLONG expected, but got %1.",variant.typeName()).toLatin1().data()) );
    return variant.toLongLong();
}

qulonglong Variant::toULLONG(Object* object)
{
    const QVariant& variant = toVariant(object);
    if(! variant.canConvert(QVariant::ULongLong))
        throw Exception::Ptr( new Exception(i18n("Kross::Api::Variant::ULLONG expected, but got %1.",variant.typeName()).toLatin1().data()) );
    return variant.toULongLong();
}

bool Variant::toBool(Object* object)
{
    const QVariant& variant = toVariant(object);
    if(! variant.canConvert(QVariant::Bool))
        throw Exception::Ptr( new Exception(i18n("Kross::Api::Variant::Bool expected, but got %1.",variant.typeName()).toLatin1().data()) );
    return variant.toBool();
}

QStringList Variant::toStringList(Object* object)
{
    List* list = dynamic_cast< List* >( object );
    if(list) {
        QStringList l;
        QList<Object::Ptr> valuelist = list->getValue();
        QList<Object::Ptr>::Iterator it(valuelist.begin()), end(valuelist.end());
        for(; it != end; ++it)
            l.append( toString( (*it).data() ) );
        return l;
    }
    const QVariant& variant = toVariant(object);
    if(! variant.canConvert(QVariant::StringList))
        throw Exception::Ptr( new Exception(i18n("Kross::Api::Variant::StringList expected, but got '%1'.",variant.typeName()).toLatin1().data()) );
    return variant.toStringList();
}

QList<QVariant> Variant::toList(Object* object)
{
    List* list = dynamic_cast< List* >( object );
    if(list) {
        QList<QVariant> l;
        QList<Object::Ptr> valuelist = list->getValue();
        QList<Object::Ptr>::Iterator it(valuelist.begin()), end(valuelist.end());
        for(; it != end; ++it)
            l.append( toVariant( (*it).data() ) );
        return l;
    }
    const QVariant& variant = toVariant(object);
    if(! variant.canConvert(QVariant::List))
        throw Exception::Ptr( new Exception(i18n("Kross::Api::Variant::List expected, but got '%1'.",variant.typeName()).toLatin1().data()) );
    return variant.toList();
}
