/* This file is part of the KDE project
   Copyright (C) 2009 Jeremias Epperlein <jeeree@web.de>

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

#include "FixedElement.h"
#include "FormulaCursor.h"
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <QPainter>

#include <kdebug.h>

FixedElement::FixedElement( BasicElement* parent ) : BasicElement( parent )
{
}

FixedElement::~FixedElement()
{
}


BasicElement* FixedElement::elementAfter ( int position ) const
{
    if (position % 2 == 0) {
        return elementNext(position);
    } else {
        return 0;
    }
}

BasicElement* FixedElement::elementBefore ( int position ) const
{
    if (position % 2 == 1) {
        return elementNext(position);
    } else {
        return 0;
    }
}

BasicElement* FixedElement::elementNext ( int position ) const
{
        return childElements()[position/2];
}


QPainterPath FixedElement::selectionRegion(const int pos1, const int pos2) const 
{
    QPainterPath temp;
    Q_UNUSED(pos1);
    Q_UNUSED(pos2);
    return temp;
}

bool FixedElement::moveHorSituation(FormulaCursor& newcursor, FormulaCursor& oldcursor, int pos1, int pos2) {
    if ((newcursor.position()/2==pos1 && newcursor.direction()==MoveUp) ||
        (newcursor.position()/2==pos2 && newcursor.direction()==MoveDown) ||
        (newcursor.position()==2*pos1 && newcursor.direction()==MoveLeft) ||
        (newcursor.position()==2*pos2+1 && newcursor.direction()==MoveRight) ) {
        return false;
    }
    switch (newcursor.direction()) {
    case MoveLeft:
        if (newcursor.position()==2*pos2+1) {
            newcursor.moveTo(newcursor.currentElement()->childElements()[pos2]);
        } else {
            newcursor.moveTo(newcursor.currentElement()->childElements()[pos1]);
        }
        break;
    case MoveRight:
        if (newcursor.position()==2*pos1) {
            newcursor.moveTo(newcursor.currentElement()->childElements()[pos1]);
        } else {
            newcursor.moveTo(newcursor.currentElement()->childElements()[pos2]);
        }
        break;
    case MoveUp:
    case MoveDown:
        return newcursor.moveCloseTo(childElements()[newcursor.direction()==MoveUp ? pos1 : pos2],oldcursor);
    case NoDirection:
        break;
    }
    return true;
}

bool FixedElement::moveVertSituation(FormulaCursor& newcursor, FormulaCursor& oldcursor, int pos1, int pos2) {
    if ((newcursor.position()/2==pos1 && newcursor.direction()==MoveUp) ||
        (newcursor.position()/2==pos2 && newcursor.direction()==MoveDown) ||
        (newcursor.position()%2==0 && newcursor.direction()==MoveLeft) ||
        (newcursor.position()%2==1 && newcursor.direction()==MoveRight) ) {
        return false;
    }
    switch (newcursor.direction()) {
    case MoveLeft:
    case MoveRight:
        if (newcursor.position()/2==pos1) {
            newcursor.moveTo(newcursor.currentElement()->childElements()[pos1]);
        } else {
            newcursor.moveTo(newcursor.currentElement()->childElements()[pos2]);
        }
        break;
    case MoveUp:
    case MoveDown:
        return newcursor.moveCloseTo(childElements()[newcursor.direction()==MoveUp ? pos1 : pos2],oldcursor);
    case NoDirection:
        break;
    }
    return true;
}

bool FixedElement::moveSingleSituation ( FormulaCursor& newcursor, FormulaCursor& oldcursor, int pos )
{
    Q_UNUSED( oldcursor )
    switch (newcursor.direction()) {
    case MoveLeft:
        if (newcursor.position()%2==1) {
            newcursor.moveTo(newcursor.currentElement()->childElements()[pos]);
            break;
        }
        return false;
    case MoveRight:
        if (newcursor.position()%2==0) {
            newcursor.moveTo(newcursor.currentElement()->childElements()[pos]);
            break;
        }
    case MoveUp:
    case MoveDown:
        return false;
    case NoDirection:
        break;
    }
    return true;
}


bool FixedElement::acceptCursor ( const FormulaCursor& cursor )
{
    Q_UNUSED (cursor)
    return false;
}

QLineF FixedElement::cursorLine ( int position ) const
{
    QRectF tmp;
    if (position%2==1) {
        tmp=elementBefore(position)->absoluteBoundingRect();
        return QLineF(tmp.topRight(),tmp.bottomRight());
    } else {
        tmp=elementAfter(position)->absoluteBoundingRect();
        return QLineF(tmp.topLeft(),tmp.bottomLeft());
    }
}

int FixedElement::positionOfChild ( BasicElement* child ) const
{
    int tmp=childElements().indexOf(child);
    if (tmp==-1) {
        return -1;
    } else {
        return 2*tmp;
    }
}

bool FixedElement::loadElement ( KoXmlElement& tmp, RowElement** child )
{
    BasicElement *element;
    element = ElementFactory::createElement( tmp.tagName(), this );
    if( !element->readMathML( tmp ) ) {
        return false;
    }
    if (element->elementType()==Row) {
        delete (*child);
        (*child)=static_cast<RowElement*>(element);
    } else {
        (*child)->insertChild(0,element);
    }
    return true;
}


int FixedElement::endPosition() const
{
    return childElements().length()*2-1;
}
