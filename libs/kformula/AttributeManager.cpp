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
#include <KoUnit.h>
#include <KoViewConverter.h>
#include <QFontMetrics>
#include <QColor>

namespace FormulaShape {

AttributeManager::AttributeManager()
{
    m_viewConverter = 0;
    m_scriptLevelStack.push( 0 ); // set startup scriptlevel to 0
}

AttributeManager::~AttributeManager()
{}

void AttributeManager::inheritAttributes( const BasicElement* element )
{
    if( element->elementType() == Formula )   // FormulaElement has no attributes
        return;

    // check for the parent element and rebuild attribute heritage if needed
    if( m_attributeStack.isEmpty() ||
        element->parentElement() != m_attributeStack.first() )
        buildHeritageOf( element ); 
    else
    { 
        m_attributeStack.push( element );       // add the element to the stack
        alterScriptLevel( element );            // adapt the scritplevel
    }
}

void AttributeManager::disinheritAttributes()
{
    m_attributeStack.pop();   // remove it from stack
    m_scriptLevelStack.pop();                         
}

QVariant AttributeManager::valueOf( const QString& attribute )
{
    // check if the current element has a value assigned
    QString value;
    value = m_attributeStack.first()->hasAttribute( attribute );
    if( !value.isEmpty() )
        return parseValue( value );

    // if not, check if any element in the stack might inherit a value
    foreach( const BasicElement* tmp, m_attributeStack )
    {
        value = tmp->inheritsAttribute( attribute );
        if( !value.isEmpty() )
            return parseValue( value );
    }
    
    // if not, return the default value of the attribute 
    return m_attributeStack.first()->attributesDefaultValue( attribute );
}

void AttributeManager::buildHeritageOf( const BasicElement* element )
{
    m_attributeStack.clear();
    m_scriptLevelStack.clear();
    m_scriptLevelStack.push( 0 );     // see 3.3.4.2 of the MathML spec

    const BasicElement* tmpElement = element;
    while( tmpElement )
    {
        m_attributeStack.push( tmpElement );
        alterScriptLevel( tmpElement );
        tmpElement = tmpElement->parentElement();
    }
}

void AttributeManager::alterScriptLevel( const BasicElement* element )
{
    // set the scriptlevel explicitly
    QString value = element->hasAttribute( "scriptlevel" );
    if( !value.isEmpty() )
    {
        // catch increments or decrements that may only affect scriptlevel
        if( value.startsWith( "+" ) )
        {
            int tmp = m_scriptLevelStack.top() + value.remove( 0, 1 ).toInt();
            m_scriptLevelStack.push( tmp );
        }
        else if( value.startsWith( "-" ) )
        {
            int tmp = m_scriptLevelStack.top() - value.remove( 0, 1 ).toInt();
            m_scriptLevelStack.push( tmp );
	}
        else
            m_scriptLevelStack.push( value.toInt() );
    }
    else if( element->parentElement()->elementType() == MultiScript ||
             element->parentElement()->elementType() == UnderOver )
        m_scriptLevelStack.push( m_scriptLevelStack.top()++ );
    else if( element->parentElement()->elementType() == Fraction && 
             valueOf( "displaystyle" ).toBool() )
        m_scriptLevelStack.push( m_scriptLevelStack.top()++ );
    else if( element->parentElement()->elementType() == Root && 
             element == element->parentElement()->childElements().value( 1 ) )
        m_scriptLevelStack.push( m_scriptLevelStack.top()+2 ); // only for roots index
    else
        m_scriptLevelStack.push( m_scriptLevelStack.top() );
}

QVariant AttributeManager::parseValue( const QString& value ) const
{
    // check if the value includes metric units
    if( parseMetrics( value ).isValid() )
        return parseMetrics( value );

    // look for attributes that are enums
    if( alignValue( value ) != -1 )
        return alignValue( value );
    else if( mathVariantValue( value ) != -1 )
        return mathVariantValue( value );
    else if( mathSpaceValue( value ) != -1.0)
        return mathSpaceValue( value );
    else if( formValue( value ) != -1 )
        return formValue( value );

    // look if the value is color
    QColor tmpColor( value );
    if( tmpColor.isValid() )
        return tmpColor;    
    
    // all other values don't need special treatment
    return QVariant( value );
}

int AttributeManager::alignValue( const QString& value ) const
{
    if( value == "right" )
        return Right;
    else if( value == "left" )
        return Left;
    else if( value == "center" )
        return Center;
    else
        return -1;
}

int AttributeManager::formValue( const QString& value ) const
{
    if( value == "prefix" )
        return Prefix;
    else if( value == "infix" )
        return Infix;
    else if( value == "postfix" )
        return Postfix;
    else
        return -1;
}

int AttributeManager::mathVariantValue( const QString& value ) const
{
    if( value == "normal" )
        return Normal;
    else if( value == "bold" )
        return Bold;
    else if( value == "italic" )
        return Italic;
    else if( value == "bold-italic" )
        return BoldItalic;
    else if( value == "double-struck" )
        return DoubleStruck;
    else if( value == "bold-fraktur" )
        return BoldFraktur;
    else if( value == "script" )
        return Script;
    else if( value == "bold-script" )
        return BoldScript;
    else if( value == "fraktur" )
        return Fraktur;
    else if( value == "sans-serif" )
        return SansSerif;
    else if( value == "bold-sans-serif" )
        return BoldSansSerif;
    else if( value == "sans-serif-italic" )
        return SansSerifItalic;
    else if( value == "sans-serif-bold-italic" )
        return SansSerifBoldItalic;
    else if( value == "monospace" )
        return Monospace;
    else
        return -1; 
}

double AttributeManager::mathSpaceValue( const QString& value ) const
{
    if( value == "negativeveryverythinmathspace" )
        return -1*calculateEmExUnits( 0.055556, true );
    else if( value == "negativeverythinmathspace" )
        return -1*calculateEmExUnits( 0.111111, true );
    else if( value == "negativethinmathspace" )
        return -1*calculateEmExUnits( 0.166667, true );
    else if( value == "negativemediummathspace" )
        return -1*calculateEmExUnits( 0.222222, true );
    else if( value == "negativethickmathspace" )
        return -1*calculateEmExUnits( 0.277778, true );
    else if( value == "negativeverythickmathspace" )
        return -1*calculateEmExUnits( 0.333333, true );
    else if( value == "negativeveryverythickmathspace" )
        return -1*calculateEmExUnits( 0.388889, true );
    else if( value == "veryverythinmathspace" )
        return calculateEmExUnits( 0.055556, true );
    else if( value == "verythinmathspace" )
        return calculateEmExUnits( 0.111111, true );
    else if( value == "thinmathspace" )
        return calculateEmExUnits( 0.166667, true );
    else if( value == "mediummathspace" )
        return calculateEmExUnits( 0.222222, true );
    else if( value == "thickmathspace" )
        return calculateEmExUnits( 0.277778, true );
    else if( value == "verythickmathspace" )
        return calculateEmExUnits( 0.333333, true );
    else if( value == "veryverythickmathspace" )
        return calculateEmExUnits( 0.388889, true );
    else
        return -1.0;
}

QVariant AttributeManager::parseMetrics( const QString& value ) const
{
    QString tmpValue = value.trimmed();
    QString unit = tmpValue.right( 2 );
    tmpValue.chop( 2 );

    if( unit == "em" )
        return calculateEmExUnits( tmpValue.toDouble(), true );
    else if( unit == "ex" )
        return calculateEmExUnits( tmpValue.toDouble(), false );
    else if( unit == "px" )
        return m_viewConverter->viewToDocumentX( tmpValue.toInt() );
    else if( unit == "in" || unit == "cm" || unit == "pc" || unit == "mm" ||
             unit == "pt" )
        return KoUnit::parseValue( value );
//    else if( value.endsWith( "%" ) )
//        return defaultValueOf( attribute )*value.trimmed().chop( 1 ).toInt();

    return QVariant();
}

double AttributeManager::calculateEmExUnits( double value, bool isEm ) const
{
    // ThomasZ said that with KoViewConverter it does not work but atm otherwise
    // it is to hard to realize
    // The constructor of QFontMetrics needs as second argument a postscript based
    // QPaintDevice so that width() and xHeight() return values that are also
    // postscript based.
    if( !m_paintDevice )
        return 0.0;

    QFontMetrics fm( m_currentFont, m_paintDevice );
    if( isEm )
        return value * fm.width( 'm' );
    else
        return value* fm.xHeight();
}


int AttributeManager::scriptLevel() const
{
    return m_scriptLevelStack.top();
}

void AttributeManager::setViewConverter( KoViewConverter* converter )
{
    m_viewConverter = converter;
}

void AttributeManager::setPaintDevice( QPaintDevice* paintDevice )
{
    m_paintDevice = paintDevice;
}

} // namespace FormulaShape
