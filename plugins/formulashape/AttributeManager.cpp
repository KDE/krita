/* This file is part of the KDE project
   Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net>

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
   Boston, MA 02110-1301, USA.
*/

#include "AttributeManager.h"
#include "BasicElement.h"
#include "ElementFactory.h"
#include <KoUnit.h>
#include <KoViewConverter.h>
#include <KoPostscriptPaintDevice.h>
#include <QFontMetricsF>
#include <QColor>
#include <kdebug.h>
//// Copied from calligra KoUnit.h
//
//// 1 inch ^= 72 pt
//// 1 inch ^= 25.399956 mm (-pedantic ;p)
//// 1 pt = 1/12 pi
//// 1 pt ^= 0.0077880997 cc
//// 1 cc = 12 dd
//// Note: I don't use division but multiplication with the inverse value
//// because it's faster ;p (Werner)
//#define POINT_TO_MM(px) ((px)*0.352777167)
//#define MM_TO_POINT(mm) ((mm)*2.83465058)
//#define POINT_TO_CM(px) ((px)*0.0352777167)
//#define CM_TO_POINT(cm) ((cm)*28.3465058)
//#define POINT_TO_DM(px) ((px)*0.00352777167)
//#define DM_TO_POINT(dm) ((dm)*283.465058)
//#define POINT_TO_INCH(px) ((px)*0.01388888888889)
//#define INCH_TO_POINT(inch) ((inch)*72.0)
//#define MM_TO_INCH(mm) ((mm)*0.039370147)
//#define INCH_TO_MM(inch) ((inch)*25.399956)
//#define POINT_TO_PI(px)((px)*0.083333333)
//#define POINT_TO_CC(px)((px)*0.077880997)
//#define PI_TO_POINT(pi)((pi)*12)
//#define CC_TO_POINT(cc)((cc)*12.840103)

AttributeManager::AttributeManager()
{
    m_viewConverter = 0;
}

AttributeManager::~AttributeManager()
{}

QString AttributeManager::findValue( const QString& attribute, const BasicElement* element ) const
{
    // check if the current element has a value assigned
    QString value = element->attribute( attribute );
    if( !value.isEmpty() ) {
//         kDebug()<<"checking for attribute "<<attribute <<" returning (s)"<<value;
        return value;
    }
    // if not, check if any of the parent elements inherits a value
    BasicElement* tmpParent = element->parentElement();
    while( tmpParent )
    {
        value = tmpParent->inheritsAttribute( attribute );
        if( !value.isEmpty() ) {
//             kDebug()<<"checking for attribute "<<attribute <<" returning (p)"<<value;
            return value;
        }
        else {
            tmpParent = tmpParent->parentElement();
        }
    }

    // if not, return the default value of the attribute
//     kDebug()<<"checking for attribute "<<attribute <<" returning (d) "<<element->attributesDefaultValue( attribute );
    return element->attributesDefaultValue( attribute );
}

bool AttributeManager::boolOf( const QString& attribute,
                               const BasicElement* element ) const
{
    return findValue( attribute, element ) == "true";
}

qreal AttributeManager::doubleOf( const QString& attribute,
                                  const BasicElement* element ) const
{

    return lengthToPixels(parseUnit( findValue( attribute, element ), element ), element, attribute);
}

QList<qreal> AttributeManager::doubleListOf( const QString& attribute,
                                             const BasicElement* element ) const
{
    QList<qreal> doubleList;
    QStringList tmp = findValue( attribute, element ).split( ' ' );
    foreach( const QString &doubleValue, tmp )
        doubleList << lengthToPixels( parseUnit( doubleValue, element ), element, attribute);

    return doubleList;
}

QString AttributeManager::stringOf( const QString& attribute, const BasicElement* element  ) const
{
    return findValue( attribute, element );
}

QColor AttributeManager::colorOf( const QString& attribute, const BasicElement* element  ) const
{
    QString tmpColor = findValue( attribute, element );
    if( attribute == "mathbackground" && tmpColor.isEmpty() )
        return Qt::transparent;

    return QColor( tmpColor );
}

Align AttributeManager::alignOf( const QString& attribute, const BasicElement* element  ) const
{
    return parseAlign( findValue( attribute, element ) );
}

QList<Align> AttributeManager::alignListOf( const QString& attribute,
                                            const BasicElement* element  ) const
{
    QList<Align> alignList;
    QStringList tmpList = findValue( attribute, element ).split( ' ' );

    foreach( const QString &tmp, tmpList )
        alignList << parseAlign( tmp );

    return alignList;
}

