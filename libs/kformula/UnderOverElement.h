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

#ifndef UNDEROVERELEMENT_H
#define UNDEROVERELEMENT_H

#include "BasicElement.h"

namespace KFormula {

class UnderOverElement : public BasicElement
{
public:
    /// The standard constructor
    UnderOverElement( BasicElement* parent = 0 );

    /// The standard destructor
    virtual ~UnderOverElement();

    /**
     * Obtain a list of all child elements of this element
     * @return a QList with pointers to all child elements
     */
    virtual const QList<BasicElement*>& childElements() = 0;

    /// Saves the element to MathML
    virtual void writeMathML( QDomDocument& doc, QDomNode& parent, bool oasisFormat = false );

protected:
    /// Draws the element internally, means it paints into @ref m_elementPath
    virtual void drawInternal() = 0;
    
private:
    BasicElement* m_baseElement;

    BasicElement* m_underElement;

    BasicElement* m_overElement;
};

} // namespace KFormula

#endif // UNDEROVERELEMENT_H
