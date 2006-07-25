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

#ifndef MATRIXELEMENT_H
#define MATRIXELEMENT_H

#include "BasicElement.h"

namespace KFormula {

class MatrixRowElement;
class MatrixEntryElement;
	
/**
 * @short A matrix or table element in a formula
 *
 * A matrix element contains a list of rows which are of class MatrixRowElement.
 * These rows contain single entries which are of class MatrixEntryElement. The
 * MatrixElement takes care that the different MatrixRowElements are informed how
 * to lay out their children correctly as they need to be synced.
 */
class MatrixElement : public BasicElement {
    friend class KFCRemoveColumn;
    friend class KFCRemoveRow;

public:
    /// The standard constructor
    MatrixElement( int rows = 1, int columns = 1, BasicElement* parent = 0);
    
    /// The standard destructor
    ~MatrixElement();

    /**
     * Obtain a list of all child elements of this element
     * @return a QList with pointers to all child elements
     */
    virtual const QList<BasicElement*>& childElements();

    /// Return the number of the rows of this matrix
    int rows() const;

    /// Return the number of the columns of this matrix
    int cols() const;
    
    /// Obtain a pointer to the element at @p row and @p col in the matrix
    MatrixEntryElement* matrixEntryAt( int row, int col );

    /// Save this element to MathMl
    virtual void writeMathML( QDomDocument& doc, QDomNode& parent, bool oasisFormat = false );




    
    /// A copy constructor
    MatrixElement( const MatrixElement& );
    
    /// Returns a clone of this element
    virtual MatrixElement* clone() { return new MatrixElement( *this ); }

    
    /**
     * The cursor has entered one of our child sequences.
     * This is a good point to tell the user where he is.
     */
    virtual void entered( SequenceElement* child );

    /**
     * Sets the cursor and returns the element the point is in.
     * The handled flag shows whether the cursor has been set.
     * This is needed because only the innermost matching element
     * is allowed to set the cursor.
     */
//    virtual BasicElement* goToPos( FormulaCursor*, bool& handled,
//                                   const LuPixelPoint& point, const LuPixelPoint& parentOrigin );
                                   
    /** Calculates our width and height and our children's parentPosition. */
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

    /** Dispatch this FontCommand to all our TextElement children. */
    virtual void dispatchFontCommand( FontCommand* cmd );

    /**
     * Enters this element while moving to the left starting inside
     * the element `from'. Searches for a cursor position inside
     * this element or to the left of it.
     */
    virtual void moveLeft(FormulaCursor* cursor, BasicElement* from);

    /**
     * Enters this element while moving to the right starting inside
     * the element `from'. Searches for a cursor position inside
     * this element or to the right of it.
     */
    virtual void moveRight(FormulaCursor* cursor, BasicElement* from);

    /**
     * Enters this element while moving up starting inside
     * the element `from'. Searches for a cursor position inside
     * this element or above it.
     */
    virtual void moveUp(FormulaCursor*, BasicElement*);

    /**
     * Enters this element while moving down starting inside
     * the element `from'. Searches for a cursor position inside
     * this element or below it.
     */
    virtual void moveDown(FormulaCursor*, BasicElement*);

    /**
     * Sets the cursor inside this element to its start position.
     * For most elements that is the main child.
     */
    virtual void goInside(FormulaCursor* cursor);

    /// We define the Main Child of a matrix to be the first row/column.
    // If there is a main child we must provide the insert/remove semantics.
    virtual SequenceElement* getMainChild();

    /// Sets the cursor to select the child. The mark is palced after this element.
    virtual void selectChild( FormulaCursor*, BasicElement* );


protected:
    /// Draws the element internally, means it paints into @ref m_elementPath
    virtual void drawInternal();

    

    /// Returns the tag name of this element type.
    virtual QString getTagName() const { return "MATRIX"; }

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
    /// The rows a matrix contains
    QList< MatrixRowElement* > m_matrixRowElements;



    
    /**
     * Searches through the matrix for the element. Sets the
     * row and column if found.
     * Returns true if the element was found. false otherwise.
     */
    bool searchElement( BasicElement* element, int& row, int& column );
};

} // namespace KFormula

#endif // MATRIXELEMENT_H
