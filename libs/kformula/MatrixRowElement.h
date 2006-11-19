/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
   Copyright (C) 2001 Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
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

#ifndef MATRIXROWELEMENT_H
#define MATRIXROWELEMENT_H

#include "BasicElement.h"


#include "contextstyle.h"

namespace KFormula {
	
class MatrixEntryElement;

/**
 * @short Representing the MathML mtr element.
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
    const QList<BasicElement*> childElements();
 
    /// @return The position of the given @p entry   
    int positionOfEntry( BasicElement* entry ) const;

    /// @return The MatrixEntryElement at the @p pos position in the MatrixRowElement
    MatrixEntryElement* entryAt( int pos );

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
    
    /// Read the element from MathML
    void readMathML( const QDomElement& element );
    
    /// Save this element to MathML
    void writeMathML( KoXmlWriter* writer, bool oasisFormat = false );





    /**
     * Sets the cursor inside this element to its start position.
     * For most elements that is the main child.
     */
    virtual void goInside(FormulaCursor* cursor);


    /// Calculates our width and height and our children's parentPosition.
    virtual void calcSizes( const ContextStyle& context,
                            ContextStyle::TextStyle tstyle,
                            ContextStyle::IndexStyle istyle,
                            StyleAttributes& style );

    /**
     * Draws the whole element including its children.
     * The `parentOrigin' is the point this element's parent starts.
     * We can use our parentPosition to get our own origin then.
     */
    virtual void draw( QPainter& painter, const LuPixelRect& r,
                       const ContextStyle& context,
                       ContextStyle::TextStyle tstyle,
                       ContextStyle::IndexStyle istyle,
                       StyleAttributes& style,
                       const LuPixelPoint& parentOrigin );

    virtual void insert(FormulaCursor*, QList<BasicElement*>&, Direction);
    virtual void remove(FormulaCursor*, QList<BasicElement*>&, Direction);

    /**
     * Sets the cursor to select the child. The mark is placed before,
     * the position behind it.
     */
    virtual void selectChild(FormulaCursor* cursor, BasicElement* child);


protected:
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
    /// The list of entries in this row of the matrix 
    QList<MatrixEntryElement*> m_matrixEntryElements;
};

} // namespace KFormula

#endif

