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



#include <QPaintDevice>
#include <QStack>
#include <QKeyEvent>


#include <kcommand.h>
#include <kdebug.h>
#include <klocale.h>

#include "MatrixDialog.h"
#include "BracketElement.h"

#include "FormulaElement.h"
#include "FractionElement.h"
#include "kformulacommand.h"
#include "FormulaContainer.h"
#include "MatrixElement.h"
#include "RootElement.h"
#include "SpaceElement.h"
#include "symboltable.h"
#include "TextElement.h"
#include "MatrixRowElement.h"

namespace KFormula {

SequenceElement::SequenceElement( BasicElement* parent ) : BasicElement(parent),
							   textSequence(true),
							   singlePipe(true)
{
}

SequenceElement::~SequenceElement()
{
}

void SequenceElement::paint( QPainter& painter ) const
{
    // just paint all children, a sequence has no visual representation
    foreach( BasicElement* childElement, m_sequenceChildren )
        childElement->paint( painter );
}

void SequenceElement::calculateSize()
{
    if( m_sequenceChildren.isEmpty() )  // do not do anything if there are no children
        return;

    QPointF origin = boundingRect().topLeft();
    double width = 0.0;
    double topToBaseline = 0.0;
    double baselineToBottom = 0.0;
    foreach( BasicElement* child, m_sequenceChildren  )  // iterate through the children
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
    Q_ASSERT( cursor->position() > m_sequenceChildren.count() );
    
    m_sequenceChildren.insert( cursor->position(), child );
}
   
void SequenceElement::removeChild( BasicElement* element )
{
    int i = m_sequenceChildren.indexOf( element );
    Q_ASSERT( i == -1 );
    delete m_sequenceChildren.takeAt( i );
}

void SequenceElement::moveLeft( FormulaCursor* cursor, BasicElement* from )
{
    // the parent element enters the seqeunce from the left
    if( from == parentElement() )
        cursor->setCursorTo( this, m_sequenceChildren.count() );
    else   // the cursor comes from a child element
        cursor->setCursorTo( this, m_sequenceChildren.indexOf( from ) );
}

void SequenceElement::moveRight( FormulaCursor* cursor, BasicElement* from )
{
    // the parent element enters the seqeunce from the right 
    if( from == parentElement() )
        cursor->setCursorTo( this, 0 );
    else   // the cursor comes from a child element
        cursor->setCursorTo( this, m_sequenceChildren.indexOf( from )+1 );
}

const QList<BasicElement*> SequenceElement::childElements()
{
    return m_sequenceChildren;
}

BasicElement* SequenceElement::childAt( int i )
{
    return m_sequenceChildren[ i ];
}

void SequenceElement::readMathML( const QDomElement& element )
{
    readMathMLAttributes( element );
   
    BasicElement* tmpElement = 0;
    QDomElement tmp = element.firstChildElement();
    while( !tmp.isNull() )
    {
        tmpElement = ElementFactory::createElement( tmp.tagName(), this );
        m_sequenceChildren << tmpElement;
	tmpElement->readMathML( tmp );
	tmp = tmp.nextSiblingElement();
    }
}

void SequenceElement::writeMathML( KoXmlWriter* writer, bool oasisFormat )
{
    writer->startElement( oasisFormat ? "math:mrow" : "mrow" );
    writeMathMLAttributes( writer );

    foreach( BasicElement* tmpChild, m_sequenceChildren )
        tmpChild->writeMathML( writer, oasisFormat );
   
    writer->endElement();
}










bool SequenceElement::readOnly( const FormulaCursor* ) const
{
    return getParent()->readOnly( this );
}




bool SequenceElement::isEmpty()
{
/*    uint count = m_sequenceChildren.count();
    for (uint i = 0; i < count; i++) {
        BasicElement* child = m_sequenceChildren.at(i);
        if (!child->isInvisible()) {
            return false;
        }
    }*/
    return true;
}





void SequenceElement::setChildrenPositions()
{
    foreach( BasicElement* child, m_sequenceChildren )
        child->setY(getBaseline() - child->getBaseline());
}




/**
 * Draws the whole element including its m_sequenceChildren.
 * The `parentOrigin' is the point this element's parent starts.
 * We can use our parentPosition to get our own origin then.
 */
void SequenceElement::draw( QPainter& painter, const LuPixelRect& r,
                            const ContextStyle& context,
                            ContextStyle::TextStyle tstyle,
                            ContextStyle::IndexStyle istyle,
                            const LuPixelPoint& parentOrigin )
{
/*    QPointF myPos( parentOrigin.x() + getX(), parentOrigin.y() + getY() );

    if( m_sequenceChildren.isEmpty() )
        drawEmptyRect( painter, context, myPos );
    else
    {
        foreach( BasicElement* child, m_sequenceChildren )
	{
            if (!child->isInvisible())
	    {
                child->draw(painter, r, context, tstyle, istyle, myPos);

                // Each starting element draws the whole token
                // This only concerns TextElements.
               // ElementType* token = child->getElementType();
               // if ( token )
               //     child += token->end() - token->start();
            }
        }
    }*/
}

luPixel SequenceElement::getChildPosition( const ContextStyle& context, int child )
{
    if (child < m_sequenceChildren.count())
        return m_sequenceChildren.at(child)->getX();
    else
    {
        if( !m_sequenceChildren.isEmpty() )
            return m_sequenceChildren.at(child-1)->getX() + m_sequenceChildren.at(child-1)->getWidth();
        else 
            return context.ptToLayoutUnitPixX( 2 );
    }
}

/*void SequenceElement::moveWordLeft(FormulaCursor* cursor)
{
    uint pos = cursor->getPos();
    if (pos > 0) {
        ElementType* type = m_sequenceChildren.at(pos-1)->getElementType();
        if (type != 0) {
            cursor->setTo(this, type->start());
        }
    }
    else {
        moveLeft(cursor, this);
    }
}*/


/*void SequenceElement::moveWordRight(FormulaCursor* cursor)
{
    int pos = cursor->getPos();
    if (pos < m_sequenceChildren.count()) {
        ElementType* type = m_sequenceChildren.at(pos)->getElementType();
        if (type != 0) {
            cursor->setTo(this, type->end());
        }
    }
    else {
        moveRight(cursor, this);
    }
}*/

/**
 * Sets the cursor inside this element to its start position.
 * For most elements that is the main child.
 */
void SequenceElement::goInside(FormulaCursor* cursor)
{
    cursor->setSelecting(false);
    cursor->setCursorTo(this, 0);
}

/**
 * Inserts all new m_sequenceChildren at the cursor position. Places the
 * cursor according to the direction. The inserted elements will
 * be selected.
 *
 * The list will be emptied but stays the property of the caller.
 */
void SequenceElement::insert(FormulaCursor* cursor,
                             QList<BasicElement*>& newChildren,
                             Direction direction)
{
/*    int pos = cursor->getPos();
    int count = newChildren.count();
    for (int i = 0; i < count; i++) {
        BasicElement* child = newChildren.takeAt(0);
        child->setParent(this);
        m_sequenceChildren.insert(pos+i, child);
    }
    if (direction == beforeCursor) {
        cursor->setTo(this, pos+count, pos);
    }
    else {
        cursor->setTo(this, pos, pos+count);
    }
*/
    //formula()->changed();
    //parse();
}


/**
 * Removes all selected m_sequenceChildren and returns them. Places the
 * cursor to where the m_sequenceChildren have been.
 *
 * The ownership of the list is passed to the caller.
 */
void SequenceElement::remove(FormulaCursor* cursor,
                             QList<BasicElement*>& removedChildren,
                             Direction direction)
{
/*    if (cursor->isSelection()) {
        int from = cursor->getSelectionStart();
        int to = cursor->getSelectionEnd();
        for (int i = from; i < to; i++) {
            removeChild(removedChildren, from);
        }
        cursor->setTo(this, from);
        cursor->setSelection(false);
    }
    else {
        if (direction == beforeCursor) {
            int pos = cursor->getPos() - 1;
            if (pos >= 0) {
                while (pos >= 0) {
                    BasicElement* child = m_sequenceChildren.at(pos);
                    formula()->elementRemoval(child);
                    m_sequenceChildren.takeAt(pos);
                    removedChildren.prepend(child);
                    if (!child->isInvisible()) {
                        break;
                    }
                    pos--;
                }
                cursor->setTo(this, pos);
                formula()->changed();
            }
        }
        else {
            int pos = cursor->getPos();
            if (pos < m_sequenceChildren.count()) {
                while (pos < m_sequenceChildren.count()) {
                    BasicElement* child = m_sequenceChildren.at(pos);
                    formula()->elementRemoval(child);
                    m_sequenceChildren.takeAt(pos);
                    removedChildren.append(child);
                    if (!child->isInvisible()) {
                        break;
                    }
                }
                // It is necessary to set the cursor to its old
                // position because it got a notification and
                // moved to the beginning of this sequence.
                cursor->setTo(this, pos);
                formula()->changed();
            }
        }
    }*/
    //parse();
}

/**
 * Moves the cursor to a normal place where new elements
 * might be inserted.
 */
void SequenceElement::normalize(FormulaCursor* cursor, Direction)
{
    cursor->setSelecting(false);
}


/**
 * Returns the child at the cursor.
 * Does not care about the selection.
 */
BasicElement* SequenceElement::getChild( FormulaCursor* cursor, Direction direction )
{
/*    if ( direction == beforeCursor ) {
        if ( cursor->getPos() > 0 ) {
            return m_sequenceChildren.at( cursor->getPos() - 1 );
        }
    }
    else {
        if ( cursor->getPos() < qRound( m_sequenceChildren.count() ) ) {
            return m_sequenceChildren.at( cursor->getPos() );
        }
    }
*/ // TODO
    return 0;
}


/**
 * Sets the cursor to select the child. The mark is placed before,
 * the position behind it.
 */
void SequenceElement::selectChild(FormulaCursor* cursor, BasicElement* child)
{
/*    int pos = m_sequenceChildren.indexOf(child);
    if (pos > -1) {
        cursor->setTo(this, pos+1, pos);
    }*/
}

bool SequenceElement::isFirstOfToken( BasicElement* child )
{
    return true;//( child->getElementType() != 0 ) && isChildNumber( child->getElementType()->start(), child );
}

void SequenceElement::childWillVanish(FormulaCursor* cursor, BasicElement* child)
{
/*    int childPos = m_sequenceChildren.indexOf(child);
    if (childPos > -1) {
        int pos = cursor->getPos();
        if (pos > childPos) {
            pos--;
        }
        int mark = cursor->getMark();
        if (mark > childPos) {
            mark--;
        }
        cursor->setTo(this, pos, mark);
    }*/
}


/**
 * Selects all m_sequenceChildren. The cursor is put behind, the mark before them.
 */
void SequenceElement::selectAllChildren(FormulaCursor* cursor)
{
//    cursor->setTo(this, m_sequenceChildren.count(), 0);
}

bool SequenceElement::onlyTextSelected( FormulaCursor* cursor )
{
 /*   if ( cursor->hasSelection() ) {
        uint from = qMin( cursor->getPos(), cursor->getMark() );
        uint to = qMax( cursor->getPos(), cursor->getMark() );
        for ( uint i = from; i < to; i++ ) {
            BasicElement* element = childAt( i );
            if ( element->getCharacter() == QChar::Null ) {
                return false;
            }
        }
    }
    return true;*/
    return false; // TODO
}







/**
 * Stores the given m_sequenceChildrens dom in the element.
 */
void SequenceElement::getChildrenDom( QDomDocument& doc, QDomElement elem,
                                     uint from, uint to)
{
    for (uint i = from; i < to; i++) {
        QDomElement tmpEleDom=m_sequenceChildren.at(i)->getElementDom(doc);
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

    uint count = m_sequenceChildren.count();
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

    return buildChildrenFromDom(m_sequenceChildren, node);
}

} // namespace KFormula
