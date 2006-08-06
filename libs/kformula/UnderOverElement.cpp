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

#include "UnderOverElement.h"

namespace KFormula {

UnderOverElement::UnderOverElement( BasicElement* parent ) : BasicElement( parent )
{
}

const QList<BasicElement*>& UnderOverElement::childElements()
{
    return SequenceElement::childElements();
}

void UnderOverElement::drawInternal()
{
    // A UnderOverElement has no own representation if it has children,
    // if not it paints a rectangle, this behavior is the same as SequenceElement
    SequenceElement::drawInternal();
}

void UnderOverElement::writeMathML( QDomDocument& doc,
                                            QDomNode& parent, bool oasisFormat )
{

}

void UnderOverElement::calcSizes( const ContextStyle& context, ContextStyle::TextStyle tstyle,
                                          ContextStyle::IndexStyle istyle )
{
    tabs.clear();
    SequenceElement::calcSizes( context, tstyle, istyle );
}

} // namespace KFormula
