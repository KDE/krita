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

#include <QPainter>
#include <QPen>

#include <kdebug.h>

#include "FormulaCursor.h"
#include "FormulaElement.h"
#include "kformulacommand.h"
#include "RootElement.h"
#include "SequenceElement.h"

namespace KFormula {

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

void RootElement::readMathML( const QDomElement& element )
{
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

/**
 * Calculates our width and height and
 * our children's parentPosition.
 */
void RootElement::calcSizes(const ContextStyle& style, ContextStyle::TextStyle tstyle, ContextStyle::IndexStyle istyle)
{
    m_radicand->calcSizes(style, tstyle,
		       style.convertIndexStyleLower(istyle));

    luPixel indexWidth = 0;
    luPixel indexHeight = 0;
    if ( m_exponent) {
	m_exponent->calcSizes(style,
			 style.convertTextStyleIndex(tstyle),
			 style.convertIndexStyleUpper(istyle));
        indexWidth = m_exponent->getWidth();
        indexHeight = m_exponent->getHeight();
    }

    luPixel distX = style.ptToPixelX( style.getThinSpace( tstyle ) );
    luPixel distY = style.ptToPixelY( style.getThinSpace( tstyle ) );
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

/**
 * Draws the whole element including its children.
 * The `parentOrigin' is the point this element's parent starts.
 * We can use our parentPosition to get our own origin then.
 */
void RootElement::draw( QPainter& painter, const LuPixelRect& r,
                        const ContextStyle& style,
                        ContextStyle::TextStyle tstyle,
                        ContextStyle::IndexStyle istyle,
                        const LuPixelPoint& parentOrigin )
{
    LuPixelPoint myPos( parentOrigin.x()+getX(), parentOrigin.y()+getY() );
    //if ( !LuPixelRect( myPos.x(), myPos.y(), getWidth(), getHeight() ).intersects( r ) )
    //    return;

    m_radicand->draw(painter, r, style, tstyle,
		  style.convertIndexStyleLower(istyle), myPos);
    if ( m_exponent ) {
        m_exponent->draw(painter, r, style,
		    style.convertTextStyleIndex(tstyle),
		    style.convertIndexStyleUpper(istyle), myPos);
    }

    luPixel x = myPos.x() + rootOffset.x();
    luPixel y = myPos.y() + rootOffset.y();
    //int distX = style.getDistanceX(tstyle);
    luPixel distY = style.ptToPixelY( style.getThinSpace( tstyle ) );
    luPixel unit = (m_radicand->getHeight() + distY)/ 3;

    painter.setPen( QPen( style.getDefaultColor(),
                          style.layoutUnitToPixelX( 2*style.getLineWidth() ) ) );
    painter.drawLine( style.layoutUnitToPixelX( x+unit/3 ),
                      style.layoutUnitToPixelY( y+unit+distY/3 ),
                      style.layoutUnitToPixelX( x+unit/2+unit/3 ),
                      style.layoutUnitToPixelY( myPos.y()+getHeight() ) );

    painter.setPen( QPen( style.getDefaultColor(),
                          style.layoutUnitToPixelY( style.getLineWidth() ) ) );

    painter.drawLine( style.layoutUnitToPixelX( x+unit+unit/3 ),
                      style.layoutUnitToPixelY( y+distY/3 ),
                      style.layoutUnitToPixelX( x+unit/2+unit/3 ),
                      style.layoutUnitToPixelY( myPos.y()+getHeight() ) );
    painter.drawLine( style.layoutUnitToPixelX( x+unit+unit/3 ),
                      style.layoutUnitToPixelY( y+distY/3 ),
                      style.layoutUnitToPixelX( x+unit+unit/3+m_radicand->getWidth() ),
                      style.layoutUnitToPixelY( y+distY/3 ) );
    painter.drawLine( style.layoutUnitToPixelX( x+unit/3 ),
                      style.layoutUnitToPixelY( y+unit+distY/2 ),
                      style.layoutUnitToPixelX( x ),
                      style.layoutUnitToPixelY( y+unit+unit/2 ) );
}

/**
 * Enters this element while moving to the left starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or to the left of it.
 */
void RootElement::moveLeft(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveLeft(cursor, this);
    }
    else {
        bool linear = cursor->getLinearMovement();
        if (from == getParent()) {
            m_radicand->moveLeft(cursor, this);
        }
        else if (from == m_radicand) {
            if (linear && m_exponent) {
                m_radicand->moveLeft(cursor, this);
            }
            else {
                getParent()->moveLeft(cursor, this);
            }
        }
        else {
            getParent()->moveLeft(cursor, this);
        }
    }
}

/**
 * Enters this element while moving to the right starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or to the right of it.
 */
void RootElement::moveRight(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveRight(cursor, this);
    }
    else {
        bool linear = cursor->getLinearMovement();
        if (from == getParent()) {
            if (linear && m_exponent) {
                m_exponent->moveRight(cursor, this);
            }
            else {
                m_radicand->moveRight(cursor, this);
            }
        }
        else if (from == m_radicand) {
            m_radicand->moveRight(cursor, this);
        }
        else {
            getParent()->moveRight(cursor, this);
        }
    }
}

/**
 * Enters this element while moving up starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or above it.
 */
void RootElement::moveUp(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveUp(cursor, this);
    }
    else {
        if (from == getParent()) {
            m_radicand->moveRight(cursor, this);
        }
        else if (from == m_radicand) {
            if (m_exponent) {
                m_radicand->moveRight(cursor, this);
            }
            else {
                getParent()->moveUp(cursor, this);
            }
        }
        else {
            getParent()->moveUp(cursor, this);
        }
    }
}

