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
 * Boston, MA 02110-1301, USA.
*/

#include <stdlib.h>
#include <math.h>

#include <QPainter>
#include <QPaintDevice>
#include <q3valuestack.h>
//Added by qt3to4:
#include <QKeyEvent>
#include <Q3PtrList>

#include <kcommand.h>
#include <kdebug.h>
#include <klocale.h>

//#include <boost/spirit.hpp>

#include "MatrixDialog.h"
#include "bracketelement.h"
#include "creationstrategy.h"
#include "elementtype.h"
#include "elementvisitor.h"
#include "formulacursor.h"
#include "formulaelement.h"
#include "fractionelement.h"
#include "indexelement.h"
#include "kformulacommand.h"
#include "kformulacontainer.h"
#include "kformuladocument.h"
#include "matrixelement.h"
#include "rootelement.h"
#include "sequenceelement.h"
#include "sequenceparser.h"
#include "spaceelement.h"
#include "symbolelement.h"
#include "symboltable.h"
#include "textelement.h"

#include <assert.h>

KFORMULA_NAMESPACE_BEGIN
//using namespace std;

ElementCreationStrategy* SequenceElement::creationStrategy = 0;

void SequenceElement::setCreationStrategy( ElementCreationStrategy* strategy )
{
    creationStrategy = strategy;
}


SequenceElement::SequenceElement(BasicElement* parent)
        : BasicElement(parent), parseTree(0), textSequence(true),singlePipe(true)
{
    assert( creationStrategy != 0 );
    children.setAutoDelete(true);
}


SequenceElement::~SequenceElement()
{
    delete parseTree;
}

SequenceElement::SequenceElement( const SequenceElement& other )
    : BasicElement( other )
{
    children.setAutoDelete(true);
    uint count = other.children.count();
    for (uint i = 0; i < count; i++) {
        BasicElement* child = children.at(i)->clone();
        child->setParent( this );
        children.append( child );
    }
}


bool SequenceElement::accept( ElementVisitor* visitor )
{
    return visitor->visit( this );
}


bool SequenceElement::readOnly( const FormulaCursor* ) const
{
    return getParent()->readOnly( this );
}


/**
 * Returns the element the point is in.
 */
BasicElement* SequenceElement::goToPos( FormulaCursor* cursor, bool& handled,
                                        const LuPixelPoint& point, const LuPixelPoint& parentOrigin )
{
    BasicElement* e = BasicElement::goToPos(cursor, handled, point, parentOrigin);
    if (e != 0) {
        LuPixelPoint myPos(parentOrigin.x() + getX(),
                           parentOrigin.y() + getY());

        uint count = children.count();
        for (uint i = 0; i < count; i++) {
            BasicElement* child = children.at(i);
            e = child->goToPos(cursor, handled, point, myPos);
            if (e != 0) {
                if (!handled) {
                    handled = true;
                    if ((point.x() - myPos.x()) < (e->getX() + e->getWidth()*2/3)) {
                        cursor->setTo(this, children.find(e));
                    }
                    else {
                        cursor->setTo(this, children.find(e)+1);
                    }
                }
                return e;
            }
        }

        luPixel dx = point.x() - myPos.x();
        //int dy = point.y() - myPos.y();

        for (uint i = 0; i < count; i++) {
            BasicElement* child = children.at(i);
            if (dx < child->getX()) {
                cursor->setTo( this, i );
                handled = true;
                return children.at( i );
            }
        }

        cursor->setTo(this, countChildren());
        handled = true;
        return this;
    }
    return 0;
}


bool SequenceElement::isEmpty()
{
    uint count = children.count();
    for (uint i = 0; i < count; i++) {
        BasicElement* child = children.at(i);
        if (!child->isInvisible()) {
            return false;
        }
    }
    return true;
}


/**
 * Calculates our width and height and
 * our children's parentPosition.
 */
void SequenceElement::calcSizes(const ContextStyle& style,
                                ContextStyle::TextStyle tstyle,
                                ContextStyle::IndexStyle istyle)
{
    if (!isEmpty()) {
        luPixel width = 0;
        luPixel toBaseline = 0;
        luPixel fromBaseline = 0;

        // Let's do all normal elements that have a base line.
        Q3PtrListIterator<BasicElement> it( children );
        for ( ; it.current(); ++it ) {
            BasicElement* child = it.current();

            luPixel spaceBefore = 0;
            if ( isFirstOfToken( child ) ) {
                spaceBefore =
                    style.ptToPixelX( child->getElementType()->getSpaceBefore( style,
                                                                               tstyle ) );
            }

            if ( !child->isInvisible() ) {
                child->calcSizes( style, tstyle, istyle );
                child->setX( width + spaceBefore );
                width += child->getWidth() + spaceBefore;

                luPixel childBaseline = child->getBaseline();
                if ( childBaseline > -1 ) {
                    toBaseline = qMax( toBaseline, childBaseline );
                    fromBaseline = qMax( fromBaseline,
                                         child->getHeight() - childBaseline );
                }
                else {
                    luPixel bl = child->getHeight()/2 + style.axisHeight( tstyle );
                    toBaseline = qMax( toBaseline, bl );
                    fromBaseline = qMax( fromBaseline, child->getHeight() - bl );
                }
            }
            else {
                width += spaceBefore;
                child->setX( width );
            }
        }

        setWidth(width);
        setHeight(toBaseline+fromBaseline);
        setBaseline(toBaseline);

        setChildrenPositions();
    }
    else {
        luPixel w = style.getEmptyRectWidth();
        luPixel h = style.getEmptyRectHeight();
        setWidth( w );
        setHeight( h );
        setBaseline( h );
        //setMidline( h*.5 );
    }
}


void SequenceElement::setChildrenPositions()
{
    Q3PtrListIterator<BasicElement> it( children );
    for ( ; it.current(); ++it ) {
        BasicElement* child = it.current();
        child->setY(getBaseline() - child->getBaseline());
    }
}


