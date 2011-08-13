
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

#include "TableElement.h"
#include "AttributeManager.h"
#include "TableRowElement.h"
#include "FormulaCursor.h"
#include <KoXmlReader.h>
#include <QPainter>
#include <QList>
#include <kdebug.h>

TableElement::TableElement( BasicElement* parent ) : BasicElement( parent )
{
    m_framePenStyle = Qt::NoPen;
}

TableElement::~TableElement()
{}

void TableElement::paint( QPainter& painter, AttributeManager* am )
{
    // draw frame
    //if( m_framePenStyle != Qt::NoPen ) {
    //    painter.setPen( QPen( m_framePenStyle ) );
    // painter.drawRect( QRectF( 0.0, 0.0, width(), height() ) );
    //}
    painter.save();
    QList<qreal> frameSpacing = am->doubleListOf( "framespacing", this );
    QList<qreal> rowSpacing = am->doubleListOf( "rowspacing", this );
    kDebug()<<frameSpacing;
    painter.setPen(QPen(Qt::NoPen));//debugging 
    painter.drawRect( QRectF( 0.0, 0.0, width(), height() ) );
    // draw rowlines
    qreal offset = frameSpacing[1];
    for( int i = 0; i < m_rowHeights.count()-1; i++ ) {
        offset += m_rowHeights[ i ];
        painter.drawLine( QPointF( 0.0, offset ), QPointF( width(), offset ) );     
    }

    // draw columnlines
    offset = frameSpacing[0];
    for( int i = 0; i < m_colWidths.count()-1; i++ ) {
        offset += m_colWidths[ i ];
        painter.drawLine( QPointF( offset, 0.0 ), QPointF( offset, height() ) );
    }
    painter.restore();
}

void TableElement::layout( const AttributeManager* am )
{
    // lookup attribute values needed for this table
    m_framePenStyle = am->penStyleOf( "frame", this );
    m_rowLinePenStyles = am->penStyleListOf( "rowlines", this );
    m_colLinePenStyles = am->penStyleListOf( "columnlines", this );
    QList<qreal> frameSpacing = am->doubleListOf( "framespacing", this );
    QList<qreal> rowSpacing = am->doubleListOf( "rowspacing", this );

    // layout the rows vertically
    qreal tmpX = frameSpacing[ 0 ];
    qreal tmpY = frameSpacing[ 1 ];
    for( int i = 0; i < m_rows.count(); i++ ) {
        m_rows[ i ]->setOrigin( QPointF( tmpX, tmpY ) );
        tmpY += m_rows[ i ]->height();
        tmpY += ( i < rowSpacing.count() ) ? rowSpacing[ i ] : rowSpacing.last();
    }

    // add the spacing to tmpX and tmpY
    tmpX += m_rows.first()->width();
    tmpX += frameSpacing[ 0 ];
    tmpY += frameSpacing[ 1 ];
    setWidth( tmpX );
    setHeight( tmpY );
    setBaseLine( height() / 2 );
}

ElementType TableElement::elementType() const 
{
    return Table;
}


void TableElement::determineDimensions()
{
    AttributeManager am;
    bool equalRows = am.boolOf( "equalrows", this );
    bool equalColumns = am.boolOf( "equalcolumns", this );
    m_rowHeights.clear();
    m_colWidths.clear();
    // determine the dimensions of each row and col based on the biggest element in it
    BasicElement* entry;
    qreal maxWidth = 0.0;
    qreal maxHeight = 0.0;
    for( int row = 0; row < m_rows.count(); row++ ) {
        m_rowHeights << 0.0;
        for( int col = 0; col < m_rows.first()->childElements().count(); col++ ) {
             if( m_colWidths.count() <= col )
                 m_colWidths << 0.0;

             entry = m_rows[ row ]->childElements()[ col ];
             m_colWidths[ col ] = qMax( m_colWidths[ col ], entry->width() );
             m_rowHeights[ row ] = qMax( m_rowHeights[ row ], entry->height() );
             maxWidth = qMax( entry->width(), maxWidth );
        }
        maxHeight = qMax( m_rowHeights[ row ], maxHeight );
    }

    // treat equalcol and equalrow attributes
    if( equalRows )
        for( int i = 0; i < m_rowHeights.count(); i++ )
            m_rowHeights.replace( i, maxHeight );

    if( equalColumns )
        for( int i = 0; i < m_colWidths.count(); i++ )
            m_colWidths.replace( i, maxWidth );
}

qreal TableElement::columnWidth( int column )
{
    //if( m_colWidths.isEmpty() )
        determineDimensions();

    return m_colWidths[ column ];
}

qreal TableElement::rowHeight( TableRowElement* row )
{
    //if( m_rowHeights.isEmpty() )
        determineDimensions();

    return m_rowHeights[ m_rows.indexOf( row ) ];
}

const QList<BasicElement*> TableElement::childElements() const
{
    QList<BasicElement*> tmp;
    foreach( TableRowElement* tmpRow, m_rows )
        tmp << tmpRow;
    return tmp;
}

int TableElement::positionOfChild(BasicElement* child) const 
{
    TableRowElement* temp=dynamic_cast<TableRowElement*>(child);
    if (temp==0) {
        return -1;
    } else {
        int p=m_rows.indexOf(temp);
        return (p==-1) ? -1 : 2*p;
    }
}


