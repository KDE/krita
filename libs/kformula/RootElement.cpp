/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
		 2006 Martin Pfeiffer <hubipete@gmx.net>

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

#include "RootElement.h"
#include "FormulaCursor.h"

namespace FormulaShape {

RootElement::RootElement( BasicElement* parent ) : BasicElement( parent )
{
    m_radicand = new BasicElement( this );
    m_exponent = new BasicElement( this );
}

RootElement::~RootElement()
{
    delete m_radicand;
    delete m_exponent;
}

const QList<BasicElement*> RootElement::childElements()
{
    QList<BasicElement*> tmp;
    if( m_exponent )
	tmp << m_exponent;
    return tmp << m_radicand;
}

void RootElement::insertChild( FormulaCursor* cursor, BasicElement* child )
{
    BasicElement* tmp = cursor->currentElement();
    if( tmp == m_radicand && m_radicand->elementType == Basic )
    {
        m_radicand = child;
        cursor->moveCursorTo( m_radicand, 1 );
        delete tmp;
    }
    else if( tmp == m_radicand && m_radicant->elementType != Basic )
    {
        m_radicand = new SequenceElement;
    }
    else if( tmp == m_exponent && m_exponent->elementType == Basic )
    {
        m_exponent = child;
        cursor->moveCursorTo( m_exponent, 1 );
        delete tmp;
    }
}

void RootElement::removeChild( BasicElement* element )
{
}

void RootElement::paint( QPainter& painter, const AttributeManager* am )
{
}

void RootElement::layout( const AttributeManager* am )
{
    luPixel indexWidth = 0;
    luPixel indexHeight = 0;
    if ( m_exponent) {
        m_exponent->calcSizes( context,
                               context.convertTextStyleIndex(tstyle),
                               context.convertIndexStyleUpper(istyle),
                               style );
        indexWidth = m_exponent->getWidth();
        indexHeight = m_exponent->getHeight();
    }

    double factor = style.sizeFactor();
    luPixel distX = context.ptToPixelX( context.getThinSpace( tstyle, factor ) );
    luPixel distY = context.ptToPixelY( context.getThinSpace( tstyle, factor ) );
    luPixel unit = (m_radicand->getHeight() + distY)/ 3;

    if (m_exponent) {
        if (indexWidth > unit) {
            m_exponent->setX(0);
            rootOffset.setX( indexWidth - unit );
        }
        else {
            m_exponent->setX( ( unit - indexWidth )/2 );
            rootOffset.setX(0);
        }
        if (indexHeight > unit) {
            m_exponent->setY(0);
            rootOffset.setY( indexHeight - unit );
        }
        else {
            m_exponent->setY( unit - indexHeight );
            rootOffset.setY(0);
        }
    }
    else {
        rootOffset.setX(0);
        rootOffset.setY(0);
    }

    setWidth( m_radicand->getWidth() + unit+unit/3+ rootOffset.x() + distX/2 );
    setHeight( m_radicand->getHeight() + distY*2 + rootOffset.y() );

    m_radicand->setX( rootOffset.x() + unit+unit/3 );
    m_radicand->setY( rootOffset.y() + distY );
    setBaseline(m_radicand->getBaseline() + m_radicand->getY());
}

void RootElement::moveLeft( FormulaCursor* cursor, BasicElement* from )
{
}

void RootElement::moveRight( FormulaCursor* cursor, BasicElement* from )
{
}

void RootElement::moveUp( FormulaCursor* cursor, BasicElement* from )
{
    if( from == m_radicand )
        cursor->moveCursorTo( m_exponent, -1 );
    else
        parentElement()->moveUp( cursor, from );
}

void RootElement::moveDown( FormulaCursor* cursor, BasicElement* from )
{
    if( from == m_exponent )
        cursor->moveCursorTo( m_radicand, 0 );
    else
        parentElement()->moveDown( cursor, this );
}

void RootElement::calcSizes( const ContextStyle& context,
                             ContextStyle::TextStyle tstyle, 
                             ContextStyle::IndexStyle istyle,
                             StyleAttributes& style )
{
}

void RootElement::draw( QPainter& painter, const LuPixelRect& r,
                        const ContextStyle& context,
                        ContextStyle::TextStyle tstyle,
                        ContextStyle::IndexStyle istyle,
                        StyleAttributes& style,
                        const LuPixelPoint& parentOrigin )
{
    LuPixelPoint myPos( parentOrigin.x()+getX(), parentOrigin.y()+getY() );
    //if ( !LuPixelRect( myPos.x(), myPos.y(), getWidth(), getHeight() ).intersects( r ) )
    //    return;

    m_radicand->draw( painter, r, context, tstyle,
                      context.convertIndexStyleLower( istyle ), style, myPos);
    if ( m_exponent ) {
        m_exponent->draw( painter, r, context,
                          context.convertTextStyleIndex(tstyle),
                          context.convertIndexStyleUpper(istyle), style, myPos);
    }

    luPixel x = myPos.x() + rootOffset.x();
    luPixel y = myPos.y() + rootOffset.y();
    //int distX = style.getDistanceX(tstyle);
    double factor = style.sizeFactor();
    luPixel distY = context.ptToPixelY( context.getThinSpace( tstyle, factor ) );
    luPixel unit = (m_radicand->getHeight() + distY)/ 3;

    painter.setPen( QPen( style.color(),
                          context.layoutUnitToPixelX( 2*context.getLineWidth( factor ) ) ) );
    painter.drawLine( context.layoutUnitToPixelX( x+unit/3 ),
                      context.layoutUnitToPixelY( y+unit+distY/3 ),
                      context.layoutUnitToPixelX( x+unit/2+unit/3 ),
                      context.layoutUnitToPixelY( myPos.y()+getHeight() ) );

    painter.setPen( QPen( style.color(),
                          context.layoutUnitToPixelY( context.getLineWidth( factor ) ) ) );

    painter.drawLine( context.layoutUnitToPixelX( x+unit+unit/3 ),
                      context.layoutUnitToPixelY( y+distY/3 ),
                      context.layoutUnitToPixelX( x+unit/2+unit/3 ),
                      context.layoutUnitToPixelY( myPos.y()+getHeight() ) );
    painter.drawLine( context.layoutUnitToPixelX( x+unit+unit/3 ),
                      context.layoutUnitToPixelY( y+distY/3 ),
                      context.layoutUnitToPixelX( x+unit+unit/3+m_radicand->getWidth() ),
                      context.layoutUnitToPixelY( y+distY/3 ) );
    painter.drawLine( context.layoutUnitToPixelX( x+unit/3 ),
                      context.layoutUnitToPixelY( y+unit+distY/2 ),
                      context.layoutUnitToPixelX( x ),
                      context.layoutUnitToPixelY( y+unit+unit/2 ) );
}

void RootElement::writeMathML( KoXmlWriter* writer, bool oasisFormat )
{
/*    if( m_exponent->elementType() == Basic )
        writer->startElement( oasisFormat ? "math:msqrt" : "msqrt" );
    else
        writer->startElement( oasisFormat ? "math:mroot" : "mroot" );

    writeMathMLAttributes( writer );
    m_radicand->writeMathML( writer, oasisFormat );
    if( m_exponent->elementType() != Basic )
        m_exponent->writeMathML( writer, oasisFormat );

    writer->endElement();*/
}

} // namespace FormulaShape
