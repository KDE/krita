/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef INDEXELEMENT_H
#define INDEXELEMENT_H

// Formula include
#include "basicelement.h"
#include <QList>

KFORMULA_NAMESPACE_BEGIN
class SequenceElement;


/**
 * The element with up to four indexes in the four corners.
 */
class IndexElement : public BasicElement {
    IndexElement& operator=( const IndexElement& ) { return *this; }
public:

    IndexElement(BasicElement* parent = 0);
    ~IndexElement();

    IndexElement( const IndexElement& );

    virtual IndexElement* clone() {
        return new IndexElement( *this );
    }

    virtual bool accept( ElementVisitor* visitor );

    /**
     * @returns the character that represents this element. Used for
     * parsing a sequence.
     * This is guaranteed to be QChar::null for all non-text elements.
     */
    virtual QChar getCharacter() const;

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
    virtual BasicElement* goToPos( FormulaCursor*, bool& handled,
                                   const LuPixelPoint& point, const LuPixelPoint& parentOrigin );

    // drawing
    //
    // Drawing depends on a context which knows the required properties like
    // fonts, spaces and such.
    // It is essential to calculate elements size with the same context
    // before you draw.

    /**
     * Calculates our width and height and
     * our children's parentPosition.
     */
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
    virtual void dispatchFontCommand( FontCommand* cmd );

    // navigation
    //
    // The elements are responsible to handle cursor movement themselves.
    // To do this they need to know the direction the cursor moves and
    // the element it comes from.
    //
    // The cursor might be in normal or in selection mode.

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

    // children

    /**
     * Removes the child. If this was the main child this element might
     * request its own removal.
     * The cursor is the one that caused the removal. It has to be moved
     * to the place any user expects the cursor after that particular
     * element has been removed.
     */
    //virtual void removeChild(FormulaCursor* cursor, BasicElement* child);


    // main child
    //
    // If an element has children one has to become the main one.

    virtual SequenceElement* getMainChild() { return content; }
    //SequenceElement* upperLeft;
    //SequenceElement* upperMiddle;
    SequenceElement* getExponent() { return upperRight; }
    //SequenceElement* lowerLeft;
    //SequenceElement* lowerMiddle;
    //SequenceElement* lowerRight;


    /**
     * Inserts all new children at the cursor position. Places the
     * cursor according to the direction.
     *
     * You only can insert one index at a time. So the list must contain
     * exactly on SequenceElement. And the index you want to insert
     * must not exist already.
     *
     * The list will be emptied but stays the property of the caller.
     */
    virtual void insert(FormulaCursor*, QList<BasicElement*>&, Direction);

    /**
     * Removes all selected children and returns them. Places the
     * cursor to where the children have been.
     *
     * The cursor has to be inside one of our indexes which is supposed
     * to be empty. The index will be removed and the cursor will
     * be placed to the removed index so it can be inserted again.
     * This methode is called by SequenceElement::remove only.
     *
     * The ownership of the list is passed to the caller.
     */
    virtual void remove(FormulaCursor*, QList<BasicElement*>&, Direction);

    /**
     * Moves the cursor to a normal place where new elements
     * might be inserted.
     */
    virtual void normalize(FormulaCursor*, Direction);

    /**
     * Returns the child at the cursor.
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
    //virtual void childWillVanish(FormulaCursor* cursor, BasicElement* child) = 0;

    /**
     * Returns wether the element has no more useful
     * children (except its main child) and should therefore
     * be replaced by its main child's content.
     */
    virtual bool isSenseless();


    bool hasUpperLeft()   const { return upperLeft   != 0; }
    bool hasUpperMiddle() const { return upperMiddle != 0; }
    bool hasUpperRight()  const { return upperRight  != 0; }
    bool hasLowerLeft()   const { return lowerLeft   != 0; }
    bool hasLowerMiddle() const { return lowerMiddle != 0; }
    bool hasLowerRight()  const { return lowerRight  != 0; }

    // If we want to create an index we need a cursor that points there.

    void setToUpperLeft(FormulaCursor* cursor);
    void setToUpperMiddle(FormulaCursor* cursor);
    void setToUpperRight(FormulaCursor* cursor);
    void setToLowerLeft(FormulaCursor* cursor);
    void setToLowerMiddle(FormulaCursor* cursor);
    void setToLowerRight(FormulaCursor* cursor);

    // If the index is there we need a way to move into it.

    void moveToUpperLeft(FormulaCursor* cursor, Direction direction);
    void moveToUpperMiddle(FormulaCursor* cursor, Direction direction);
    void moveToUpperRight(FormulaCursor* cursor, Direction direction);
    void moveToLowerLeft(FormulaCursor* cursor, Direction direction);
    void moveToLowerMiddle(FormulaCursor* cursor, Direction direction);
    void moveToLowerRight(FormulaCursor* cursor, Direction direction);

    // Generic access to each index.