QLineF TableElement::cursorLine ( int position ) const
{
    QPointF top;
    QRectF rect=m_rows[position/2]->absoluteBoundingRect();
    if (position%2==0) {
        //we are in front of a row
        top=rect.topLeft();
    } else {
        top=rect.topRight();
    }
    QPointF bottom = top + QPointF( 0.0, rect.height() );
    return QLineF(top, bottom);
}

bool TableElement::setCursorTo(FormulaCursor& cursor, QPointF point)
{
    if (cursor.isSelecting()) {
        return false;
    }
    int i;
    for (i=0;i<m_rows.count()-1;i++) {
        if (m_rows[i]->boundingRect().bottom()>point.y()) {
            break;
        }
    }
    point-=m_rows[i]->origin();
    return m_rows[i]->setCursorTo(cursor, point);
}

bool TableElement::moveCursor(FormulaCursor& newcursor, FormulaCursor& oldcursor)
{
    Q_UNUSED( oldcursor )
    int p=newcursor.position();
    switch (newcursor.direction()) {
    case MoveLeft:
        if (p%2==0) {
            //we are in front of a table row
            return false;
        } else {
            if (newcursor.isSelecting()) {
                newcursor.moveTo( this , p-1 );
            } else {
                newcursor.moveTo( m_rows[ p / 2 ] , m_rows[ p / 2 ]->endPosition() );
            }
            break;
        }
    case MoveRight:
        if (p%2==1) {
            //we are behind a table row
            return false;
        } else {
            if (newcursor.isSelecting()) {
                newcursor.moveTo( this , p+1 );
            } else {
                newcursor.moveTo( m_rows[ p / 2 ] , 0 );
            }
            break;
        }
    case MoveUp:
        if (p<=1) {
            return false;
        } else {
            newcursor.moveTo(this,p-2);
            break;
        }
    case MoveDown:
        if (p<(m_rows.count()-1)*2) {
            newcursor.moveTo(this,p+2);
            break;
        } else {
            return false;
        }
    default:
        break;
    }
    return true;
}

int TableElement::endPosition() const 
{
    if (m_rows.count()>0) {
        return 2*m_rows.count()-1;
    } else {
        return 1;
    }
}


bool TableElement::acceptCursor( const FormulaCursor& cursor )
{
//    return false;
    return cursor.isSelecting();
}


QPainterPath TableElement::selectionRegion ( const int pos1, const int pos2 ) const
{
    int p1=pos1 % 2 == 0 ? pos1 : pos1+1; //pos1 should be before an element
    int p2=pos2 % 2 == 0 ? pos2 : pos2+1; //pos2 should be behind an element
    QPainterPath P;
    
    for (int i=p1;i<p2;i+=2) {
        P.addRect(m_rows[i/2]->absoluteBoundingRect());
    }
    return P;
}


QString TableElement::attributesDefaultValue( const QString& attribute ) const
{
    if( attribute == "align" )
        return "axis";
    else if( attribute == "rowalign" )
        return "baseline";
    else if( attribute == "columnalign" )
        return "center";
    else if( attribute == "groupalign" )
        return "left";
    else if( attribute == "alignmentscope" )
        return "true";
    else if( attribute == "columnwidth" )
        return "auto";
    else if( attribute == "width" )
        return "auto";
    else if( attribute == "rowspacing" )
        return "1.0ex";
    else if( attribute == "columnspacing" )
        return "0.8em";
    else if( attribute == "rowlines" || attribute == "columnlines" ||
             attribute == "frame" )
        return "none";
    else if( attribute == "framespacing" )
        return "0.4em 0.5ex";
    else if( attribute == "equalrows" || attribute == "equalcolumns" ||
             attribute == "displaystyle" )
        return "false";
    else if( attribute == "side" )
        return "right";
    else if( attribute == "minlabelspacing" )
        return "0.8em";
    else
        return QString();
}

bool TableElement::readMathMLContent( const KoXmlElement& element )
{  
    BasicElement* tmpElement = 0;
    KoXmlElement tmp;
    forEachElement( tmp, element )   // iterate over the elements
    {
        tmpElement = ElementFactory::createElement( tmp.tagName(), this );
        if( tmpElement->elementType() != TableRow )
            return false;

        m_rows << static_cast<TableRowElement*>( tmpElement );
    tmpElement->readMathML( tmp );
    }

    return true;
}

void TableElement::writeMathMLContent( KoXmlWriter* writer, const QString& ns ) const
{
    foreach( TableRowElement* tmpRow, m_rows ) {  // write each mtr element
        tmpRow->writeMathML( writer, ns );
    }
}


bool TableElement::insertChild ( int position, BasicElement* child )
{
    if (child->elementType()==TableRow &&
        !child->childElements().isEmpty() &&
        child->childElements()[0]->elementType()==TableData) {
        TableRowElement* tmp=static_cast<TableRowElement*>(child);
        m_rows.insert(position,tmp);
        tmp->setParentElement(this);
        //TODO: there must be a more efficient way for this
        determineDimensions();
        
        return true;
    } else {
        return false;
    }
}

bool TableElement::removeChild ( BasicElement* child )
{
    if (child->elementType()!=TableRow) {
        return false;
    }
    TableRowElement* tmp=static_cast<TableRowElement*>(child);
    if (m_rows.indexOf(tmp)==-1) {
        return false;
    } else {
        m_rows.removeAll(tmp);
        tmp->setParentElement(0);
    }
    return true;
}
