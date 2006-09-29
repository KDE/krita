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

#include "FractionElement.h"
#include "ElementFactory.h"
#include "FormulaCursor.h"
#include <KoXmlWriter.h>
#include <QPainter>

namespace KFormula {

FractionElement::FractionElement( BasicElement* parent ) : BasicElement( parent )
{
    m_numerator = new BasicElement( this );
    m_denominator = new BasicElement( this );
}

FractionElement::~FractionElement()
{
    delete m_numerator;
    delete m_denominator;
}

void FractionElement::paint( QPainter& painter ) const
{
    m_numerator->paint( painter );       // first paint the children
    m_denominator->paint( painter );

//    if( bevelled )
//        paintBevelled( painter );
    
    // now paint the line
/*    QPointF startPoint = origin() + QPointF( m_numerator->height()+distY );
    QPointF endPoint = startPoint + QPointF( width(), 0 );
    
    painter.save();
    painter.setPen();
    painter.drawLine( startPoint, endPoint );
    painter.restore();*/
}

void FractionElement::calculateSize()
{
/*    m_numerator->calculateSize();
    m_denominator->calculateSize();

    if( bevelled )
        calculateSizeBevelled();
    
    QPointF numeratorOrigin;
    QPointF denominatorOrigin;
    double linethickness = AttributeManager::valueOf( attribute( "linethickness" ) );
//    double distY = AttributeManager::valueOf( "" );
    Align numalign = AttributeManager::valueOf( attribute( "numalign" ) ); 
    Align denomalign = AttributeManager::valueOf( attribute( "denomalign" ) ); 
    
    setWidth( qMax( m_numerator->width(), m_denominator->width() ) );
    setHeight( m_numerator->height() + m_denominator->height() +
               linethickness + 2*distY );
    setBaseline( m_numerator->height() + distY + 0.5*linethickness );

    if( numalign == Left )
        numeratorOrigin.setX( 0.0 )
    else if( numalign == Right )
        numeratorOrigin.setX( width() - m_numerator->width() );
    else
	numeratorOrigin.setX( ( width() - m_numerator->width() ) / 2 );

    if( denomalign == Left )
        denominatorOrigin.setX( 0.0 )
    else if( denomalign == Right )
        denominatorOrigin.setX( width() - m_denominator->width() );
    else
	denominatorOrigin.setX( ( width() - m_denominator->width() ) / 2 );
		    
    m_numerator->setOrigin( numeratorOrigin );
    m_denominator->setOrigin( denominatorOrigin );*/
}

const QList<BasicElement*> FractionElement::childElements()
{
    QList<BasicElement*> list;
    list << m_denominator << m_numerator;
    return list;
}

void FractionElement::insertChild( FormulaCursor* cursor, BasicElement* child )
{
}
   
void FractionElement::removeChild( BasicElement* element )
{
    if( element == m_numerator )         
    {
        delete m_numerator;                      // delete the numerator and
        m_numerator = new BasicElement( this );  // assign a new empty BasicElement   
    }
    else if( element == m_denominator )
    {
        delete m_denominator;
        m_denominator = new BasicElement( this );
    }
}

void FractionElement::readMathML( const QDomElement& element )
{
    readMathMLAttributes( element );

    if( element.childNodes().count() != 2 ) // a fraction element has always two children
	return;
    
    QDomElement tmp = element.firstChildElement();  // create first the numerator
    m_numerator = ElementFactory::createElement( tmp.tagName(), this );
    m_numerator->readMathML( tmp );
    
    tmp = element.lastChildElement();               // then the denominator
    m_denominator = ElementFactory::createElement( tmp.tagName(), this );
    m_denominator->readMathML( tmp );
}

void FractionElement::writeMathML( KoXmlWriter* writer, bool oasisFormat )
{
    writer->startElement( oasisFormat ? "math:mfrac": "mfrac" );
    writeMathMLAttributes( writer );
   
    m_numerator->writeMathML( writer, oasisFormat );
    m_denominator->writeMathML( writer, oasisFormat );

    writer->endElement();
}

void FractionElement::moveLeft( FormulaCursor* cursor, BasicElement* from )
{
    if( from == parentElement() )
        m_denominator->moveLeft( cursor, this );
    else
        parentElement()->moveLeft( cursor, this ); 
}

void FractionElement::moveRight( FormulaCursor* cursor, BasicElement* from )
{
    if( from == parentElement() )
        m_numerator->moveRight( cursor, this );
    else
        parentElement()->moveRight( cursor, this ); 
}

void FractionElement::moveUp( FormulaCursor* cursor, BasicElement* from )
{
    if( from == m_denominator )
        m_numerator->moveUp( cursor, this );
    else
        parentElement()->moveUp( cursor, this );
}

void FractionElement::moveDown(FormulaCursor* cursor, BasicElement* from)
{
    if( from == m_numerator )
        m_denominator->moveDown( cursor, this );
    else
        parentElement()->moveDown( cursor, this );
}








/**
 * Appends our attributes to the dom element.
 */
void FractionElement::writeDom(QDomElement element)
{
    BasicElement::writeDom(element);

    QDomDocument doc = element.ownerDocument();
//    if (!withLine) element.setAttribute("NOLINE", 1);

    QDomElement num = doc.createElement("NUMERATOR");
    num.appendChild(m_numerator->getElementDom(doc));
    element.appendChild(num);

    QDomElement den = doc.createElement("DENOMINATOR");
    den.appendChild(m_denominator->getElementDom(doc));
    element.appendChild(den);
}

/**
 * Reads our attributes from the element.
 * Returns false if it failed.
 */
bool FractionElement::readAttributesFromDom(QDomElement element)
{
    if (!BasicElement::readAttributesFromDom(element)) {
        return false;
    }
    QString lineStr = element.attribute("NOLINE");
    if(!lineStr.isNull()) {
//        withLine = lineStr.toInt() == 0;
    }
    return true;
}

/**
 * Reads our content from the node. Sets the node to the next node
 * that needs to be read.
 * Returns false if it failed.
 */
bool FractionElement::readContentFromDom(QDomNode& node)
{
/*    if (!BasicElement::readContentFromDom(node)) {
        return false;
    }

    if ( !buildChild( m_numerator, node, "NUMERATOR" ) ) {
        kWarning( DEBUGID ) << "Empty numerator in FractionElement." << endl;
        return false;
    }
    node = node.nextSibling();

    if ( !buildChild( m_denominator, node, "DENOMINATOR" ) ) {
        kWarning( DEBUGID ) << "Empty denominator in FractionElement." << endl;
        return false;
    }
    node = node.nextSibling();
*/
    return true;
}

}  // namespace KFormula