/**
 * Draws the whole element including its children.
 * The `parentOrigin' is the point this element's parent starts.
 * We can use our parentPosition to get our own origin then.
 */
void SequenceElement::draw( QPainter& painter, const LuPixelRect& r,
                            const ContextStyle& context,
                            ContextStyle::TextStyle tstyle,
                            ContextStyle::IndexStyle istyle,
                            const LuPixelPoint& parentOrigin )
{
    LuPixelPoint myPos( parentOrigin.x() + getX(), parentOrigin.y() + getY() );
    // There might be zero sized elements that still want to be drawn at least
    // in edit mode. (EmptyElement)
    //if ( !LuPixelRect( myPos.x(), myPos.y(), getWidth(), getHeight() ).intersects( r ) )
    //    return;

    if (!isEmpty()) {
        Q3PtrListIterator<BasicElement> it( children );
        for ( ; it.current(); ) {
            BasicElement* child = it.current();
            if (!child->isInvisible()) {
                child->draw(painter, r, context, tstyle, istyle, myPos);

                // Each starting element draws the whole token
                // This only concerns TextElements.
                ElementType* token = child->getElementType();
                if ( token != 0 ) {
                    it += token->end() - token->start();
                }
                else {
                    ++it;
                }
            }
            else {
                ++it;
            }
        }
    }
    else {
        drawEmptyRect( painter, context, myPos );
    }
    // Debug
    //painter.setPen(Qt::green);
    //painter.drawRect(parentOrigin.x() + getX(), parentOrigin.y() + getY(),
    //                 getWidth(), getHeight());
//     painter.drawLine( context.layoutUnitToPixelX( parentOrigin.x() + getX() ),
//                       context.layoutUnitToPixelY( parentOrigin.y() + getY() + axis( context, tstyle ) ),
//                       context.layoutUnitToPixelX( parentOrigin.x() + getX() + getWidth() ),
//                       context.layoutUnitToPixelY( parentOrigin.y() + getY() + axis( context, tstyle ) ) );
//     painter.setPen(Qt::red);
//     painter.drawLine( context.layoutUnitToPixelX( parentOrigin.x() + getX() ),
//                       context.layoutUnitToPixelY( parentOrigin.y() + getY() + getBaseline() ),
//                       context.layoutUnitToPixelX( parentOrigin.x() + getX() + getWidth() ),
//                       context.layoutUnitToPixelY( parentOrigin.y() + getY() + getBaseline() ) );
}


void SequenceElement::dispatchFontCommand( FontCommand* cmd )
{
    Q3PtrListIterator<BasicElement> it( children );
    for ( ; it.current(); ++it ) {
        BasicElement* child = it.current();
        child->dispatchFontCommand( cmd );
    }
}


void SequenceElement::drawEmptyRect( QPainter& painter, const ContextStyle& context,
                                     const LuPixelPoint& upperLeft )
{
    if ( context.edit() ) {
        painter.setBrush(Qt::NoBrush);
        painter.setPen( QPen( context.getEmptyColor(),
                              context.layoutUnitToPixelX( context.getLineWidth() ) ) );
        painter.drawRect( context.layoutUnitToPixelX( upperLeft.x() ),
                          context.layoutUnitToPixelY( upperLeft.y() ),
                          context.layoutUnitToPixelX( getWidth() ),
                          context.layoutUnitToPixelY( getHeight() ) );
    }
}

void SequenceElement::calcCursorSize( const ContextStyle& context,
                                      FormulaCursor* cursor, bool smallCursor )
{
    LuPixelPoint point = widgetPos();
    uint pos = cursor->getPos();

    luPixel posX = getChildPosition( context, pos );
    luPixel height = getHeight();

    luPixel unitX = context.ptToLayoutUnitPixX( 1 );
    luPixel unitY = context.ptToLayoutUnitPixY( 1 );

    // Here are those evil constants that describe the cursor size.

    if ( cursor->isSelection() ) {
        uint mark = cursor->getMark();
        luPixel markX = getChildPosition( context, mark );
        luPixel x = qMin(posX, markX);
        luPixel width = abs(posX - markX);

        if ( smallCursor ) {
            cursor->cursorSize.setRect( point.x()+x, point.y(), width, height );
        }
        else {
            cursor->cursorSize.setRect( point.x()+x, point.y() - 2*unitY,
                                        width + unitX, height + 4*unitY );
        }
    }
    else {
        if ( smallCursor ) {
            cursor->cursorSize.setRect( point.x()+posX, point.y(),
                                        unitX, height );
        }
        else {
            cursor->cursorSize.setRect( point.x(), point.y() - 2*unitY,
                                        getWidth() + unitX, height + 4*unitY );
        }
    }

    cursor->cursorPoint.setX( point.x()+posX );
    cursor->cursorPoint.setY( point.y()+getHeight()/2 );
}


/**
 * If the cursor is inside a sequence it needs to be drawn.
 */
void SequenceElement::drawCursor( QPainter& painter, const ContextStyle& context,
                                  FormulaCursor* cursor, bool smallCursor,
                                  bool activeCursor )
{
    //painter.setRasterOp( Qt::XorROP );
    if ( cursor->isSelection() ) {
        const LuPixelRect& r = cursor->cursorSize;
        painter.fillRect( context.layoutUnitToPixelX( r.x() ),
                          context.layoutUnitToPixelY( r.y() ),
                          context.layoutUnitToPixelX( r.width() ),
                          context.layoutUnitToPixelY( r.height() ),
                          Qt::white );
    }
    painter.setPen( QPen( Qt::white,
                    context.layoutUnitToPixelX( context.getLineWidth()/2 ) ) );
    const LuPixelPoint& point = cursor->getCursorPoint();
    const LuPixelRect& size = cursor->getCursorSize();
    if ( activeCursor )
    {
        int offset = 0;
        if ( cursor->isSelection() && cursor->getPos() > cursor->getMark() )
            offset = -1;
        painter.drawLine( context.layoutUnitToPixelX( point.x() ) + offset,
                          context.layoutUnitToPixelY( size.top() ),
                          context.layoutUnitToPixelX( point.x() ) + offset,
                          context.layoutUnitToPixelY( size.bottom() )-1 );
        painter.drawLine( context.layoutUnitToPixelX( point.x() ) + offset + 1,
                          context.layoutUnitToPixelY( size.top() ),
                          context.layoutUnitToPixelX( point.x() ) + offset + 1,
                          context.layoutUnitToPixelY( size.bottom() )-1 );
    }
    if ( !smallCursor && !cursor->isSelection() )
        painter.drawLine( context.layoutUnitToPixelX( size.left() ),
                          context.layoutUnitToPixelY( size.bottom() )-1,
                          context.layoutUnitToPixelX( size.right() )-1,
                          context.layoutUnitToPixelY( size.bottom() )-1 );
    // This might be wrong but probably isn't.
   // painter.setRasterOp( Qt::CopyROP );
}


