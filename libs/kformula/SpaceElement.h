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

#ifndef SPACEELEMENT_H
#define SPACEELEMENT_H

#include "BasicElement.h"

namespace KFormula {

/**
 * A element that represents a space.
 */
class SpaceElement : public BasicElement {
public:
    /// The standard constructor
    SpaceElement( BasicElement* parent = 0 );

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
};

} // namespace KFormula

#endif // SPACEELEMENT_H
