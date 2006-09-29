/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
		 2006 Martin Pfeiffer <hubipete@gmx.net>

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

#ifndef FRACTIONELEMENT_H
#define FRACTIONELEMENT_H

#include "BasicElement.h"

namespace KFormula {
	
/**
 * @short A fraction element in a formula
 *
 * The fraction consists of two @see SequenceElement, the denominator and the numerator.
 * The SequenceElements can be set but actually altered they are with the
 * insertElementInNumerator() and insertElementInDenominator() methods.
 */
class FractionElement : public BasicElement {
public:
    /// The standard constructor
    FractionElement( BasicElement* parent = 0 );
   
    /// The standard destructor 
    ~FractionElement();

    /**
     * Render the element to the given QPainter
     * @param painter The QPainter to paint the element to
     */
    void paint( QPainter& painter ) const;

    /// Calculate the element's sizes and the size of its children
    void calculateSize();

    /**
     * Insert a new child at the cursor position - reimplemented from BasicElement
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
     * Obtain a list of all child elements of this element
     * @return a QList with pointers to all child elements
     */
    const QList<BasicElement*> childElements();

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
     */
    void moveDown( FormulaCursor* cursor, BasicElement* from );

    
    void readMathML( const QDomElement& element );
    
    void writeMathML( KoXmlWriter* writer, bool oasisFormat = false );

protected:
    /// Returns the tag name of this element type.
    virtual QString getTagName() const { return "FRACTION"; }

    /// Appends our attributes to the dom element.
    virtual void writeDom(QDomElement element);

    /// Reads our attributes from the element. Returns false if it failed.
    virtual bool readAttributesFromDom(QDomElement element);

    /**
     * Reads our content from the node. Sets the node to the next node
     * that needs to be read. Returns false if it failed.
     */
    virtual bool readContentFromDom(QDomNode& node);

private:
    /// The element representing the fraction's numerator
    BasicElement* m_numerator;

    /// The element representing the fraction's denominator 
    BasicElement* m_denominator;
};

} // namespace KFormula

#endif // FRACTIONELEMENT_H