luPixel SequenceElement::getChildPosition( const ContextStyle& context, uint child )
{
    if (child < children.count()) {
        return children.at(child)->getX();
    }
    else {
        if (children.count() > 0) {
            return children.at(child-1)->getX() + children.at(child-1)->getWidth();
        }
        else {
            return context.ptToLayoutUnitPixX( 2 );
        }
    }
}


// navigation
//
// The elements are responsible to handle cursor movement themselves.
// To do this they need to know the direction the cursor moves and
// the element it comes from.
//
// The cursor might be in normal or in selection mode.

/**
 * Enters this element while moving to the left starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or to the left of it.
 */
void SequenceElement::moveLeft(FormulaCursor* cursor, BasicElement* from)
{
    // Our parent asks us for a cursor position. Found.
    if (from == getParent()) {
        cursor->setTo(this, children.count());
        from->entered( this );
    }

    // We already owned the cursor. Ask next child then.
    else if (from == this) {
        if (cursor->getPos() > 0) {
            if (cursor->isSelectionMode()) {
                cursor->setTo(this, cursor->getPos()-1);

                // invisible elements are not visible so we move on.
                if (children.at(cursor->getPos())->isInvisible()) {
                    moveLeft(cursor, this);
                }
            }
            else {
                children.at(cursor->getPos()-1)->moveLeft(cursor, this);
            }
        }
        else {
            // Needed because FormulaElement derives this.
            if (getParent() != 0) {
                getParent()->moveLeft(cursor, this);
            }
            else {
                formula()->moveOutLeft( cursor );
            }
        }
    }

    // The cursor came from one of our children or
    // something is wrong.
    else {
        int fromPos = children.find(from);
        cursor->setTo(this, fromPos);
        if (cursor->isSelectionMode()) {
            cursor->setMark(fromPos+1);
        }

        // invisible elements are not visible so we move on.
        if (from->isInvisible()) {
            moveLeft(cursor, this);
        }
        formula()->tell( "" );
    }
}

/**
 * Enters this element while moving to the right starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or to the right of it.
 */
void SequenceElement::moveRight(FormulaCursor* cursor, BasicElement* from)
{
    // Our parent asks us for a cursor position. Found.
    if (from == getParent()) {
        cursor->setTo(this, 0);
        from->entered( this );
    }

    // We already owned the cursor. Ask next child then.
    else if (from == this) {
        uint pos = cursor->getPos();
        if (pos < children.count()) {
            if (cursor->isSelectionMode()) {
                cursor->setTo(this, pos+1);

                // invisible elements are not visible so we move on.
                if (children.at(pos)->isInvisible()) {
                    moveRight(cursor, this);
                }
            }
            else {
                children.at(pos)->moveRight(cursor, this);
            }
        }
        else {
            // Needed because FormulaElement derives this.
            if (getParent() != 0) {
                getParent()->moveRight(cursor, this);
            }
            else {
                formula()->moveOutRight( cursor );
            }
        }
    }

    // The cursor came from one of our children or
    // something is wrong.
    else {
        int fromPos = children.find(from);
        cursor->setTo(this, fromPos+1);
        if (cursor->isSelectionMode()) {
            cursor->setMark(fromPos);
        }

        // invisible elements are not visible so we move on.
        if (from->isInvisible()) {
            moveRight(cursor, this);
        }
        formula()->tell( "" );
    }
}


void SequenceElement::moveWordLeft(FormulaCursor* cursor)
{
    uint pos = cursor->getPos();
    if (pos > 0) {
        ElementType* type = children.at(pos-1)->getElementType();
        if (type != 0) {
            cursor->setTo(this, type->start());
        }
    }
    else {
        moveLeft(cursor, this);
    }
}


void SequenceElement::moveWordRight(FormulaCursor* cursor)
{
    uint pos = cursor->getPos();
    if (pos < children.count()) {
        ElementType* type = children.at(pos)->getElementType();
        if (type != 0) {
            cursor->setTo(this, type->end());
        }
    }
    else {
        moveRight(cursor, this);
    }
}


/**
 * Enters this element while moving up starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or above it.
 */
void SequenceElement::moveUp(FormulaCursor* cursor, BasicElement* from)
{
    if (from == getParent()) {
        moveRight(cursor, this);
    }
    else {
        if (getParent() != 0) {
            getParent()->moveUp(cursor, this);
        }
        else {
            formula()->moveOutAbove( cursor );
        }
    }
}

/**
 * Enters this element while moving down starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or below it.
 */
void SequenceElement::moveDown(FormulaCursor* cursor, BasicElement* from)
{
    if (from == getParent()) {
        moveRight(cursor, this);
    }
    else {
        if (getParent() != 0) {
            getParent()->moveDown(cursor, this);
        }
        else {
            formula()->moveOutBelow( cursor );
        }
    }
}

/**
 * Moves the cursor to the first position in this sequence.
 * (That is before the first child.)
 */
void SequenceElement::moveHome(FormulaCursor* cursor)
{
    if (cursor->isSelectionMode()) {
        BasicElement* element = cursor->getElement();
        if (element != this) {
            while (element->getParent() != this) {
                element = element->getParent();
            }
            cursor->setMark(children.find(element)+1);
        }
    }
    cursor->setTo(this, 0);
}

