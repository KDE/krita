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
#include "BasicElement.h"

#include <KoXmlWriter.h>

namespace KFormula {
	
FormulaShape::FormulaShape()
{
    m_formulaElement = 0;
}

FormulaShape::~FormulaShape()
{
}

void FormulaShape::paint( QPainter &painter, KoViewConverter &converter )
{
    // TODO adapt the QPainter's QMatrix to convert the points correctly to pixels
    m_formulaElement->paint( painter );
}

BasicElement* FormulaShape::formulaElement() const
{
    return m_formulaElement;
}
   
void FormulaShape::saveMathML( KoXmlWriter* writer, bool oasisFormat )
{
    if( !m_formulaElement )
        return;

    // TODO start a MathML doc or the OASIS pendant
    
    m_formulaElement->writeMathML( writer, oasisFormat );
}

void FormulaShape::loadMathML( const QDomDocument &doc, bool oasisFormat )
{
    // TODO combine the implementations of FormulaContainer and FormulaElement
}

} // namespace KFormula

