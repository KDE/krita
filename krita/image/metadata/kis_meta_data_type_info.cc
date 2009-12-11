/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include "kis_meta_data_type_info.h"

#include <QVariant>

#include "kis_meta_data_parser_p.h"
#include "kis_meta_data_type_info_p.h"
#include "kis_meta_data_value.h"
#include "kis_meta_data_schema.h"

using namespace KisMetaData;

QHash< const TypeInfo*, const TypeInfo*> TypeInfo::Private::orderedArrays;
QHash< const TypeInfo*, const TypeInfo*> TypeInfo::Private::unorderedArrays;
QHash< const TypeInfo*, const TypeInfo*> TypeInfo::Private::alternativeArrays;

const TypeInfo* TypeInfo::Private::Boolean = new TypeInfo(TypeInfo::BooleanType);
const TypeInfo* TypeInfo::Private::Integer = new TypeInfo(TypeInfo::IntegerType);
const TypeInfo* TypeInfo::Private::Date = new TypeInfo(TypeInfo::DateType);
const TypeInfo* TypeInfo::Private::Text = new TypeInfo(TypeInfo::TextType);
const TypeInfo* TypeInfo::Private::Rational = new TypeInfo(TypeInfo::RationalType);
const TypeInfo* TypeInfo::Private::GPSCoordinate = new TypeInfo(TypeInfo::GPSCoordinateType);

const TypeInfo* TypeInfo::Private::orderedArray(const TypeInfo* _typeInfo)
{
    if (Private::orderedArrays.contains(_typeInfo)) {
        return Private::orderedArrays[ _typeInfo ];
    }
    const TypeInfo* info = new TypeInfo(TypeInfo::OrderedArrayType, _typeInfo);
    Private::orderedArrays[ _typeInfo ] = info;
    return info;
}

const TypeInfo* TypeInfo::Private::unorderedArray(const TypeInfo* _typeInfo)
{
    if (Private::unorderedArrays.contains(_typeInfo)) {
        return Private::unorderedArrays[ _typeInfo ];
    }
    const TypeInfo* info = new TypeInfo(TypeInfo::UnorderedArrayType, _typeInfo);
    Private::unorderedArrays[ _typeInfo ] = info;
    return info;
}

const TypeInfo* TypeInfo::Private::alternativeArray(const TypeInfo* _typeInfo)
{
    if (Private::alternativeArrays.contains(_typeInfo)) {
        return Private::alternativeArrays[ _typeInfo ];
    }
    const TypeInfo* info = new TypeInfo(TypeInfo::AlternativeArrayType, _typeInfo);
    Private::alternativeArrays[ _typeInfo ] = info;
    return info;
}

const TypeInfo* TypeInfo::Private::createChoice(PropertyType _propertiesType, const TypeInfo* _embedded, const QList< Choice >& _choices)
{
    return new TypeInfo(_propertiesType, _embedded, _choices);
}

const TypeInfo* TypeInfo::Private::createStructure(Schema* _structureSchema, const QString& name)
{
    return new TypeInfo(_structureSchema, name);
}

const TypeInfo* TypeInfo::Private::LangArray = new TypeInfo(TypeInfo::LangArrayType);

TypeInfo::TypeInfo(TypeInfo::PropertyType _propertyType) : d(new Private)
{
    d->propertyType = _propertyType;
    if (d->propertyType == TypeInfo::LangArrayType) {
        d->embeddedTypeInfo = TypeInfo::Private::Text;
    }
    switch (d->propertyType) {
    case IntegerType:
        d->parser = new IntegerParser;
        break;
    case TextType:
        d->parser = new TextParser;
        break;
    case DateType:
        d->parser = new DateParser;
        break;
    case RationalType:
        d->parser = new RationalParser;
        break;
    default:
        ;
    }
}

struct TypeInfo::Choice::Private {
    Value value;
    QString hint;
};

TypeInfo::Choice::Choice(const Value& value, const QString& hint) : d(new Private)
{
    d->value = value;
    d->hint = hint;
}

TypeInfo::Choice::Choice(const Choice& _rhs) : d(new Private(*_rhs.d))
{
}

TypeInfo::Choice& TypeInfo::Choice::operator=(const Choice & _rhs)
{
    *d = *_rhs.d;
    return *this;
}

TypeInfo::Choice::~Choice()
{
    delete d;
}
const Value& TypeInfo::Choice::value() const
{
    return d->value;
}

