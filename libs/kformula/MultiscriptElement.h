/* This file is part of the KDE project
   Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net>

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

#ifndef MULTISCRIPTELEMENT_H
#define MULTISCRIPTELEMENT_H

#include "BasicElement.h"

namespace KFormula {

/**
 * @short 
 */
class MultiscriptElement : public BasicElement {
public:
    /// The standard constructor
    MultiscriptElement( BasicElement* parent = 0 );

    /// The destructor
    ~MultiscriptElement();

    /**
     * Obtain a list of all child elements of this element
     * @return a QList with pointers to all child elements
     */
    const QList<BasicElement*> childElements();

    void calcSizes(const ContextStyle& context, ContextStyle::TextStyle tstyle, ContextStyle::IndexStyle istyle);
    virtual void draw( QPainter& painter, const LuPixelRect& r,
                       const ContextStyle& context,
                       ContextStyle::TextStyle tstyle,
                       ContextStyle::IndexStyle istyle,
                       const LuPixelPoint& parentOrigin );

    void readMathML( const QDomElement& element );
    
    void writeMathML( KoXmlWriter* writer, bool oasisFormat = false );
    
private:
    BasicElement* m_baseElement;
    BasicElement* m_preSubscript;
    BasicElement* m_preSuperscript;
    BasicElement* m_postSubscript;
    BasicElement* m_postSuperscript;
};

} // namespace KFormula

#endif // MULTISCRIPTELEMENT_H
