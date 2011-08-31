/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
   Copyright (C) 2001 Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
   Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net>
   Copyright (C) 2006-2007 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>
                 2009 Jeremias Epperlein <jeeree@web.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You shouldlemente received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "TableRowElement.h"
#include "TableElement.h"
#include "FormulaCursor.h"
#include "TableDataElement.h"
#include "AttributeManager.h"
#include <KoXmlReader.h>
#include <QStringList>
#include <QPainter>
#include <kdebug.h>

TableRowElement::TableRowElement( BasicElement* parent ) : BasicElement( parent )
{}

TableRowElement::~TableRowElement()
{}

void TableRowElement::paint( QPainter& painter, AttributeManager* am )
{
    // nothing to paint
    Q_UNUSED( painter )
    Q_UNUSED( am )
}

void TableRowElement::layout( const AttributeManager* am )
{
    Q_UNUSED( am )
    // Get the parent table to query width/ height values
    TableElement* parentTable = static_cast<TableElement*>( parentElement() );
    setHeight( parentTable->rowHeight( this ) );

    // Get alignment for every table data
    QList<Align> verticalAlign = alignments( Qt::Vertical );
    QList<Align> horizontalAlign = alignments( Qt::Horizontal );

    // align the row's entries
    QPointF origin;
    qreal hOffset = 0.0;
    for ( int i = 0; i < m_data.count(); i++ ) {
//         origin = QPointF();
        hOffset = 0.0;
        if( verticalAlign[ i ] == Bottom )
            origin.setY( height() - m_data[ i ]->height() );
        else if( verticalAlign[ i ] == Center || verticalAlign[ i ] == BaseLine )
            origin.setY( ( height() - m_data[ i ]->height() ) / 2 );
            // Baseline is treated like Center for the moment until someone also refines
            // TableElement::determineDimensions so that it pays attention to baseline.
            // Axis as alignment option is ignored as it is tought to be an option for
            // the table itsself.
//         kDebug() << horizontalAlign[ i ]<<","<<Axis;
        if( horizontalAlign[ i ] == Center ) {
            hOffset = ( parentTable->columnWidth( i ) - m_data[ i ]->width() ) / 2;
        }
        else if( horizontalAlign[ i ] == Right ) {
            hOffset = parentTable->columnWidth( i ) - m_data[ i ]->width();
        }

        m_data[ i ]->setOrigin( origin + QPointF( hOffset, 0.0 ) );
        origin += QPointF( parentTable->columnWidth( i ), 0.0 );
    }

    setWidth( origin.x() );
    // setting of the baseline should not be needed as the table row will only occur
    // inside a table where it does not matter if a table row has a baseline or not
}

bool TableRowElement::acceptCursor( const FormulaCursor& cursor )
{
     return (cursor.isSelecting());
}

int TableRowElement::positionOfChild(BasicElement* child) const 
{
    TableDataElement* temp=dynamic_cast<TableDataElement*>(child);
    if (temp==0) {
        return -1;
    } else {
        return m_data.indexOf(temp);
    }
}

int TableRowElement::endPosition() const {
    return m_data.count();
}

QLineF TableRowElement::cursorLine ( int position ) const
{
    TableElement* parentTable = static_cast<TableElement*>( parentElement() );
    QPointF top=absoluteBoundingRect().topLeft();
    qreal hOffset = 0;
    if( childElements().isEmpty() ) {
        // center cursor in elements that have no children
        top += QPointF( width()/2, 0 );
    } else { 
        for (int i=0; i<position; ++i) {
            hOffset+=parentTable->columnWidth(i);
        }
        top += QPointF( hOffset, 0.0 );
	}
    QPointF bottom = top + QPointF( 0.0, height() );
    return QLineF(top, bottom);
}

bool TableRowElement::setCursorTo(FormulaCursor& cursor, QPointF point)
{
    if (cursor.isSelecting()) {
        if (m_data.isEmpty() || point.x()<0.0) {
            cursor.setCurrentElement(this);
            cursor.setPosition(0);
            return true;
        }
        //check if the point is behind all child elements
        if (point.x() >= width()) {
            cursor.setCurrentElement(this);
            cursor.setPosition(endPosition());
            return true;
        }
    }
    int i=0;
    qreal x=0.0;
    TableElement* parentTable = static_cast<TableElement*>( parentElement() );
    for (; i<m_data.count()-1; ++i) {
        //Find the child element the point is in
        x+=parentTable->columnWidth( i );
        if (x>=point.x()) {
            break;
        }
    }
    if (cursor.isSelecting()) {
        //we don't need to change current element because we are already in this element
        if (cursor.mark()<=i) {
            cursor.setPosition(i+1);
        }
        else {
            cursor.setPosition(i);
        }
        return true;
        } else {
        point-=m_data[i]->origin();
        return m_data[i]->setCursorTo(cursor,point);
    }
}

