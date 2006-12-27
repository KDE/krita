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

#ifndef BRACKETELEMENT_H
#define BRACKETELEMENT_H

#include <QPoint>
#include <QSize>
#include <QList>

#include "BasicElement.h"

namespace KFormula {

class Artwork;
class SequenceElement;


/**
 * A left and/or right bracket around one child.
 */
class BracketElement : public BasicElement {
public:

    enum { contentPos };

    BracketElement( BasicElement* parent = 0 );
    ~BracketElement();


    /**
     * @returns the type of this element. Used for
     * parsing a sequence.
     */
    virtual TokenType getTokenType() const { return BRACKET; }

    /**
     * Obtain a list of all child elements of this element
     * @return a QList with pointers to all child elements
     */
    const QList<BasicElement*> childElements();
	

    /**
     * Calculates our width and height and
     * our children's parentPosition.
     */
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
                       const ContextStyle& style,
                       ContextStyle::TextStyle tstyle,
                       ContextStyle::IndexStyle istyle,
					   StyleAttributes& style,
                       const LuPixelPoint& parentOrigin );

protected:
    
    //Save/load support
    virtual void readMathMLContent( const KoXmlElement& element );
    
    virtual void writeMathMLContent( KoXmlWriter* writer, bool oasisFormat = false );

    virtual QString getElementName() const { return "mfenced"; }

    /**
     * Returns the tag name of this element type.
     */
    virtual QString getTagName() const { return "BRACKET"; }

    /**
     * Reads our attributes from the element.
     * Returns false if it failed.
     */
    virtual bool readAttributesFromDom(QDomElement element);

    virtual void writeDom(QDomElement element);

private:

    /**
     * @return a LaTex string for the given symbol
     */
     QString latexString(char);

    /**
     * Set left and right types in operator fences
     * @param open if true set SymbolType for open (left) bracket, 
     * otherwise set for close (right) bracket.
     */
    bool operatorType( QDomNode& node, bool open );

    /**
     * Search through the nodes to find the close operator to match current
     * open bracket.
     */
    int searchOperator( const QDomNode& node );

    /**
     * The brackets we are showing.
     */
    Artwork* left;
    Artwork* right;

    SymbolType leftType;
    SymbolType rightType;

    QString m_separators;
    bool m_operator;
    bool m_customLeft;
    bool m_customRight;
};

} // namespace KFormula

#endif // BRACKETELEMENT_H