Qt::PenStyle AttributeManager::penStyleOf( const QString& attribute,
                                           const BasicElement* element  ) const
{
    return parsePenStyle( findValue( attribute, element ) );
}

QList<Qt::PenStyle> AttributeManager::penStyleListOf( const QString& attribute,
                                                      const BasicElement* element  ) const
{
    QList<Qt::PenStyle> penStyleList;
    QStringList tmpList = findValue( attribute, element ).split( ' ' );

    foreach( const QString &tmp, tmpList )
        penStyleList << parsePenStyle( tmp );

    return penStyleList;
}

int AttributeManager::scriptLevel( const BasicElement* parent, int index ) const
{
    ElementType parentType = parent->elementType();
    int current_scaleLevel = parent->scaleLevel();

    /** First check for types where all children are scaled */
    switch(parentType) {
        case Fraction:
            if( parent->displayStyle() == false )
                return current_scaleLevel+1;
            else
                return current_scaleLevel;
        case Style: {
            QString tmp = parent->attribute( "scriptlevel" );
            if( tmp.startsWith( '+' ) )
                    return current_scaleLevel + tmp.remove(0,1).toInt();
            if( tmp.startsWith( '-' ) )
                    return current_scaleLevel - tmp.remove(0,1).toInt();
            return tmp.toInt();
        }
        case MultiScript:
            return current_scaleLevel + 1;
        case Table:
            return current_scaleLevel + 1;
        default:
            break;
    }
    if( index == 0) return current_scaleLevel;
    /** Now check for types where the first child isn't scaled, but the rest are */
    switch(parentType) {
            case SubScript:
            case SupScript:
            case SubSupScript:
                return current_scaleLevel + 1;
            case Under:
                if( boolOf("accentunder", parent) )
                    return current_scaleLevel + 1;
                else
                    return current_scaleLevel;
            case Over:
                if( boolOf("accent", parent) )
                    return current_scaleLevel + 1;
                else
                    return current_scaleLevel;
            case UnderOver:
                if( (index == 1 && boolOf("accentunder", parent)) || (index == 2 && boolOf("accent", parent)) )
                    return current_scaleLevel + 1;
                else
                    return current_scaleLevel;
            case Root:
                /* second argument to root is the base */
                return current_scaleLevel + 1;
            default:
                return current_scaleLevel;
    }
}

qreal AttributeManager::lineThickness( const BasicElement* element ) const
{
    QFontMetricsF fm(font(element));
    return fm.height() * 0.06 ;
}

qreal AttributeManager::layoutSpacing( const BasicElement* element  ) const
{
    QFontMetricsF fm(font(element));
//    return fm.height() * 0.166667 ;
    return fm.height() * 0.05 ;
}

qreal AttributeManager::lengthToPixels( Length length, const BasicElement* element, const QString &attribute) const
{
    if(length.value == 0)
        return 0;

    switch(length.unit) {
    case Length::Em: {
        QFontMetricsF fm(font(element));
        return fm.height() * length.value;
    }
    case Length::Ex: {
        QFontMetricsF fm(font(element));
        return fm.xHeight() * length.value;
    }
    case Length::Percentage:
        return lengthToPixels( parseUnit( element->attributesDefaultValue(attribute), element),element, attribute) * length.value / 100.0;
    case Length::Px: //pixels
        return length.value;
    case Length::In:  /* Note for the units below we assume point == pixel.  */
        return INCH_TO_POINT(length.value);
    case Length::Cm:
        return CM_TO_POINT(length.value);
    case Length::Mm:
        return MM_TO_POINT(length.value);
    case Length::Pt:
        return length.value;
    case Length::Pc:
        return PI_TO_POINT(length.value);
    case Length::None:
    default:
        return length.value;
    }
}

Length AttributeManager::parseUnit( const QString& value,
                                    const BasicElement* element ) const
{
    Q_UNUSED(element)
    Length length;

    if (value.isEmpty())
        return length;
    QRegExp re("(-?[\\d\\.]*) *(px|em|ex|in|cm|pc|mm|pt|%)?", Qt::CaseInsensitive);
    if (re.indexIn(value) == -1)
        return length;
    QString real = re.cap(1);
    QString unit = re.cap(2).toLower();

    bool ok;
    qreal number = real.toDouble(&ok);
    if (!ok)
        return length;

    length.value = number;
    if(!unit.isEmpty()) {
        if (unit == "em") {
            length.unit = Length::Mm;
            length.type = Length::Relative;
        }
        else if (unit == "ex") {
            length.unit = Length::Ex;
            length.type = Length::Relative;
        }
        else if (unit == "px") {
            length.unit = Length::Px;
            length.type = Length::Pixel;
        }
        else if (unit == "in") {
            length.unit = Length::In;
            length.type = Length::Absolute;
        }
        else if (unit == "cm") {
            length.unit = Length::Cm;
            length.type = Length::Absolute;
        }
        else if (unit == "mm") {
            length.unit = Length::Mm;
            length.type = Length::Absolute;
        }
        else if (unit == "pt") {
            length.unit = Length::Pt;
            length.type = Length::Relative;
        }
        else if (unit == "pc") {
            length.unit = Length::Pc;
            length.type = Length::Relative;
        }
        else if (unit == "%") {
            length.unit = Length::Percentage;
            length.type = Length::Relative;
        }
        else {
            length.unit = Length::None;
            length.type = Length::NoType;
        }
    }

    return length;
}

