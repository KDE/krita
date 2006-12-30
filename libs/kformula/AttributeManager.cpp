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

namespace KFormula {

AttributeManager::AttributeManager( const KoViewConverter& converter ),
                  m_converter( converter ), m_paintDevice( pd )
{
    m_paintDevice = 0;
}

AttributeManager::~AttributeManager()
{
}

void AttributeManager::inheritAttributes( BasicElement* element )
{
    // check for the parent element and rebuild attribute heritage if needed
    if( element->parentElement() != m_attributeStack.first() )
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
    if( m_attributeStack.first()->attributes().contain( attribute ) )
        return parseValue( m_attributeStack.first()->attributes().value( attribute ) );

    // if not, check if any element in the stack might inherit a value
    QString value;
    foreach( BasicElement* tmp, m_attributeStack )
    {
        value = tmp->inheritAttribute( attribute );
        if( !value.isEmpty() )
            return parseValue( value );
    }
    
    // if not, return the default value of the attribute 
    if( value.isEmpty() )                     // there was no attribute found
        return defaultValueOf( attribute );   // return default value
}

void AttributeManager::buildHeritageOf( BasicElement* element )
{
    m_attributeStack.clear();
    BasicElement* tmpElement = element;
    while( tmpElement )
    {
        m_attributeStack << tmpElement;
        tmpElement = tmpElement->parentElement();
    }

    determineScriptLevel();
}

void AttributeManager::determineScriptLevel()
{
    m_scriptLevel = 0;    // see 3.3.4.2 of the MathML spec 

    // iterating the tree from behind
    QListIterator it( m_attributeStack );
    it.toBack();
    while( it.hasPrevious() )
        alterScriptLevel( it.previous() );
}

int AttributeManager::scriptLevel() const
{
    return m_scriptLevelStack.top();
}

void AttributeManager::alterScriptLevel( BasicElement* element ) const
{
    // set the scriptlevel explicitly
    if( element->attributes()->contain( "scriptlevel" ) )
    {
        // catch increments or decrements that may only affect scriptlevel
        if( value.startsWith( "+" ) )
        {
            int tmp = m_scriptLevelStack.top() + QString::number( value.remove( 0, 1 ) );
            m_scriptLevelStack.push( tmp );
        }
        else if( value.startsWith( "-" ) )
        {
            int tmp = m_scriptLevelStack.top() - QString::number( value.remove( 0, 1 ) );
            m_scriptLevelStack.push( tmp );
	}
        else
            m_scriptLevelStack.push( QString::number( value ) );
    }
    else if( element->parentElement()->elementType() == MultiScript ||
             element->parentElement()->elementType() == UnderOver )
        m_scriptLevelStack.push( m_scriptLevelStack.top()++ );
    else if( element->parentElement()->elementType() == Fraction
             && valueOf( "displaystyle" ) )
        m_scriptLevelStack.push( m_scriptLevelStack.top()++ );
    else if( element->parentElement()->elementType() == Root && 
             element == element->parentElement()->childElements()->value( 1 ) )
        m_scriptLevelStack.push( m_scriptLevelStack.top()+2 ); // only for roots index
}

QVariant AttributeManager::defaultValueOf( const QString& attribute )
{
    QVariant tmp = m_attributeStack.first()->defaultValueOf( attribute );
    if( !tmp.isNull() )
        return tmp;
    
    // there is a global default value for the attribute
    // TODO implement all default values listed in the spec


    return QVariant(); 
}

QVariant AttributeManager::parseValue( const QString& value ) const
{
    // catch values with metrics
    if( value.endsWith( "em" ) )
        return calculateEmExUnits( QString::number( value.chop( 2 ) ), true );
    else if( value.endsWith( "ex" ) )
        return calculateEmExUnits( QString::number( value.chop( 2 ) ), false );
    else if( value.endsWith( "px" ) )
        return m_converter.viewToDocumentX( QString::number( value.chop( 2 ) ) );
    else if( value.endsWith( "in" ) || value.endsWith( "cm" ) || value.endsWith( "pc" )
             value.endsWith( "mm" ) || value.endsWith( "pt" ) )
        return KoUnit::parseValue( value );
//    else if( value.endsWith( "%" ) )
//        return defaultValueOf( attribute )*QString::number( value.chop( 1 ) );

    // look for attributes that are enums
    if( alignValue( value ) =! -1 )
        return alignValue( value );
    else if( mathVariantValue( value ) =! -1 )
        return mathVariantValue( value );
    else if( mathSpaceValue( value ) =! -1.0)
        return mathSpaceValue;
    else if( formValue( value ) =! -1 )
        return formValue( value );

    // look if the value is color
    QColor tmpColor( value );
    if( tmpColor.isValid() )
        return tmpColor;    
    
    // all other values don't need special treatment
    return QVariant( value );
}

int AttributeManager::alignValue( const QString& value )
{
    if( value == "right" )
        return Align::Right;
    else if( value == "left" )
        return Align::Left;
    else if( value == "center" )
        return Align::Center;
    else
        return -1;
}

int AttributeManager::formValue( const QString& value )
{
    if( value == "prefix" )
        return Form::Prefix;
    else if( value == "infix" )
        return Form::Infix;
    else if( value == "postfix" )
        return Form::Postfix;
    else
        return -1;
}

int AttributeManager::mathVariantValue( const QString& value )
{
    if( value == "normal" )
        return MathVariant::Normal;
    else if( value == "bold" )
        return MathVariant::Bold;
    else if( value == "italic" )
        return MathVariant::Italic;
    else if( value == "bold-italic" )
        return MathVariant::BoldItalic;
    else if( value == "double-struck" )
        return MathVariant::DoubleStruck;
    else if( value == "bold-fraktur" )
        return MathVariant::BoldFraktur;
    else if( value == "script" )
        return MathVariant::Script;
    else if( value == "bold-script" )
        return MathVariant::BoldScript;
    else if( value == "fraktur" )
        return MathVariant::Fraktur;
    else if( value == "sans-serif" )
        return MathVariant::SansSerif;
    else if( value == "bold-sans-serif" )
        return MathVariant::BoldSansSerif;
    else if( value == "sans-serif-italic" )
        return MathVariant::SansSerifItalic;
    else if( value == "sans-serif-bold-italic" )
        return MathVariant::SansSerifBoldItalic;
    else if( value == "monospace" )
        return MathVariant::MonoSpace;
    else
        return -1; 
}

double AttributeManager::mathSpaceValue( const QString& value )
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

double AttributeManager::calculateEmExUnits( double value, bool isEm )
{
    // ThomasZ said that with KoViewConverter it does not work but atm otherwise
    // it is to hard to realise
    // The constructor of QFontMetrics needs as second argument a postscript based
    // QPaintDevice so that width() and xHeight() return values that are also
    // postscript based.
//    if( !m_paintDevice )
//        return 0.0;

//    QFontMetrics fm( m_currentFont, m_paintDevice );
    QFontMetrics fm( m_currentFont );
    if( isEm )
        return m_converter.viewToDocumentX( value*fm.width( 'm' ) );
    else
        return m_converter.viewToDocumentY( value*fm.xHeight() );
}

} // namespace KFormula
