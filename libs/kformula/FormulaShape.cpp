/* This file is part of the KDE project
   Copyright (C) Martin Pfeiffer <hubipete@gmx.net>

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

#include "FormulaShape.h"
#include "FormulaElement.h"
#include <KoXmlWriter.h>

namespace KFormula {
	
FormulaShape::FormulaShape()
{
    m_formulaElement = new FormulaElement();
}

FormulaShape::~FormulaShape()
{
    delete m_formulaElement;
}

void FormulaShape::paint( QPainter &painter, KoViewConverter &converter ) 
{
    applyConversion( painter, converter );   // apply zooming and coordinate translation
    m_formulaElement->paint( painter );      // paint the formula
}

BasicElement* FormulaShape::elementAt( const QPointF& p )
{
    return m_formulaElement->childElementAt( p );
}

QSizeF FormulaShape::size() const
{
    return m_formulaElement->boundingRect().size();
}

void FormulaShape::resize( const QSizeF& )
{
    // do nothing as FormulaShape is fixed size
}

QRectF FormulaShape::boundingRect() const
{
    return m_invMatrix.inverted().mapRect( m_formulaElement->boundingRect() );
}

BasicElement* FormulaShape::formulaElement() const
{
    return m_formulaElement;
}

void FormulaShape::loadMathML( const KoXmlDocument &doc, bool )
{
    delete m_formulaElement;                                // delete the old formula
    m_formulaElement = new FormulaElement();                // create a new root element
    m_formulaElement->readMathML( doc.documentElement() );  // and load the new formula
}

void FormulaShape::saveMathML( KoXmlWriter* writer, bool oasisFormat )
{
    if( m_formulaElement->childElements().isEmpty() )  // if the formula is empty
	return;                                        // do not save it
    
    m_formulaElement->writeMathML( writer, oasisFormat );
}

} // namespace KFormula

