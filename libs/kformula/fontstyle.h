/* This file is part of the KDE project
   Copyright (C) 2003 Ulrich Kuettler <ulrich.kuettler@gmx.de>
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

#ifndef FONTSTYLE_H
#define FONTSTYLE_H

#include <QString>
#include <QFont>

#include "contextstyle.h"
#include "kformuladefs.h"
#include "symboltable.h"


KFORMULA_NAMESPACE_BEGIN

class Artwork;
class SymbolTable;


/**
 * Base class for all supported font styles.
 */
class FontStyle {
public:

    ~FontStyle() {}

    /**
     * lazy init support. Needs to be run before anything else.
     * @param install if true fonts may be installed if needed
     */
    bool init( ContextStyle* context, bool install = true );

    /// the table for ordinary symbols (those that have a unicode value)
    const SymbolTable* symbolTable() const { return &m_symbolTable; }
    SymbolTable* symbolTable() { return &m_symbolTable; }

    Artwork* createArtwork(SymbolType type = EmptyBracket) const;

    static QStringList missingFonts( bool install = true );

    static bool m_installed;

    static void testFont( QStringList& missing, const QString& fontName );

private:

    static QStringList missingFontsInternal();
    static void installFonts();

    SymbolTable m_symbolTable;
};

const QChar spaceChar = 0x0020;
const QChar leftParenthesisChar = 0x0028;
const QChar rightParenthesisChar = 0x0029;
const QChar leftSquareBracketChar = 0x005B;
const QChar rightSquareBracketChar = 0x005D;
const QChar leftCurlyBracketChar = 0x007B;
const QChar verticalLineChar = 0x007C;
const QChar rightCurlyBracketChar = 0x007D;
const QChar leftAngleBracketChar = 0x2329;
const QChar rightAngleBracketChar = 0x232A;
const QChar slashChar = 0x002F;
const QChar backSlashChar = 0x005C;
const QChar integralChar = 0x222B;
const QChar summationChar = 0x2211;
const QChar productChar = 0x220F;
const QChar applyFunctionChar = 0x2061;
const QChar invisibleTimes = 0x2062;
const QChar invisibleComma = 0x2063;

extern const uchar leftRoundBracket[];
extern const uchar leftSquareBracket[];
extern const uchar leftCurlyBracket[];

extern const uchar leftLineBracket[];
extern const uchar rightLineBracket[];

extern const uchar rightRoundBracket[];
extern const uchar rightSquareBracket[];
extern const uchar rightCurlyBracket[];

/*
 * A piece of art that may be used by any element.
 */
class Artwork {
public:

    Artwork(SymbolType type = EmptyBracket);
    virtual ~Artwork() {}

    virtual void calcSizes( const ContextStyle& style,
                            ContextStyle::TextStyle tstyle,
                            double factor,
                            luPt parentSize );
    virtual void calcSizes( const ContextStyle& style,
                            ContextStyle::TextStyle tstyle,
                            double factor );

    virtual void draw( QPainter& painter, const LuPixelRect& r,
                       const ContextStyle& context,
                       ContextStyle::TextStyle tstyle,
					   StyleAttributes& style,
                       luPt parentSize, const LuPixelPoint& origin );
    virtual void draw( QPainter& painter, const LuPixelRect& r,
                       const ContextStyle& context,
                       ContextStyle::TextStyle tstyle,
					   StyleAttributes& style,
                       const LuPixelPoint& parentOrigin );

    luPixel getWidth() const { return size.width(); }
    luPixel getHeight() const { return size.height(); }

    void setWidth( luPixel width ) { size.setWidth(width); }
    void setHeight( luPixel height ) { size.setHeight(height); }

    luPixel getBaseline() const { return baseline; }
    void setBaseline( luPixel line ) { baseline = line; }

    luPixel getX() const { return point.x(); }
    luPixel getY() const { return point.y(); }

    void setX( luPixel x ) { point.setX( x ); }
    void setY( luPixel y ) { point.setY( y ); }

    SymbolType getType() const { return type; }
    void setType(SymbolType t) { type = t; }

    virtual bool isNormalChar() const { return getBaseline() != -1 && ( cmChar == -1 ); }

    virtual double slant() const { return 0; }

protected:

    void calcCharSize( const ContextStyle& style, luPt height, QChar ch );
    void drawCharacter( QPainter& painter, const ContextStyle& style,
                        luPixel x, luPixel y, luPt height, QChar ch );

    void calcCharSize( const ContextStyle& style, QFont f, luPt height, QChar c );
    void drawCharacter( QPainter& painter, const ContextStyle& style,
                        QFont f,
                        luPixel x, luPixel y, luPt height, QChar c );

    void calcRoundBracket( const ContextStyle& style, const uchar chars[], luPt height, luPt charHeight );
    void calcCurlyBracket( const ContextStyle& style, const uchar chars[], luPt height, luPt charHeight );

    void drawBigRoundBracket( QPainter& p, const ContextStyle& style, const uchar chars[], luPixel x, luPixel y, luPt charHeight );
    void drawBigCurlyBracket( QPainter& p, const ContextStyle& style, const uchar chars[], luPixel x, luPixel y, luPt charHeight );

private:

    LuPixelSize size;
    LuPixelPoint point;

    /**
     * Used if we are a character.
     */
    luPixel baseline;

    SymbolType type;

    bool calcCMDelimiterSize( const ContextStyle& context, uchar c,
                              luPt fontSize, luPt parentSize );
    void calcLargest( const ContextStyle& context, uchar c, luPt fontSize );
    void drawCMDelimiter( QPainter& painter, const ContextStyle& style,
                          luPixel x, luPixel y, luPt height );

    short cmChar;
};


KFORMULA_NAMESPACE_END

#endif
