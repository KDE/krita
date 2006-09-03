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
 * Boston, MA 02110-1301, USA.
*/

#include "kformuladefs.h"
#include "textelement.h"
#include "identifierelement.h"

KFORMULA_NAMESPACE_BEGIN

IdentifierElement::IdentifierElement( BasicElement* parent ) : TokenElement( parent ) {
}

void IdentifierElement::setStyleVariant( StyleAttributes& style )
{
    if ( customMathVariant() ) {
        style.setCustomMathVariant ( true );
        style.setCustomFontWeight( false );
        style.setCustomFontStyle( false );
        style.setCustomFont( false );
        if ( customMathVariant() ) {
            style.setCharFamily ( charFamily() );
            style.setCharStyle( charStyle() );
        }
        else {
            style.setCharFamily( style.charFamily() );
            style.setCharStyle( style.charStyle() );
        }
    }
    else {
        style.setCustomMathVariant( false );
        if ( customFontFamily() ) {
            style.setCustomFont( true );
            style.setFont( QFont(fontFamily()) );
        }

        bool fontweight = false;
        if ( customFontWeight() || style.customFontWeight() ) {
            style.setCustomFontWeight( true );
            if ( customFontWeight() ) {
                fontweight = fontWeight();
            }
            else {
                fontweight = style.customFontWeight();
            }
            style.setFontWeight( fontweight );
        }
        else {
            style.setCustomFontWeight( false );
        }

        bool fontstyle;
        if ( customFontStyle() ) {
            fontstyle = fontStyle();
        }
        else if ( countChildren() == 1 ) {
            fontstyle = true;
        }
        else {
            fontstyle = false;
        }

        if ( fontweight && fontstyle ) {
            style.setCharStyle( boldItalicChar );
        }
        else if ( fontweight && ! fontstyle ) {
            style.setCharStyle( boldChar );
        }
        else if ( ! fontweight && fontstyle ) {
            style.setCharStyle( italicChar );
        }
        else {
            style.setCharStyle( normalChar );
        }
    }
}

KFORMULA_NAMESPACE_END
