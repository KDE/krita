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

#ifndef SEQUENCEELEMENT_H
#define SEQUENCEELEMENT_H

#include "BasicElement.h"

namespace KFormula {

/**
 * The element that contains a number of children.
 * The children are aligned in one line.
 */
class SequenceElement : public BasicElement {
public:
    /// The standard constructor
    SequenceElement( BasicElement* parent = 0 );
   
    /// The standard destructor 
    ~SequenceElement();

    /**
     * Render the element to the given QPainter
     * @param painter The QPainter to paint the element to
     */
    virtual void paint( QPainter& painter ) const;

    virtual void calculateSize();

    /**
     * Insert a new child at the cursor position - reimplemented from BasicElement
     * @param cursor The cursor holding the position where to inser
     * @param child A BasicElement to insert
     */
    virtual void insertChild( FormulaCursor* cursor, BasicElement* child );
   
    /**
     * Remove a child element
     * @param element The BasicElement to remove
     */ 
    virtual void removeChild( BasicElement* element );

    /**
     * Obtain a list of all child elements of this element
     * reimplementated from @see BasicElement
     * @return a QList with pointers to all child elements
     */
    virtual const QList<BasicElement*> childElements();

    /// @return The child element at the position @p index - 0 if the sequence is empty
    BasicElement* childAt( int index );

    /// @return The index of the @p element in the sequence - -1 if not in sequence
    int indexOfElement( const BasicElement* element ) const;

    /**
     * Move the FormulaCursor left - reimplemented from BasicElement
     * @param cursor The FormulaCursor to be moved
     * @param from The BasicElement which was the last owner of the FormulaCursor
     */
    virtual void moveLeft( FormulaCursor* cursor, BasicElement* from );

    /**
     * Move the FormulaCursor right - reimplemented from BasicElement
     * @param cursor The FormulaCursor to be moved
     * @param from The BasicElement which was the last owner of the FormulaCursor
     */
    virtual void moveRight( FormulaCursor* cursor, BasicElement* from );

    virtual void readMathML( const QDomElement& element );
    
    virtual void writeMathML( KoXmlWriter* writer, bool oasisFormat = false );
   



 

   /**
     * Stores the given childrens dom in the element.
     */
    void getChildrenDom( QDomDocument& doc, QDomElement elem, uint from, uint to);

    /**
     * Builds elements from the given node and its siblings and
     * puts them into the list.
     * Returns false if an error occures.
     */
    bool buildChildrenFromDom(QList<BasicElement*>& list, QDomNode n);

protected:
    /**
     * Returns the tag name of this element type.
     */
    virtual QString getTagName() const { return "SEQUENCE"; }

    /**
     * Appends our attributes to the dom element.
     */
    virtual void writeDom(QDomElement element);

    /**
     * Reads our attributes from the element.
     * Returns false if it failed.
     */
    virtual bool readAttributesFromDom(QDomElement element);

    /**
     * Reads our content from the node. Sets the node to the next node
     * that needs to be read.
     * Returns false if it failed.
     */
    virtual bool readContentFromDom(QDomNode& node);

private:
    /// The sorted list of all elements in this sequence
    QList<BasicElement*> m_sequenceElements;
};

} // namespace KFormula

#endif // SEQUENCEELEMENT_H
