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

#include "FractionElement.h"
#include "ElementFactory.h"
#include "FormulaCursor.h"
#include "AttributeManager.h"
#include <KoXmlWriter.h>
#include <QPainter>

namespace FormulaShape {

FractionElement::FractionElement( BasicElement* parent ) : BasicElement( parent )
{
    m_numerator = new BasicElement( this );
    m_denominator = new BasicElement( this );
}

FractionElement::~FractionElement()
{
    delete m_numerator;
    delete m_denominator;
}

void FractionElement::paint( QPainter& painter, const AttributeManager* am )
{
    QPen pen( painter.pen() );
    pen.setWidthF( am->valueOf( "linethickness" ).toDouble() );

    painter.save();
    painter.setPen( pen );                           // set the line width
    painter.drawLine( m_fractionLine );              // draw the line
    painter.restore();
}

void FractionElement::layout( const AttributeManager* am )
{
    if( am->valueOf( "bevelled" ).toBool() )
    {
        layoutBevelledFraction( am );
        return;
    }

    QPointF numeratorOrigin;
    QPointF denominatorOrigin;
    double linethickness = am->valueOf( "linethickness" ).toDouble();
    double distY = am->mathSpaceValue( "thinmathspace" );
    Align numalign = am->alignValueOf( "numalign" ); 
    Align denomalign = am->alignValueOf( "denomalign" ); 
    
    setWidth( qMax( m_numerator->width(), m_denominator->width() ) );
    setHeight( m_numerator->height() + m_denominator->height() +
               linethickness + 2*distY );
    setBaseLine( m_numerator->height() + distY + 0.5*linethickness );
    
    if( numalign == Left )
        numeratorOrigin.setX( 0.0 );
    else if( numalign == Right )
        numeratorOrigin.setX( width() - m_numerator->width() );
    else
	numeratorOrigin.setX( ( width() - m_numerator->width() ) / 2 );

    if( denomalign == Left )
        denominatorOrigin.setX( 0.0 );
    else if( denomalign == Right )
        denominatorOrigin.setX( width() - m_denominator->width() );
    else
	denominatorOrigin.setX( ( width() - m_denominator->width() ) / 2 );

    m_numerator->setOrigin( numeratorOrigin );
    m_denominator->setOrigin( denominatorOrigin );
    m_fractionLine = QLineF( QPointF( 0.0, baseLine() ),
                             QPointF( width(), baseLine() ) );
}

void FractionElement::layoutBevelledFraction( const AttributeManager* am )
{
    // the shown line should have a width that has 1/3 of the height
    // the line is heigher as the content by 2*thinmathspace = 2*borderY

    double borderY = am->valueOf( "thinmathspace" ).toDouble();
    setHeight( m_numerator->height() + m_denominator->height() + 2*borderY );
    setWidth( m_numerator->width() + m_denominator->width() + height()/3 );
    setBaseLine( height()/2 );

    m_numerator->setOrigin( QPointF( 0.0, borderY ) );
    m_denominator->setOrigin( QPointF( width()-m_denominator->width(),
                                       borderY+m_numerator->height() ) );
    m_fractionLine = QLineF( QPointF( m_numerator->width(), height() ),
                             QPointF( width()-m_denominator->width(), 0.0 ) );
}

const QList<BasicElement*> FractionElement::childElements()
{
    QList<BasicElement*> list;
    list << m_denominator << m_numerator;
    return list;
}

void FractionElement::insertChild( FormulaCursor* cursor, BasicElement* child )
{
    BasicElement* tmp = cursor->currentElement();
    if( tmp == m_numerator && m_numerator->elementType() == Basic )
        m_numerator = child;
    else if( tmp == m_denominator && m_denominator->elementType() == Basic )
        m_denominator = child;

    delete tmp;       // finally delete the old BasicElement
}
   
void FractionElement::removeChild( BasicElement* element )
{
    if( element == m_numerator )         
    {
        delete m_numerator;                      // delete the numerator and
        m_numerator = new BasicElement( this );  // assign a new empty BasicElement   
    }
    else if( element == m_denominator )
    {
        delete m_denominator;
        m_denominator = new BasicElement( this );
    }
}

void FractionElement::moveUp( FormulaCursor* cursor, BasicElement* from )
{
    if( from == m_denominator )
        m_numerator->moveUp( cursor, this );
    else
        parentElement()->moveUp( cursor, this );
}

void FractionElement::moveDown(FormulaCursor* cursor, BasicElement* from)
{
    if( from == m_numerator )
        m_denominator->moveDown( cursor, this );
    else
        parentElement()->moveDown( cursor, this );
}

void FractionElement::readMathML( const QDomElement& element )
{
    readMathMLAttributes( element );

    if( element.childNodes().count() != 2 ) // a fraction element has always two children
	return;
    
    QDomElement tmp = element.firstChildElement();  // create first the numerator
    m_numerator = ElementFactory::createElement( tmp.tagName(), this );
    m_numerator->readMathML( tmp );
    
    tmp = element.lastChildElement();               // then the denominator
    m_denominator = ElementFactory::createElement( tmp.tagName(), this );
    m_denominator->readMathML( tmp );
}

void FractionElement::writeMathML( KoXmlWriter* writer, bool oasisFormat ) const
{
    writer->startElement( oasisFormat ? "math:mfrac": "mfrac" );
    writeMathMLAttributes( writer );
   
    m_numerator->writeMathML( writer, oasisFormat );
    m_denominator->writeMathML( writer, oasisFormat );

    writer->endElement();
}

ElementType FractionElement::elementType() const
{
    return Fraction;
}

} // namespace FormulaShape
