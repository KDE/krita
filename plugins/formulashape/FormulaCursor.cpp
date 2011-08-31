/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
                      Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
                 2006 Martin Pfeiffer <hubipete@gmx.net>
                 2009 Jeremias Epperlein <jeeree@web.de>

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

#include "FormulaCursor.h"
#include "BasicElement.h"
#include "RowElement.h"
#include "FixedElement.h"
#include "NumberElement.h"
#include "ElementFactory.h"
#include "OperatorElement.h"
#include "IdentifierElement.h"
#include "ElementFactory.h"
#include "FormulaCommand.h"
#include <QPainter>
#include <QPen>
#include <algorithm>

#include <kdebug.h>
#include <kundo2command.h>
#include <KoOdfLoadingContext.h>
#include <KoOdfStylesReader.h>

FormulaCursor::FormulaCursor(BasicElement* element, bool selecting, int position, int mark) {
    m_currentElement=element;
    m_selecting=selecting;
    m_position=position;
    m_mark=mark;
}

FormulaCursor::FormulaCursor ( BasicElement* element, int position )
{
    m_currentElement=element;
    m_position=position;
    m_mark=0;
    m_selecting=false;
}


FormulaCursor::FormulaCursor()
{
    FormulaCursor(0,0);
}

FormulaCursor::FormulaCursor (const FormulaCursor& other )
{
    m_currentElement=other.currentElement();
    m_position=other.position();
    m_mark=other.mark();
    m_selecting=other.isSelecting();
}


void FormulaCursor::paint( QPainter& painter ) const
{
    kDebug() << "Drawing cursor with selecting: "<< isSelecting() << " from "
    << mark()<<" to " << position() << " in "<<ElementFactory::elementName(m_currentElement->elementType());
    if( !m_currentElement )
        return;
    painter.save();
    QPointF origin=m_currentElement->absoluteBoundingRect().topLeft();
    qreal baseline=m_currentElement->baseLine();
    QPen pen;
    pen.setWidthF( 0.5 );
    pen.setColor(Qt::black);
    painter.setPen( pen );
    painter.drawLine(m_currentElement->cursorLine( m_position ));
    pen.setWidth( 0.1);
    pen.setColor(Qt::blue);
    pen.setStyle(Qt::DashLine);
    painter.setPen( pen );
    painter.drawLine( origin+QPointF(0.0,baseline),origin+QPointF(m_currentElement->width(), baseline) );
    pen.setStyle(Qt::DotLine);
    //Only here for debug purpose for now
    switch(m_currentElement->elementType()) {
    case Number:
        pen.setColor(Qt::red);
        break;
    case Identifier:
        pen.setColor(Qt::darkRed);
        break;
    case Row:
        pen.setColor(Qt::yellow);
        break;
    case Fraction:
        pen.setColor(Qt::blue);
        break;
    case Table:
        pen.setColor(Qt::darkGreen);
        break;
    case TableRow:
        pen.setColor(Qt::green);
        break;
    default:
        pen.setColor(Qt::darkGray);
        break;
    }
    painter.setPen(pen);
    painter.drawRect( m_currentElement->absoluteBoundingRect() );
    //draw the selection rectangle
    if ( m_selecting ) {
        QBrush brush;
        QColor color(Qt::blue);
        color.setAlpha(128);
        brush.setColor(color);
        brush.setStyle(Qt::SolidPattern);
        painter.setBrush(brush);
        painter.setPen(Qt::NoPen);
        int p1=position()<mark()? position() : mark();
        int p2=position()<mark()? mark() : position() ;
        painter.drawPath(m_currentElement->selectionRegion(p1,p2));
    }
    painter.restore();
}

void FormulaCursor::selectElement(BasicElement* element)
{
    m_selecting=true;
    m_currentElement=element;
    m_mark=0;
    m_position=m_currentElement->endPosition();
}

void FormulaCursor::move( CursorDirection direction )
{
    FormulaCursor oldcursor(*this);
    m_direction = direction;
    if (performMovement(oldcursor)==false) {
        (*this)=oldcursor;
    }
    m_direction=NoDirection;
}

bool FormulaCursor::moveCloseTo(BasicElement* element, FormulaCursor& cursor)
{
    if (element->setCursorTo(*this,cursor.getCursorPosition()-element->absoluteBoundingRect().topLeft())) {
        return true;
    } else {
        return false;
    }
}

QPointF FormulaCursor::getCursorPosition() 
{
    return ( m_currentElement->cursorLine(m_position).p1()
           + m_currentElement->cursorLine(m_position).p2())/2.;
}

void FormulaCursor::moveTo ( const FormulaCursor& pos )
{
    m_currentElement=pos.currentElement();
    m_position=pos.position();
    m_selecting=pos.isSelecting();
    m_mark=pos.mark();
}


void FormulaCursor::moveTo ( BasicElement* element )
{
    moveTo(element,0);
    if (direction()==MoveLeft) {
        moveEnd();
    }
}


void FormulaCursor::moveTo ( BasicElement* element, int position )
{
    moveTo(FormulaCursor(element,position));
}


