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

#ifndef SEQUENCEELEMENT_H
#define SEQUENCEELEMENT_H

#include <QList>
#include <QString>
#include <QKeyEvent>

#include "basicelement.h"

class QKeyEvent;

KFORMULA_NAMESPACE_BEGIN

class SymbolTable;
class ElementCreationStrategy;

/**
 * The element that contains a number of children.
 * The children are aligned in one line.
 */
class SequenceElement : public BasicElement {
    SequenceElement& operator=( const SequenceElement& ) { return *this; }
public:

    SequenceElement(BasicElement* parent = 0);
    ~SequenceElement();

    SequenceElement( const SequenceElement& );

    virtual SequenceElement* clone() {
        return new SequenceElement( *this );
    }

    virtual bool accept( ElementVisitor* visitor );

    /**
     * @returns whether its prohibited to change the sequence with this cursor.
     */
    virtual bool readOnly( const FormulaCursor* ) const;

    /**
     * @returns true if the sequence contains only text.
     */
    virtual bool isTextOnly() const { return textSequence; }

    /**
     * Sets the cursor and returns the element the point is in.
     * The handled flag shows whether the cursor has been set.
     * This is needed because only the innermost matching element
     * is allowed to set the cursor.
     */
    virtual BasicElement* goToPos( FormulaCursor*, bool& handled,
                                   const LuPixelPoint& point,
                                   const LuPixelPoint& parentOrigin );

    // drawing
    //
    // Drawing depends on a context which knows the required properties like
    // fonts, spaces and such.
    // It is essential to calculate elements size with the same context
    // before you draw.

    /**
     * Tells the sequence to have a smaller size than its parant.
     */
    void setSizeReduction(const ContextStyle& context);

    /**
     * @returns true if there is no visible element in the sequence.
     * Please note that there might be phantom elements.
     */
    bool isEmpty();

    /**
     * Calculates our width and height and
     * our children's parentPosition.
     */
    virtual void calcSizes(const ContextStyle& context,
                           ContextStyle::TextStyle tstyle,
                           ContextStyle::IndexStyle istyle);

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

    virtual void drawEmptyRect( QPainter& painter, const ContextStyle& context,
                                const LuPixelPoint& upperLeft );

    virtual void calcCursorSize( const ContextStyle& context,
                                 FormulaCursor* cursor, bool smallCursor );

    /**
     * If the cursor is inside a sequence it needs to be drawn.
     */
    virtual void drawCursor( QPainter& painter, const ContextStyle& context,
                             FormulaCursor* cursor, bool smallCursor,
                             bool activeCursor );

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
     * Moves to the beginning of this word or if we are there already
     * to the beginning of the previous.
     */
    virtual void moveWordLeft(FormulaCursor* cursor);

    /**
     * Moves to the end of this word or if we are there already
     * to the end of the next.
     */
    virtual void moveWordRight(FormulaCursor* cursor);

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
     * Moves the cursor to the first position in this sequence.
     * (That is before the first child.)
     */
    virtual void moveHome(FormulaCursor* cursor);

    /**
     * Moves the cursor to the last position in this sequence.
     * (That is behind the last child.)
     */
    virtual void moveEnd(FormulaCursor* cursor);

    /**
     * Sets the cursor inside this element to its start position.
     * For most elements that is the main child.
     */
    virtual void goInside(FormulaCursor* cursor);


    // children

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
    int countChildren() const { return children.count(); }

    /**
     * @returns whether the child has the given number.
     */
    bool isChildNumber( uint pos, BasicElement* child )
        { return children.at( pos ) == child; }

    /**
     * Selects all children. The cursor is put behind, the mark before them.
     */
    void selectAllChildren(FormulaCursor* cursor);

    bool onlyTextSelected( FormulaCursor* cursor );


    /**
     * This is called by the container to get a command depending on
     * the current cursor position (this is how the element gets chosen)
     * and the request.
     *
     * @returns the command that performs the requested action with
     * the containers active cursor.
     */
    virtual KCommand* buildCommand( Container*, Request* );


    /**
     * Parses the input. It's the container which does create
     * new elements because it owns the undo stack. But only the
     * sequence knows what chars are allowed.
     */
    virtual KCommand* input( Container* container, QChar ch );
    virtual KCommand* input( Container* container, QKeyEvent* event );

    /**
     * Parses the sequence and generates a new syntax tree.
     * Has to be called after each modification.
     */
    virtual void parse();

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

    /**
     * @returns the latex representation of the element and
     * of the element's children
     */
    virtual QString toLatex();

    virtual QString formulaString();

    virtual void writeMathML( QDomDocument& doc, QDomNode parent, bool oasisFormat = false );

    /**
     * @returns the child at position i.
     */
    BasicElement* getChild(uint i) { return children.at(i); }
    //const BasicElement* getChild(uint i) const { return children.at(i); }

    int childPos( BasicElement* child ) { return children.indexOf( child ); }
    int childPos( const BasicElement* child ) const;

    class ChildIterator {
    public:
        ChildIterator( SequenceElement* sequence, int pos=0 )
            : m_sequence( sequence ), m_pos( pos ) {}

        typedef BasicElement value_type;
        typedef BasicElement* pointer;
        typedef BasicElement& reference;

