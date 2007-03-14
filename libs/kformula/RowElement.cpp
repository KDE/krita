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

#include "RowElement.h"
#include "ElementFactory.h"
#include "FormulaCursor.h"
#include <KoXmlWriter.h>
#include <QPainter>

namespace FormulaShape {

RowElement::RowElement( BasicElement* parent ) : BasicElement( parent )
{}

RowElement::~RowElement()
{
    qDeleteAll( m_rowElements );     // Delete all children
}

void RowElement::paint( QPainter& , const AttributeManager* )
{ /* There is nothing to paint but BasicElement::paint should not be called */ }

void RowElement::layout( const AttributeManager* am )
{
    Q_UNUSED( am )          // there are no attributes that can be processed here

    if( m_rowElements.isEmpty() )  // do not do anything if there are no children
        return;

    QPointF origin = boundingRect().topLeft();
    double width = 0.0;
    double topToBaseline = 0.0;
    double baselineToBottom = 0.0;
    foreach( BasicElement* child, m_rowElements  )  // iterate through the children
    {
        child->setOrigin( origin );    // set their origin
        topToBaseline = qMax( topToBaseline, child->baseLine() );
        baselineToBottom = qMax( baselineToBottom, child->height()-child->baseLine() );
        width += child->width();       // add their width
        origin += QPointF( width, 0 ); // and move the current origin
    }

    setWidth( width );
    setHeight( topToBaseline + baselineToBottom );
    setBaseLine( topToBaseline );
}

void RowElement::insertChild( FormulaCursor* cursor, BasicElement* child )
{
    Q_ASSERT( cursor->position() > m_rowElements.count() );
    m_rowElements.insert( cursor->position(), child );
}

void RowElement::removeChild( BasicElement* element )
{
    int i = m_rowElements.indexOf( element );
    Q_ASSERT( i == -1 );
    delete m_rowElements.takeAt( i );
}

void RowElement::moveLeft( FormulaCursor* cursor, BasicElement* from )
{
    if( from == parentElement() )  // parent element enters the seqeunce from the left
        cursor->moveCursorTo( this, m_rowElements.count() );
    else if( from == this )        // moveLeft was invoked in this element
        m_rowElements[ cursor->position()-1 ]->moveLeft( cursor, this );
    else                           // the cursor comes from a child element
        cursor->moveCursorTo( this, m_rowElements.indexOf( from ) );
}

void RowElement::moveRight( FormulaCursor* cursor, BasicElement* from )
{
    if( from == parentElement() )  // parent element enters the seqeunce from the right
        cursor->moveCursorTo( this, 0 );
    else if( from == this )        // moveRight was invoked in this element
        m_rowElements[ cursor->position() ]->moveRight( cursor, this );
    else                           // the cursor comes from a child element
        cursor->moveCursorTo( this, m_rowElements.indexOf( from )+1 );
}

const QList<BasicElement*> RowElement::childElements()
{
    return m_rowElements;
}

BasicElement* RowElement::childAt( int i )
{
    return m_rowElements[ i ];
}

void RowElement::readMathML( const KoXmlElement& element )
{
    readMathMLAttributes( element );

    BasicElement* tmpElement = 0;
    QDomElement tmp = element.firstChildElement();
    while( !tmp.isNull() )    // for each child element, create a element
    {
        tmpElement = ElementFactory::createElement( tmp.tagName(), this );
        m_rowElements << tmpElement;
        tmpElement->readMathML( tmp );       // and read the MathML
        tmp = tmp.nextSiblingElement();
    }
}

void RowElement::writeMathML( KoXmlWriter* writer, bool oasisFormat ) const
{
    writer->startElement( oasisFormat ? "math:mrow" : "mrow" );
    writeMathMLAttributes( writer );                            // write the attributes

    foreach( BasicElement* tmpChild, m_rowElements )       // just write all
        tmpChild->writeMathML( writer, oasisFormat );           // children elements

    writer->endElement();
}

} // namespace FormulaShape

