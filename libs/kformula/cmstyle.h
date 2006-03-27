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

#ifndef CMSTYLE_H
#define CMSTYLE_H

#include "fontstyle.h"

KFORMULA_NAMESPACE_BEGIN


class CMAlphaTable : public AlphaTable {
public:

    CMAlphaTable();

    virtual AlphaTableEntry entry( short pos, CharFamily family, CharStyle style ) const;

private:

};


class CMStyle : public FontStyle {
public:

    /**
     * lazy init support. Needs to be run before anything else.
     * @param install if true fonts may be installed if needed
     */
    virtual bool init( ContextStyle* context, bool install = true );

    /// the table for special alphabets.
    virtual const AlphaTable* alphaTable() const;

    virtual Artwork* createArtwork( SymbolType type = EmptyBracket ) const;

    static QStringList missingFonts( bool install = true );

    static bool m_installed;

private:

    static QStringList missingFontsInternal();
    static void installFonts();

    CMAlphaTable m_alphaTable;
};


class CMArtwork : public Artwork {
public:
    CMArtwork( SymbolType t );

    virtual void calcSizes( const ContextStyle& style,
                            ContextStyle::TextStyle tstyle,
                            luPt parentSize );
    virtual void calcSizes( const ContextStyle& style,
                            ContextStyle::TextStyle tstyle );

    virtual void draw( QPainter& painter, const LuPixelRect& r,
                       const ContextStyle& style,
                       ContextStyle::TextStyle tstyle,
                       luPt parentSize, const LuPixelPoint& origin );
    virtual void draw( QPainter& painter, const LuPixelRect& r,
                       const ContextStyle& style,
                       ContextStyle::TextStyle tstyle,
                       const LuPixelPoint& parentOrigin );

    virtual bool isNormalChar() const;

    virtual double slant() const;

private:

    bool calcCMDelimiterSize( const ContextStyle& context, uchar c,
                              luPt fontSize, luPt parentSize );
    void calcLargest( const ContextStyle& context, uchar c, luPt fontSize );
    void drawCMDelimiter( QPainter& painter, const ContextStyle& style,
                          luPixel x, luPixel y, luPt height );

    short cmChar;
};

KFORMULA_NAMESPACE_END

#endif
