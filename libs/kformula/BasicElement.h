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

#ifndef BASICELEMENT_H
#define BASICELEMENT_H

#include <QPainterPath>



#include <QString>
#include <QKeyEvent>
#include <QList>
#include <QDomElement>
#include <QDomDocument>

#include "contextstyle.h"
#include "kformuladefs.h"

class QKeyEvent;
class KCommand;

namespace KFormula {

class ComplexElement;
class Container;
class ElementType;
class ElementVisitor;
class FontCommand;
class FormulaCursor;
class FormulaElement;
class SequenceElement;


/**
 * @short The base class for all elements of a formula
 *
 * The BasicElement class is constructed with a parent and normally an element in a
 * formula has a parent. The only exception is the @see FormulaElement which is the
 * root of the element tree and has no parent element.
 * Most of the elements have children but the number of it can be fixed or variable
 * and the type of child element is not certain. So with the childElements() method you
 * can obtain a list of all direct children of an element. Note that the returned list
 * can be empty when the element is eg a token. This is also the reason why each class
 * inheriting BasicElement has to implement the childElements() method on its own.
 * With the childElementAt method you can test if the given point is in the element.
 * This method is generically implemented for all element types only once in
 * BasicElement.
 * The BasicElement knows its size and position in the formula. This data is normally
 * only used for drawing and stored in the m_boundingRect attribute.
 * To adapt both variables, size and coordinates, to fit in the formula each and every
 * BasicElement derived class has to implement layoutElement() and calculateSize()
 * methods. The former adaptes the position, means the coordinates, when the formula
 * changes and the latter calculates the size of the element. After a formula change
 * first calculateSize is called for all elements then layoutElement().
 */
class BasicElement {
public:
    /*
     * The standard constructor
     * @param parent pointer to the BasicElement's parent
     */
    BasicElement( BasicElement* parent = 0 );

    /// The standard destructor
    virtual ~BasicElement();

    /**
     * Get the element of the formula at the given point
     * @param p the point to look for 
     * @return a pointer to a BasicElement
     */
    BasicElement* childElementAt( const QPointF& p );

    /**
     * Obtain a list of all child elements of this element
     * @return a QList with pointers to all child elements
     */
    virtual const QList<BasicElement*>& childElements() = 0;

    /// @return The element's painter path used for painting it
    const QPainterPath& elementPath() const;

    /// @returns True if the element is invisible, a phantomelement
    virtual bool isInvisible() const;




//  virtual BasicElement* clone() = 0;    
    
    /**
     * @returns whether the child should be read-only. The idea is
     * that a read-only parent has read-only children.
     */
    virtual bool readOnly( const BasicElement* child ) const;

    /**
     * Provide fast access to the rootElement for each child.
     */
    virtual FormulaElement* formula();

    /**
     * Provide fast access to the rootElement for each child.
     */
    virtual const FormulaElement* formula() const { return m_parentElement->formula(); }

    /**
     * @returns the character that represents this element. Used for
     * parsing a sequence.
     * This is guaranteed to be QChar::null for all non-text elements.
     */
    virtual QChar getCharacter() const { return QChar::Null; }

    /**
     * @returns the type of this element. Used for
     * parsing a sequence.
     */
    virtual TokenType getTokenType() const { return ELEMENT; }


    /**
     * Returns our position inside the widget.
     */
    LuPixelPoint widgetPos();


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
    virtual void calcSizes(const ContextStyle& context, ContextStyle::TextStyle tstyle, ContextStyle::IndexStyle istyle) = 0;

    /**
     * Draws the whole element including its children.
     * The `parentOrigin' is the point this element's parent starts.
     * We can use our parentPosition to get our own origin then.
     */
    virtual void draw( QPainter& painter, const LuPixelRect& r,
                       const ContextStyle& context,
                       ContextStyle::TextStyle tstyle,
                       ContextStyle::IndexStyle istyle,
                       const LuPixelPoint& parentOrigin ) = 0;


    /**
     * Dispatch this FontCommand to all our TextElement children.
     */
    virtual void dispatchFontCommand( FontCommand* /*cmd*/ ) {}

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
    virtual void moveUp(FormulaCursor*, BasicElement*) {}

    /**
     * Enters this element while moving down starting inside
     * the element `from'. Searches for a cursor position inside
     * this element or below it.
     */
    virtual void moveDown(FormulaCursor*, BasicElement* ) {}

    /**
     * Moves the cursor to the first position in this sequence.
     * (That is before the first child.)
     */
    virtual void moveHome(FormulaCursor*) {}

    /**
     * Moves the cursor to the last position in this sequence.
     * (That is behind the last child.)
     */
    virtual void moveEnd(FormulaCursor*) {}

    /**
     * Sets the cursor inside this element to its start position.
     * For most elements that is the main child.
     */
    virtual void goInside(FormulaCursor* cursor);

