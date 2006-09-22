/* This file is part of the KDE project
   Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net>

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

#include "UnderOverElement.h"
#include "ElementFactory.h"
#include <KoXmlWriter.h>

namespace KFormula {

UnderOverElement::UnderOverElement( BasicElement* parent ) : BasicElement( parent )
{
    m_baseElement = new BasicElement( this );
    m_underElement = new BasicElement( this );
    m_overElement = new BasicElement( this );
}

UnderOverElement::~UnderOverElement()
{
    delete m_baseElement;
    delete m_underElement;
    delete m_overElement;
}

const QList<BasicElement*> UnderOverElement::childElements()
{
    QList<BasicElement*> tmp;
    return tmp << m_baseElement << m_underElement << m_overElement;
}

void UnderOverElement::readMathML( const QDomElement& element )
{
    readMathMLAttributes( element );

    if( element.childNodes().count() != 3 && element.childNodes().count() != 2 )
	return;

    QDomElement tmp = element.firstChildElement();
    delete m_baseElement;
    m_baseElement = ElementFactory::createElement( tmp.tagName(), this );
    m_baseElement->readMathML( tmp );

    tmp = tmp.nextSiblingElement();
    if( element.tagName() == "mover" )
    {
	delete m_overElement;
	m_overElement = ElementFactory::createElement( tmp.tagName(), this );
        m_overElement->readMathML( tmp );
    }
    else
    {
	delete m_underElement;
	m_underElement = ElementFactory::createElement( tmp.tagName(), this );
        m_underElement->readMathML( tmp );
    }

    tmp = tmp.nextSiblingElement();
    if( !tmp.isNull() )
    {
	delete m_overElement;
	m_overElement = ElementFactory::createElement( tmp.tagName(), this );
        m_overElement->readMathML( tmp );
    }
}

void UnderOverElement::writeMathML( KoXmlWriter* writer, bool oasisFormat )
{
    if( m_underElement->elementType() != Basic && m_overElement->elementType() == Basic )
    {
	writer->startElement( oasisFormat ? "math:munder" : "munder" );
        writeMathMLAttributes( writer );
        m_baseElement->writeMathML( writer, oasisFormat );
        m_underElement->writeMathML( writer, oasisFormat );
    }
    if( m_overElement->elementType() != Basic && m_underElement->elementType() == Basic )
    {
	writer->startElement( oasisFormat ? "math:mover" : "mover" );
        writeMathMLAttributes( writer );
        m_baseElement->writeMathML( writer, oasisFormat );
        m_overElement->writeMathML( writer, oasisFormat );
    }
    else
    {
    	writer->startElement( oasisFormat ? "math:munderover" : "munderover" );
        writeMathMLAttributes( writer );
        m_baseElement->writeMathML( writer, oasisFormat );
        m_underElement->writeMathML( writer, oasisFormat );
        m_overElement->writeMathML( writer, oasisFormat );
    }

    writer->endElement();
}

void UnderOverElement::calcSizes( const ContextStyle& context, ContextStyle::TextStyle tstyle,
                                          ContextStyle::IndexStyle istyle )
{
}

void UnderOverElement::draw( QPainter& painter, const LuPixelRect& r,
                       const ContextStyle& context,
                       ContextStyle::TextStyle tstyle,
                       ContextStyle::IndexStyle istyle,
                       const LuPixelPoint& parentOrigin )
{}


} // namespace KFormula