Align AttributeManager::parseAlign( const QString& value ) const
{
    if( value == "right" )
        return Right;
    else if( value == "left" )
        return Left;
    else if( value == "center" )
        return Center;
    else if( value == "top" )
        return Top;
    else if( value == "bottom" )
        return Bottom;
    else if( value == "baseline" )
        return BaseLine;
    else if( value == "axis" )
        return Axis;
    else
        return InvalidAlign;
}

Qt::PenStyle AttributeManager::parsePenStyle( const QString& value ) const
{
    if( value == "solid" )
        return Qt::SolidLine;
    else if( value == "dashed" )
        return Qt::DashLine;
    else
        return Qt::NoPen;
}

QFont AttributeManager::font( const BasicElement* element ) const
{

    // TODO process the mathvariant values partly
    // normal -> do nothing.
    // if contains bold -> font.setBold( true )
    // if contains italic -> font.setItalic( true )
    // if contains sans-serif setStyleHint( SansSerif ) --> Helvetica

    QFont font;
    Length unit = parseUnit( findValue( "fontsize", element ), element );
    if ( unit.type == Length::Absolute ) {
        font.setPointSizeF( lengthToPixels( unit,  element,  "fontsize" ) );
    } else if ( unit.type == Length::Relative ) {
        font.setPointSizeF( lengthToPixels( unit,  element,  "fontsize" ) * element->scaleFactor() );
    } else if ( unit.type == Length::Pixel ) {
        font.setPixelSize( lengthToPixels( unit,  element,  "fontsize" ) * element->scaleFactor() );
    }
    return font;
}

void AttributeManager::setViewConverter( KoViewConverter* converter )
{
    m_viewConverter = converter;
}

qreal AttributeManager::maxHeightOfChildren( const BasicElement* element ) const
{
    qreal maxHeight = 0.0;
    foreach( BasicElement* tmp, element->childElements() )
        maxHeight = qMax( maxHeight, tmp->height() );

    return maxHeight;
}

qreal AttributeManager::maxWidthOfChildren( const BasicElement* element  ) const
{
    qreal maxWidth = 0.0;
    foreach( BasicElement* tmp, element->childElements() )
        maxWidth = qMax( maxWidth, tmp->width() );

    return maxWidth;
}
qreal AttributeManager::parseMathSpace( const QString& value, const BasicElement * element )  const
{
    QFontMetricsF fm(font(element));
    qreal conversionEmToPixels = fm.xHeight();

    if( value == "negativeveryverythinmathspace" )
        return -1*conversionEmToPixels*0.055556;
    else if( value == "negativeverythinmathspace" )
        return -1*conversionEmToPixels*0.111111;
    else if( value == "negativethinmathspace" )
        return -1*conversionEmToPixels*0.166667;
    else if( value == "negativemediummathspace" )
        return -1*conversionEmToPixels*0.222222;
    else if( value == "negativethickmathspace" )
        return -1*conversionEmToPixels*0.277778;
    else if( value == "negativeverythickmathspace" )
        return -1*conversionEmToPixels*0.333333;
    else if( value == "negativeveryverythickmathspace" )
        return -1*conversionEmToPixels*0.388889;
    else if( value == "veryverythinmathspace" )
        return conversionEmToPixels*0.055556;
    else if( value == "verythinmathspace" )
        return conversionEmToPixels*0.111111;
    else if( value == "thinmathspace" )
        return conversionEmToPixels*0.166667;
    else if( value == "mediummathspace" )
        return conversionEmToPixels*0.222222;
    else if( value == "thickmathspace" )
        return conversionEmToPixels*0.277778;
    else if( value == "verythickmathspace" )
        return conversionEmToPixels*0.333333;
    else if( value == "veryverythickmathspace" )
        return conversionEmToPixels*0.388889;
    else
        return 0;
}