        // we simply expect the compared iterators to belong
        // to the same sequence.
        bool operator== ( const ChildIterator& it ) const
            { return /*m_sequence==it.m_sequence &&*/ m_pos==it.m_pos; }
        bool operator!= ( const ChildIterator& it ) const
            { return /*m_sequence!=it.m_sequence ||*/ m_pos!=it.m_pos; }

        const BasicElement& operator* () const
            { return *m_sequence->getChild( m_pos ); }
        BasicElement& operator* ()
            { return *m_sequence->getChild( m_pos ); }

        ChildIterator& operator++ ()
            { ++m_pos; return *this; }
        ChildIterator operator++ ( int )
            { ChildIterator it( *this ); ++m_pos; return it; }
        ChildIterator& operator-- ()
            { --m_pos; return *this; }
        ChildIterator operator-- ( int )
            { ChildIterator it( *this ); --m_pos; return it; }
        ChildIterator& operator+= ( int j )
            { m_pos+=j; return *this; }
        ChildIterator & operator-= ( int j )
            { m_pos-=j; return *this; }

    private:
        SequenceElement* m_sequence;
        int m_pos;
    };

    typedef ChildIterator iterator;

    iterator begin() { return ChildIterator( this, 0 ); }
    iterator end() { return ChildIterator( this, countChildren() ); }

    static void setCreationStrategy( ElementCreationStrategy* strategy );

protected:

    //Save/load support

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
     * Creates a new element with the given type.
     *
     * @param type the desired type of the element
     */
    virtual BasicElement* createElement(QString type);

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

    /**
     * Removes the children at pos and appends it to the list.
     */
    void removeChild(QList<BasicElement*>& removedChildren, int pos);


    /**
     * Our children. Be sure to notify the rootElement before
     * you remove any.
     */
    QList<BasicElement*> children;

    /**
     * the syntax tree of the sequence
     */
    ElementType* parseTree;

    /**
     * true if the sequence contains only text
     */
    bool textSequence;

    static ElementCreationStrategy* creationStrategy;
    
    bool singlePipe; //The key '|' produces one '|' not '| |', '||' produces '| |'
};


/**
 * The sequence thats a name. Actually the purpose
 * is to be able to insert any element by keyboard.
 */
class NameSequence : public SequenceElement {
    typedef SequenceElement inherited;
public:

    NameSequence( BasicElement* parent = 0 );

    virtual NameSequence* clone() {
        return new NameSequence( *this );
    }

    virtual bool accept( ElementVisitor* visitor );

    /**
     * @returns true if the sequence contains only text.
     */
    //virtual bool isTextOnly() const { return true; }

    /**
     * @returns the character that represents this element. Used for
     * parsing a sequence.
     * This is guaranteed to be QChar::null for all non-text elements.
     */
    virtual QChar getCharacter() const { return '\\'; }

    /**
     * @returns the type of this element. Used for
     * parsing a sequence.
     */
    virtual TokenType getTokenType() const { return NAME; }

    /**
     * We are our own main child. This causes interessting effects.
     */
    virtual SequenceElement* getMainChild() { return this; }

    virtual void calcCursorSize( const ContextStyle& context,
                                 FormulaCursor* cursor, bool smallCursor );

    /**
     * If the cursor is inside a sequence it needs to be drawn.
     */
    virtual void drawCursor( QPainter& painter, const ContextStyle& context,
                             FormulaCursor* cursor, bool smallCursor,
                             bool activeCursor );

    /**
     * Moves to the beginning of this word or if we are there already
     * to the beginning of the previous.
     */
    virtual void moveWordLeft(FormulaCursor* cursor);

    /**
     * Moves to the end of this word or if we are there already
     * to the end of the next.
     */
    virtual void moveWordRight(FormulaCursor* cursor);

    /**
     * This is called by the container to get a command depending on
     * the current cursor position (this is how the element gets chosen)
     * and the request.
     *
     * @returns the command that performs the requested action with
     * the containers active cursor.
     */
    virtual KCommand* buildCommand( Container*, Request* );


    /**
     * Parses the input. It's the container which does create
     * new elements because it owns the undo stack. But only the
     * sequence knows what chars are allowed.
     */
    virtual KCommand* input( Container* container, QChar ch );

    /**
     * Sets a new type. This is done during parsing.
     */
    virtual void setElementType( ElementType* t );

    /**
     * @returns the element this sequence is to be replaced with.
     */
    BasicElement* replaceElement( const SymbolTable& table );

    /**
     * Tests whether the selected elements can be inserted in a
     * name sequence.
     */
    static bool isValidSelection( FormulaCursor* cursor );

    virtual void writeMathML( QDomDocument& doc, QDomNode parent, bool oasisFormat = false );

protected:

    /**
     * Returns the tag name of this element type.
     */
    virtual QString getTagName() const { return "NAMESEQUENCE"; }

    /**
     * Creates a new element with the given type.
     *
     * @param type the desired type of the element
     */
    virtual BasicElement* createElement( QString type );

    /**
     * Parses the sequence and generates a new syntax tree.
     * Has to be called after each modification.
     */
    //virtual void parse();

    /**
     * @returns whether the child is the first element of its token.
     * This can never happen here. Our children reuse our own
     * element type.
     */
    virtual bool isFirstOfToken( BasicElement* ) { return false; }

private:

    KCommand* compactExpressionCmd( Container* container );

    QString buildName();
};

KFORMULA_NAMESPACE_END

#endif // SEQUENCEELEMENT_H