bool TableRowElement::moveCursor(FormulaCursor& newcursor, FormulaCursor& oldcursor)
{
    //TODO: Moving the cursor vertically in the tableelement is a little bit fragile
    if ( (newcursor.isHome() && newcursor.direction()==MoveLeft) ||
        (newcursor.isEnd() && newcursor.direction()==MoveRight) ) {
        return false;
    }
    int rowpos=parentElement()->positionOfChild(this);
    int colpos=(newcursor.position()!=endPosition() ? newcursor.position() : newcursor.position()-1);
    if (newcursor.isSelecting()) {
        switch(newcursor.direction()) {
        case MoveLeft:
            newcursor.moveTo(this,newcursor.position()-1);
            break;
        case MoveRight:
            newcursor.moveTo(this,newcursor.position()+1);
            break;
        case MoveUp:
        case MoveDown:
            return false;
        default:
            break;
        }
    } else {
        switch(newcursor.direction()) {
        case MoveLeft:
            newcursor.setCurrentElement(m_data[newcursor.position()-1]);
            newcursor.moveEnd();
            break;
        case MoveRight:
            newcursor.setCurrentElement(m_data[newcursor.position()]);
            newcursor.moveHome();
            break;
        case MoveUp:
            if ( rowpos>1 ) {
                BasicElement* b=parentElement()->childElements()[rowpos/2-1]->childElements()[colpos];
                return newcursor.moveCloseTo(b, oldcursor);
            } else {
                return false;
            }
        case MoveDown:
            if ( rowpos<endPosition()-1 ) {
                BasicElement* b=parentElement()->childElements()[rowpos/2+1]->childElements()[colpos];
                return newcursor.moveCloseTo(b, oldcursor);
            } else {
                return false;
            }
        default:
            break;
        }
    }
    
    return true;	
}

const QList<BasicElement*> TableRowElement::childElements() const
{
    QList<BasicElement*> tmp;
    foreach( TableDataElement* element, m_data )
        tmp << element;

    return tmp;
}

QList<Align> TableRowElement::alignments( Qt::Orientation orientation )
{
    // choose name of the attribute to query
    QString align = ( orientation == Qt::Horizontal ) ? "columnalign" : "rowalign";

    // get the alignment values of the parental TableElement
    AttributeManager am;
    QList<Align> parentAlignList = am.alignListOf( align, parentElement() );
    // iterate over all entries and look on per data specification of alignment
    QList<Align> alignList;
    for( int i = 0; i < m_data.count(); i++ ) {
        // element got own value for align
        if( !m_data[ i ]->attribute( align ).isEmpty() )
            alignList << am.alignOf( align, m_data[ i ] );
        else if( i < parentAlignList.count() )
            alignList << parentAlignList[ i ];
        else
            alignList << parentAlignList.last();
    }
    return alignList;
}

bool TableRowElement::readMathMLContent( const KoXmlElement& element )
{
    BasicElement* tmpElement = 0;
    KoXmlElement tmp;
    forEachElement( tmp, element ) {
        tmpElement = ElementFactory::createElement( tmp.tagName(), this );
        if (tmpElement->elementType() != TableData)
            return false;

        m_data << static_cast<TableDataElement*>( tmpElement );
        tmpElement->readMathML( tmp );
    }

    return true;
}

void TableRowElement::writeMathMLContent( KoXmlWriter* writer, const QString& ns ) const
{
    foreach( TableDataElement* tmpData, m_data ) {
        tmpData->writeMathML( writer, ns );
    }
}

ElementType TableRowElement::elementType() const
{
    return TableRow;
}

bool TableRowElement::insertChild ( int position, BasicElement* child )
{
    if (child->elementType()==TableData) {
        TableDataElement* tmp=static_cast<TableDataElement*>(child);
        m_data.insert(position,tmp);
        tmp->setParentElement(this);
        return true;
    } else {
        return false;
    }
}

bool TableRowElement::removeChild ( BasicElement* child )
{
    if (child->elementType()!=TableData) {
        return false;
    }
    TableDataElement* tmp=static_cast<TableDataElement*>(child);
    if (m_data.indexOf(tmp)==-1) {
        return false;
    } else {
        m_data.removeAll(tmp);
        tmp->setParentElement(0);
    }
    return true;
}

