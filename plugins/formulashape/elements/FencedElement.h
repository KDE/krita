/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
		 2006 Martin Pfeiffer <hubipete@gmx.net>
   Copyright (C) 2006-2007 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>
                 2007 Martin Pfeiffer <hubipete@gmx.net>

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

#ifndef FENCEDELEMENT_H
#define FENCEDELEMENT_H

#include "RowElement.h"
#include "kformula_export.h"
#include <QPainterPath>

/**
 * A left and/or right bracket around one child.
 */
class KOFORMULA_EXPORT FencedElement : public RowElement {
public:
    /// The standart constructor
    FencedElement( BasicElement* parent = 0 );

    /**
     * Render the element to the given QPainter
     * @param painter The QPainter to paint the element to
     * @param am AttributeManager containing style info
     */
    virtual void paint( QPainter& painter, AttributeManager* am );

    /**
     * Calculate the size of the element and the positions of its children
     * @param am The AttributeManager providing information about attributes values
     */
    void layout( const AttributeManager* am );

    /// @return The default value of the attribute for this element
    QString attributesDefaultValue( const QString& attribute ) const;
    
    /// @return The element's ElementType
    ElementType elementType() const;
    
private:
    /// The buffer the element paints its visual content
    QPainterPath m_fence;
};

#endif // FENCEDELEMENT_H
