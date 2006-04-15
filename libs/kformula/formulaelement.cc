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

#include <iostream>
#include <QPainter>
//Added by qt3to4:
#include <QKeyEvent>

#include <kdebug.h>

#include "contextstyle.h"
#include "formulacursor.h"
#include "formulaelement.h"
#include "kformulacontainer.h"
#include "kformuladocument.h"

KFORMULA_NAMESPACE_BEGIN

FormulaElement::FormulaElement(FormulaDocument* container)
    : document( container ), baseSize( 20 ), ownBaseSize( false )
{
}


void FormulaElement::setBaseSize( int size )
{
    if ( size > 0 ) {
        baseSize = size;
        ownBaseSize = true;
    }
    else {
        ownBaseSize = false;
    }
    document->baseSizeChanged( size, ownBaseSize );
}


/**
 * Returns the element the point is in.
 */
BasicElement* FormulaElement::goToPos( FormulaCursor* cursor, const LuPixelPoint& point )
{
    bool handled = false;
    BasicElement* element = inherited::goToPos(cursor, handled, point, LuPixelPoint());
    if (element == 0) {
        //if ((point.x() > getWidth()) || (point.y() > getHeight())) {
            cursor->setTo(this, countChildren());
            //}
            return this;
    }
    return element;
}

void FormulaElement::elementRemoval(BasicElement* child)
{
    document->elementRemoval(child);
}

void FormulaElement::changed()
{
    document->changed();
}

void FormulaElement::cursorHasMoved( FormulaCursor* cursor )
{
    document->cursorHasMoved( cursor );
}

void FormulaElement::moveOutLeft( FormulaCursor* cursor )
{
    document->moveOutLeft( cursor );
}

void FormulaElement::moveOutRight( FormulaCursor* cursor )
{
    document->moveOutRight( cursor );
}

void FormulaElement::moveOutBelow( FormulaCursor* cursor )
{
    document->moveOutBelow( cursor );
}

void FormulaElement::moveOutAbove( FormulaCursor* cursor )
{
    document->moveOutAbove( cursor );
}

void FormulaElement::tell( const QString& msg )
{
    document->tell( msg );
}

void FormulaElement::removeFormula( FormulaCursor* cursor )
{
    document->removeFormula( cursor );
}

void FormulaElement::insertFormula( FormulaCursor* cursor )
{
    document->insertFormula( cursor );
}

void FormulaElement::calcSizes( const ContextStyle& style,
                                ContextStyle::TextStyle tstyle,
                                ContextStyle::IndexStyle istyle )
{
    inherited::calcSizes( style, tstyle, istyle );
}


void FormulaElement::draw( QPainter& painter, const LuPixelRect& r,
                           const ContextStyle& context,
                           ContextStyle::TextStyle tstyle,
                           ContextStyle::IndexStyle istyle,
                           const LuPixelPoint& parentOrigin )
{
    inherited::draw( painter, r, context, tstyle, istyle, parentOrigin );
}


/**
 * Calculates the formulas sizes and positions.
 */
void FormulaElement::calcSizes( ContextStyle& context )
{
    //kDebug( DEBUGID ) << "FormulaElement::calcSizes" << endl;
    if ( ownBaseSize ) {
        context.setSizeFactor( static_cast<double>( getBaseSize() )/context.baseSize() );
    }
    else {
        context.setSizeFactor( 1 );
    }
    calcSizes( context, context.getBaseTextStyle(),
               ContextStyle::normal );
}

/**
 * Draws the whole thing.
 */
void FormulaElement::draw( QPainter& painter, const LuPixelRect& r,
                           ContextStyle& context )
{
    //kDebug( DEBUGID ) << "FormulaElement::draw" << endl;
    if ( ownBaseSize ) {
        context.setSizeFactor( static_cast<double>( getBaseSize() )/context.baseSize() );
    }
    else {
        context.setSizeFactor( 1 );
    }
    draw( painter, r, context, context.getBaseTextStyle(),
          ContextStyle::normal, LuPixelPoint() );
}

KCommand* FormulaElement::buildCommand( Container* container, Request* request )
{
    switch ( *request ) {
    case req_compactExpression:
        return 0;
    default:
        break;
    }
    return inherited::buildCommand( container, request );
}

