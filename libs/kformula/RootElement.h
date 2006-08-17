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

#ifndef ROOTELEMENT_H
#define ROOTELEMENT_H

#include "BasicElement.h"



#include <QPoint>
#include <QList>


namespace KFormula {
class SequenceElement;


/**
 * A nice graphical root.
 */
class RootElement : public BasicElement {
public:
    /// The standard constructor
    RootElement( BasicElement* parent = 0 );

    ~RootElement();

    void insertInExponent( int index, BasicElement* element );

    void insertInRadicand( int index, BasicElement* element );
     
    /**
     * Obtain a list of all child elements of this element
     * @return a QList with pointers to all child elements
     */
    const QList<BasicElement*>& childElements();

    void readMathML( const QDomElement& element );
    
    void writeMathML( const KoXmlWriter* writer, bool oasisFormat = false );


    
    /**
     * Calculates our width and height and
     * our children's parentPosition.
     */
    virtual void calcSizes(const ContextStyle& style, ContextStyle::TextStyle tstyle, ContextStyle::IndexStyle istyle);

    /**
     * Draws the whole element including its children.
     * The `parentOrigin' is the point this element's parent starts.
     * We can use our parentPosition to get our own origin then.
     */
    virtual void draw( QPainter& painter, const LuPixelRect& r,
                       const ContextStyle& style,
                       ContextStyle::TextStyle tstyle,
                       ContextStyle::IndexStyle istyle,
                       const LuPixelPoint& parentOrigin );

    /**
     * Dispatch this FontCommand to all our TextElement children.
     */
 //   virtual void dispatchFontCommand( FontCommand* cmd );

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
    virtual void moveUp(FormulaCursor* cursor, BasicElement* from);

    /**
     * Enters this element while moving down starting inside
     * the element `from'. Searches for a cursor position inside
     * this element or below it.
     */
    virtual void moveDown(FormulaCursor* cursor, BasicElement* from);

    /**
     * Reinserts the index if it has been removed.
     */
    virtual void insert(FormulaCursor*, QList<BasicElement*>&, Direction);

    /**
     * Removes all selected children and returns them. Places the
     * cursor to where the children have been.
     *
     * We remove ourselve if we are requested to remove our content.
     */
    virtual void remove(FormulaCursor*, QList<BasicElement*>&, Direction);

    /**
     * Moves the cursor to a normal place where new elements
     * might be inserted.
     */
//    virtual void normalize(FormulaCursor*, Direction);

    // main child
    //
    // If an element has children one has to become the main one.

    virtual SequenceElement* getMainChild();

    /**
     * Sets the cursor to select the child. The mark is placed before,
     * the position behind it.
     */
    virtual void selectChild(FormulaCursor* cursor, BasicElement* child);

    // Moves the cursor inside the index. The index has to exist.
    void moveToIndex(FormulaCursor*, Direction);

    // Sets the cursor to point to the place where the index normaly
    // is. These functions are only used if there is no such index and
    // we want to insert them.
    void setToIndex(FormulaCursor*);

protected:
    void drawInternal();

    void readMathMLAttributes( const QDomElement& element );


    
    //Save/load support

    /**
     * Returns the tag name of this element type.
     */
    virtual QString getTagName() const { return "ROOT"; }

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
/*
    class RootElementIndex : public ElementIndex {
    public:
        RootElementIndex(RootElement* p) : parent(p) {}
        virtual void moveToIndex(FormulaCursor* cursor, Direction direction)
            { parent->moveToIndex(cursor, direction); }
        virtual void setToIndex(FormulaCursor* cursor)
            { parent->setToIndex(cursor); }
        virtual bool hasIndex() const
            { return parent->hasIndex(); }
        virtual RootElement* getElement() { return parent; }
    protected:
        RootElement* parent;
    };*/


    /// The element that is the radicand of the root
    SequenceElement* m_radicand;

    /// The element that is the exponent of the root
    SequenceElement* m_exponent;

    /// The point the artwork relates to.
    LuPixelPoint rootOffset;
};

} // namespace KFormula

#endif // ROOTELEMENT_H