const QString& TypeInfo::Choice::hint() const
{
    return d->hint;
}

TypeInfo::TypeInfo(PropertyType _propertyType, const TypeInfo* _embedded) : d(new Private)
{
    Q_ASSERT(_propertyType == OrderedArrayType || _propertyType == UnorderedArrayType || _propertyType == AlternativeArrayType);
    d->propertyType = _propertyType;
    d->embeddedTypeInfo = _embedded;
}

TypeInfo::TypeInfo(PropertyType _propertyType, const TypeInfo* _embedded, const QList< Choice >& _choices) : d(new Private)
{
    Q_ASSERT(_propertyType == ClosedChoice || _propertyType == OpenedChoice);
    d->propertyType = _propertyType;
    d->embeddedTypeInfo = _embedded;
    d->parser = _embedded->parser();
    d->choices = _choices;
}

TypeInfo::TypeInfo(Schema* _structureSchema, const QString& name) : d(new Private)
{
    d->propertyType = TypeInfo::StructureType;
    d->structureSchema = _structureSchema;
    d->structureName = name;
}

TypeInfo::~TypeInfo()
{
    delete d->parser;
    delete d;
}

TypeInfo::PropertyType TypeInfo::propertyType() const
{
    return d->propertyType;
}

const TypeInfo* TypeInfo::embeddedPropertyType() const
{
    return d->embeddedTypeInfo;
}

const QList< TypeInfo::Choice >& TypeInfo::choices() const
{
    return d->choices;
}

Schema* TypeInfo::structureSchema() const
{
    return d->structureSchema;
}

const QString& TypeInfo::structureName() const
{
    return d->structureName;
}

const Parser* TypeInfo::parser() const
{
    return d->parser;
}

bool checkArray(const Value& value, const TypeInfo* typeInfo)
{
    QList< Value > values = value.asArray();
    foreach(const Value& val, values) {
        if (!typeInfo->hasCorrectType(val)) {
            return false;
        }
    }
    return true;
}

bool TypeInfo::hasCorrectType(const Value& value) const
{
    switch (d->propertyType) {
    case BooleanType:
        return value.type() == Value::Variant && value.asVariant().type() == QVariant::Bool;
    case IntegerType:
        return value.type() == Value::Variant && value.asVariant().type() == QVariant::Int;
    case DateType:
        return value.type() == Value::Variant && value.asVariant().type() == QVariant::DateTime;
    case GPSCoordinateType:
    case TextType:
        return value.type() == Value::Variant && value.asVariant().type() == QVariant::String;
    case OrderedArrayType:
        if (value.type() == Value::OrderedArray) {
            return checkArray(value, d->embeddedTypeInfo);
        } else {
            return false;
        }
    case UnorderedArrayType:
        if (value.type() == Value::UnorderedArray) {
            return checkArray(value, d->embeddedTypeInfo);
        } else {
            return false;
        }
    case AlternativeArrayType:
        if (value.type() == Value::AlternativeArray) {
            return checkArray(value, d->embeddedTypeInfo);
        } else {
            return false;
        }
    case LangArrayType:
        if (value.type() == Value::LangArray) {
            QList< Value > values = value.asArray();
            foreach(const Value& vallang, values) {
                if (!Private::Text->hasCorrectType(vallang) ||
                        !Private::Text->hasCorrectType(vallang.propertyQualifiers()["xml:lang"])) {
                    return false;
                }
            }
        } else {
            return false;
        }
    case StructureType:
        if (value.type() == Value::Structure) {
            QMap<QString, KisMetaData::Value> structure = value.asStructure();
            for (QMap<QString, KisMetaData::Value>::iterator it = structure.begin();
                    it != structure.end(); ++it) {
                const TypeInfo* typeInfo = d->structureSchema->propertyType(it.key());
                if (!typeInfo || !typeInfo->hasCorrectType(it.value())) {
                    return false;
                }
            }
            return true;
        } else {
            return false;
        }
    case RationalType:
        return value.type() == Value::Rational;
    case OpenedChoice:
    case ClosedChoice:
        return d->embeddedTypeInfo->hasCorrectType(value);
    }
    return false;
}

bool TypeInfo::hasCorrectValue(const Value& value) const
{
    if (d->propertyType == ClosedChoice) {
        foreach(const Choice& choice, d->choices) {
            if (choice.value() == value) {
                return true;
            }
        }
        return false;
    } else {
        return true;
    }
}
