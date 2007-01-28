/* This file is part of the KDE project
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

#ifndef MULTISCRIPTELEMENT_H
#define MULTISCRIPTELEMENT_H

#include "BasicElement.h"

namespace KFormula {

/**
 * @short Implementation of the msub, msup, msubsup and mmultiscript element
 */
class MultiscriptElement : public BasicElement {
public:
    /// The standard constructor
    MultiscriptElement( BasicElement* parent = 0 );

    /// The destructor
    ~MultiscriptElement();

    /**
     * Obtain a list of all child elements of this element
     * @return a QList with pointers to all child elements
     */
    const QList<BasicElement*> childElements();

    /**
     * Insert a new child at the cursor position
     * @param cursor The cursor holding the position where to inser
     * @param child A BasicElement to insert
     */
    void insertChild( FormulaCursor* cursor, BasicElement* child );
   
    /**
     * Remove a child element
     * @param element The BasicElement to remove
     */ 
    void removeChild( BasicElement* element );

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
    
    /**
     * Move the FormulaCursor left
     * @param cursor The FormulaCursor to be moved
     * @param from The BasicElement which was the last owner of the FormulaCursor
     */
    void moveLeft( FormulaCursor* cursor, BasicElement* from );

    /**
     * Move the FormulaCursor right 
     * @param cursor The FormulaCursor to be moved
     * @param from The BasicElement which was the last owner of the FormulaCursor
     */
    void moveRight( FormulaCursor* cursor, BasicElement* from );

    /**
     * Move the FormulaCursor up 
     * @param cursor The FormulaCursor to be moved
     * @param from The BasicElement which was the last owner of the FormulaCursor
     */
    void moveUp( FormulaCursor* cursor, BasicElement* from );

    /**
     * Move the FormulaCursor down 
     * @param cursor The FormulaCursor to be moved
     * @param from The BasicElement which was the last owner of the FormulaCursor

    void readMathML( const QDomElement& element );
    
private:
    /// The BasicElement representing the base element of the multiscript
    BasicElement* m_baseElement;

    /// The BasicElement representing the subscript left to the base element
    BasicElement* m_preSubscript;

    /// The BasicElement representing the superscript left to the base element
    BasicElement* m_preSuperscript;

    /// The BasicElement representing the subscript right to the base element
    BasicElement* m_postSubscript;

    /// The BasicElement representing the superscript right to the base element
    BasicElement* m_postSuperscript;


    virtual QString getElementName() const { return "mmultiscript"; }
    virtual void writeMathMLContent( KoXmlWriter* writer, bool oasisFormat = false );
};

} // namespace KFormula

#endif // MULTISCRIPTELEMENT_H