/**
 * Moves the cursor to the last position in this sequence.
 * (That is behind the last child.)
 */
void SequenceElement::moveEnd(FormulaCursor* cursor)
{
    if (cursor->isSelectionMode()) {
        BasicElement* element = cursor->getElement();
        if (element != this) {
            while (element->getParent() != this) {
                element = element->getParent();
                if (element == 0) {
                    cursor->setMark(children.count());
                    break;
                }
            }
            if (element != 0) {
                cursor->setMark(children.find(element));
            }
        }
    }
    cursor->setTo(this, children.count());
}

/**
 * Sets the cursor inside this element to its start position.
 * For most elements that is the main child.
 */
void SequenceElement::goInside(FormulaCursor* cursor)
{
    cursor->setSelection(false);
    cursor->setTo(this, 0);
}


// children

/**
 * Removes the child. If this was the main child this element might
 * request its own removal.
 * The cursor is the one that caused the removal. It has to be moved
 * to the place any user expects the cursor after that particular
 * element has been removed.
 */
// void SequenceElement::removeChild(FormulaCursor* cursor, BasicElement* child)
// {
//     int pos = children.find(child);
//     formula()->elementRemoval(child, pos);
//     cursor->setTo(this, pos);
//     children.remove(pos);
//     /*
//         if len(self.children) == 0:
//             if self.parent != None:
//                 self.parent.removeChild(cursor, self)
//                 return
//     */
//     formula()->changed();
// }


/**
 * Inserts all new children at the cursor position. Places the
 * cursor according to the direction. The inserted elements will
 * be selected.
 *
 * The list will be emptied but stays the property of the caller.
 */
void SequenceElement::insert(FormulaCursor* cursor,
                             Q3PtrList<BasicElement>& newChildren,
                             Direction direction)
{
    int pos = cursor->getPos();
    uint count = newChildren.count();
    for (uint i = 0; i < count; i++) {
        BasicElement* child = newChildren.take(0);
        child->setParent(this);
        children.insert(pos+i, child);
    }
    if (direction == beforeCursor) {
        cursor->setTo(this, pos+count, pos);
    }
    else {
        cursor->setTo(this, pos, pos+count);
    }

    formula()->changed();
    parse();
}


/**
 * Removes all selected children and returns them. Places the
 * cursor to where the children have been.
 *
 * The ownership of the list is passed to the caller.
 */
