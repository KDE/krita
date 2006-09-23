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



#include <QList>
#include <QString>
#include <QKeyEvent>

class QKeyEvent;

namespace KFormula {

class SymbolTable;

/**
 * The element that contains a number of children.
 * The children are aligned in one line.
 */
class SequenceElement : public BasicElement {
public:
    /// The standard constructor
    SequenceElement( BasicElement* parent = 0 );
    
    ~SequenceElement();

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
     * @returns whether its prohibited to change the sequence with this cursor.
     */
    virtual bool readOnly( const FormulaCursor* ) const;

    /**
     * @returns true if the sequence contains only text.
     */
    virtual bool isTextOnly() const { return textSequence; }


    /**
     * @returns true if there is no visible element in the sequence.
     * Please note that there might be phantom elements.
     */
    bool isEmpty();

    /**
     * Calculates our width and height and
     * our children's parentPosition.
     */
    //virtual void calcSizes(const ContextStyle& context,
    //                       ContextStyle::TextStyle tstyle,
    //                       ContextStyle::IndexStyle istyle);

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
     * Sets the cursor inside this element to its start position.
     * For most elements that is the main child.
     */
    virtual void goInside(FormulaCursor* cursor);

    /**
     * Inserts all new children at the cursor position. Places the
     * cursor according to the direction. The inserted elements will
     * be selected.
     *
     * The list will be emptied but stays the property of the caller.
     */
    virtual void insert(FormulaCursor*, QList<BasicElement*>&, Direction);

    /**
     * Removes all selected children and returns them. Places the
     * cursor to where the children have been.
     */
    virtual void remove(FormulaCursor*, QList<BasicElement*>&, Direction);

    /**
     * Moves the cursor to a normal place where new elements
     * might be inserted.
     */
    virtual void normalize(FormulaCursor*, Direction);

    /**
     * Returns the child at the cursor.
     * Does not care about the selection.
     */
    virtual BasicElement* getChild(FormulaCursor*, Direction = beforeCursor);

    /**
     * Sets the cursor to select the child. The mark is placed before,
     * the position behind it.
     */
    virtual void selectChild(FormulaCursor* cursor, BasicElement* child);

    /**
     * Moves the cursor away from the given child. The cursor is
     * guaranteed to be inside this element.
     */
    virtual void childWillVanish(FormulaCursor* cursor, BasicElement* child);

    /**
     * @returns the number of children we have.
     */
    int countChildren() const { return m_sequenceChildren.count(); }

    /**
     * @returns whether the child has the given number.
     */
    bool isChildNumber( uint pos, BasicElement* child )
        { return m_sequenceChildren.at( pos ) == child; }

    /**
     * Selects all children. The cursor is put behind, the mark before them.
     */
    void selectAllChildren(FormulaCursor* cursor);

    bool onlyTextSelected( FormulaCursor* cursor );

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

    /**
     * Sets the childrens' positions after their size has been
     * calculated.
     *
     * @see #calcSizes
     */
    virtual void setChildrenPositions();

    /**
     * @returns the position where the child starts.
     *
     * @param context the context the child is in
     * @param child the child's number
     */
    luPixel getChildPosition( const ContextStyle& context, int child );

    /**
     * @returns whether the child is the first element of its token.
     */
    virtual bool isFirstOfToken( BasicElement* child );

private:
    /// The sorted list of all elements in this sequence
    QList<BasicElement*> m_sequenceChildren;


    /**
     * true if the sequence contains only text
     */
    bool textSequence;

    bool singlePipe; //The key '|' produces one '|' not '| |', '||' produces '| |'
};

} // namespace KFormula

#endif // SEQUENCEELEMENT_H
