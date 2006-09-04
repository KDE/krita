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

#include "MultiscriptElement.h"
#include <KoXmlWriter.h>

namespace KFormula {

MultiscriptElement::MultiscriptElement( BasicElement* parent ) : BasicElement( parent )
{
    m_baseElement = new BasicElement( this );
    m_preSubscript = new BasicElement( this );
    m_preSuperscript = new BasicElement( this );
    m_postSubscript = new BasicElement( this );
    m_postSuperscript = new BasicElement( this );
}

MultiscriptElement::~MultiscriptElement()
{
    delete m_baseElement;
    delete m_preSubscript;
    delete m_preSuperscript;
    delete m_postSubscript;
    delete m_postSuperscript;
}

const QList<BasicElement*> MultiscriptElement::childElements()
{
    return QList<BasicElement*>();
}

void MultiscriptElement::readMathML( const QDomElement& element )
{
}

void MultiscriptElement::writeMathML( KoXmlWriter* writer, bool oasisFormat )
{
    writer->startElement( oasisFormat ? "math:mmultiscripts" : "mmultiscripts" );
    writeMathMLAttributes( writer );
   
    m_baseElement->writeMathML( writer, oasisFormat );
    // saving of mmultiscript is a bit more complicated, still to be done
    
    writer->endElement();
}

void MultiscriptElement::calcSizes( const ContextStyle& context, ContextStyle::TextStyle tstyle,
                                          ContextStyle::IndexStyle istyle )
{
}

void MultiscriptElement::draw( QPainter& painter, const LuPixelRect& r,
                       const ContextStyle& context,
                       ContextStyle::TextStyle tstyle,
                       ContextStyle::IndexStyle istyle,
                       const LuPixelPoint& parentOrigin )
{}


} // namespace KFormula