    ElementIndexPtr getUpperLeft() { return ElementIndexPtr( new UpperLeftIndex( this ) ); }
    ElementIndexPtr getLowerLeft() { return ElementIndexPtr( new LowerLeftIndex( this ) ); }
    ElementIndexPtr getUpperMiddle() { return ElementIndexPtr( new UpperMiddleIndex( this ) ); }
    ElementIndexPtr getLowerMiddle() { return ElementIndexPtr( new LowerMiddleIndex( this ) ); }
    ElementIndexPtr getUpperRight() { return ElementIndexPtr( new UpperRightIndex( this ) ); }
    ElementIndexPtr getLowerRight() { return ElementIndexPtr( new LowerRightIndex( this ) ); }

    /**
     * Returns the index at the position. Defaults to upperRight.
     */
    ElementIndexPtr getIndex( int position );

    /**
     * @returns the latex representation of the element and
     * of the element's children
     */
    virtual QString toLatex();

    // the upper right index is the only one we show
    virtual QString formulaString();

    virtual void writeMathML( QDomDocument& doc, QDomNode& parent, bool oasisFormat = false );

protected:

    //Save/load support

    /**
     * Returns the tag name of this element type.
     */
    virtual QString getTagName() const { return "INDEX"; }

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

    /**
     * An index that belongs to us.
     */
    class IndexElementIndex : public ElementIndex {
    public:
        IndexElementIndex(IndexElement* p) : parent(p) {}
        virtual IndexElement* getElement() { return parent; }
    protected:
        IndexElement* parent;
    };

    // We have a (very simple) type for every index.

    class UpperLeftIndex : public IndexElementIndex {
    public:
        UpperLeftIndex(IndexElement* parent) : IndexElementIndex(parent) {}
        virtual void moveToIndex(FormulaCursor* cursor, Direction direction)
            { parent->moveToUpperLeft(cursor, direction); }
        virtual void setToIndex(FormulaCursor* cursor)
            { parent->setToUpperLeft(cursor); }
        virtual bool hasIndex() const
            { return parent->hasUpperLeft(); }
    };

    class LowerLeftIndex : public IndexElementIndex {
    public:
        LowerLeftIndex(IndexElement* parent) : IndexElementIndex(parent) {}
        virtual void moveToIndex(FormulaCursor* cursor, Direction direction)
            { parent->moveToLowerLeft(cursor, direction); }
        virtual void setToIndex(FormulaCursor* cursor)
            { parent->setToLowerLeft(cursor); }
        virtual bool hasIndex() const
            { return parent->hasLowerLeft(); }
    };

    class UpperMiddleIndex : public IndexElementIndex {
    public:
        UpperMiddleIndex(IndexElement* parent) : IndexElementIndex(parent) {}
        virtual void moveToIndex(FormulaCursor* cursor, Direction direction)
            { parent->moveToUpperMiddle(cursor, direction); }
        virtual void setToIndex(FormulaCursor* cursor)
            { parent->setToUpperMiddle(cursor); }
        virtual bool hasIndex() const
            { return parent->hasUpperMiddle(); }
    };

    class LowerMiddleIndex : public IndexElementIndex {
    public:
        LowerMiddleIndex(IndexElement* parent) : IndexElementIndex(parent) {}
        virtual void moveToIndex(FormulaCursor* cursor, Direction direction)
            { parent->moveToLowerMiddle(cursor, direction); }
        virtual void setToIndex(FormulaCursor* cursor)
            { parent->setToLowerMiddle(cursor); }
        virtual bool hasIndex() const
            { return parent->hasLowerMiddle(); }
    };

    class UpperRightIndex : public IndexElementIndex {
    public:
        UpperRightIndex(IndexElement* parent) : IndexElementIndex(parent) {}
        virtual void moveToIndex(FormulaCursor* cursor, Direction direction)
            { parent->moveToUpperRight(cursor, direction); }
        virtual void setToIndex(FormulaCursor* cursor)
            { parent->setToUpperRight(cursor); }
        virtual bool hasIndex() const
            { return parent->hasUpperRight(); }
    };

    class LowerRightIndex : public IndexElementIndex {
    public:
        LowerRightIndex(IndexElement* parent) : IndexElementIndex(parent) {}
        virtual void moveToIndex(FormulaCursor* cursor, Direction direction)
            { parent->moveToLowerRight(cursor, direction); }
        virtual void setToIndex(FormulaCursor* cursor)
            { parent->setToLowerRight(cursor); }
        virtual bool hasIndex() const
            { return parent->hasLowerRight(); }
    };


    /**
     * Sets the x value of the three middle elements. (Two indexes and the content.)
     */
    void setMiddleX(int xOffset, int middleWidth);

    /**
     * @returns the position describtion to the provided element.
     */
    int getFromPos(BasicElement* from);

    /**
     * Sets the cursor to point to the place where the content is.
     * There always is a content so this is not a useful place.
     * No insertion or removal will succeed as long as the cursor is
     * there.
     */
    void setToContent(FormulaCursor* cursor);

    /**
     * Our main child. This is guaranteed not to be null.
     */
    SequenceElement* content;

    /**
     * The six indexes. Each one might be null.
     * If the last one is removed the whole IndexElement
     * should be replaced by its main child.
     */
    SequenceElement* upperLeft;
    SequenceElement* upperMiddle;
    SequenceElement* upperRight;
    SequenceElement* lowerLeft;
    SequenceElement* lowerMiddle;
    SequenceElement* lowerRight;
};

KFORMULA_NAMESPACE_END

#endif // INDEXELEMENT_H
