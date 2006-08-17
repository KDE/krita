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

#include "MultiscriptElement.h"

namespace KFormula {

MultiscriptElement::MultiscriptElement( BasicElement* parent ) : BasicElement( parent )
{
}

MultiscriptElement::~MultiscriptElement()
{}

const QList<BasicElement*>& MultiscriptElement::childElements()
{
    return QList<BasicElement*>();
}

void MultiscriptElement::drawInternal()
{
}

void MultiscriptElement::writeMathML( QDomDocument& doc,
                                            QDomNode& parent, bool oasisFormat )
{

}

void MultiscriptElement::calcSizes( const ContextStyle& context, ContextStyle::TextStyle tstyle,
                                          ContextStyle::IndexStyle istyle )
{
}

void MultiscriptElement::draw( QPainter& painter, const LuPixelRect& r,
                       const ContextStyle& context,
                       ContextStyle::TextStyle tstyle,
                       ContextStyle::IndexStyle istyle,
                       const LuPixelPoint& parentOrigin )
{}


} // namespace KFormula
