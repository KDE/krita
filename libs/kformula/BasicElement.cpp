/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
   Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net>
   Copyright (C) 2006 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>

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

#include "BasicElement.h"
#include "FormulaCursor.h"
#include "AttributeManager.h"
#include <KoXmlWriter.h>
#include <QPainter>

namespace FormulaShape {

BasicElement::BasicElement( BasicElement* p ) : m_parentElement( p )
{
    m_baseLine = 0.0;
    m_boundingRect.setTopLeft( QPointF( 0.0, 0.0 ) );
    m_boundingRect.setWidth( 100.0 );       // standard values
    m_boundingRect.setHeight( 100.0 );
}

BasicElement::~BasicElement()
{
    m_attributes.clear();
}

void BasicElement::paint( QPainter& painter, const AttributeManager* )
{
    // draw a blue rectangle
    painter.setPen( QPen( Qt::blue ) );
    painter.drawRect( m_boundingRect );
}

void BasicElement::layout( const AttributeManager* )
{ /* do nothing */ }

void BasicElement::insertChild( FormulaCursor* cursor, BasicElement* element )
{
    // call the parentElement to notify it that there is something to be inserted
    m_parentElement->insertChild( cursor, element );
}

void BasicElement::removeChild( BasicElement* )
{ /* do nothing a BasicElement has no children */ }

const QList<BasicElement*> BasicElement::childElements() 
{
    return QList<BasicElement*>();
}

BasicElement* BasicElement::childElementAt( const QPointF& p )
{
    if( !m_boundingRect.contains( p ) )
        return 0;
	  	
    if( childElements().isEmpty() ) 
        return this;
	      
    BasicElement* ownerElement = 0;
    foreach( BasicElement* tmpElement, childElements() )  
    {
        ownerElement = tmpElement->childElementAt( p );
	
        if( ownerElement )
            return ownerElement;
    }
    
    return this;    // if no child contains the point, it's the FormulaElement itsself
}

void BasicElement::setAttribute( const QString& name, QVariant value )
{
    if( name.isEmpty() || !value.canConvert( QVariant::String ) )
        return;

    if( value.isNull() )
        m_attributes.remove( name );
    else
        m_attributes.insert( name, value.toString() );
}

QString BasicElement::attribute( const QString& attribute ) const
{
    QString tmp = m_attributes.value( attribute );
    if( tmp.isEmpty() )
        return QString();

    return tmp;
}

QString BasicElement::inheritsAttribute( const QString& ) const
{
    return QString();   // do nothing
}

QVariant BasicElement::attributesDefaultValue( const QString& ) const
{
    return QVariant();  // do nothing
}

void BasicElement::moveLeft( FormulaCursor* cursor, BasicElement* )
{
    if( cursor->currentElement() == this )
        parentElement()->moveLeft( cursor, this );
    else
        cursor->moveCursorTo( this, 0 );
}

void BasicElement::moveRight( FormulaCursor* cursor, BasicElement* )
{
    if( cursor->currentElement() == this )
        parentElement()->moveRight( cursor, this );
    else
        cursor->moveCursorTo( this, 0 );
}

void BasicElement::moveUp( FormulaCursor* cursor, BasicElement* )
{
    if( cursor->currentElement() == this )
        parentElement()->moveUp( cursor, this );
    else
        cursor->moveCursorTo( this, 0 );
}

void BasicElement::moveDown( FormulaCursor* cursor, BasicElement* )
{
    if( cursor->currentElement() == this )
        parentElement()->moveDown( cursor, this );
    else
        cursor->moveCursorTo( this, 0 );
}

void BasicElement::readMathML( const KoXmlElement& element )
{
    readMathMLAttributes( element );
    KoXmlNode node = element.firstChild();
    readMathMLContent( node );
}

void BasicElement::readMathMLAttributes( const KoXmlElement& element )
{
    QStringList attributeList = KoXml::attributeNames( element );
    foreach( QString attributeName, attributeList ) {
        m_attributes.insert( attributeName, element.attribute( attributeName ) );
    }
}

int BasicElement::readMathMLContent( KoXmlNode &node )
{
    Q_UNUSED( node )
    return 1;
}

void BasicElement::writeMathML( KoXmlWriter* writer, bool oasisFormat ) const
{
    QString name = oasisFormat ? "math:"  + getElementName() : getElementName();
    writer->startElement( name.toLatin1() );
    writeMathMLAttributes( writer );
    writeMathMLContent( writer, oasisFormat );
    writer->endElement();
}

void BasicElement::writeMathMLAttributes( KoXmlWriter* writer ) const
{
    foreach( QString value, m_attributes )
        writer->addAttribute( m_attributes.key( value ).toLatin1(), value );
}

ElementType BasicElement::elementType() const
{
    return Basic;
}

const QRectF& BasicElement::boundingRect() const
{
    return m_boundingRect;
}

double BasicElement::height() const
{
    return m_boundingRect.height();
}

double BasicElement::width() const
{
    return m_boundingRect.width();
}

double BasicElement::baseLine() const
{
    return m_baseLine;
}

QPointF BasicElement::origin() const
{
    return m_boundingRect.topLeft();
}

void BasicElement::setWidth( double width )
{
    m_boundingRect.setWidth( width );
}

void BasicElement::setHeight( double height )
{
    m_boundingRect.setHeight( height );
}

void BasicElement::setOrigin( QPointF origin )
{
    m_boundingRect.setTopLeft( origin );
}

void BasicElement::setBaseLine( double baseLine )
{
    m_baseLine = baseLine;
}

void BasicElement::setParentElement( BasicElement* parent )
{
    m_parentElement = parent;
}

BasicElement* BasicElement::parentElement() const
{
    return m_parentElement;
}

} // namespace FormulaShape
