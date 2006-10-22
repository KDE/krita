/* This file is part of the KDE project
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

#ifndef UNDEROVERELEMENT_H
#define UNDEROVERELEMENT_H

#include "BasicElement.h"

namespace KFormula {

/**
 * @short 
 *
 * 
 * @author Martin Pfeiffer <hubipete@gmx.net>
 * @since 2.0
 */
class UnderOverElement : public BasicElement {
public:
    /// The standard constructor
    UnderOverElement( BasicElement* parent = 0 );

    /// The standard destructor
    virtual ~UnderOverElement();

    /**
     * Obtain a list of all child elements of this element
     * @return a QList with pointers to all child elements
     */
    virtual const QList<BasicElement*> childElements();

    void insertInBaseElement( int index, BasicElement* element );
    void insertInUnderElement( int index, BasicElement* element );
    void insertInOverElement( int index, BasicElement* element );

    void readMathML( const QDomElement& element );
    
    /// Saves the element to MathML
    void writeMathML( KoXmlWriter* writer, bool oasisFormat = false );

    virtual void calcSizes( const ContextStyle& context,
                            ContextStyle::TextStyle tstyle,
                            ContextStyle::IndexStyle istyle,
                            StyleAttributes& style );

    virtual void draw( QPainter& painter, const LuPixelRect& r,
                       const ContextStyle& context,
                       ContextStyle::TextStyle tstyle,
                       ContextStyle::IndexStyle istyle,
                       StyleAttributes& style,
                       const LuPixelPoint& parentOrigin );
    
private:
    virtual QString getElementName() const ;
    virtual void writeMathMLAttributes( QDomElement& element ) const ;
    virtual void writeMathMLContent( QDomDocument& doc, 
                                     QDomElement& element,
                                     bool oasisFormat ) const ;

    virtual bool readAttributesFromMathMLDom( const QDomElement& element );
    /**
     * Reads our content from the MathML node. Sets the node to the next node
     * that needs to be read. It is sometimes needed to read more than one node
     * (e. g. for fence operators).
     * Returns the number of nodes processed or -1 if it failed.
     */
    virtual int readContentFromMathMLDom( QDomNode& node );

    BasicElement* m_baseElement;

    BasicElement* m_underElement;

    BasicElement* m_overElement;
};

} // namespace KFormula

#endif // UNDEROVERELEMENT_H
