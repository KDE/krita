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

#include "FormulaRenderer.h"
#include "AttributeManager.h"
#include "BasicElement.h"
#include <kdebug.h>

FormulaRenderer::FormulaRenderer()
{
    m_dirtyElement = 0;
    m_attributeManager = new AttributeManager();
}

FormulaRenderer::~FormulaRenderer()
{
    delete m_attributeManager;
}

void FormulaRenderer::paintElement( QPainter& p, BasicElement* element, bool hints )
{
    p.save();
    p.setRenderHint( QPainter::Antialiasing );
    p.translate( element->origin() );          // setup painter
    if (!hints) {
        element->paint( p, m_attributeManager );   // let element paint itself
    } else {
        element->paintEditingHints( p, m_attributeManager );
    }

    // eventually paint all its children
    if( !element->childElements().isEmpty() && element->elementType() != Phantom ) {
        foreach( BasicElement* tmpElement, element->childElements() ) {
            paintElement( p, tmpElement, hints );
        }
    }

    p.restore();
}


void FormulaRenderer::layoutElement( BasicElement* element )
{
    int i = 0;
    element->setDisplayStyle( m_attributeManager->boolOf("displaystyle", element));
    foreach( BasicElement* tmp, element->childElements() ) {
	int scale = m_attributeManager->scriptLevel( element, i++ ); 
        tmp->setScaleLevel( scale );
        layoutElement( tmp );              // first layout all children
    }
    element->layout( m_attributeManager );      // actually layout the element
    element->stretch();
}

void FormulaRenderer::update( QPainter& p, BasicElement* element )
{
    updateElementLayout( element );              // relayout the changed element
    paintElement( p, m_dirtyElement );     // and then repaint as much as needed
}

void FormulaRenderer::updateElementLayout( BasicElement* element )
{
    QRectF tmpBoundingRect;
    bool parentLayoutAffected = true;
    BasicElement* tmpElement = element;
    while( parentLayoutAffected )
    {
        tmpBoundingRect = tmpElement->boundingRect();   // cache the former boundingRect
        tmpElement->layout( m_attributeManager );       // layout the element

        // check whether the new layout affects the parent element's layout
        if( tmpBoundingRect == tmpElement->boundingRect() )
        {
            parentLayoutAffected = false;               // stop the layouting
            m_dirtyElement = tmpElement;
        }
        else
            tmpElement = tmpElement->parentElement();   // prepare layouting the parent
    }
}

qreal FormulaRenderer::elementScaleFactor( BasicElement* element ) const
{
    Q_UNUSED(element)
    AttributeManager am;
    return -1;  // FIXME!
}
