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

#include "RowElement.h"
#include "FormulaCursor.h"
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <KoXmlNS.h>
#include <QPainter>

#include <kdebug.h>

RowElement::RowElement( BasicElement* parent ) : BasicElement( parent )
{}

RowElement::~RowElement()
{
    qDeleteAll( m_childElements );
}

void RowElement::paint( QPainter& painter, AttributeManager* am)
{
    /* RowElement has no visual representance so paint nothing */
    Q_UNUSED( painter )
    Q_UNUSED( am )
}

void RowElement::paintEditingHints ( QPainter& painter, AttributeManager* am )
{
    Q_UNUSED( am )
    if (childElements().count()==0) {
        painter.save();
        QBrush brush (Qt::NoBrush);
        brush.setColor( Qt::transparent);
        painter.setBrush(brush);
        painter.setPen( Qt::blue);
        painter.drawRect( childrenBoundingRect() );
        painter.restore();
    }
}

void RowElement::layout( const AttributeManager* am )
{
    Q_UNUSED( am )          // there are no attributes that can be processed here

    if( m_childElements.isEmpty() ) {
        //set standart values for empty formulas
        setOrigin( QPointF( 0.0, 0.0 ) );
        setWidth( 7.0 );       // standard values
        setHeight( 10.0 );
        setBaseLine( 10.0 );
        setChildrenBoundingRect(QRectF(0,0, width(), height()));
        return;
    }

    QPointF origin;
    qreal width = 0.0;
    qreal topToBaseline = 0.0;
    qreal baselineToBottom = 0.0;    
    foreach( BasicElement* child, m_childElements ) // iterate through the children and
        topToBaseline = qMax( topToBaseline, child->baseLine() );  // find max baseline

    foreach( BasicElement* child, m_childElements )  // iterate through the children
    {
        child->setOrigin( QPointF( width, topToBaseline - child->baseLine() ) );
        baselineToBottom = qMax( baselineToBottom, child->height()-child->baseLine() );
        width += child->width();       // add their width
    }

    setWidth( width );
    setHeight( topToBaseline + baselineToBottom );
    setBaseLine( topToBaseline );
    setChildrenBoundingRect(QRectF(0,0, width, height()));
}

void RowElement::stretch()
{
    //The elements can grow vertically, so make sure we reposition their vertical 
    //origin appropriately
/*    foreach( BasicElement* tmpElement, childElements() ) {
        tmpElement->stretch();
        //Set the origin.  Note that we ignore the baseline and center the object
        //vertically
        //I think we need to FIXME for symmetric situations or something?
        tmpElement->setOrigin( QPointF(tmpElement->origin().x(), childrenBoundingRect().y() + (childrenBoundingRect().height() - tmpElement->height())/2 ));
    }*/
}

int RowElement::endPosition() const
{
    return m_childElements.count();
}

const QList<BasicElement*> RowElement::childElements() const
{
    return m_childElements;
}

bool RowElement::insertChild( int position, BasicElement* child )
{
    if (0<=position && position<=endPosition()) {
        m_childElements.insert( position, child );
        child->setParentElement(this);
        return true;
    } else {
        return false;
    }
}

bool RowElement::removeChild( BasicElement* child )
{
    bool tmp=m_childElements.removeOne(child);
    if (tmp) {
        child->setParentElement(0);
    }
    return tmp;
}

bool RowElement::acceptCursor( const FormulaCursor& cursor )
{
    Q_UNUSED( cursor )
    return true;
}

bool RowElement::moveCursor(FormulaCursor& newcursor, FormulaCursor& oldcursor)
{
    Q_UNUSED (oldcursor)
    if ((newcursor.direction()==MoveUp) ||
        (newcursor.direction()==MoveDown) ||
        (newcursor.isHome() && newcursor.direction()==MoveLeft) ||
        (newcursor.isEnd() && newcursor.direction()==MoveRight) ) {
        //the newcursor can't be moved vertically
        //TODO: check what happens with linebreaks in <mspace> elements
        return false;
    }
    if (newcursor.isSelecting()) {
        switch(newcursor.direction()) {
        case MoveLeft:
            newcursor+=-1;
            break;
        case MoveRight:
            newcursor+=1;
            break;
        default:
            break;
        }
    } else {
        switch(newcursor.direction()) {
        case MoveLeft:
            newcursor.setCurrentElement(m_childElements[newcursor.position()-1]);
            newcursor.moveEnd();
            break;
        case MoveRight:
            newcursor.setCurrentElement(m_childElements[newcursor.position()]);
            newcursor.moveHome();
            break;
        default:
            break;
        }
    }
    return true;
}

