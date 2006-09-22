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

int MatrixRowElement::numberOfEntries() const
{
    return m_matrixEntryElements.count();
}

MatrixEntryElement* MatrixRowElement::entryAtPosition( int pos )
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

void MatrixRowElement::readMathML( const QDomElement& element )
{
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

void MatrixRowElement::moveLeft( FormulaCursor* cursor, BasicElement* from )
{
/*    // If you want to select more than one line you'll have to
    // select the whole element.
    if (cursor->isSelectionMode()) {
        getParent()->moveLeft(cursor, this);
    }
    else {
        // Coming from the parent (sequence) we go to
        // the very last position
        if (from == getParent()) {
            m_matrixEntryElements.at( m_matrixEntryElements.count()-1 )->moveLeft(cursor, this);
        }
        else {
            // Coming from one of the lines we go to the previous line
            // or to the parent if there is none.
            int pos = m_matrixEntryElements.indexOf( static_cast<MatrixEntryElement*>( from ) );
            if ( pos > -1 ) {
                if ( pos > 0 ) {
                    m_matrixEntryElements.at( pos-1 )->moveLeft( cursor, this );
                }
                else {
                    getParent()->moveLeft(cursor, this);
                }
            }
            else {
                kDebug( DEBUGID ) << k_funcinfo << endl;
                kDebug( DEBUGID ) << "Serious confusion. Must never happen." << endl;
            }
        }
    }*/
}

void MatrixRowElement::moveRight( FormulaCursor* cursor, BasicElement* from )
{
/*    if (cursor->isSelectionMode()) {
        getParent()->moveRight(cursor, this);
    }
    else {
        if (from == getParent()) {
            m_matrixEntryElements.at( 0 )->moveRight(cursor, this);
        }
        else {
            int pos = m_matrixEntryElements.indexOf( static_cast<MatrixEntryElement*>( from ) );
            if ( pos > -1 ) {
                int upos = pos;
                if ( upos < m_matrixEntryElements.count() ) {
                    if ( upos < m_matrixEntryElements.count()-1 ) {
                        m_matrixEntryElements.at( upos+1 )->moveRight( cursor, this );
                    }
                    else {
                        getParent()->moveRight(cursor, this);
                    }
                    return;
                }
            }
            kDebug( DEBUGID ) << k_funcinfo << endl;
            kDebug( DEBUGID ) << "Serious confusion. Must never happen." << endl;
        }
    }*/
}

void MatrixRowElement::moveUp( FormulaCursor* cursor, BasicElement* from )
{
/*    // If you want to select more than one line you'll have to
    // select the whole element.
    if (cursor->isSelectionMode()) {
        getParent()->moveLeft(cursor, this);
    }
    else {
        // Coming from the parent (sequence) we go to
        // the very last position
        if (from == getParent()) {
            m_matrixEntryElements.last()->moveLeft(cursor, this);
        }
        else {
            // Coming from one of the lines we go to the previous line
            // or to the parent if there is none.
            int pos = m_matrixEntryElements.indexOf( static_cast<MatrixEntryElement*>( from ) );
            if ( pos > -1 ) {
                if ( pos > 0 ) {
                    //content.at( pos-1 )->moveLeft( cursor, this );
                    // This is rather hackish.
                    // But we know what elements we have here.
                    int cursorPos = cursor->getPos();
                    MatrixEntryElement* current = m_matrixEntryElements.at( pos );
                    MatrixEntryElement* newLine = m_matrixEntryElements.at( pos-1 );
                    int tabNum = current->tabBefore( cursorPos );
                    if ( tabNum > -1 ) {
                        int oldTabPos = current->tabPos( tabNum );
                        int newTabPos = newLine->tabPos( tabNum );
                        if ( newTabPos > -1 ) {
                            cursorPos += newTabPos-oldTabPos;
                            int nextNewTabPos = newLine->tabPos( tabNum+1 );
                            if ( nextNewTabPos > -1 ) {
                                cursorPos = qMin( cursorPos, nextNewTabPos );
                            }
                        }
                        else {
                            cursorPos = newLine->countChildren();
                        }
                    }
                    else {
                        int nextNewTabPos = newLine->tabPos( 0 );
                        if ( nextNewTabPos > -1 ) {
                            cursorPos = qMin( cursorPos, nextNewTabPos );
                        }
                    }
                    cursor->setTo( newLine,
                                   qMin( cursorPos,
                                         newLine->countChildren() ) );
                }
                else {
                    getParent()->moveLeft(cursor, this);
                }
            }
            else {
                kDebug( DEBUGID ) << k_funcinfo << endl;
                kDebug( DEBUGID ) << "Serious confusion. Must never happen." << endl;
            }
        }
    }*/
}

void MatrixRowElement::moveDown( FormulaCursor* cursor, BasicElement* from )
{
/*    if (cursor->isSelectionMode()) {
        getParent()->moveRight(cursor, this);
    }
    else {
        if (from == getParent()) {
            m_matrixEntryElements.first()->moveRight(cursor, this);
        }
        else {
            int pos = m_matrixEntryElements.indexOf( static_cast<MatrixEntryElement*>( from ) );
            if ( pos > -1 ) {
                int upos = pos;
                if ( upos < m_matrixEntryElements.count() ) {
                    if ( upos < m_matrixEntryElements.count()-1 ) {
                        //content.at( upos+1 )->moveRight( cursor, this );
                        // This is rather hackish.
                        // But we know what elements we have here.
                        int cursorPos = cursor->getPos();
                        MatrixEntryElement* current = m_matrixEntryElements.at( upos );
                        MatrixEntryElement* newLine = m_matrixEntryElements.at( upos+1 );
                        int tabNum = current->tabBefore( cursorPos );
                        if ( tabNum > -1 ) {
                            int oldTabPos = current->tabPos( tabNum );
                            int newTabPos = newLine->tabPos( tabNum );
                            if ( newTabPos > -1 ) {
                                cursorPos += newTabPos-oldTabPos;
                                int nextNewTabPos = newLine->tabPos( tabNum+1 );
                                if ( nextNewTabPos > -1 ) {
                                    cursorPos = qMin( cursorPos, nextNewTabPos );
                                }
                            }
                            else {
                                cursorPos = newLine->countChildren();
                            }
                        }
                        else {
                            int nextNewTabPos = newLine->tabPos( 0 );
                            if ( nextNewTabPos > -1 ) {
                                cursorPos = qMin( cursorPos, nextNewTabPos );
                            }
                        }
                        cursor->setTo( newLine,
                                       qMin( cursorPos,
                                             newLine->countChildren() ) );
                    }
                    else {
                        getParent()->moveRight(cursor, this);
                    }
                    return;
                }
            }
            kDebug( DEBUGID ) << k_funcinfo << endl;
            kDebug( DEBUGID ) << "Serious confusion. Must never happen." << endl;
        }
    }*/
}


void MatrixRowElement::calcSizes( const ContextStyle& context,
                                  ContextStyle::TextStyle tstyle,
                                  ContextStyle::IndexStyle istyle )
{
    luPt mySize = context.getAdjustedSize( tstyle );
    QFont font = context.getDefaultFont();
    font.setPointSizeF( context.layoutUnitPtToPt( mySize ) );
    QFontMetrics fm( font );
    luPixel leading = context.ptToLayoutUnitPt( fm.leading() );
    luPixel distY = context.ptToPixelY( context.getThinSpace( tstyle ) );

    int count = m_matrixEntryElements.count();
    luPixel height = -leading;
    luPixel width = 0;
    int tabCount = 0;
    for ( int i = 0; i < count; ++i ) {
        MatrixEntryElement* line = m_matrixEntryElements[i];
        line->calcSizes( context, tstyle, istyle );
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
        setBaseline( height/2 + context.axisHeight( tstyle ) );
    }
}

void MatrixRowElement::draw( QPainter& painter, const LuPixelRect& r,
                             const ContextStyle& context,
                             ContextStyle::TextStyle tstyle,
                             ContextStyle::IndexStyle istyle,
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
        line->draw( painter, r, context, tstyle, istyle, myPos );
    }
}

