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

#ifndef MATRIXROWELEMENT_H
#define MATRIXROWELEMENT_H

#include "BasicElement.h"


#include "contextstyle.h"

namespace KFormula {
	
class MatrixEntryElement;

/**
 * Any number of lines. Representing the MathML mtr element.
 */
class MatrixRowElement : public BasicElement {
    friend class KFCNewLine;

public:
    /// The standard constructor
    MatrixRowElement( BasicElement* parent = 0 );

    /// The standard destructor
    ~MatrixRowElement();

    /**
     * Obtain a list of all child elements of this element
     * @return a QList with pointers to all child elements
     */
    virtual const QList<BasicElement*>& childElements();
    
    /// @return The number of @see MatrixEntryElement in this MatrixRowElement
    int numberOfEntries() const;

    /// @return The MatrixEntryElement at the @p pos position in the MatrixRowElement
    MatrixEntryElement* entryAtPosition( int pos );

    virtual void writeMathML( QDomDocument& doc, QDomNode& parent, bool oasisFormat = false );





    /**
     * The cursor has entered one of our child sequences.
     * This is a good point to tell the user where he is.
     */
//    virtual void entered( SequenceElement* child );

    /**
     * Sets the cursor inside this element to its start position.
     * For most elements that is the main child.
     */
    virtual void goInside(FormulaCursor* cursor);

    /**
     * Enters this element while moving to the left starting inside
     * the element `from'. Searches for a cursor position inside
     * this element or to the left of it.
     */
    virtual void moveLeft( FormulaCursor* cursor, BasicElement* from );

    /**
     * Enters this element while moving to the right starting inside
     * the element `from'. Searches for a cursor position inside
     * this element or to the right of it.
     */
    virtual void moveRight( FormulaCursor* cursor, BasicElement* from );

    /**
     * Enters this element while moving up starting inside
     * the element `from'. Searches for a cursor position inside
     * this element or above it.
     */
    virtual void moveUp( FormulaCursor* cursor, BasicElement* from );

    /**
     * Enters this element while moving down starting inside
     * the element `from'. Searches for a cursor position inside
     * this element or below it.
     */
    virtual void moveDown( FormulaCursor* cursor, BasicElement* from );

    /// Calculates our width and height and our children's parentPosition.
    virtual void calcSizes(const ContextStyle& context, ContextStyle::TextStyle tstyle, ContextStyle::IndexStyle istyle);

    /**
     * Draws the whole element including its children.
     * The `parentOrigin' is the point this element's parent starts.
     * We can use our parentPosition to get our own origin then.
     */
    virtual void draw( QPainter& painter, const LuPixelRect& r,
                       const ContextStyle& context,
                       ContextStyle::TextStyle tstyle,
                       ContextStyle::IndexStyle istyle,
                       const LuPixelPoint& parentOrigin );

    /**
     * Dispatch this FontCommand to all our TextElement children.
     */
 //   virtual void dispatchFontCommand( FontCommand* cmd );

    virtual void insert(FormulaCursor*, QList<BasicElement*>&, Direction);
    virtual void remove(FormulaCursor*, QList<BasicElement*>&, Direction);

//    virtual void normalize(FormulaCursor*, Direction);

//    virtual SequenceElement* getMainChild();

    /**
     * Sets the cursor to select the child. The mark is placed before,
     * the position behind it.
     */
    virtual void selectChild(FormulaCursor* cursor, BasicElement* child);


protected:
    /// Draws the element internally, means it paints into m_elementPath
    virtual void drawInternal();


    
    /// Returns the tag name of this element type.
    virtual QString getTagName() const { return "MULTILINE"; }

    /// Appends our attributes to the dom element.
    virtual void writeDom(QDomElement element);

    /// Reads our attributes from the element. Returns false if it failed.
    virtual bool readAttributesFromDom(QDomElement element);

    /**
     * Reads our content from the node. Sets the node to the next node
     * that needs to be read. Returns false if it failed.
     */
    virtual bool readContentFromDom( QDomNode& node );


private:
    /// The list of sequences. Each one is a line.
    QList<MatrixEntryElement*> m_matrixEntryElements;
};

} // namespace KFormula

#endif