QLineF RowElement::cursorLine(int position) const {
    QPointF top=absoluteBoundingRect().topLeft();
    if( childElements().isEmpty() ) {
        // center cursor in elements that have no children
        top += QPointF( width()/2, 0 );
    } else { 
        if ( position==endPosition()) {
            top += QPointF(width(),0.0);
        } else {
            top += QPointF( childElements()[ position ]->boundingRect().left(), 0.0 );
        }
    }
    QPointF bottom = top + QPointF( 0.0, height() );
    return QLineF(top, bottom);
}

bool RowElement::setCursorTo(FormulaCursor& cursor, QPointF point)
{
    if (m_childElements.isEmpty() || point.x()<m_childElements[0]->origin().x()) {
        cursor.moveTo(this,0);
        return true;
    }
    int i;
    for (i=0; i<m_childElements.count(); i++) {
        //Find the child element the point is in
        if (m_childElements[i]->boundingRect().right()>=point.x()) {
            break;
        }
    }
    //check if the point is behind all child elements
    if (i==m_childElements.count()) {
        cursor.moveTo(this,endPosition());
        return true;
    } else {
        if (cursor.isSelecting()) {
            //we don't need to change current element because we are already in this element
            if (cursor.mark()<=i) {
                cursor.setPosition(i+1);
            } else {
                cursor.setPosition(i);
            }
            return true;
        } else {
            return m_childElements[i]->setCursorTo(cursor,point-m_childElements[i]->origin());
        }
    }
}

ElementType RowElement::elementType() const
{
    return Row;
}

bool RowElement::readMathMLContent( const KoXmlElement& parent )
{
    KoXmlElement realParent = parent;

    // Go deeper in the xml tree and 'skip' the semantics elements.
    while (!realParent.namedItemNS( KoXmlNS::math, "semantics" ).isNull()) {            // while there is a child 'semantics'
        realParent = realParent.namedItemNS( KoXmlNS::math, "semantics" ).toElement();  // move to it
    }

    // Read the actual content.
    BasicElement* tmpElement = 0;
    KoXmlElement tmp;
    forEachElement ( tmp, realParent ) {
        tmpElement = ElementFactory::createElement( tmp.tagName(), this );
        Q_ASSERT( tmpElement );
        if ( !tmpElement->readMathML( tmp ) ) {
            return false;
        }

#if 1 // compact empty mrows and mrows with only one element
        // Treat rows in rows specially.
        if (tmpElement->elementType() == Row) {
            if (tmpElement->childElements().count() == 0) {
                // We don't load in this case, empty elements in rows are not needed.
            } else if (tmpElement->childElements().count() == 1) {
                // An mrow with 1 child is equivalent to the child itself.
                // So dig it out and place it directly in this row.
                //
                // TODO: Investigate, if we should load them nevertheless.
                RowElement   *row   = static_cast<RowElement*>(tmpElement);
                BasicElement *child = row->childElements()[0];
                row->removeChild(child);
                delete row;

                //insertChild(childElements().count(), child);
                m_childElements << child;
            } else {
                // If the mrow has > 1 child, then enter it
                m_childElements << tmpElement;
            }
        } else {
            // All other elements than mrow are immediately entered.
            m_childElements << tmpElement;
        }
#else
        m_childElements << tmpElement;
#endif
    }
    return true;
}

int RowElement::positionOfChild(BasicElement* child) const {
    return m_childElements.indexOf(child);
}

void RowElement::writeMathMLContent( KoXmlWriter* writer, const QString& ns ) const
{
    foreach( BasicElement* tmp, m_childElements )
        tmp->writeMathML( writer, ns );
}

BasicElement* RowElement::elementAfter ( int position ) const
{
    if (position<endPosition()) {
        return m_childElements[position];
    } else {
        return 0;
    }
}

BasicElement* RowElement::elementBefore ( int position ) const
{
    if (position>1) {
        return m_childElements[position-1];
    } else {
        return 0;
    }
}

QList< BasicElement* > RowElement::elementsBetween ( int pos1, int pos2 ) const
{
    return m_childElements.mid(pos1,pos2-pos1);
}


bool RowElement::replaceChild ( BasicElement* oldelement, BasicElement* newelement )
{
    int oldElementIndex = m_childElements.indexOf(oldelement);
    if ( oldElementIndex < 0)
        return false;

    m_childElements.replace(oldElementIndex,newelement);
    oldelement->setParentElement(0);
    newelement->setParentElement(this);

    return true;
}

bool RowElement::isEmpty() const
{
    return (elementType()==Row && m_childElements.count()==0);
}


bool RowElement::isInferredRow() const
{
    return true;
}

