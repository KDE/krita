/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
   Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net>
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
   Boston, MA 02110-1301, USA.
*/

#ifndef SPACEELEMENT_H
#define SPACEELEMENT_H

#include "BasicElement.h"

namespace FormulaShape {

/**
 * @short Implementation of the MathML mspace element
 *
 * The mspace element is specified in the MathML spec section 3.2.7. As
 * FormulaShape currently does not implement linebreaking the linebreaking
 * attributes of SpaceElement are ignored.
 */
class SpaceElement : public BasicElement {
public:
    /// The standard constructor
    SpaceElement( BasicElement* parent = 0 );

    /**
     * Render the element to the given QPainter
     * @param painter The QPainter to paint the element to
     */
    void paint( QPainter& painter, const AttributeManager* am );

    /**
     * Calculate the size of the element and the positions of its children
     * @param am The AttributeManager providing information about attributes values
     */
    void layout( const AttributeManager* am );

    /// @return The element's ElementType
    ElementType elementType() const;

    /// @return The default value of the attribute for this element
    QVariant attributesDefaultValue( const QString& attribute ) const;
    
    /// Read the element from MathML
    void readMathML( const KoXmlElement& element );

    /// Save the element to MathML 
    void writeMathML( KoXmlWriter* writer, bool oasisFormat = false ) const;
};

} // namespace FormulaShape

#endif // SPACEELEMENT_H