void SequenceElement::remove(FormulaCursor* cursor,
                             Q3PtrList<BasicElement>& removedChildren,
                             Direction direction)
{
    if (cursor->isSelection()) {
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
                    BasicElement* child = children.at(pos);
                    formula()->elementRemoval(child);
                    children.take(pos);
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
            uint pos = cursor->getPos();
            if (pos < children.count()) {
                while (pos < children.count()) {
                    BasicElement* child = children.at(pos);
                    formula()->elementRemoval(child);
                    children.take(pos);
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
    }
    parse();
}


/**
 * Removes the children at pos and appends it to the list.
 */
void SequenceElement::removeChild(Q3PtrList<BasicElement>& removedChildren, int pos)
{
    BasicElement* child = children.at(pos);
    formula()->elementRemoval(child);
    children.take(pos);
    removedChildren.append(child);
    //cerr << *removedChildren.at(0) << endl;
    formula()->changed();
}


/**
 * Moves the cursor to a normal place where new elements
 * might be inserted.
 */
void SequenceElement::normalize(FormulaCursor* cursor, Direction)
{
    cursor->setSelection(false);
}


/**
 * Returns the child at the cursor.
 * Does not care about the selection.
 */
BasicElement* SequenceElement::getChild( FormulaCursor* cursor, Direction direction )
{
    if ( direction == beforeCursor ) {
        if ( cursor->getPos() > 0 ) {
            return children.at( cursor->getPos() - 1 );
        }
    }
    else {
        if ( cursor->getPos() < qRound( children.count() ) ) {
            return children.at( cursor->getPos() );
        }
    }
    return 0;
}


/**
 * Sets the cursor to select the child. The mark is placed before,
 * the position behind it.
 */
void SequenceElement::selectChild(FormulaCursor* cursor, BasicElement* child)
{
    int pos = children.find(child);
    if (pos > -1) {
        cursor->setTo(this, pos+1, pos);
    }
}

void SequenceElement::childWillVanish(FormulaCursor* cursor, BasicElement* child)
{
    int childPos = children.find(child);
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
    }
}


/**
 * Selects all children. The cursor is put behind, the mark before them.
 */
void SequenceElement::selectAllChildren(FormulaCursor* cursor)
{
    cursor->setTo(this, children.count(), 0);
}

bool SequenceElement::onlyTextSelected( FormulaCursor* cursor )
{
    if ( cursor->isSelection() ) {
        uint from = qMin( cursor->getPos(), cursor->getMark() );
        uint to = qMax( cursor->getPos(), cursor->getMark() );
        for ( uint i = from; i < to; i++ ) {
            BasicElement* element = getChild( i );
            if ( element->getCharacter() == QChar::Null ) {
                return false;
            }
        }
    }
    return true;
}


KCommand* SequenceElement::buildCommand( Container* container, Request* request )
{
    FormulaCursor* cursor = container->activeCursor();
    if ( cursor->isReadOnly() ) {
        formula()->tell( i18n( "write protection" ) );
        return 0;
    }

    switch ( *request ) {
    case req_addText: {
        KFCReplace* command = new KFCReplace( i18n("Add Text"), container );
        TextRequest* tr = static_cast<TextRequest*>( request );
        for ( int i = 0; i < tr->text().length(); i++ ) {
            command->addElement( creationStrategy->createTextElement( tr->text()[i] ) );
        }
        return command;
    }
    case req_addTextChar: {
        KFCReplace* command = new KFCReplace( i18n("Add Text"), container );
        TextCharRequest* tr = static_cast<TextCharRequest*>( request );
        TextElement* element = creationStrategy->createTextElement( tr->ch(), tr->isSymbol() );
        command->addElement( element );
        return command;
    }
    case req_addEmptyBox: {
        EmptyElement* element = creationStrategy->createEmptyElement();
        if ( element != 0 ) {
            KFCReplace* command = new KFCReplace( i18n("Add Empty Box"), container );
            command->addElement( element );
            return command;
        }
        break;
    }
    case req_addNameSequence:
        if ( onlyTextSelected( container->activeCursor() ) ) {
            NameSequence* nameSequence = creationStrategy->createNameSequence();
            if ( nameSequence != 0 ) {
                KFCAddReplacing* command = new KFCAddReplacing( i18n( "Add Name" ), container );
                command->setElement( nameSequence );
                return command;
            }
        }
        break;
    case req_addBracket: {
        BracketRequest* br = static_cast<BracketRequest*>( request );
        BracketElement* bracketElement =
            creationStrategy->createBracketElement( br->left(), br->right() );
        if ( bracketElement != 0 ) {
            KFCAddReplacing* command = new KFCAddReplacing(i18n("Add Bracket"), container);
            command->setElement( bracketElement );
            return command;
        }
        break;
    }
    case req_addOverline: {
        OverlineElement* overline = creationStrategy->createOverlineElement();
        if ( overline != 0 ) {
            KFCAddReplacing* command = new KFCAddReplacing(i18n("Add Overline"), container);
            command->setElement( overline );
            return command;
        }
        break;
    }
    case req_addUnderline: {
        UnderlineElement* underline = creationStrategy->createUnderlineElement();
        if ( underline != 0 ) {
            KFCAddReplacing* command = new KFCAddReplacing(i18n("Add Underline"), container);
            command->setElement( underline );
            return command;
        }
        break;
    }
    case req_addMultiline: {
        MultilineElement* multiline = creationStrategy->createMultilineElement();
        if ( multiline != 0 ) {
            KFCAddReplacing* command = new KFCAddReplacing(i18n("Add Multiline"), container);
            command->setElement( multiline );
            return command;
        }
        break;
    }
    case req_addSpace: {
        SpaceRequest* sr = static_cast<SpaceRequest*>( request );
        SpaceElement* element = creationStrategy->createSpaceElement( sr->space() );
        if ( element != 0 ) {
            KFCReplace* command = new KFCReplace( i18n("Add Space"), container );
            command->addElement( element );
            return command;
        }
        break;
    }
    case req_addFraction: {
        FractionElement* fraction = creationStrategy->createFractionElement();
        if ( fraction != 0 ) {
            KFCAddReplacing* command = new KFCAddReplacing(i18n("Add Fraction"), container);
            command->setElement( fraction );
            return command;
        }
        break;
    }
    case req_addRoot: {
        RootElement* root = creationStrategy->createRootElement();
        if ( root != 0 ) {
            KFCAddReplacing* command = new KFCAddReplacing(i18n("Add Root"), container);
            command->setElement( root );
            return command;
        }
        break;
    }
    case req_addSymbol: {
        SymbolRequest* sr = static_cast<SymbolRequest*>( request );
        SymbolElement* symbol = creationStrategy->createSymbolElement( sr->type() );
        if ( symbol != 0 ) {
            KFCAddReplacing* command = new KFCAddReplacing( i18n( "Add Symbol" ), container );
            command->setElement( symbol );
            return command;
        }
        break;
    }
    case req_addOneByTwoMatrix: {
        FractionElement* element = creationStrategy->createFractionElement();
        if ( element != 0 ) {
            KFCAddReplacing* command = new KFCAddReplacing( i18n("Add 1x2 Matrix"), container );
            element->showLine(false);
            command->setElement(element);
            return command;
        }
    }
    case req_addMatrix: {
        MatrixRequest* mr = static_cast<MatrixRequest*>( request );
        uint rows = mr->rows(), cols = mr->columns();
        if ( ( rows == 0 ) || ( cols == 0 ) ) {
            MatrixDialog* dialog = new MatrixDialog( 0 );
            if ( dialog->exec() ) {
                rows = dialog->h;
                cols = dialog->w;
            }
            delete dialog;
        }

        if ( ( rows != 0 ) && ( cols != 0 ) ) {
            KFCAddReplacing* command = new KFCAddReplacing( i18n( "Add Matrix" ), container );
            command->setElement( creationStrategy->createMatrixElement( rows, cols ) );
            return command;
        }
        else
            return 0L;
    }
    case req_addIndex: {
        if ( cursor->getPos() > 0 && !cursor->isSelection() ) {
            IndexElement* element =
                dynamic_cast<IndexElement*>( children.at( cursor->getPos()-1 ) );
            if ( element != 0 ) {
                element->getMainChild()->goInside( cursor );
                return element->getMainChild()->buildCommand( container, request );
            }
        }
        IndexElement* element = creationStrategy->createIndexElement();
        if ( element != 0 ) {
            if ( !cursor->isSelection() ) {
                cursor->moveLeft( SelectMovement | WordMovement );
            }
            IndexRequest* ir = static_cast<IndexRequest*>( request );
            KFCAddIndex* command = new KFCAddIndex( container, element,
                                                    element->getIndex( ir->index() ) );
            return command;
        }
        break;
    }
    case req_removeEnclosing: {
        if ( !cursor->isSelection() ) {
            DirectedRemove* dr = static_cast<DirectedRemove*>( request );
            KFCRemoveEnclosing* command = new KFCRemoveEnclosing( container, dr->direction() );
            return command;
        }
    }
    case req_remove: {
        SequenceElement* sequence = cursor->normal();
        if ( sequence &&
             ( sequence == sequence->formula() ) &&
             ( sequence->countChildren() == 0 ) ) {
            sequence->formula()->removeFormula( cursor );
            return 0;
        }
        else {
            DirectedRemove* dr = static_cast<DirectedRemove*>( request );

            // empty removes are not legal!
            if ( !cursor->isSelection() ) {
                if ( countChildren() > 0 ) {
                    if ( ( cursor->getPos() == 0 ) && ( dr->direction() == beforeCursor ) ) {
                        return 0;
                    }
                    if ( ( cursor->getPos() == countChildren() ) && ( dr->direction() == afterCursor ) ) {
                        return 0;
                    }
                }
                else if ( getParent() == 0 ) {
                    return 0;
                }
            }

            KFCRemove* command = new KFCRemove( container, dr->direction() );
            return command;
        }
    }
    case req_compactExpression: {
        cursor->moveEnd();
        cursor->moveRight();
        formula()->cursorHasMoved( cursor );
        break;
    }
    case req_makeGreek: {
        TextElement* element = cursor->getActiveTextElement();
        if ((element != 0) && !element->isSymbol()) {
            cursor->selectActiveElement();
            const SymbolTable& table = container->document()->getSymbolTable();
            if (table.greekLetters().contains(element->getCharacter())) {
                KFCReplace* command = new KFCReplace( i18n( "Change Char to Symbol" ), container );
                TextElement* symbol = creationStrategy->createTextElement( table.unicodeFromSymbolFont( element->getCharacter() ), true );
                command->addElement( symbol );
                return command;
            }
            cursor->setSelection( false );
        }
        break;
    }
    case req_paste:
    case req_copy:
    case req_cut:
        break;
    case req_formatBold:
    case req_formatItalic: {
        if ( cursor->isSelection() ) {
            CharStyleRequest* csr = static_cast<CharStyleRequest*>( request );
            CharStyle cs = normalChar;
            if ( csr->bold() ) cs = static_cast<CharStyle>( cs | boldChar );
            if ( csr->italic() ) cs = static_cast<CharStyle>( cs | italicChar );
            CharStyleCommand* cmd = new CharStyleCommand( cs, i18n( "Change Char Style" ), container );
            int end = cursor->getSelectionEnd();
            for ( int i = cursor->getSelectionStart(); i<end; ++i ) {
                cmd->addElement( children.at( i ) );
            }
            return cmd;
        }
        break;
    }
    case req_formatFamily: {
        if ( cursor->isSelection() ) {
            CharFamilyRequest* cfr = static_cast<CharFamilyRequest*>( request );
            CharFamily cf = cfr->charFamily();
            CharFamilyCommand* cmd = new CharFamilyCommand( cf, i18n( "Change Char Family" ), container );
            int end = cursor->getSelectionEnd();
            for ( int i = cursor->getSelectionStart(); i<end; ++i ) {
                cmd->addElement( children.at( i ) );
            }
            return cmd;
        }
        break;
    }
    default:
        break;
    }
    return 0;
}


KCommand* SequenceElement::input( Container* container, QKeyEvent* event )
{
    QChar ch = event->text().at( 0 );
    if ( ch.isPrint() ) {
        return input( container, ch );
    }
    else {
        int action = event->key();
        int state = event->modifiers();
        MoveFlag flag = movementFlag(state);

	switch ( action ) {
        case Qt::Key_Backspace: {
            DirectedRemove r( req_remove, beforeCursor );
            return buildCommand( container, &r );
        }
        case Qt::Key_Delete: {
            DirectedRemove r( req_remove, afterCursor );
            return buildCommand( container, &r );
        }
	case Qt::Key_Left: {
            FormulaCursor* cursor = container->activeCursor();
            cursor->moveLeft( flag );
            formula()->cursorHasMoved( cursor );
            break;
        }
        case Qt::Key_Right: {
            FormulaCursor* cursor = container->activeCursor();
            cursor->moveRight( flag );
            formula()->cursorHasMoved( cursor );
            break;
        }
        case Qt::Key_Up: {
            FormulaCursor* cursor = container->activeCursor();
            cursor->moveUp( flag );
            formula()->cursorHasMoved( cursor );
            break;
        }
        case Qt::Key_Down: {
            FormulaCursor* cursor = container->activeCursor();
            cursor->moveDown( flag );
            formula()->cursorHasMoved( cursor );
            break;
        }
        case Qt::Key_Home: {
            FormulaCursor* cursor = container->activeCursor();
            cursor->moveHome( flag );
            formula()->cursorHasMoved( cursor );
            break;
        }
        case Qt::Key_End: {
            FormulaCursor* cursor = container->activeCursor();
            cursor->moveEnd( flag );
            formula()->cursorHasMoved( cursor );
            break;
        }
        default:
            if ( state & Qt::ControlModifier ) {
                switch ( event->key() ) {
                case Qt::Key_AsciiCircum: {
                    IndexRequest r( upperLeftPos );
                    return buildCommand( container, &r );
                }
                case Qt::Key_Underscore: {
                    IndexRequest r( lowerLeftPos );
                    return buildCommand( container, &r );
                }
                default:
                    break;
                }
            }
        }
    }
    return 0;
}


KCommand* SequenceElement::input( Container* container, QChar ch )
{
    int unicode = ch.unicode();
    switch (unicode) {
    case '(': {
        BracketRequest r( container->document()->leftBracketChar(),
                          container->document()->rightBracketChar() );
        singlePipe = true;
        return buildCommand( container, &r );
    }
    case '[': {
        BracketRequest r( LeftSquareBracket, RightSquareBracket );
        singlePipe = true;
        return buildCommand( container, &r );
    }
    case '{': {
        BracketRequest r( LeftCurlyBracket, RightCurlyBracket );
        singlePipe = true;
        return buildCommand( container, &r );
    }
    case '|': {	
        if (!singlePipe) { // We have had 2 '|' in a row so we want brackets

          DirectedRemove rDelete( req_remove, beforeCursor ); //Delete the previous '|' we dont need it any more
          KCommand* command = buildCommand( container, &rDelete );
          command->execute();
          
          BracketRequest rBracket( LeftLineBracket , RightLineBracket);
          singlePipe = true;  //the next '|' will be a single pipe again
          return buildCommand( container, &rBracket );    
        }
        else { // We really do only want 1 '|'
          TextCharRequest r(ch);

          //in case another '|' character is entered right after this one, '| |' brackets are made; see above
          singlePipe = false;

          return buildCommand( container, &r );   
        }
    }
    case '^': {
        IndexRequest r( upperRightPos );
        singlePipe = true;
        return buildCommand( container, &r );
    }
    case '_': {
        IndexRequest r( lowerRightPos );
        singlePipe = true;
        return buildCommand( container, &r );
    }
    case ' ': {
        Request r( req_compactExpression );
        singlePipe = true;
        return buildCommand( container, &r );
    }
    case '}': {
        Request r( req_addEmptyBox );
        singlePipe = true;
        return buildCommand( container, &r );
    }
    case ']':
    case ')':
        singlePipe = true; 
        break;
    case '\\': {
        Request r( req_addNameSequence );
        singlePipe = true;
        return buildCommand( container, &r );
    }
    default: {
        TextCharRequest r( ch );
        singlePipe = true;
        return buildCommand( container, &r );
    }
    }
    return 0;
}

/**
 * Stores the given childrens dom in the element.
 */
void SequenceElement::getChildrenDom( QDomDocument& doc, QDomElement elem,
                                     uint from, uint to)
{
    for (uint i = from; i < to; i++) {
        QDomElement tmpEleDom=children.at(i)->getElementDom(doc);
	elem.appendChild(tmpEleDom);
    }
}


/**
 * Builds elements from the given node and its siblings and
 * puts them into the list.
 * Returns false if an error occures.
 */
bool SequenceElement::buildChildrenFromDom(Q3PtrList<BasicElement>& list, QDomNode n)
{
    while (!n.isNull()) {
        if (n.isElement()) {
            QDomElement e = n.toElement();
            BasicElement* child = 0;
            QString tag = e.tagName().toUpper();

            child = createElement(tag);
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
    parse();
    return true;
}


BasicElement* SequenceElement::createElement( QString type )
{
    return creationStrategy->createElement( type );
}

/**
 * Appends our attributes to the dom element.
 */
void SequenceElement::writeDom(QDomElement element)
{
    BasicElement::writeDom(element);

    uint count = children.count();
    QDomDocument doc = element.ownerDocument();
    getChildrenDom(doc, element, 0, count);
}

/**
 * Reads our attributes from the element.
 * Returns false if it failed.
 */
bool SequenceElement::readAttributesFromDom(QDomElement element)
{
    if (!BasicElement::readAttributesFromDom(element)) {
        return false;
    }
    return true;
}

/**
 * Reads our content from the node. Sets the node to the next node
 * that needs to be read.
 * Returns false if it failed.
 */
bool SequenceElement::readContentFromDom(QDomNode& node)
{
    if (!BasicElement::readContentFromDom(node)) {
        return false;
    }

    return buildChildrenFromDom(children, node);
}


void SequenceElement::parse()
{
    delete parseTree;

    textSequence = true;
    for (BasicElement* element = children.first();
         element != 0;
         element = children.next()) {

        // Those types are gone. Make sure they won't
        // be used.
        element->setElementType(0);

        if (element->getCharacter().isNull()) {
            textSequence = false;
        }
    }

    const SymbolTable& symbols = formula()->getSymbolTable();
    SequenceParser parser(symbols);
    parseTree = parser.parse(children);

    // With the IndexElement dynamically changing its text/non-text
    // behaviour we need to reparse your parent, too. Hacky!
    BasicElement* p = getParent();
    if ( p != 0 ) {
        SequenceElement* seq = dynamic_cast<SequenceElement*>( p->getParent() );
        if ( seq != 0 ) {
            seq->parse();
        }
    }
    // debug
    //parseTree->output();
}


bool SequenceElement::isFirstOfToken( BasicElement* child )
{
    return ( child->getElementType() != 0 ) && isChildNumber( child->getElementType()->start(), child );
}


QString SequenceElement::toLatex()
{
    QString content;
    uint count = children.count();
    for ( uint i = 0; i < count; i++ ) {
        BasicElement* child = children.at( i );
//         if ( isFirstOfToken( child ) ) {
//             content += "";
//         }
        content += child->toLatex();
    }
    return content;
}


QString SequenceElement::formulaString()
{
    QString content;
    uint count = children.count();
    for ( uint i = 0; i < count; i++ ) {
        BasicElement* child = children.at( i );
        //if ( isFirstOfToken( child ) ) {
        //    content += " ";
        //}
        content += child->formulaString();
    }
    return content;
}


void SequenceElement::writeMathML( QDomDocument& doc, QDomNode parent, bool oasisFormat )
{
    QDomElement de = doc.createElement( oasisFormat ? "math:mrow" : "mrow" );

    BasicElement* last = children.last();
    if ( last != 0 ) {
        // Create a list (right order!)
        Q3PtrList<ElementType> tokenList;
        ElementType* token = last->getElementType();
        while ( token != 0 ) {
            // Add to the list.
            tokenList.prepend( token );
            token = token->getPrev();
        }

        if ( tokenList.count() == 1 ) {
            tokenList.first()->saveMathML( this, doc, parent.toElement(), oasisFormat );
            return;
        }

        for ( uint i = 0; i < tokenList.count(); ++i ) {
            tokenList.at( i )->saveMathML( this, doc, de, oasisFormat );
        }
    }
    parent.appendChild( de );
}


int SequenceElement::childPos( const BasicElement* child ) const
{
    Q3PtrListIterator<BasicElement> it( children );
    uint count = it.count();
    for ( uint i=0; i<count; ++i, ++it ) {
        if ( it.current() == child ) {
            return i;
        }
    }
    return -1;
}


NameSequence::NameSequence( BasicElement* parent )
    : SequenceElement( parent )
{
}


bool NameSequence::accept( ElementVisitor* visitor )
{
    return visitor->visit( this );
}


void NameSequence::calcCursorSize( const ContextStyle& context,
                                   FormulaCursor* cursor, bool smallCursor )
{
    inherited::calcCursorSize( context, cursor, smallCursor );
    LuPixelPoint point = widgetPos();
    luPixel unitX = context.ptToLayoutUnitPixX( 1 );
    luPixel unitY = context.ptToLayoutUnitPixY( 1 );
    cursor->addCursorSize( LuPixelRect( point.x()-unitX, point.y()-unitY,
                                        getWidth()+2*unitX, getHeight()+2*unitY ) );
}

void NameSequence::drawCursor( QPainter& painter, const ContextStyle& context,
                               FormulaCursor* cursor, bool smallCursor,
                               bool activeCursor )
{
    LuPixelPoint point = widgetPos();
    painter.setPen( QPen( context.getEmptyColor(),
                          context.layoutUnitToPixelX( context.getLineWidth()/2 ) ) );
    luPixel unitX = context.ptToLayoutUnitPixX( 1 );
    luPixel unitY = context.ptToLayoutUnitPixY( 1 );
    painter.drawRect( context.layoutUnitToPixelX( point.x()-unitX ),
                      context.layoutUnitToPixelY( point.y()-unitY ),
                      context.layoutUnitToPixelX( getWidth()+2*unitX ),
                      context.layoutUnitToPixelY( getHeight()+2*unitY ) );

    inherited::drawCursor( painter, context, cursor, smallCursor, activeCursor );
}

void NameSequence::moveWordLeft( FormulaCursor* cursor )
{
    uint pos = cursor->getPos();
    if ( pos > 0 ) {
        cursor->setTo( this, 0 );
    }
    else {
        moveLeft( cursor, this );
    }
}

void NameSequence::moveWordRight( FormulaCursor* cursor )
{
    int pos = cursor->getPos();
    if ( pos < countChildren() ) {
        cursor->setTo( this, countChildren() );
    }
    else {
        moveRight( cursor, this );
    }
}


KCommand* NameSequence::compactExpressionCmd( Container* container )
{
    BasicElement* element = replaceElement( container->document()->getSymbolTable() );
    if ( element != 0 ) {
        getParent()->selectChild( container->activeCursor(), this );

        KFCReplace* command = new KFCReplace( i18n( "Add Element" ), container );
        command->addElement( element );
        return command;
    }
    return 0;
}

KCommand* NameSequence::buildCommand( Container* container, Request* request )
{
    switch ( *request ) {
    case req_compactExpression:
        return compactExpressionCmd( container );
    case req_addSpace:
    case req_addIndex:
    case req_addMatrix:
    case req_addOneByTwoMatrix:
    case req_addSymbol:
    case req_addRoot:
    case req_addFraction:
    case req_addBracket:
    case req_addNameSequence:
        return 0;
    default:
        break;
    }
    return inherited::buildCommand( container, request );
}


KCommand* NameSequence::input( Container* container, QChar ch )
{
    int unicode = ch.unicode();
    switch (unicode) {
    case '(':
    case '[':
    case '|':
    case '^':
    case '_':
    case '}':
    case ']':
    case ')':
    case '\\': {
//         KCommand* compact = compactExpressionCmd( container );
//         KCommand* cmd = static_cast<SequenceElement*>( getParent() )->input( container, ch );
//         if ( compact != 0 ) {
//             KMacroCommand* macro = new KMacroCommand( cmd->name() );
//             macro->addCommand( compact );
//             macro->addCommand( cmd );
//             return macro;
//         }
//         else {
//             return cmd;
//         }
        break;
    }
    case '{':
    case ' ': {
        Request r( req_compactExpression );
        return buildCommand( container, &r );
    }
    default: {
        TextCharRequest r( ch );
        return buildCommand( container, &r );
    }
    }
    return 0;
}

void NameSequence::setElementType( ElementType* t )
{
    inherited::setElementType( t );
    parse();
}

BasicElement* NameSequence::replaceElement( const SymbolTable& table )
{
    QString name = buildName();
    QChar ch = table.unicode( name );
    if ( !ch.isNull() ) {
        return new TextElement( ch, true );
    }
    else {
        ch = table.unicode( i18n( name.toLatin1() ) );
        if ( !ch.isNull() ) {
            return new TextElement( ch, true );
        }
    }

    if ( name == "!" )    return new SpaceElement( NEGTHIN );
    if ( name == "," )    return new SpaceElement( THIN );
    if ( name == ">" )    return new SpaceElement( MEDIUM );
    if ( name == ";" )    return new SpaceElement( THICK );
    if ( name == "quad" ) return new SpaceElement( QUAD );

    if ( name == "frac" ) return new FractionElement();
    if ( name == "atop" ) {
        FractionElement* frac = new FractionElement();
        frac->showLine( false );
        return frac;
    }
    if ( name == "sqrt" ) return new RootElement();

    return 0;
}

BasicElement* NameSequence::createElement( QString type )
{
    if      ( type == "TEXT" )         return new TextElement();
    return 0;
}

// void NameSequence::parse()
// {
//     // A name sequence is known as name and so are its children.
//     // Caution: this is fake!
//     for ( int i = 0; i < countChildren(); i++ ) {
//         getChild( i )->setElementType( getElementType() );
//     }
// }

QString NameSequence::buildName()
{
    QString name;
    for ( int i = 0; i < countChildren(); i++ ) {
        name += getChild( i )->getCharacter();
    }
    return name;
}

bool NameSequence::isValidSelection( FormulaCursor* cursor )
{
    SequenceElement* sequence = cursor->normal();
    if ( sequence == 0 ) {
        return false;
    }
    return sequence->onlyTextSelected( cursor );
}

void NameSequence::writeMathML( QDomDocument& doc, QDomNode parent,bool oasisFormat )
{
    QDomElement de = doc.createElement( oasisFormat ? "math:mi" : "mi" );
    QString value;
    for ( int i = 0; i < countChildren(); ++i ) {
        // these are supposed to by TextElements
        value += getChild( i )->getCharacter();
    }
    de.appendChild( doc.createTextNode( value ) );
    parent.appendChild( de );
}

KFORMULA_NAMESPACE_END
