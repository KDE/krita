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

#include "kis_meta_data_type_info.h"

#include "kis_meta_data_value.h"

using namespace KisMetaData;

struct TypeInfo::Private {
    Private() : embeddedTypeInfo(0) {}
    PropertyType propertyType;
    const TypeInfo* embeddedTypeInfo;
    QHash< QString, Value> choices;
    static QHash< const TypeInfo*, const TypeInfo*> orderedArrays;
    static QHash< const TypeInfo*, const TypeInfo*> unorderedArrays;
    static QHash< const TypeInfo*, const TypeInfo*> alternativeArrays;
};

QHash< const TypeInfo*, const TypeInfo*> TypeInfo::Private::orderedArrays;
QHash< const TypeInfo*, const TypeInfo*> TypeInfo::Private::unorderedArrays;
QHash< const TypeInfo*, const TypeInfo*> TypeInfo::Private::alternativeArrays;


const TypeInfo* TypeInfo::Integer = new TypeInfo( TypeInfo::IntegerType );
const TypeInfo* TypeInfo::Date = new TypeInfo( TypeInfo::DateType );
const TypeInfo* TypeInfo::Text = new TypeInfo( TypeInfo::TextType );
const TypeInfo* TypeInfo::SignedRational = new TypeInfo( TypeInfo::SignedRationalType );
const TypeInfo* TypeInfo::UnsignedRational = new TypeInfo( TypeInfo::UnsignedRationalType );
const TypeInfo* TypeInfo::GPSCoordinate = new TypeInfo( TypeInfo::GPSCoordinateType );

const TypeInfo* TypeInfo::orderedArray( const TypeInfo* _typeInfo)
{
    if( Private::orderedArrays.contains( _typeInfo ) )
    {
        return Private::orderedArrays[ _typeInfo ];
    }
    const TypeInfo* info = new TypeInfo( TypeInfo::OrderedArrayType, _typeInfo );
    Private::orderedArrays[ _typeInfo ] = info;
    return info;
}

const TypeInfo* TypeInfo::unorderedArray( const TypeInfo* _typeInfo)
{
    if( Private::unorderedArrays.contains( _typeInfo ) )
    {
        return Private::unorderedArrays[ _typeInfo ];
    }
    const TypeInfo* info = new TypeInfo( TypeInfo::UnorderedArrayType, _typeInfo );
    Private::unorderedArrays[ _typeInfo ] = info;
    return info;
}

const TypeInfo* TypeInfo::alternativeArray( const TypeInfo* _typeInfo)
{
    if( Private::alternativeArrays.contains( _typeInfo ) )
    {
        return Private::alternativeArrays[ _typeInfo ];
    }
    const TypeInfo* info = new TypeInfo( TypeInfo::AlternativeArrayType, _typeInfo );
    Private::alternativeArrays[ _typeInfo ] = info;
    return info;
}

const TypeInfo* TypeInfo::LangArray = new TypeInfo( TypeInfo::LangArrayType );

TypeInfo::TypeInfo( TypeInfo::PropertyType _propertyType ) : d(new Private )
{
    d->propertyType = _propertyType;
    if( d->propertyType == TypeInfo::LangArrayType )
    {
        d->embeddedTypeInfo = TypeInfo::Text;
    }
}

TypeInfo::TypeInfo( PropertyType _propertyType, const TypeInfo* _embedded ) : d(new Private)
{
    Q_ASSERT( _propertyType == OrderedArrayType || _propertyType == UnorderedArrayType || _propertyType == AlternativeArrayType );
    d->propertyType = _propertyType;
    d->embeddedTypeInfo = _embedded;
}

TypeInfo::TypeInfo( PropertyType _propertyType, const TypeInfo* _embedded, const QHash< QString, Value>& _choices) : d(new Private)
{
    Q_ASSERT(_propertyType == ClosedChoice || _propertyType == OpenedChoice );
    d->propertyType = _propertyType;
    d->choices = _choices;
}

TypeInfo::~TypeInfo()
{
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

const QHash< QString, Value>& TypeInfo::choices() const
{
    return d->choices;
}
