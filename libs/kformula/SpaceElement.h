/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
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

#ifndef SPACEELEMENT_H
#define SPACEELEMENT_H

#include "BasicElement.h"

namespace KFormula {

/**
 * A element that represents a space.
 */
class SpaceElement : public BasicElement {
    enum LineBreakType {
        NoBreakType,
        AutoBreak,
        NewLineBreak,
        IndentingNewLineBreak,
        NoBreak,
        GoodBreak,
        BadBreak
    };
public:
    /// The standard constructor
    SpaceElement( BasicElement* parent = 0 );

    /**
     * Calculates our width and height and
     * our children's parentPosition.
     * DEPRECATED: use calculateSize()
     */
    virtual void calcSizes( const ContextStyle& style,
						    ContextStyle::TextStyle tstyle,
						    ContextStyle::IndexStyle istyle,
							StyleAttributes& style );

    /**
     * Draws the whole element including its children.
     * The `parentOrigin' is the point this element's parent starts.
     * We can use our parentPosition to get our own origin then.
     * DEPRECATED: Use paint()
     */
    virtual void draw( QPainter& painter, const LuPixelRect& r,
                       const ContextStyle& context,
                       ContextStyle::TextStyle tstyle,
                       ContextStyle::IndexStyle istyle,
					   StyleAttributes& style,
                       const LuPixelPoint& parentOrigin );

    /**
     * Render the element to the given QPainter
     * @param painter The QPainter to paint the element to
     */
    void paint( QPainter& painter ) const;
    
    /// Calculate the element's sizes and the size of its children
    void calculateSize();

    /// Read the element from MathML
    void readMathML( const QDomElement& element );

    /// Save the element to MathML 
    void writeMathML( KoXmlWriter* writer, bool oasisFormat = false );

protected:
    /**
     * @returns the tag name of this element type.
     */
    virtual QString getTagName() const { return "SPACE"; }

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
     * Reads our attributes from the MathML element.
     * Returns false if it failed.
     */
	virtual bool readAttributesFromMathMLDom(const QDomElement& element);

private:

    virtual QString getElementName() const { return "mspace"; }
    virtual void writeMathMLAttributes( QDomElement& element ) const ;

    /**
     * Whether this space behaves like a tab.
     */
    bool m_tab;

    // MathML Attributes, Section 3.2.7.2
    SizeType m_widthType;
    double m_width;
    SizeType m_heightType;
    double m_height;
    SizeType m_depthType;
    double m_depth;
    LineBreakType m_lineBreak;
};

} // namespace KFormula

#endif // SPACEELEMENT_H
