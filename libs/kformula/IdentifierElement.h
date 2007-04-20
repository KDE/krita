/* This file is part of the KDE project
   Copyright (C) 2006-2007 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>

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

#ifndef IDENTIFIERELEMENT_H
#define IDENTIFIERELEMENT_H

#include "TokenElement.h"

namespace FormulaShape {

class IdentifierElement : public TokenElement {
    typedef TokenElement inherited;
public:
    IdentifierElement( BasicElement* parent = 0 );

    /**
     * This is called by the container to get a command depending on
     * the current cursor position (this is how the element gets chosen)
     * and the request.
     *
     * @returns the command that performs the requested action with
     * the containers active cursor.
     */
//    virtual KCommand* buildCommand( Container*, Request* );

    virtual QString elementName() const { return "mi"; }

protected:

    /**
     * Space around sequence
     */
    virtual luPt getSpaceBefore( const ContextStyle& context, 
                                 ContextStyle::TextStyle tstyle,
                                 double factor ) { return 0; }
    virtual luPt getSpaceAfter( const ContextStyle& context, 
                                 ContextStyle::TextStyle tstyle,
                                double factor ) { return 0; }
};

} // namespace FormulaShape

#endif // IDENTIFIERELEMENT_H