void FormulaCursor::setCursorTo( const QPointF& point )
{
    if (m_selecting) {
        while (!m_currentElement->absoluteBoundingRect().contains(point)) {
            if ( m_currentElement->parentElement() ) {
                m_position=0;
                if (point.x()<m_currentElement->cursorLine(m_mark).p1().x()) {
                    //the point is left of the old selection start, so we move the selection 
                    //start after the old current element
                    m_mark=m_currentElement->parentElement()->positionOfChild(m_currentElement)+1;
                } else {
                    m_mark=m_currentElement->parentElement()->positionOfChild(m_currentElement);
                }
                m_currentElement=m_currentElement->parentElement();
            } else {
                return;
            }
        }
        while (!m_currentElement->setCursorTo(*this,point-m_currentElement->absoluteBoundingRect().topLeft())) {
            if ( m_currentElement->parentElement() ) {
                m_mark=m_currentElement->parentElement()->positionOfChild(m_currentElement);
                m_position=0;
                if (point.x()<m_currentElement->cursorLine(m_mark).p1().x()) {
                    //the point is left of the old selection start, so we move the selection 
                    //start after the old current element
                    m_mark++;
                }
                m_currentElement=m_currentElement->parentElement();
            } else {
                    return;
            }
        }
    } else {
        BasicElement* formulaElement = m_currentElement;
        while( formulaElement->parentElement() != 0 ) {
            formulaElement = formulaElement->parentElement();
        }
        formulaElement->setCursorTo(*this,point);
    }
}

int FormulaCursor::mark() const 
{
    return m_mark;
}

void FormulaCursor::moveHome()
{
    m_position = 0;
}

void FormulaCursor::moveEnd()
{
    m_position=m_currentElement->endPosition();
}

bool FormulaCursor::isHome() const
{
    return m_position == 0;
}

bool FormulaCursor::isEnd() const
{
    return m_position == m_currentElement->endPosition();
}

bool FormulaCursor::insideToken() const
{
    if( m_currentElement->elementType() == Number ||
        m_currentElement->elementType() == Operator ||
        m_currentElement->elementType() == Identifier ) {
        return true;
    }
    return false;
}

bool FormulaCursor::insideInferredRow() const
{
    return m_currentElement->isInferredRow();
}

bool FormulaCursor::insideFixedElement() const
{
    if (m_currentElement->elementType() == Fraction ||
        m_currentElement->elementType() == Root ||
        m_currentElement->elementType() == SubScript ||
        m_currentElement->elementType() == SupScript ||
        m_currentElement->elementType() == SubScript ||
        m_currentElement->elementType() == SubSupScript ) {
        return true;
    }
    return false;
}
 


BasicElement* FormulaCursor::currentElement() const
{
    return m_currentElement;
}

int FormulaCursor::position() const
{
    return m_position;
}

void FormulaCursor::setCurrentElement(BasicElement* element) {
    m_currentElement=element;
}

void FormulaCursor::setPosition(int position) {
    m_position=position;
}

CursorDirection FormulaCursor::direction() const
{
    return m_direction;
}

bool FormulaCursor::isSelecting() const
{
    return m_selecting;
}

void FormulaCursor::setSelecting( bool selecting )
{
    if (selecting) {
        if (!m_selecting) {
            //we start a new selection
            m_selecting = selecting;
            m_mark=m_position;
        }
    } else {
        m_selecting = selecting;
        m_mark=0;
    }
}

void FormulaCursor::setMark(int position) {
    m_mark=position;
}

QPair< int,int > FormulaCursor::selection() const
{
    if (m_mark<m_position) {
        return QPair<int,int>(m_mark,m_position);
    } else {
        return QPair<int,int>(m_position,m_mark);
    }
}


bool FormulaCursor::hasSelection() const
{
    return (m_selecting && m_mark!=m_position);
}


bool FormulaCursor::isAccepted() const
{
    if (mark()<0 || mark()>m_currentElement->endPosition() ||
        position()<0 || position()>m_currentElement->endPosition()) {
        return false;
    }
    return m_currentElement->acceptCursor(*this);
}

bool FormulaCursor::performMovement ( FormulaCursor& oldcursor )
{
    //handle selecting and not selecting case separately, which makes more clear
    if (isSelecting()) {
        while ( m_currentElement ) {
            if ( m_currentElement->moveCursor( *this, oldcursor ) ) {
                if (isAccepted()) {
                    return true;
                }
            } else {
                if ( m_currentElement->parentElement() ) {
                    bool ltr=m_mark<=m_position;
                    //update the starting point of the selection
                    m_mark=m_currentElement->parentElement()->positionOfChild(m_currentElement);
                    //move the cursor to the parent and place it before the old element
                    m_position=m_currentElement->parentElement()->positionOfChild(m_currentElement);
                    m_currentElement=m_currentElement->parentElement();
                    if (ltr) {
                        m_position++; //place the cursor behind
                    } else {
                        m_mark++; //place the selection beginning behind 
                    }
                    if (isAccepted()) {
                        return true;
                    }
                } else {
                    //we arrived at the toplevel element
                    return false;
                }
            }
        }
    } else {
        while ( m_currentElement ) {
            if ( m_currentElement->moveCursor( *this, oldcursor ) ) {
                if (isAccepted()) {
                    return true;
                }
            } else {
                if ( m_currentElement->parentElement() ) {
                    //move the cursor to the parent and place it before the old element
                    m_position=m_currentElement->parentElement()->positionOfChild(m_currentElement);
                    m_currentElement=m_currentElement->parentElement();
                    if (m_direction==MoveRight || m_direction==MoveDown) {
                        m_position++; //place the cursor behin
                    }
                    if (m_direction==MoveRight || m_direction==MoveLeft) {
                        if (isAccepted()) {
                            return true;
                        }
                    }   
                } else {
                    //We arrived at the top level element
                    return false;
                }
            }
        }
    }
    return false;
}

FormulaCursor& FormulaCursor::operator+= ( int step )
{
    m_position+=step;
    return *this;
}

int FormulaCursor::offset ( )
{
    if (m_direction==MoveDown || m_direction==MoveRight) {
        return -1;
    } else if (m_direction==MoveUp || m_direction==MoveLeft) {
        return 1;
    }
    return 0;
}

