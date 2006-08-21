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

#include "SequenceElement.h"

#include <QPainter>
#include <QPaintDevice>
#include <QStack>
#include <QKeyEvent>


#include <kcommand.h>
#include <kdebug.h>
#include <klocale.h>

#include "MatrixDialog.h"
#include "BracketElement.h"
#include "FormulaCursor.h"
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

SequenceElement::SequenceElement( BasicElement* parent ) : BasicElement(parent),  textSequence(true),singlePipe(true)
{
}

SequenceElement::~SequenceElement()
{
}

BasicElement* SequenceElement::childAt( int i )
{
    return m_sequenceChildren[ i ];
}

void SequenceElement::drawInternal()
{
}   

void SequenceElement::readMathML( const QDomElement& element )
{
}

void SequenceElement::readMathMLAttributes( const QDomElement& element )
{
}

void SequenceElement::writeMathML( const KoXmlWriter* writer, bool oasisFormat )
{
/*    QDomElement de = doc.createElement( oasisFormat ? "math:mrow" : "mrow" );

    foreach( BasicElement* tmpChild, m_sequenceChildren )
        tmpChild->writeMathML( doc, de, oasisFormat );

    parent.appendChild( de );*/
}




bool SequenceElement::readOnly( const FormulaCursor* ) const
{
    return getParent()->readOnly( this );
}

const QList<BasicElement*>& SequenceElement::childElements()
{
    return QList<BasicElement*>();
}


bool SequenceElement::isEmpty()
{
    uint count = m_sequenceChildren.count();
    for (uint i = 0; i < count; i++) {
        BasicElement* child = m_sequenceChildren.at(i);
        if (!child->isInvisible()) {
            return false;
        }
    }
    return true;
}


/**
 * Calculates our width and height and
 * our m_sequenceChildren's parentPosition.
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
        foreach( BasicElement* child, m_sequenceChildren  )
       	{
            luPixel spaceBefore = 0;
            if ( isFirstOfToken( child ) ) {
                //spaceBefore =
                //    style.ptToPixelX( child->getElementType()->getSpaceBefore( style,
                //                                                               tstyle ) );
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
    QPointF myPos( parentOrigin.x() + getX(), parentOrigin.y() + getY() );

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
    }
}

/*
void SequenceElement::dispatchFontCommand( FontCommand* cmd )
{
  foreach( BasicElement* child, m_sequenceChildren )
    child->dispatchFontCommand( cmd );
}
*/

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
        cursor->setTo(this, m_sequenceChildren.count());
        //from->entered( this );
    }

    // We already owned the cursor. Ask next child then.
    else if (from == this) {
        if (cursor->getPos() > 0) {
            if (cursor->isSelectionMode()) {
                cursor->setTo(this, cursor->getPos()-1);

                // invisible elements are not visible so we move on.
                if (m_sequenceChildren.at(cursor->getPos())->isInvisible()) {
                    moveLeft(cursor, this);
                }
            }
            else {
                m_sequenceChildren.at(cursor->getPos()-1)->moveLeft(cursor, this);
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

    // The cursor came from one of our m_sequenceChildren or
    // something is wrong.
    else {
        int fromPos = m_sequenceChildren.indexOf(from);
        cursor->setTo(this, fromPos);
        if (cursor->isSelectionMode()) {
            cursor->setMark(fromPos+1);
        }

        // invisible elements are not visible so we move on.
        if (from->isInvisible()) {
            moveLeft(cursor, this);
        }
        //formula()->tell( "" );
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
        //from->entered( this );
    }

    // We already owned the cursor. Ask next child then.
    else if (from == this) {
        int pos = cursor->getPos();
        if (pos < m_sequenceChildren.count()) {
            if (cursor->isSelectionMode()) {
                cursor->setTo(this, pos+1);

                // invisible elements are not visible so we move on.
                if (m_sequenceChildren.at(pos)->isInvisible()) {
                    moveRight(cursor, this);
                }
            }
            else {
                m_sequenceChildren.at(pos)->moveRight(cursor, this);
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

    // The cursor came from one of our m_sequenceChildren or
    // something is wrong.
    else {
        int fromPos = m_sequenceChildren.indexOf(from);
        cursor->setTo(this, fromPos+1);
        if (cursor->isSelectionMode()) {
            cursor->setMark(fromPos);
        }

        // invisible elements are not visible so we move on.
        if (from->isInvisible()) {
            moveRight(cursor, this);
        }
        //formula()->tell( "" );
    }
}


void SequenceElement::moveWordLeft(FormulaCursor* cursor)
{
/*    uint pos = cursor->getPos();
    if (pos > 0) {
        ElementType* type = m_sequenceChildren.at(pos-1)->getElementType();
        if (type != 0) {
            cursor->setTo(this, type->start());
        }
    }
    else {
        moveLeft(cursor, this);
    }*/
}


void SequenceElement::moveWordRight(FormulaCursor* cursor)
{
/*    int pos = cursor->getPos();
    if (pos < m_sequenceChildren.count()) {
        ElementType* type = m_sequenceChildren.at(pos)->getElementType();
        if (type != 0) {
            cursor->setTo(this, type->end());
        }
    }
    else {
        moveRight(cursor, this);
    }*/
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
            cursor->setMark(m_sequenceChildren.indexOf(element)+1);
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
                    cursor->setMark(m_sequenceChildren.count());
                    break;
                }
            }
            if( element )
                cursor->setMark(m_sequenceChildren.indexOf(element));
        }
    }
    cursor->setTo(this, m_sequenceChildren.count());
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


// m_sequenceChildren

/**
 * Removes the child. If this was the main child this element might
 * request its own removal.
 * The cursor is the one that caused the removal. It has to be moved
 * to the place any user expects the cursor after that particular
 * element has been removed.
 */
// void SequenceElement::removeChild(FormulaCursor* cursor, BasicElement* child)
// {
//     int pos = m_sequenceChildren.find(child);
//     formula()->elementRemoval(child, pos);
//     cursor->setTo(this, pos);
//     m_sequenceChildren.remove(pos);
//     /*
//         if len(self.m_sequenceChildren) == 0:
//             if self.parent != None:
//                 self.parent.removeChild(cursor, self)
//                 return
//     */
//     formula()->changed();
// }


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
    int pos = cursor->getPos();
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

    formula()->changed();
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
    }
    //parse();
}


