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

#include "EncloseElement.h"

namespace KFormula {

EncloseElement::EncloseElement( BasicElement* parent ) : SequenceElement( parent ) {
}

void EncloseElement::readMathMLAttributes(const QDomElement& element)
{
    m_notation = element.attribute( "notation" );
}

void EncloseElement::writeMathMLAttributes( QDomElement& element ) const
{
    if ( ! m_notation.isNull() ) {
        element.setAttribute( "notation", m_notation );
    }
}

} // namespace KFormula
