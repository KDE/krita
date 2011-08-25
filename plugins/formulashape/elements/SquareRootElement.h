/* This file is part of the KDE project
   Copyright (C) 2007 Martin Pfeiffer <hubipete@gmx.net>

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

#ifndef SQUAREROOTELEMENT_H
#define SQUAREROOTELEMENT_H

#include "RowElement.h"
#include "kformula_export.h"

#include <QPainterPath>

/**
 * @short Implementation of the MathML msqrt element
 *
 * The msqrt element is not implemented along with mroot element in a single class
 * as it is an inferred row element. That means that it takes any number of child
 * elements whereas mroot only accepts exactly 2 child elements. From a painting
 * and layouting perspective both are very similar except for the lack of a exponent
 * in square root.
 */
class KOFORMULA_EXPORT SquareRootElement : public RowElement {
public:
    /// The standard constructor
    SquareRootElement( BasicElement* parent = 0 );

    /// The standard destructor
    ~SquareRootElement();

    /**
     * Render the element to the given QPainter
     * @param painter The QPainter to paint the element to
     */
    void paint( QPainter& painter, AttributeManager* am );

    /**
     * Calculate the size of the element and the positions of its children
     * @param am The AttributeManager providing information about attributes values
     */
    void layout( const AttributeManager* am );

    /// @return The element's ElementType
    ElementType elementType() const;

private:
    /// The point the artwork relates to.
    QPointF m_rootOffset;

    /// The QPainterPath that holds the lines for the root sign   
    QPainterPath m_rootSymbol;
    
    /// Line thickness, in pixels
    qreal m_lineThickness;
};

#endif // SQUAREROOTELEMENT_H
