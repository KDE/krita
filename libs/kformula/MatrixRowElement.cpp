/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
   Copyright (C) 2001 Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
   Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net>
   Copyright (C) 2006 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>

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

#include "MatrixRowElement.h"
#include "MatrixEntryElement.h"
#include "FormulaCursor.h"
#include <KoXmlWriter.h>

#include <klocale.h>

#include <QPainter>
#include <QList>

namespace KFormula {

MatrixRowElement::MatrixRowElement( BasicElement* parent ) : BasicElement( parent )
{
    m_matrixEntryElements.append( new MatrixEntryElement( this ) );
}

MatrixRowElement::~MatrixRowElement()
{
}

int MatrixRowElement::positionOfEntry( BasicElement* entry ) const
{
    for( int i = 0; i < m_matrixEntryElements.count(); i++ )
         if( m_matrixEntryElements[ i ] == entry )
             return i;
}

MatrixEntryElement* MatrixRowElement::entryAt( int pos )
{
    return m_matrixEntryElements[ pos ];
}

const QList<BasicElement*> MatrixRowElement::childElements()
{
    QList<BasicElement*> tmp;
    foreach( MatrixEntryElement* element, m_matrixEntryElements )
        tmp.append( element );

    return tmp;
}

void MatrixRowElement::moveLeft( FormulaCursor* cursor, BasicElement* from )
{
    if( from == parentElement() )   // coming from the parent go to the very right entry
        m_matrixEntryElements.last()->moveLeft( cursor, this );
    else                            // coming from a child go to the parent
        parentElement()->moveLeft( cursor, from );
}

void MatrixRowElement::moveRight( FormulaCursor* cursor, BasicElement* from )
{
    if( from == parentElement() )
        m_matrixEntryElements.first()->moveRight( cursor, this );
    else
        parentElement()->moveRight( cursor, this );
}

void MatrixRowElement::moveUp( FormulaCursor* cursor, BasicElement* from )
{
    parentElement()->moveUp( cursor, from );   // just forward the call to MatrixElement   
}

void MatrixRowElement::moveDown( FormulaCursor* cursor, BasicElement* from )
{
    parentElement()->moveDown( cursor, from ); // just forward the call to MatrixElement
}

void MatrixRowElement::readMathML( const QDomElement& element )
{
    readMathMLAttributes( element );
   
    MatrixEntryElement* tmpEntry = 0;
    QDomElement tmp = element.firstChildElement();
    while( !tmp.isNull() )
    {
        tmpEntry = new MatrixEntryElement( this );
	m_matrixEntryElements << tmpEntry;
	tmpEntry->readMathML( tmp );
	tmp = tmp.nextSiblingElement();
    }
}

void MatrixRowElement::writeMathML( KoXmlWriter* writer, bool oasisFormat )
{
    writer->startElement( oasisFormat ? "math:mtr" : "mtr" );
    writeMathMLAttributes( writer );

    foreach( MatrixEntryElement* tmpEntry, m_matrixEntryElements )
        tmpEntry->writeMathML( writer, oasisFormat );

    writer->endElement();
}




void MatrixRowElement::goInside( FormulaCursor* cursor )
{
    m_matrixEntryElements.at( 0 )->goInside( cursor );
}




void MatrixRowElement::calcSizes( const ContextStyle& context,
                                  ContextStyle::TextStyle tstyle,
                                  ContextStyle::IndexStyle istyle,
                                  StyleAttributes& style )
{
    double factor = style.sizeFactor();
    luPt mySize = context.getAdjustedSize( tstyle, factor );
    QFont font = context.getDefaultFont();
    font.setPointSizeF( context.layoutUnitPtToPt( mySize ) );
    QFontMetrics fm( font );
    luPixel leading = context.ptToLayoutUnitPt( fm.leading() );
    luPixel distY = context.ptToPixelY( context.getThinSpace( tstyle, factor ) );

    int count = m_matrixEntryElements.count();
    luPixel height = -leading;
    luPixel width = 0;
    int tabCount = 0;
    for ( int i = 0; i < count; ++i ) {
        MatrixEntryElement* line = m_matrixEntryElements[i];
        line->calcSizes( context, tstyle, istyle, style );
        tabCount = qMax( tabCount, line->tabCount() );

        height += leading;
        line->setX( 0 );
        line->setY( height );
        height += line->getHeight() + distY;
        width = qMax( line->getWidth(), width );
    }

    // calculate the tab positions
    for ( int t = 0; t < tabCount; ++t ) {
        luPixel pos = 0;
        for ( int i = 0; i < count; ++i ) {
            MatrixEntryElement* line = m_matrixEntryElements[i];
            if ( t < line->tabCount() ) {
                pos = qMax( pos, line->tab( t )->getX() );
            }
            else {
                pos = qMax( pos, line->getWidth() );
            }
        }
        for ( int i = 0; i < count; ++i ) {
            MatrixEntryElement* line = m_matrixEntryElements[i];
            if ( t < line->tabCount() ) {
                line->moveTabTo( t, pos );
                width = qMax( width, line->getWidth() );
            }
        }
    }

    setHeight( height );
    setWidth( width );
    if ( count == 1 ) {
        setBaseline( m_matrixEntryElements.at( 0 )->getBaseline() );
    }
    else {
        // There's always a first line. No formulas without lines.
        setBaseline( height/2 + context.axisHeight( tstyle, factor ) );
    }
}

void MatrixRowElement::draw( QPainter& painter, const LuPixelRect& r,
                             const ContextStyle& context,
                             ContextStyle::TextStyle tstyle,
                             ContextStyle::IndexStyle istyle,
                             StyleAttributes& style,
                             const LuPixelPoint& parentOrigin )
{
    LuPixelPoint myPos( parentOrigin.x() + getX(), parentOrigin.y() + getY() );
    int count = m_matrixEntryElements.count();

    if ( context.edit() ) {
        int tabCount = 0;
        painter.setPen( context.getHelpColor() );
        for ( int i = 0; i < count; ++i ) {
            MatrixEntryElement* line = m_matrixEntryElements[i];
            if ( tabCount < line->tabCount() ) {
                for ( int t = tabCount; t < line->tabCount(); ++t ) {
                    BasicElement* marker = line->tab( t );
                    painter.drawLine( context.layoutUnitToPixelX( myPos.x()+marker->getX() ),
                                      context.layoutUnitToPixelY( myPos.y() ),
                                      context.layoutUnitToPixelX( myPos.x()+marker->getX() ),
                                      context.layoutUnitToPixelY( myPos.y()+getHeight() ) );
                }
                tabCount = line->tabCount();
            }
        }
    }

    for ( int i = 0; i < count; ++i ) {
        MatrixEntryElement* line = m_matrixEntryElements[i];
        line->draw( painter, r, context, tstyle, istyle, style, myPos );
    }
}

void MatrixRowElement::insert( FormulaCursor* cursor,
                               QList<BasicElement*>& newChildren,
                               Direction direction )
{
/*    MatrixEntryElement* e = static_cast<MatrixEntryElement*>(newChildren.takeAt(0));
    e->setParent(this);
    m_matrixEntryElements.insert( cursor->getPos(), e );

    if (direction == beforeCursor) {
        e->moveLeft(cursor, this);
    }
    else {
        e->moveRight(cursor, this);
    }
    cursor->setSelecting(false);*/
    //formula()->changed();
}

void MatrixRowElement::remove( FormulaCursor* cursor,
                               QList<BasicElement*>& removedChildren,
                               Direction direction )
{
/*    if ( m_matrixEntryElements.count() == 1 ) { //&& ( cursor->getPos() == 0 ) ) {
        getParent()->selectChild(cursor, this);
        getParent()->remove(cursor, removedChildren, direction);
    }
    else {
        MatrixEntryElement* e = m_matrixEntryElements.takeAt( cursor->getPos() );
        removedChildren.append( e );
        //formula()->elementRemoval( e );
        //cursor->setTo( this, denominatorPos );
        //formula()->changed();
    }*/
}
/*
void MatrixRowElement::normalize( FormulaCursor* cursor, Direction direction )
{
    int pos = cursor->getPos();
    if ( ( cursor->getElement() == this ) &&
         ( pos > -1 ) && ( pos <= m_matrixEntryElements.count() ) ) {
        switch ( direction ) {
        case beforeCursor:
            if ( pos > 0 ) {
                m_matrixEntryElements.at( pos-1 )->moveLeft( cursor, this );
                break;
            }
            // no break! intended!
        case afterCursor:
            if ( pos < m_matrixEntryElements.count() ) {
                m_matrixEntryElements.at( pos )->moveRight( cursor, this );
            }
            else {
                m_matrixEntryElements.at( pos-1 )->moveLeft( cursor, this );
            }
            break;
        }
    }
    else {
        BasicElement::normalize( cursor, direction );
    }
}*/
/*
SequenceElement* MatrixRowElement::getMainChild()
{
    return m_matrixEntryElements.at( 0 );
}
*/
void MatrixRowElement::selectChild(FormulaCursor* cursor, BasicElement* child)
{
/*    int pos = m_matrixEntryElements.indexOf( dynamic_cast<MatrixEntryElement*>( child ) );
    if ( pos > -1 ) {
        cursor->setTo( this, pos );
        //content.at( pos )->moveRight( cursor, this );
    }*/
}


/**
 * Appends our attributes to the dom element.
 */
void MatrixRowElement::writeDom(QDomElement element)
{
    BasicElement::writeDom(element);

    int lineCount = m_matrixEntryElements.count();
    element.setAttribute( "LINES", lineCount );

    QDomDocument doc = element.ownerDocument();
    for ( int i = 0; i < lineCount; ++i ) {
        QDomElement tmp = m_matrixEntryElements.at( i )->getElementDom(doc);
        element.appendChild(tmp);
    }
}






/**
 * Reads our attributes from the element.
 * Returns false if it failed.
 */
bool MatrixRowElement::readAttributesFromDom(QDomElement element)
{
    if (!BasicElement::readAttributesFromDom(element)) {
        return false;
    }
    int lineCount = 0;
    QString lineCountStr = element.attribute("LINES");
    if(!lineCountStr.isNull()) {
        lineCount = lineCountStr.toInt();
    }
    if (lineCount == 0) {
        kWarning( DEBUGID ) << "lineCount <= 0 in MultilineElement." << endl;
        return false;
    }

    m_matrixEntryElements.clear();
    for ( int i = 0; i < lineCount; ++i ) {
        MatrixEntryElement* element = new MatrixEntryElement(this);
        m_matrixEntryElements.append(element);
    }
    return true;
}

/**
 * Reads our content from the node. Sets the node to the next node
 * that needs to be read.
 * Returns false if it failed.
 */
bool MatrixRowElement::readContentFromDom(QDomNode& node)
{
    if (!BasicElement::readContentFromDom(node)) {
        return false;
    }

    int lineCount = m_matrixEntryElements.count();
    int i = 0;
    while ( !node.isNull() && i < lineCount ) {
        if ( node.isElement() ) {
            SequenceElement* element = m_matrixEntryElements.at( i );
            QDomElement e = node.toElement();
            if ( !element->buildFromDom( e ) ) {
                return false;
            }
            ++i;
        }
        node = node.nextSibling();
    }
    return true;
}

} // namespace KFormula
