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

#include "FormulaRenderer.h"
#include "FormulaCursor.h"
#include "FormulaContainer.h"
#include "MathMLLoader.h"

namespace KFormula {
	
FormulaShape::FormulaShape()
{
    m_formulaContainer = new FormulaContainer();
    m_formulaCursor = 0;
    m_formulaRenderer = 0;
}

FormulaShape::~FormulaShape()
{
    if( m_formulaRenderer )
         delete m_formulaRenderer;

    delete m_formulaContainer;
}

void FormulaShape::paint( QPainter &painter, KoViewConverter &converter )
{
    m_formulaRenderer = new FormulaRenderer();
    m_formulaRenderer->render( painter ); 
}
   
void FormulaShape::setFormulaCursor( FormulaCursor* cursor )
{
}

FormulaCursor* FormulaShape::formulaCursor() const
{
}

void FormulaShape::keyPressEvent( QKeyEvent* event )
{
    m_formulaContainer->deleteElementBeforeCursor();
    m_formulaContainer->deleteElementAfterCursor();
    m_formulaContainer->moveSelectedElements();
    m_formulaContainer->deleteSelectedElements();
}


void FormulaShape::saveMathML( QTextStream& stream, bool oasisFormat = false )
{
}

bool FormulaShape::loadMathML( const QDomDocument &doc, bool oasisFormat = false )
{
    MathMLLoader loader( m_formulaContainer );
    if( !loader.parse( doc ) )
        return false;
}

} // namespace KFormula