/**
 * Removes the m_sequenceChildren at pos and appends it to the list.
 */
void SequenceElement::removeChild(QList<BasicElement*>& removedChildren, int pos)
{
    BasicElement* child = m_sequenceChildren.at(pos);
    formula()->elementRemoval(child);
    m_sequenceChildren.takeAt(pos);
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
            return m_sequenceChildren.at( cursor->getPos() - 1 );
        }
    }
    else {
        if ( cursor->getPos() < qRound( m_sequenceChildren.count() ) ) {
            return m_sequenceChildren.at( cursor->getPos() );
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
    int pos = m_sequenceChildren.indexOf(child);
    if (pos > -1) {
        cursor->setTo(this, pos+1, pos);
    }
}

void SequenceElement::childWillVanish(FormulaCursor* cursor, BasicElement* child)
{
    int childPos = m_sequenceChildren.indexOf(child);
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
 * Selects all m_sequenceChildren. The cursor is put behind, the mark before them.
 */
void SequenceElement::selectAllChildren(FormulaCursor* cursor)
{
    cursor->setTo(this, m_sequenceChildren.count(), 0);
}

bool SequenceElement::onlyTextSelected( FormulaCursor* cursor )
{
    if ( cursor->isSelection() ) {
        uint from = qMin( cursor->getPos(), cursor->getMark() );
        uint to = qMax( cursor->getPos(), cursor->getMark() );
        for ( uint i = from; i < to; i++ ) {
            BasicElement* element = childAt( i );
            if ( element->getCharacter() == QChar::Null ) {
                return false;
            }
        }
    }
    return true;
}

/*
KCommand* SequenceElement::buildCommand( Container* container, Request* request )
{
    FormulaCursor* cursor = container->activeCursor();
    if ( cursor->isReadOnly() ) {
        //formula()->tell( i18n( "write protection" ) );
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
        MatrixRowElement* matrixrow = creationStrategy->createMatrixRowElement();
        if ( matrixrow != 0 ) {
            KFCAddReplacing* command = new KFCAddReplacing(i18n("Add Matrixrow"), container);
            command->setElement( matrixrow );
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
            //element->showLine(false);
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
                dynamic_cast<IndexElement*>( m_sequenceChildren.at( cursor->getPos()-1 ) );
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
        if ( sequence &&   ( sequence == sequence->formula() ) && ( sequence->countChildren() == 0 ) ) {
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
                cmd->addElement( m_sequenceChildren.at( i ) );
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
                cmd->addElement( m_sequenceChildren.at( i ) );
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
*/
/*
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
}*/

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

bool SequenceElement::isFirstOfToken( BasicElement* child )
{
    return true;//( child->getElementType() != 0 ) && isChildNumber( child->getElementType()->start(), child );
}

} // namespace KFormula
