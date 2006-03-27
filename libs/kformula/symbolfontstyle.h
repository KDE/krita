/* This file is part of the KDE project
   Copyright (C) 2003 Ulrich Kuettler <ulrich.kuettler@gmx.de>

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

#ifndef SYMBOLFONTSTYLE_H
#define SYMBOLFONTSTYLE_H

#include "fontstyle.h"

KFORMULA_NAMESPACE_BEGIN


/**
 * The style of the standard symbol font.
 */
class SymbolFontStyle : public FontStyle {
public:

    /**
     * lazy init support. Needs to be run before anything else.
     * @param install if true fonts may be installed if needed
     */
    virtual bool init( ContextStyle* context, bool install = true );

    virtual Artwork* createArtwork( SymbolType type = EmptyBracket ) const;

    static QStringList missingFonts();
};


class SymbolArtwork : public Artwork {
public:
    SymbolArtwork( SymbolType t ) : Artwork( t ) {}

    virtual void calcSizes( const ContextStyle& style,
                            ContextStyle::TextStyle tstyle,
                            luPt parentSize );

    virtual void draw( QPainter& painter, const LuPixelRect& r,
                       const ContextStyle& style,
                       ContextStyle::TextStyle tstyle,
                       luPt parentSize, const LuPixelPoint& origin );
};

KFORMULA_NAMESPACE_END

#endif
