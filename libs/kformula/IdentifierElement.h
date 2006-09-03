/* This file is part of the KDE project
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

#ifndef IDENTIFIERELEMENT_H
#define IDENTIFIERELEMENT_H

#include "tokenelement.h"

namespace KFormula {

class IdentifierElement : public TokenElement {
    typedef TokenElement inherited;
public:
    IdentifierElement( BasicElement* parent = 0 );

protected:

    virtual void setStyleVariant( StyleAttributes& style );

private:
    virtual QString getElementName() const { return "mi"; }
    
};

} // namespace KFormula

#endif // IDENTIFIERELEMENT_H
