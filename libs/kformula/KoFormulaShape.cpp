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

#include "KoFormulaShape.h"
#include "FormulaElement.h"
#include "FormulaRenderer.h"
#include <KoXmlWriter.h>

namespace FormulaShape {
	
KoFormulaShape::KoFormulaShape()
{
    m_formulaElement = new FormulaElement();
    m_formulaRenderer = new FormulaRenderer();
}

KoFormulaShape::~KoFormulaShape()
{
    delete m_formulaElement;
    delete m_formulaRenderer;
}

void KoFormulaShape::paint( QPainter &painter, const KoViewConverter &converter ) 
{
    applyConversion( painter, converter );   // apply zooming and coordinate translation
    m_formulaRenderer->paintElement( painter, m_formulaElement );  // paint the formula
}

void KoFormulaShape::paintDecorations( QPainter &painter,
                                       const KoViewConverter &converter, bool selected )
{
    Q_UNUSED( painter )
    Q_UNUSED( converter )
    Q_UNUSED( selected )
    // TODO how to highlight things?? do we need it btw?
}


BasicElement* KoFormulaShape::elementAt( const QPointF& p )
{
    return m_formulaElement->childElementAt( p );
}

QSizeF KoFormulaShape::size() const
{
    return m_formulaElement->boundingRect().size();
}

void KoFormulaShape::resize( const QSizeF& )
{ /* do nothing as FormulaShape is fixed size */ }

QRectF KoFormulaShape::boundingRect() const
{
    return m_invMatrix.inverted().mapRect( m_formulaElement->boundingRect() );
}

BasicElement* KoFormulaShape::formulaElement() const
{
    return m_formulaElement;
}

void KoFormulaShape::loadMathML( const QDomDocument &doc, bool )
{
    Q_UNUSED( doc )

    delete m_formulaElement;                                // delete the old formula
    m_formulaElement = new FormulaElement();                // create a new root element
    m_formulaElement->readMathML( doc.documentElement() );  // and load the new formula
}

void KoFormulaShape::saveMathML( KoXmlWriter* writer, bool oasisFormat )
{
    if( m_formulaElement->childElements().isEmpty() )  // if the formula is empty
	return;                                        // do not save it
    
    m_formulaElement->writeMathML( writer, oasisFormat );
}

} // namespace FormulaShape