const SymbolTable& FormulaElement::getSymbolTable() const
{
    return document->getSymbolTable();
}


QDomElement FormulaElement::emptyFormulaElement( QDomDocument& doc )
{
    QDomElement element = doc.createElement( getTagName() );
    /*
    element.setAttribute( "VERSION", "6" );
    if ( ownBaseSize ) {
        element.setAttribute( "BASESIZE", baseSize );
    }
    */
    return element;
}

KCommand* FormulaElement::input( Container* container, QKeyEvent* event )
{
    QChar ch = event->text().at( 0 );
    if ( !ch.isPrint() ) {
        int action = event->key();
        //int state = event->state();
        //MoveFlag flag = movementFlag(state);

	switch ( action ) {
        case Qt::Key_Return:
        case Qt::Key_Enter: {
            FormulaCursor* cursor = container->activeCursor();
            insertFormula( cursor );
            return 0;
        }
        }
    }
    return inherited::input( container, event );
}

/**
 * Appends our attributes to the dom element.
 */
void FormulaElement::writeDom(QDomElement element)
{
    inherited::writeDom(element);
    element.setAttribute( "VERSION", "6" );
    if ( ownBaseSize ) {
        element.setAttribute( "BASESIZE", baseSize );
    }
}

/**
 * Reads our attributes from the element.
 * Returns false if it failed.
 */
bool FormulaElement::readAttributesFromDom(QDomElement element)
{
    if (!inherited::readAttributesFromDom(element)) {
        return false;
    }
    int version = -1;
    QString versionStr = element.attribute( "VERSION" );
    if ( !versionStr.isNull() ) {
        version = versionStr.toInt();
    }
    if ( version > -1 ) {
        // Version 6 added the MultilineElement (TabMarker)
        // Version 5 added under- and overlines
        if ( version < 4 ) {
            convertNames( element );
        }
    }
    QString baseSizeStr = element.attribute( "BASESIZE" );
    if ( !baseSizeStr.isNull() ) {
        ownBaseSize = true;
        baseSize = baseSizeStr.toInt();
    }
    else {
        ownBaseSize = false;
    }
    return true;
}

/**
 * Reads our content from the node. Sets the node to the next node
 * that needs to be read.
 * Returns false if it failed.
 */
bool FormulaElement::readContentFromDom(QDomNode& node)
{
    return inherited::readContentFromDom(node);
}

void FormulaElement::convertNames( QDomNode node )
{
    if ( node.isElement() && ( node.nodeName().toUpper() == "TEXT" ) ) {
        QDomNamedNodeMap attr = node.attributes();
        QDomAttr ch = attr.namedItem( "CHAR" ).toAttr();
        if ( ch.value() == "\\" ) {
            QDomNode sequence = node.parentNode();
            QDomDocument doc = sequence.ownerDocument();
            QDomElement nameseq = doc.createElement( "NAMESEQUENCE" );
            sequence.replaceChild( nameseq, node );

            bool inName = true;
            while ( inName ) {
                inName = false;
                QDomNode n = nameseq.nextSibling();
                if ( n.isElement() && ( n.nodeName().toUpper() == "TEXT" ) ) {
                    attr = n.attributes();
                    ch = attr.namedItem( "CHAR" ).toAttr();
                    if ( ch.value().at( 0 ).isLetter() ) {
                        nameseq.appendChild( sequence.removeChild( n ) );
                        inName = true;
                    }
                }
            }
        }
    }
    if ( node.hasChildNodes() ) {
        QDomNode n = node.firstChild();
        while ( !n.isNull() ) {
            convertNames( n );
            n = n.nextSibling();
        }
    }
}

QString FormulaElement::toLatex()
{
    return inherited::toLatex();   //Consider $$ sorround
}

void FormulaElement::writeMathML( QDomDocument& doc, QDomNode parent, bool oasisFormat )
{
    QDomElement de;
    if ( !oasisFormat )
        de = doc.createElementNS( "http://www.w3.org/1998/Math/MathML",
                                              "math" );
    else
        de =doc.createElement( "math:semantics" );

    inherited::writeMathML( doc, de, oasisFormat );
    parent.appendChild( de );
}

KFORMULA_NAMESPACE_END