    /**
     * The cursor has entered one of our child sequences.
     * This is a good point to tell the user where he is.
     */
//    virtual void entered( SequenceElement* /*child*/ );

    virtual SequenceElement* getMainChild() { return 0; }

    /**
     * Inserts all new children at the cursor position. Places the
     * cursor according to the direction.
     *
     * The list will be emptied but stays the property of the caller.
     */
    virtual void insert(FormulaCursor*, QList<BasicElement*>&, Direction) {}

    /**
     * Removes all selected children and returns them. Places the
     * cursor to where the children have been.
     */
    virtual void remove(FormulaCursor*, QList<BasicElement*>&, Direction) {}

    /**
     * Moves the cursor to a normal place where new elements
     * might be inserted.
     */
    virtual void normalize(FormulaCursor*, Direction);


    /**
     * Returns wether the element has no more useful
     * children (except its main child) and should therefore
     * be replaced by its main child's content.
     */
    virtual bool isSenseless() { return false; }

    /**
     * Returns the child at the cursor.
     */
    virtual BasicElement* getChild(FormulaCursor*, Direction = beforeCursor) { return 0; }


    /**
     * Sets the cursor to select the child. The mark is placed before,
     * the position behind it.
     */
    virtual void selectChild(FormulaCursor*, BasicElement*) {}


    /**
     * Moves the cursor away from the given child. The cursor is
     * guaranteed to be inside this element.
     */
    virtual void childWillVanish(FormulaCursor*, BasicElement*) {}


    /**
     * Callback for the tabs among our children. Needed for alignment.
     */
    virtual void registerTab( BasicElement* /*tab*/ ) {}


    /**
     * This is called by the container to get a command depending on
     * the current cursor position (this is how the element gets chosen)
     * and the request.
     *
     * @returns the command that performs the requested action with
     * the containers active cursor.
     */
    virtual KCommand* buildCommand( Container*, Request* ) { return 0; }

    /**
     * Parses the input. It's the container which does create
     * new elements because it owns the undo stack. But only the
     * sequence knows what chars are allowed.
     */
    virtual KCommand* input( Container*, QKeyEvent* ) { return 0; }

    // basic support

    const BasicElement* getParent() const { return m_parentElement; }
    BasicElement* getParent() { return m_parentElement; }
    void setParent(BasicElement* p) { m_parentElement = p; }

    double getX() const;
    double getY() const;

    void setX( double x );
    void setY( double y );
    double getWidth() const;
    double getHeight() const;
    const QRectF& boundingRect() const;

    void setWidth( double width );
    void setHeight( double height );
    
    luPixel getBaseline() const { return m_baseline; }
    void setBaseline( luPixel line ) { m_baseline = line; }

    luPixel axis( const ContextStyle& style, ContextStyle::TextStyle tstyle ) const {
        return getBaseline() - style.axisHeight( tstyle ); }

    /**
     * @return a QDomElement that contain as DomChildren the
     * children, and as attribute the attribute of this
     * element.
     */
    QDomElement getElementDom( QDomDocument& doc);

    /**
     * Same as above, just MathML.
     */
    virtual void writeMathML( QDomDocument& doc, QDomNode& parent, bool oasisFormat = false );

    /**
     * Set this element attribute, build children and
     * call their buildFromDom.
     */
    bool buildFromDom(QDomElement element);

    /**
     * Heiner's test method. Should read MathML...
     */
    //bool buildFromMathMLDom( QDomElement& element );

    /**
     * @returns our type. This is an object from our parent's syntax tree
     * or 0 if there was a very bad parsing error.
     */
    ElementType* getElementType() const { return elementType; }

    /**
     * Sets a new type. This is done during parsing.
     */
    virtual void setElementType(ElementType* t) { elementType = t; }

protected:
    /// Draws the element internally, means it paints into @ref m_elementPath
    virtual void drawInternal() = 0;
    


    /**
     * Returns the tag name of this element type.
     */
    virtual QString getTagName() const { return "BASIC"; }

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
     * Returns if the SequenceElement could be constructed from the nodes first child.
     * The node name must match the given name.
     *
     * This is a service for all subclasses that contain children.
     */
    bool buildChild( SequenceElement* child, QDomNode node, QString name );

private:
    /// The element's parent element - might not be null except of FormulaElement
    BasicElement* m_parentElement;
    
    /// The boundingRect storing the element's width, height, x and y
    QRectF m_boundingRect;

    /// The path that is used to paint the element
    QPainterPath m_elementPath;

    /// True if the element is a phantom element means is not visible
    bool m_phantomElement;   


    
    
    /**
     * The position of our base line from
     * the upper border. A sequence aligns its elements
     * along this line.
     *
     * There are elements (like matrix) that don't have a base line. It is
     * -1 in this case. The alignment is done using the middle line.
     */
    luPixel m_baseline;

    /**
     * The token that describes our type. Please note that we don't
     * own it.
     */
    ElementType* elementType;
};

} // namespace KFormula

#endif // BASICELEMENT_H
