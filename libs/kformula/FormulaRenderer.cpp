/* This file is part of the KDE project
   Copyright 2006 Martin Pfeiffer <hubipete@gmx.net>

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

#include "FormulaRenderer.h"
#include <QPainter>

FormulaRenderer::FormulaRenderer( FormulaContainer* container = 0 )
{
    m_formulaContainer = container;
}

FormulaRenderer::FormulaRenderer( BasicElement* element = 0 )
{
}

FormulaRenderer::~FormulaRenderer()
{
}

void FormulaRenderer::render( QPainter* p )
{
    if( !p )
        return;
	
    if( !m_formulaContainer )
        return;
    
    QList<BasicElement> tmpElementList = m_formulaContainer->elements();
    foreach( BasicElement element, tmpElementList )
    {
	if( !element.isPhantomElement() )
	{
	    p->translate( element.coordinates() );
	    p->drawPath( element.elementPath() );
	    p->resetMatrix();
	}
    }
}

const QSize& FormulaRenderer::defaultSize() const
{
}

const QSize& FormulaRenderer::contentSize() const
{
}

void FormulaRenderer::setDecoration( bool decorate )
{
    m_paintDecoration = decorate;
}

void FormulaRenderer::setFormulaContainer( FormulaContainer* container )
{
}

void FormulaRenderer::setElement( BasicElement* element )
{
}

void FormulaRenderer::paintDecoration()
{
}
									
