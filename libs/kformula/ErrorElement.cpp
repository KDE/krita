/* This file is part of the KDE project
   Copyright (C) 2006 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>

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
 * Boston, MA 02110-1301, USA.
*/

#include <qpainter.h>

#include "errorelement.h"

KFORMULA_NAMESPACE_BEGIN

ErrorElement::ErrorElement( BasicElement* parent ) : SequenceElement( parent ) {
}

/**
 * Draws the whole element including its children.
 * The `parentOrigin' is the point this element's parent starts.
 * We can use our parentPosition to get our own origin then.
 */
void ErrorElement::draw( QPainter& painter, const LuPixelRect& r,
                        const ContextStyle& context,
                        ContextStyle::TextStyle tstyle,
                        ContextStyle::IndexStyle istyle,
                        StyleAttributes& style,
                        const LuPixelPoint& parentOrigin )
{
    LuPixelPoint myPos( parentOrigin.x()+getX(), parentOrigin.y()+getY() );

    painter.fillRect( context.layoutUnitToPixelX( myPos.x() ),
                      context.layoutUnitToPixelY( myPos.y() ),
                      context.layoutUnitToPixelX( getWidth() ),
                      context.layoutUnitToPixelY( getHeight() ),
                      context.getErrorColor() );
    inherited::draw( painter, r, context, tstyle, istyle, style, parentOrigin );
}
    
KFORMULA_NAMESPACE_END
