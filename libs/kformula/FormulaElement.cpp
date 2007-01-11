/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
   Copyright (C) 2006-2007 Martin Pfeiffer <hubipete@gmx.net>
   Copyright (C) 2006-2007 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>

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

#include "FormulaElement.h"
#include "FormulaCursor.h"
#include <KoXmlWriter.h>

namespace FormulaShape {

FormulaElement::FormulaElement() : BasicElement( 0 )
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

void FormulaElement::paint( QPainter&, const AttributeManager* )
{ /* Implement it so that BasicElement::paint() is not called */ }

void FormulaElement::layout( const AttributeManager* am )
{
    Q_UNUSED( am )
}

void FormulaElement::moveLeft( FormulaCursor* cursor, BasicElement* from )
{
    Q_UNUSED( cursor )
    Q_UNUSED( from )
}

void FormulaElement::moveRight( FormulaCursor* cursor, BasicElement* from )
{
    Q_UNUSED( cursor )
    Q_UNUSED( from )
}

void FormulaElement::moveUp( FormulaCursor* cursor, BasicElement* from )
{
    Q_UNUSED( cursor )
    Q_UNUSED( from )
}

void FormulaElement::moveDown( FormulaCursor* cursor, BasicElement* from )
{
    Q_UNUSED( cursor )
    Q_UNUSED( from )
}

void FormulaElement::readMathML( const KoXmlElement& element )
{
    readMathMLAttributes( element );
}

void FormulaElement::writeMathML( KoXmlWriter* writer, bool oasisFormat ) const
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

ElementType FormulaElement::elementType() const
{
    return Formula;
}

} // namespace FormulaShape