/**
 * Enters this element while moving down starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or below it.
 */
void RootElement::moveDown(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveDown(cursor, this);
    }
    else {
        if (from == getParent()) {
            if (m_exponent) {
                m_radicand->moveRight(cursor, this);
            }
            else {
                m_radicand->moveRight(cursor, this);
            }
        }
        else if (from == m_radicand) {
            m_radicand->moveRight(cursor, this);
        }
        else {
            getParent()->moveDown(cursor, this);
        }
    }
}

/**
 * Reinserts the index if it has been removed.
 */
void RootElement::insert(FormulaCursor* cursor,
                         QList<BasicElement*>& newChildren,
                         Direction direction)
{
    if (cursor->getPos() == upperLeftPos) {
        m_radicand = static_cast<SequenceElement*>(newChildren.takeAt(0));
        m_radicand->setParent(this);

        if (direction == beforeCursor) {
            m_radicand->moveLeft(cursor, this);
        }
        else {
            m_radicand->moveRight(cursor, this);
        }
        cursor->setSelection(false);
        //formula()->changed();
    }
}

/**
 * Removes all selected children and returns them. Places the
 * cursor to where the children have been.
 *
 * We remove ourselve if we are requested to remove our content.
 */
void RootElement::remove(FormulaCursor* cursor,
                         QList<BasicElement*>& removedChildren,
                         Direction direction)
{
    switch (cursor->getPos()) {
    case contentPos:
        getParent()->selectChild(cursor, this);
        getParent()->remove(cursor, removedChildren, direction);
        break;
    case upperLeftPos:
        removedChildren.append(m_exponent);
        //formula()->elementRemoval(m_exponent);
        m_exponent = 0;
        cursor->setTo(this, upperLeftPos);
        //formula()->changed();
        break;
    }
}


// main child
//
// If an element has children one has to become the main one.

SequenceElement* RootElement::getMainChild()
{
    return 0; //m_radicand;
}

/**
 * Sets the cursor to select the child. The mark is placed before,
 * the position behind it.
 */
void RootElement::selectChild(FormulaCursor* cursor, BasicElement* child)
{
    if (child == m_radicand) {
        cursor->setTo(this, contentPos);
    }
    else if (child == m_exponent) {
        cursor->setTo(this, upperLeftPos);
    }
}


void RootElement::moveToIndex(FormulaCursor* cursor, Direction direction)
{
    if (m_exponent) {
        if (direction == beforeCursor) {
            m_exponent->moveLeft(cursor, this);
        }
        else {
            m_exponent->moveRight(cursor, this);
        }
    }
}

void RootElement::setToIndex(FormulaCursor* cursor)
{
    cursor->setTo(this, upperLeftPos);
}


/**
 * Appends our attributes to the dom element.
 */
void RootElement::writeDom(QDomElement element)
{
    BasicElement::writeDom(element);

    QDomDocument doc = element.ownerDocument();

    QDomElement con = doc.createElement("CONTENT");
    con.appendChild(m_radicand->getElementDom(doc));
    element.appendChild(con);

    if(m_exponent) {
        QDomElement ind = doc.createElement("ROOTINDEX");
        ind.appendChild(m_exponent->getElementDom(doc));
        element.appendChild(ind);
    }
}

/**
 * Reads our attributes from the element.
 * Returns false if it failed.
 */
bool RootElement::readAttributesFromDom(QDomElement element)
{
    return BasicElement::readAttributesFromDom(element);
}

/**
 * Reads our content from the node. Sets the node to the next node
 * that needs to be read.
 * Returns false if it failed.
 */
bool RootElement::readContentFromDom(QDomNode& node)
{
/*    if (!BasicElement::readContentFromDom(node)) {
        return false;
    }

    if ( !buildChild( m_radicand, node, "CONTENT" ) ) {
        kWarning( DEBUGID ) << "Empty content in RootElement." << endl;
        return false;
    }
    node = node.nextSibling();

    if ( node.nodeName().toUpper() == "ROOTINDEX" ) {
        if ( !buildChild( m_exponent=new SequenceElement( this ), node, "ROOTINDEX" ) ) {
            return false;
        }
    }
    // backward compatibility
    else if ( node.nodeName().toUpper() == "INDEX" ) {
        if ( !buildChild( m_exponent=new SequenceElement( this ), node, "INDEX" ) ) {
            return false;
        }
    }
    node = node.nextSibling();
*/
    return true;
}

} // namespace KFormula
