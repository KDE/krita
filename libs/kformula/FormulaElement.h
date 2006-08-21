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

#ifndef FORMULAELEMENT_H
#define FORMULAELEMENT_H

#include "SequenceElement.h"
#include <QKeyEvent>

namespace KFormula {

class BasicElement;
class ContextStyle;
class SymbolTable;


/**
 * @short The element of a formula at the highest position.
 *
 * A formula consists of a tree of elements. The FormulaElement is the root of this
 * tree and therefore is the only element that doesn't have a parent element.
 * It's functionality is reduced to layouting its children in a different way. It is
 * the element with highest size and can also dictate the size to all other elements. 
 */
class FormulaElement : public BasicElement {
public:
    /// The standard constructor
    FormulaElement();

    /// The standard destructor
    ~FormulaElement();
    
    /**
     * Obtain a list of all child elements of this element
     * @return a QList with pointers to all child elements
     */
    const QList<BasicElement*>& childElements();

    void readMathML( const QDomElement& element );
    
    void writeMathML( const KoXmlWriter* writer, bool oasisFormat = false );






    void addChild( BasicElement* element );
    void deleteChild( BasicElement* element );
    // void insertChild( int position, BasicElement* element );

    /**
     * Ordinary formulas are not write protected.
     */
    virtual bool readOnly( const BasicElement* /*child*/ ) const { return false; }

    /**
     * @returns whether its prohibited to change the sequence with this cursor.
     */
    virtual bool readOnly( const FormulaCursor* ) const { return false; }

    /**
     * Provide fast access to the rootElement for each child.
     */
    virtual FormulaElement* formula() { return this; }

    /**
     * Provide fast access to the rootElement for each child.
     */
    virtual const FormulaElement* formula() const { return this; }

    /**
     * Gets called just before the child is removed from
     * the element tree.
     */
    void elementRemoval(BasicElement* child);

    /**
     * Gets called whenever something changes and we need to
     * recalc.
     */
    virtual void changed();

    /**
     * Gets called when a request has the side effect of moving the
     * cursor. In the end any operation that moves the cursor should
     * call this.
     */
    void cursorHasMoved( FormulaCursor* );

    void moveOutLeft( FormulaCursor* );
    void moveOutRight( FormulaCursor* );
    void moveOutBelow( FormulaCursor* );
    void moveOutAbove( FormulaCursor* );

    /**
     * Tell the user something has happened.
     */
//    void tell( const QString& msg );

    /**
     * Gets called when the formula wants to vanish. The one who
     * holds it should create an appropriate command and execute it.
     */
    void removeFormula( FormulaCursor* );

    void insertFormula( FormulaCursor* );

    /**
     * Calculates our width and height and
     * our children's parentPosition.
     */
    virtual void calcSizes( const ContextStyle& context,
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
     * Calculates the formulas sizes and positions.
     */
    void calcSizes( ContextStyle& context );

    /**
     * Draws the whole thing.
     */
    void draw( QPainter& painter, const LuPixelRect& r, ContextStyle& context );

    /**
     * This is called by the container to get a command depending on
     * the current cursor position (this is how the element gets chosen)
     * and the request.
     *
     * @returns the command that performs the requested action with
     * the containers active cursor.
     */
//    virtual KCommand* buildCommand( Container*, Request* );

    /**
     * @returns our documents symbol table
     */
    const SymbolTable& getSymbolTable() const;

    int getBaseSize() const { return baseSize; }
    void setBaseSize( int size );

    bool hasOwnBaseSize() const { return ownBaseSize; }

//    virtual KCommand* input( Container* container, QKeyEvent* event );

 
    /**
     * Appends our attributes to the dom element.
     */
    virtual void writeDom(QDomElement element);

    /**
     * For copy&paste we need to create an empty XML element.
     */
    QDomElement emptyFormulaElement( QDomDocument& doc );

protected:
    void drawInternal();

    void readMathMLAttributes( const QDomElement& element );

    
    //Save/load support

    /**
     * Returns the tag name of this element type.
     */
    virtual QString getTagName() const { return "FORMULA"; }

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
    /// The child elements of the FormulaElement 
    QList<BasicElement*> m_childElements;

    /**
     * The introduction of 'NameSequence' changed the DOM.
     * However, we need to read the old version.
     */
    void convertNames( QDomNode node );

    /**
     * The base font size.
     */
    int baseSize;

    /**
     * Whether we want to use the default base size.
     */
    bool ownBaseSize;
};

} // namespace KFormula

#endif // FORMULAELEMENT_H
