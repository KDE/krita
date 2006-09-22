/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

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

#include <kdebug.h>
#include <assert.h>

#include "FormulaCursor.h"


#include "SequenceElement.h"

namespace KFormula {

FormulaCursor::FormulaCursor( BasicElement* element )
              : m_wordMovement( false ),
                m_selecting( false )
{
    m_currentElement = element;
    m_positionInElement = 0;
}

BasicElement* FormulaCursor::currentElement() const
{
    return m_currentElement;
}

bool FormulaCursor::hasSelection() const
{
    return m_selecting;
}

void FormulaCursor::setSelecting( bool selecting )
{
    m_selecting = selecting;
}

void FormulaCursor::setWordMovement( bool wordMovement )
{
    m_wordMovement = wordMovement;
}

void FormulaCursor::paint( QPainter& painter ) const
{
    QPointF top = m_currentElement->boundingRect().topLeft();
    
    if( m_currentElement->elementType() == Basic )  // set the cursor in the middle
        top += QPointF( m_currentElement->width()/2, 0 );
    else
    { 
        // determine the x coordinate by summing up the elements' width before the cursor
        for( int i = 0; i < m_positionInElement; i++ )
            top += QPointF( m_currentElement->childElements().value(i)->width(), 0 );
    }
    
    QPointF bottom = top + QPointF( 0, m_currentElement->height() );
    painter.drawLine( top, bottom );
}

void FormulaCursor::setCursorTo( BasicElement* current, int position )
{
    m_currentElement = current;
    m_positionInElement = position;
}

void FormulaCursor::moveLeft()
{
    if( m_wordMovement && !isHome() )                 // move only if the 
        m_positionInElement--;                        // positionallows it
    else 
        m_currentElement->moveLeft( this, m_currentElement );
}

void FormulaCursor::moveRight()
{
    if( m_wordMovement )
    {
        if( !isEnd() )
            m_positionInElement++;
        else
            m_currentElement->moveRight( this, m_currentElement );
    }
    else 
        m_currentElement->moveRight( this, m_currentElement );
}

void FormulaCursor::moveUp()
{
    m_currentElement->moveUp( this, m_currentElement );
}

void FormulaCursor::moveDown()
{
    m_currentElement->moveDown( this, m_currentElement );
}

void FormulaCursor::moveHome()
{
    m_positionInElement = 0;
}

void FormulaCursor::moveEnd()
{
    if( m_currentElement->elementType() == Sequence )
	m_positionInElement = m_currentElement->childElements().count();
    else
        m_positionInElement = 1;
}

bool FormulaCursor::isHome() const
{
    return m_positionInElement == 0;
}

bool FormulaCursor::isEnd() const
{
    if( currentElement()->elementType() == Sequence )
        return ( m_positionInElement == m_currentElement->childElements().count() );
    else
	return ( m_positionInElement == 1 );
}







void FormulaCursor::setTo( BasicElement* element, int cursor, int mark)
{
    hasChangedFlag = true;
    m_currentElement = element;
    m_positionInElement = cursor;
    if ((mark == -1) && selectionFlag) {
        return;
    }
    if (mark != -1) {
        setSelecting(true);
    }
    markPos = mark;
}


void FormulaCursor::setPos(int pos)
{
    hasChangedFlag = true;
    m_positionInElement = pos;
}

void FormulaCursor::setMark(int mark)
{
    hasChangedFlag = true;
    markPos = mark;
}

void FormulaCursor::handleSelectState(int flag)
{
    if (flag & SelectMovement) {
        if (!hasSelection()) {
            setMark(getPos());
            setSelecting(true);
        }
    }
    else {
        setSelecting(false);
    }
}

/**
 * Moves the cursor inside the element. Selection is turned off.
 */
void FormulaCursor::goInsideElement(BasicElement* element)
{
    element->goInside(this);
}


/**
 * Moves the cursor to a normal position. That is somewhere
 * inside a SequenceElement.
 * You need to call this after each removal because the cursor
 * might point to some non existing place.
 */
void FormulaCursor::normalize(Direction direction)
{
    BasicElement* element = currentElement();
    element->normalize(this, direction);
}

/**
 * Removes the current selected children and returns them.
 * The cursor needs to be normal (that is be inside a SequenceElement)
 * for this to have any effect.
 */
void FormulaCursor::remove(QList<BasicElement*>& children,
                           Direction direction)
{
    assert( !isReadOnly() );
    SequenceElement* sequence = normal();
    if (sequence != 0) {

        // If there is no child to remove in the sequence
        // remove the sequence instead.
        if (sequence->countChildren() == 0) {
            BasicElement* parent = sequence->getParent();
            if (parent != 0) {
                parent->selectChild(this, sequence);
                parent->remove(this, children, direction);
                return;
            }
        }
        else {
            sequence->remove(this, children, direction);
        }
    }
}


/**
 * Replaces the current selection with the supplied element.
 * The replaced elements become the new element's main child's content.
 */
void FormulaCursor::replaceSelectionWith(BasicElement* element,
                                         Direction direction)
{
    assert( !isReadOnly() );
    QList<BasicElement*> list;
    // we suppres deletion here to get an error if something
    // was left in the list.
    //list.setAutoDelete(true);

    //remove(list, direction);
    if (hasSelection()) {
        currentElement()->remove(this, list, direction);
    }

    QList<BasicElement*> ldist;
    ldist.append(element);
    currentElement()->insert(this, ldist, direction);;
    SequenceElement* mainChild = element->getMainChild();
    if (mainChild != 0) {
        mainChild->goInside(this);
        currentElement()->insert(this, list, direction);
        /*
        BasicElement* parent = element->getParent();
        if (direction == beforeCursor) {
            parent->moveRight(this, element);
        }
        else {
            parent->moveLeft(this, element);
        }
        */
        element->selectChild(this, mainChild);
    }
}


/**
 * Replaces the element the cursor points to with its main child's
 * content.
 */
BasicElement* FormulaCursor::replaceByMainChildContent(Direction direction)
{
    assert( !isReadOnly() );
    QList<BasicElement*> childrenList;
    QList<BasicElement*> list;
    BasicElement* element = currentElement();
    SequenceElement* mainChild = element->getMainChild();
    if ((mainChild != 0) && (mainChild->countChildren() > 0)) {
        mainChild->selectAllChildren(this);
        remove(childrenList);
    }
    element->getParent()->moveRight(this, element);
    setSelecting(false);
    remove(list);
    currentElement()->insert(this, childrenList, direction);
    if (list.count() > 0) {
        return list.takeAt(0);
    }
    return 0;
}


/**
 * Trys to find the element we are the main child of and replace
 * it with our content.
 *
 * This is simply another form of replaceByMainChildContent. You
 * use this one if the cursor is normalized and inside the main child.
 */
BasicElement* FormulaCursor::removeEnclosingElement(Direction direction)
{
    assert( !isReadOnly() );
    BasicElement* parent = currentElement()->getParent();
    if (parent != 0) {
        if (currentElement() == parent->getMainChild()) {
            parent->selectChild(this, currentElement());
            return replaceByMainChildContent(direction);
        }
    }
    return 0;
}


/**
 * Returns wether the element the cursor points to should be replaced.
 * Elements are senseless as soon as they only contain a main child.
 */
bool FormulaCursor::elementIsSenseless()
{
    BasicElement* element = currentElement();
    return element->isSenseless();
}


/**
 * Returns the child the cursor points to. Depending on the
 * direction this might be the child before or after the
 * cursor.
 *
 * Might be 0 is there is no such child.
 */
BasicElement* FormulaCursor::getActiveChild(Direction direction)
{
    return currentElement()->getChild(this, direction);
}

BasicElement* FormulaCursor::getSelectedChild()
{
    if (hasSelection()) {
        if ((getSelectionEnd() - getSelectionStart()) > 1) {
            return 0;
        }
        return getActiveChild((getPos() > getMark()) ?
                              beforeCursor :
                              afterCursor);
    }
    else {
        return getActiveChild(beforeCursor);
    }
}


void FormulaCursor::selectActiveElement()
{
    if ( !hasSelection() && getPos() > 0 ) {
        setSelecting( true );
        setMark( getPos() - 1 );
    }
}


/**
 * Tells whether we currently point to the given elements
 * main child and to the place behind its last child.
 */
bool FormulaCursor::pointsAfterMainChild(BasicElement* element)
{
    if (element != 0) {
        SequenceElement* mainChild = element->getMainChild();
        return (currentElement() == mainChild) &&
            ((mainChild->countChildren() == getPos()) || (0 == getPos()));
    }
    return false;
}

/**
 * The element is going to leave the formula with and all its children.
 */
void FormulaCursor::elementWillVanish(BasicElement* element)
{
    BasicElement* child = currentElement();
    if (child == element->getParent()) {
        child->childWillVanish(this, element);
        return;
    }
    while (child != 0) {
        if (child == element) {
            // This is meant to catch all cursors that did not
            // cause the deletion.
            child->getParent()->moveLeft(this, child);
            setSelecting(false);
            hasChangedFlag = true;
            return;
        }
        child = child->getParent();
    }
}



bool FormulaCursor::isReadOnly() const
{
    if ( readOnly ) {
        return true;
    }
    const SequenceElement* sequence = normal();
    if ( sequence != 0 ) {
        bool ro = sequence->readOnly( this );
        //kDebug() << k_funcinfo << "readOnly=" << ro << endl;
        return ro;
    }
    return false;
}

/**
 * Returns the sequence the cursor is in if we are normal. If not returns 0.
 */
SequenceElement* FormulaCursor::normal()
{
    return dynamic_cast<SequenceElement*>(m_currentElement);
}

const SequenceElement* FormulaCursor::normal() const
{
    return dynamic_cast<SequenceElement*>(m_currentElement);
}

} // namespace KFormula
