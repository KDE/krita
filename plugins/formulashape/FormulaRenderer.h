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

#ifndef FORMULARENDERER_H
#define FORMULARENDERER_H

#include <QPainter>
#include "kformula_export.h"

class AttributeManager;
class BasicElement;

/**
 * @short FormulaRenderer takes care of painting and layouting the elements
 *
 * FormulaRenderer follows the visitor pattern. It iterates through the element
 * tree and calls layout() or paint() methods to let the single elements layout
 * or paint itsselves. This more generic approach allows more efficient repainting
 * and relayouting.
 * The update() method is the most interesting of the class as it is used to update
 * the visuals when the formula tree has changed somehow. The method calls
 * layoutElement() and then paintElement(). The former is made to be efficient so it
 * layouts only as many parental elements as needed.
 * Using a central class for painting parts or the whole tree structure that makes
 * up a formula has several advantages. First it reduces a lot of code duplication.
 * Second it takes care of painting and layouting in the right order so that there
 * no need anymore for the single element classes to do so. Third we can control 
 * instance AttributeManager and destroying and constructing it often is not needed
 * anymore.
 *
 * @author Martin Pfeiffer <hubipete@gmx.net>
 */
class KOFORMULA_EXPORT FormulaRenderer {
public:
    /// The constructor
    FormulaRenderer();

    /// The destructor
    ~FormulaRenderer();

    /**
     * Paint an element and all its children
     * @param p The QPainter that should be used to paint the element
     * @param element The element to be painted
     */
    void paintElement( QPainter& p, BasicElement* element, bool hints=false );

    /**
     * Layout an element and all its children
     * @param element The element to be layouted
     */
    void layoutElement( BasicElement* element );

    /**
     * Update an element after it has changed
     * @param p The QPainter that should be used to paint the element
     * @param element The element that has changed
     */
    void update( QPainter& p, BasicElement* element );

    /// Just for updating one elements layout after a change
    void updateElementLayout( BasicElement* element );

private:
    qreal elementScaleFactor( BasicElement* element ) const;

    /// The attribute manager used for renderering and layouting
    AttributeManager* m_attributeManager;

    /// Used by update() to store the highest element in the tree that needs repaint
    BasicElement* m_dirtyElement;
};

#endif // FORMULARENDERER_H