void MatrixRowElement::insert( FormulaCursor* cursor,
                               QList<BasicElement*>& newChildren,
                               Direction direction )
{
    MatrixEntryElement* e = static_cast<MatrixEntryElement*>(newChildren.takeAt(0));
    e->setParent(this);
    m_matrixEntryElements.insert( cursor->getPos(), e );

    if (direction == beforeCursor) {
        e->moveLeft(cursor, this);
    }
    else {
        e->moveRight(cursor, this);
    }
    cursor->setSelecting(false);
    //formula()->changed();
}

void MatrixRowElement::remove( FormulaCursor* cursor,
                               QList<BasicElement*>& removedChildren,
                               Direction direction )
{
    if ( m_matrixEntryElements.count() == 1 ) { //&& ( cursor->getPos() == 0 ) ) {
        getParent()->selectChild(cursor, this);
        getParent()->remove(cursor, removedChildren, direction);
    }
    else {
        MatrixEntryElement* e = m_matrixEntryElements.takeAt( cursor->getPos() );
        removedChildren.append( e );
        //formula()->elementRemoval( e );
        //cursor->setTo( this, denominatorPos );
        //formula()->changed();
    }
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
    int pos = m_matrixEntryElements.indexOf( dynamic_cast<MatrixEntryElement*>( child ) );
    if ( pos > -1 ) {
        cursor->setTo( this, pos );
        //content.at( pos )->moveRight( cursor, this );
    }
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
