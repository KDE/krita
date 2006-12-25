/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
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

#include <QKeyEvent>
//#include <QPainter>

#include <kdebug.h>

#include <KoXmlWriter.h>

#include "contextstyle.h"
#include "FormulaContainer.h"
#include "FormulaCursor.h"
#include "FormulaElement.h"

namespace KFormula {

FormulaElement::FormulaElement() : BasicElement( 0 ),
                                   baseSize( 20 ),
                                   ownBaseSize( false )
{
}

FormulaElement::~FormulaElement()
{
    foreach( BasicElement* tmpElement, m_childElements )    // delete all child elements
	delete tmpElement;
}

const QList<BasicElement*> FormulaElement::childElements()
{
    return m_childElements;
}

void FormulaElement::readMathML( const QDomElement& element )
{
    readMathMLAttributes( element );
}

void FormulaElement::writeMathML( KoXmlWriter* writer, bool oasisFormat )
{
    if( oasisFormat )
        writer->startElement( "math:semantics" );
    else
        writer->startDocument( "math", "http://www.w3.org/1998/Math/MathML" );

    foreach( BasicElement* tmpElement, m_childElements )
        tmpElement->writeMathML( writer, oasisFormat );
    
    if( oasisFormat )
        writer->endElement();
    else
        writer->endDocument();
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
//    m_document->baseSizeChanged( size, ownBaseSize );
}




void FormulaElement::elementRemoval(BasicElement* child)
{
 //   m_document->elementRemoval(child);
}

void FormulaElement::changed()
{
//    m_document->changed();
}

void FormulaElement::cursorHasMoved( FormulaCursor* cursor )
{
//    m_document->cursorHasMoved( cursor );
}

void FormulaElement::moveOutLeft( FormulaCursor* cursor )
{
//    m_document->moveOutLeft( cursor );
}

void FormulaElement::moveOutRight( FormulaCursor* cursor )
{
//    m_document->moveOutRight( cursor );
}

void FormulaElement::moveOutBelow( FormulaCursor* cursor )
{
  //  m_document->moveOutBelow( cursor );
}

void FormulaElement::moveOutAbove( FormulaCursor* cursor )
{
    //m_document->moveOutAbove( cursor );
}

void FormulaElement::removeFormula( FormulaCursor* cursor )
{
  //  m_document->removeFormula( cursor );
}

void FormulaElement::insertFormula( FormulaCursor* cursor )
{
//    m_document->insertFormula( cursor );
}

void FormulaElement::calcSizes( const ContextStyle& context,
                                ContextStyle::TextStyle tstyle,
                                ContextStyle::IndexStyle istyle,
                                StyleAttributes& style )
{
    //BasicElement::calcSizes( style, tstyle, istyle );
}


void FormulaElement::draw( QPainter& painter, const LuPixelRect& r,
                           const ContextStyle& context,
                           ContextStyle::TextStyle tstyle,
                           ContextStyle::IndexStyle istyle,
                           StyleAttributes& style,
                           const LuPixelPoint& parentOrigin )
{
//    BasicElement::draw( painter, r, context, tstyle, istyle, parentOrigin );
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
    StyleAttributes style;
    calcSizes( context, context.getBaseTextStyle(),
               ContextStyle::normal, style );
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
    StyleAttributes style;
    draw( painter, r, context, context.getBaseTextStyle(),
          ContextStyle::normal, style, LuPixelPoint() );
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
/**
 * Appends our attributes to the dom element.
 */
void FormulaElement::writeDom(QDomElement element)
{
    BasicElement::writeDom(element);
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
    if (!BasicElement::readAttributesFromDom(element)) {
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
    return BasicElement::readContentFromDom(node);
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



} // namespace KFormula
