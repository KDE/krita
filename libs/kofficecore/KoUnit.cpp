/* This file is part of the KDE project
   Copyright (C) 2001 David Faure <faure@kde.org>
   Copyright (C) 2004, Nicolas GOUTTE <goutte@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

//#include <KoGlobal.h>
#include "KoUnit.h"
#include <KoXmlWriter.h>

#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>

#include <QRegExp>

QStringList KoUnit::listOfUnitName(bool hidePixel)
{
    QStringList lst;
    for ( uint i = 0 ; i <= KoUnit::LastUnit ; ++i )
    {
        Unit unit = static_cast<Unit>( i );
        if(i != Pixel || hidePixel ==false)
            lst.append( KoUnit::unitDescription( KoUnit(unit) ) );
    }
    return lst;
}


uint KoUnit::indexInList(bool hidePixel)
{
    if(hidePixel and m_unit > Pixel)
        return m_unit-1;
    else
        return m_unit;
}
QString KoUnit::unitDescription( KoUnit _unit )
{
    switch ( _unit.m_unit )
    {
    case KoUnit::Millimeter:
        return i18n("Millimeters (mm)");
    case KoUnit::Centimeter:
        return i18n("Centimeters (cm)");
    case KoUnit::Decimeter:
        return i18n("Decimeters (dm)");
    case KoUnit::Inch:
        return i18n("Inches (in)");
    case KoUnit::Pica:
        return i18n("Pica (pi)");
    case KoUnit::Didot:
        return i18n("Didot (dd)");
    case KoUnit::Cicero:
        return i18n("Cicero (cc)");
    case KoUnit::Point:
        return i18n("Points (pt)" );
    case KoUnit::Pixel:
        return i18n("Pixels (px)" );
    default:
        return i18n("Error!");
    }
}

double KoUnit::toUserValue( double ptValue, KoUnit unit )
{
    switch ( unit.m_unit ) {
    case Millimeter:
        return toMM( ptValue );
    case Centimeter:
        return toCM( ptValue );
    case Decimeter:
        return toDM( ptValue );
    case Inch:
        return toInch( ptValue );
    case Pica:
        return toPI( ptValue );
    case Didot:
        return toDD( ptValue );
    case Cicero:
        return toCC( ptValue );
    case Pixel:
        return floor( ptValue * unit.m_pixelConversion + 0.5);
    case Point:
    default:
        return toPoint( ptValue );
    }
}

double KoUnit::ptToUnit( const double ptValue, const KoUnit unit )
{
    switch ( unit.m_unit )
    {
    case Millimeter:
        return POINT_TO_MM( ptValue );
    case Centimeter:
        return POINT_TO_CM( ptValue );
    case Decimeter:
        return POINT_TO_DM( ptValue );
    case Inch:
        return POINT_TO_INCH( ptValue );
    case Pica:
        return POINT_TO_PI( ptValue );
    case Didot:
        return POINT_TO_DD( ptValue );
    case Cicero:
        return POINT_TO_CC( ptValue );
    case Pixel:
        return ptValue * unit.m_pixelConversion;
    case Point:
    default:
        return ptValue;
    }
}

QString KoUnit::toUserStringValue( double ptValue, KoUnit unit )
{
    return KGlobal::locale()->formatNumber( toUserValue( ptValue, unit ) );
}

double KoUnit::fromUserValue( double value, KoUnit unit )
{
    switch ( unit.m_unit ) {
    case Millimeter:
        return MM_TO_POINT( value );
    case Centimeter:
        return CM_TO_POINT( value );
    case Decimeter:
        return DM_TO_POINT( value );
    case Inch:
        return INCH_TO_POINT( value );
    case Pica:
        return PI_TO_POINT( value );
    case Didot:
        return DD_TO_POINT( value );
    case Cicero:
        return CC_TO_POINT( value );
    case Pixel:
        return value / unit.m_pixelConversion;
    case Point:
    default:
        return value;
    }
}

double KoUnit::fromUserValue( const QString& value, KoUnit unit, bool* ok )
{
    return fromUserValue( KGlobal::locale()->readNumber( value, ok ), unit );
}

double KoUnit::parseValue( const QString& _value, double defaultVal )
{
    if( _value.isEmpty() )
        return defaultVal;

    QString value(_value);

    value.simplified();
    value.remove( ' ' );

    int index = value.indexOf( QRegExp( "[a-z]+$" ) );
    if ( index == -1 )
        return value.toDouble();

    QString unit = value.mid( index );
    value.truncate ( index );
    double val = value.toDouble();

    if ( unit == "pt" )
        return val;

    bool ok;
    KoUnit u = KoUnit::unit( unit, &ok );
    if( ok )
        return fromUserValue( val, u );

    if( unit == "m" )
        return fromUserValue( val * 10.0, Decimeter );
    else if( unit == "km" )
        return fromUserValue( val * 10000.0, Decimeter );
    kWarning() << "KoUnit::parseValue: Unit " << unit << " is not supported, please report." << endl;

    // TODO : add support for mi/ft ?
    return defaultVal;
}

KoUnit KoUnit::unit( const QString &_unitName, bool* ok )
{
    if ( ok )
        *ok = true;
    if ( _unitName == QString::fromLatin1( "mm" ) ) return KoUnit(Millimeter);
    if ( _unitName == QString::fromLatin1( "cm" ) ) return KoUnit(Centimeter);
    if ( _unitName == QString::fromLatin1( "dm" ) ) return KoUnit(Decimeter);
    if ( _unitName == QString::fromLatin1( "in" )
         || _unitName == QString::fromLatin1("inch") /*compat*/ ) return KoUnit(Inch);
    if ( _unitName == QString::fromLatin1( "pi" ) ) return KoUnit(Pica);
    if ( _unitName == QString::fromLatin1( "dd" ) ) return KoUnit(Didot);
    if ( _unitName == QString::fromLatin1( "cc" ) ) return KoUnit(Cicero);
    if ( _unitName == QString::fromLatin1( "pt" ) ) return KoUnit(Point);
    if ( ok )
        *ok = false;
    return KoUnit(Point);
}

QString KoUnit::unitName( KoUnit _unit )
{
    if ( _unit.m_unit == Millimeter ) return QString::fromLatin1( "mm" );
    if ( _unit.m_unit == Centimeter ) return QString::fromLatin1( "cm" );
    if ( _unit.m_unit == Decimeter ) return QString::fromLatin1( "dm" );
    if ( _unit.m_unit == Inch ) return QString::fromLatin1( "in" );
    if ( _unit.m_unit == Pica ) return QString::fromLatin1( "pi" );
    if ( _unit.m_unit == Didot ) return QString::fromLatin1( "dd" );
    if ( _unit.m_unit == Cicero ) return QString::fromLatin1( "cc" );
    if ( _unit.m_unit == Pixel ) return QString::fromLatin1( "px" );
    return QString::fromLatin1( "pt" );
}

void KoUnit::saveOasis(KoXmlWriter* settingsWriter, KoUnit _unit)
{
    settingsWriter->addConfigItem( "unit", unitName(_unit) );
}
