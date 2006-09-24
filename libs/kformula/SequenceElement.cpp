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

#include "SequenceElement.h"
#include "ElementFactory.h"
#include "FormulaCursor.h"
#include <KoXmlWriter.h>
#include <QPainter>

namespace KFormula {

SequenceElement::SequenceElement( BasicElement* parent ) : BasicElement( parent )
{
}

SequenceElement::~SequenceElement()
{
}

void SequenceElement::paint( QPainter& painter ) const
{
    // just paint all children, a sequence has no visual representation
    foreach( BasicElement* childElement, m_sequenceElements )
        childElement->paint( painter );
}

void SequenceElement::calculateSize()
{
    if( m_sequenceElements.isEmpty() )  // do not do anything if there are no children
        return;

    QPointF origin = boundingRect().topLeft();
    double width = 0.0;
    double topToBaseline = 0.0;
    double baselineToBottom = 0.0;
    foreach( BasicElement* child, m_sequenceElements  )  // iterate through the children
    {
        child->setOrigin( origin );    // set their origin
        child->calculateSize();        // calculate their size
        topToBaseline = qMax( topToBaseline, child->baseLine() );
        baselineToBottom = qMax( baselineToBottom, child->height()-child->baseLine() );
	width += child->width();       // add their width
	origin += QPointF( width, 0 ); // and move the current origin
    }

    setWidth( width );
    setHeight( topToBaseline + baselineToBottom );
    setBaseline( topToBaseline );
}

void SequenceElement::insertChild( FormulaCursor* cursor, BasicElement* child )
{
    Q_ASSERT( cursor->position() > m_sequenceElements.count() );
    
    m_sequenceElements.insert( cursor->position(), child );
}
   
void SequenceElement::removeChild( BasicElement* element )
{
    int i = m_sequenceElements.indexOf( element );
    Q_ASSERT( i == -1 );
    delete m_sequenceElements.takeAt( i );
}

void SequenceElement::moveLeft( FormulaCursor* cursor, BasicElement* from )
{
    // the parent element enters the seqeunce from the left
    if( from == parentElement() )
        cursor->setCursorTo( this, m_sequenceElements.count() );
    else   // the cursor comes from a child element
        cursor->setCursorTo( this, m_sequenceElements.indexOf( from ) );
}

void SequenceElement::moveRight( FormulaCursor* cursor, BasicElement* from )
{
    // the parent element enters the seqeunce from the right 
    if( from == parentElement() )
        cursor->setCursorTo( this, 0 );
    else   // the cursor comes from a child element
        cursor->setCursorTo( this, m_sequenceElements.indexOf( from )+1 );
}

const QList<BasicElement*> SequenceElement::childElements()
{
    return m_sequenceElements;
}

BasicElement* SequenceElement::childAt( int i )
{
    return m_sequenceElements[ i ];
}

void SequenceElement::readMathML( const QDomElement& element )
{
    readMathMLAttributes( element );
   
    BasicElement* tmpElement = 0;
    QDomElement tmp = element.firstChildElement();
    while( !tmp.isNull() )
    {
        tmpElement = ElementFactory::createElement( tmp.tagName(), this );
        m_sequenceElements << tmpElement;
	tmpElement->readMathML( tmp );
	tmp = tmp.nextSiblingElement();
    }
}

void SequenceElement::writeMathML( KoXmlWriter* writer, bool oasisFormat )
{
    writer->startElement( oasisFormat ? "math:mrow" : "mrow" );
    writeMathMLAttributes( writer );

    foreach( BasicElement* tmpChild, m_sequenceElements )
        tmpChild->writeMathML( writer, oasisFormat );
   
    writer->endElement();
}








/**
 * Stores the given m_sequenceElementss dom in the element.
 */
void SequenceElement::getChildrenDom( QDomDocument& doc, QDomElement elem,
                                     uint from, uint to)
{
    for (uint i = from; i < to; i++) {
        QDomElement tmpEleDom=m_sequenceElements.at(i)->getElementDom(doc);
	elem.appendChild(tmpEleDom);
    }
}


/**
 * Builds elements from the given node and its siblings and
 * puts them into the list.
 * Returns false if an error occures.
 */
bool SequenceElement::buildChildrenFromDom(QList<BasicElement*>& list, QDomNode n)
{
    while (!n.isNull()) {
        if (n.isElement()) {
            QDomElement e = n.toElement();
            BasicElement* child = 0;
            QString tag = e.tagName().toUpper();

//            child = createElement(tag);
            if (child != 0) {
                child->setParent(this);
                if (child->buildFromDom(e)) {
                    list.append(child);
                }
                else {
                    delete child;
                    return false;
                }
            }
            else {
                return false;
            }
        }
        n = n.nextSibling();
    }
    //parse();
    return true;
}

void SequenceElement::writeDom(QDomElement element)
{
    BasicElement::writeDom(element);

    uint count = m_sequenceElements.count();
    QDomDocument doc = element.ownerDocument();
    getChildrenDom(doc, element, 0, count);
}

bool SequenceElement::readAttributesFromDom(QDomElement element)
{
    if (!BasicElement::readAttributesFromDom(element)) {
        return false;
    }
    return true;
}

bool SequenceElement::readContentFromDom(QDomNode& node)
{
    if (!BasicElement::readContentFromDom(node)) {
        return false;
    }

    return buildChildrenFromDom(m_sequenceElements, node);
}

} // namespace KFormula
