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

#include <QPainter>
#include <QPen>

#include <kdebug.h>
#include <klocale.h>

#include "kformuladefs.h"
#include "esstixfontstyle.h"


KFORMULA_NAMESPACE_BEGIN

#include "esstixfontmapping.cc"

bool EsstixFontStyle::init( ContextStyle* context, bool /* install */ )
{
    SymbolTable* st = symbolTable();
    st->init( context );

    SymbolTable::NameTable tempNames;
    fillNameTable( tempNames );

    st->initFont( esstixeightMap, "esstixeight", tempNames );
    st->initFont( esstixelevenMap, "esstixeleven", tempNames );
    st->initFont( esstixfifteenMap, "esstixfifteen", tempNames );
    st->initFont( esstixfiveMap, "esstixfive", tempNames );
    st->initFont( esstixfourMap, "esstixfour", tempNames );
    st->initFont( esstixfourteenMap, "esstixfourteen", tempNames );
    st->initFont( esstixnineMap, "esstixnine", tempNames );
    st->initFont( esstixoneMap, "esstixone", tempNames );
    st->initFont( esstixsevenMap, "esstixseven", tempNames );
    st->initFont( esstixseventeenMap, "esstixseventeen", tempNames );
    st->initFont( esstixsixMap, "esstixsix", tempNames );
    st->initFont( esstixsixteenMap, "esstixsixteen", tempNames );
    st->initFont( esstixtenMap, "esstixten", tempNames );
    st->initFont( esstixthirteenMap, "esstixthirteen", tempNames );
    st->initFont( esstixthreeMap, "esstixthree", tempNames );
    st->initFont( esstixtwelveMap, "esstixtwelve", tempNames );
    st->initFont( esstixtwoMap, "esstixtwo", tempNames );

    return true;
}


const AlphaTable* EsstixFontStyle::alphaTable() const
{
    return &m_alphaTable;
}


Artwork* EsstixFontStyle::createArtwork( SymbolType type ) const
{
    return new EsstixArtwork( type );
}

QStringList EsstixFontStyle::missingFonts()
{
    QStringList missing;

    testFont( missing, "esstixeight" );
    testFont( missing, "esstixeleven" );
    testFont( missing, "esstixfifteen" );
    testFont( missing, "esstixfive" );
    testFont( missing, "esstixfour" );
    testFont( missing, "esstixfourteen" );
    testFont( missing, "esstixnine" );
    testFont( missing, "esstixone" );
    testFont( missing, "esstixseven" );
    testFont( missing, "esstixseventeen" );
    testFont( missing, "esstixsix" );
    testFont( missing, "esstixsixteen" );
    testFont( missing, "esstixten" );
    testFont( missing, "esstixthirteen" );
    testFont( missing, "esstixthree" );
    testFont( missing, "esstixtwelve" );
    testFont( missing, "esstixtwo" );

    return missing;
}


EsstixAlphaTable::EsstixAlphaTable()
    : script_font( "esstixthirteen" ),
      fraktur_font( "esstixfifteen" ),
      double_struck_font( "esstixfourteen" )
{
}


AlphaTableEntry EsstixAlphaTable::entry( short pos,
                                         CharFamily family,
                                         CharStyle /*style*/ ) const
{
    AlphaTableEntry entry;

    // This is very font specific.
    switch( family ) {
        //case normal:
    case scriptFamily:
        if ( ( ( pos >= 'A' ) && ( pos <= 'Z' ) ) ||
             ( ( pos >= 'a' ) && ( pos <= 'z' ) ) ) {
            entry.pos = pos;
            entry.font = script_font;
        }
        break;
    case frakturFamily:
        if ( ( ( pos >= 'A' ) && ( pos <= 'Z' ) ) ||
             ( ( pos >= 'a' ) && ( pos <= 'z' ) ) ) {
            entry.pos = pos;
            entry.font = fraktur_font;
        }
        break;
    case doubleStruckFamily:
        if ( ( ( pos >= 'A' ) && ( pos <= 'Z' ) ) ||
             ( ( pos >= '0' ) && ( pos <= '9' ) ) ) {
            entry.pos = pos;
            entry.font = double_struck_font;
        }
        break;
    default:
        break;
    }

    return entry;
}


static const char esstixseven_LeftSquareBracket = 0x3f;
static const char esstixseven_RightSquareBracket = 0x40;
static const char esstixseven_LeftCurlyBracket = 0x41;
static const char esstixseven_RightCurlyBracket = 0x42;
static const char esstixseven_LeftCornerBracket = 0x43;
static const char esstixseven_RightCornerBracket = 0x44;
static const char esstixseven_LeftRoundBracket = 0x3d;
static const char esstixseven_RightRoundBracket = 0x3e;
//static const char esstixseven_SlashBracket = '/';
//static const char esstixseven_BackSlashBracket = '\\';
static const char esstixseven_LeftLineBracket = 0x4b;
static const char esstixseven_RightLineBracket = 0x4b;


// esstixseven is a special font with symbols in three sizes.
static char esstixseven_nextchar( char ch )
{
    switch ( ch ) {
        // small
    case 61: return 33;
    case 62: return 35;
    case 63: return 36;
    case 64: return 37;
    case 65: return 38;
    case 66: return 40;
    case 67: return 41;
    case 68: return 42;
    case 69: return 43;
    case 70: return 44;
    case 75: return 45;
    case 76: return 47;

        // middle
    case 33: return 48;
    case 35: return 49;
    case 36: return 50;
    case 37: return 51;
    case 38: return 52;
    case 40: return 53;
    case 41: return 54;
    case 42: return 55;
    case 43: return 56;
    case 44: return 57;
    case 45: return 58;
    case 46: return 59;
    case 47: return 60;
    }
    return 0;
}

EsstixArtwork::EsstixArtwork( SymbolType t )
    : Artwork( t ), esstixChar( -1 )
{
}


void EsstixArtwork::calcSizes( const ContextStyle& style,
                               ContextStyle::TextStyle tstyle,
                               luPt parentSize )
{
    setBaseline( -1 );
    esstixChar = -1;
    luPt mySize = style.getAdjustedSize( tstyle );
    //const SymbolTable& symbolTable = style.symbolTable();
    switch (getType()) {
    case LeftSquareBracket:
        if ( calcEsstixDelimiterSize( style, esstixseven_LeftSquareBracket,
                                      mySize, parentSize ) ) {
            return;
        }
        calcRoundBracket( style, leftSquareBracket, parentSize, mySize );
        break;
    case RightSquareBracket:
        if ( calcEsstixDelimiterSize( style, esstixseven_RightSquareBracket,
                                      mySize, parentSize ) ) {
            return;
        }
        calcRoundBracket( style, rightSquareBracket, parentSize, mySize );
        break;
    case LeftLineBracket:
        if ( calcEsstixDelimiterSize( style, esstixseven_LeftLineBracket,
                                      mySize, parentSize ) ) {
            return;
        }
        calcRoundBracket( style, leftLineBracket, parentSize, mySize );
        break;
    case RightLineBracket:
        if ( calcEsstixDelimiterSize( style, esstixseven_RightLineBracket,
                                      mySize, parentSize ) ) {
            return;
        }
        calcRoundBracket( style, rightLineBracket, parentSize, mySize );
        break;
    case SlashBracket:
        //calcCharSize(style, mySize, '/');
        break;
    case BackSlashBracket:
        //calcCharSize(style, mySize, '\\');
        break;
    case LeftCornerBracket:
        if ( calcEsstixDelimiterSize( style, esstixseven_LeftCornerBracket,
                                      mySize, parentSize ) ) {
            return;
        }
        calcCharSize(style, mySize, leftAngleBracketChar);
        break;
    case RightCornerBracket:
        if ( calcEsstixDelimiterSize( style, esstixseven_RightCornerBracket,
                                      mySize, parentSize ) ) {
            return;
        }
        calcCharSize(style, mySize, rightAngleBracketChar);
        break;
    case LeftRoundBracket:
        if ( calcEsstixDelimiterSize( style, esstixseven_LeftRoundBracket,
                                      mySize, parentSize ) ) {
            return;
        }
        calcRoundBracket( style, leftRoundBracket, parentSize, mySize );
        break;
    case RightRoundBracket:
        if ( calcEsstixDelimiterSize( style, esstixseven_RightRoundBracket,
                                      mySize, parentSize ) ) {
            return;
        }
        calcRoundBracket( style, rightRoundBracket, parentSize, mySize );
        break;
    case EmptyBracket:
        setHeight(parentSize);
        //setWidth(style.getEmptyRectWidth());
        setWidth(0);
        break;
    case LeftCurlyBracket:
        if ( calcEsstixDelimiterSize( style, esstixseven_LeftCurlyBracket,
                                      mySize, parentSize ) ) {
            return;
        }
        calcCurlyBracket( style, leftCurlyBracket, parentSize, mySize );
        break;
    case RightCurlyBracket:
        if ( calcEsstixDelimiterSize( style, esstixseven_RightCurlyBracket,
                                      mySize, parentSize ) ) {
            return;
        }
        calcCurlyBracket( style, rightCurlyBracket, parentSize, mySize );
        break;
    case Integral:
        calcCharSize( style, qRound( 1.5*mySize ), integralChar );
        break;
    case Sum:
        calcCharSize( style, qRound( 1.5*mySize ), summationChar );
        break;
    case Product:
        calcCharSize( style, qRound( 1.5*mySize ), productChar );
        break;
    }
}


void EsstixArtwork::draw(QPainter& painter, const LuPixelRect& /*r*/,
                         const ContextStyle& style, ContextStyle::TextStyle tstyle,
                         luPt /*parentSize*/, const LuPixelPoint& origin)
{
    luPt mySize = style.getAdjustedSize( tstyle );
    luPixel myX = origin.x() + getX();
    luPixel myY = origin.y() + getY();
    /*
    if ( !LuPixelRect( myX, myY, getWidth(), getHeight() ).intersects( r ) )
        return;
    */

    painter.setPen(style.getDefaultColor());
    //const SymbolTable& symbolTable = style.symbolTable();

    switch (getType()) {
    case LeftSquareBracket:
        if ( esstixChar != -1 ) {
            drawEsstixDelimiter( painter, style, myX, myY, mySize );
        }
        else {
            drawBigRoundBracket( painter, style, leftSquareBracket, myX, myY, mySize );
        }
        break;
    case RightSquareBracket:
        if ( esstixChar != -1 ) {
            drawEsstixDelimiter( painter, style, myX, myY, mySize );
        }
        else {
            drawBigRoundBracket( painter, style, rightSquareBracket, myX, myY, mySize );
        }
        break;
    case LeftCurlyBracket:
        if ( esstixChar != -1 ) {
            drawEsstixDelimiter( painter, style, myX, myY, mySize );
        }
        else {
            drawBigCurlyBracket( painter, style, leftCurlyBracket, myX, myY, mySize );
        }
        break;
    case RightCurlyBracket:
        if ( esstixChar != -1 ) {
            drawEsstixDelimiter( painter, style, myX, myY, mySize );
        }
        else {
            drawBigCurlyBracket( painter, style, rightCurlyBracket, myX, myY, mySize );
        }
        break;
    case LeftLineBracket:
        if ( esstixChar != -1 ) {
            drawEsstixDelimiter( painter, style, myX, myY, mySize );
        }
        else {
            drawBigRoundBracket( painter, style, leftLineBracket, myX, myY, mySize );
        }
        break;
    case RightLineBracket:
        if ( esstixChar != -1 ) {
            drawEsstixDelimiter( painter, style, myX, myY, mySize );
        }
        else {
            drawBigRoundBracket( painter, style, rightLineBracket, myX, myY, mySize );
        }
        break;
    case SlashBracket:
        //drawCharacter(painter, style, myX, myY, mySize, '/');
        break;
    case BackSlashBracket:
        //drawCharacter(painter, style, myX, myY, mySize, '\\');
        break;
    case LeftCornerBracket:
        if ( esstixChar != -1 ) {
            drawEsstixDelimiter( painter, style, myX, myY, mySize );
        }
        else drawCharacter(painter, style, myX, myY, mySize, leftAngleBracketChar);
        break;
    case RightCornerBracket:
        if ( esstixChar != -1 ) {
            drawEsstixDelimiter( painter, style, myX, myY, mySize );
        }
        else drawCharacter(painter, style, myX, myY, mySize, rightAngleBracketChar);
        break;
    case LeftRoundBracket:
        if ( esstixChar != -1 ) {
            drawEsstixDelimiter( painter, style, myX, myY, mySize );
        }
        else {
            drawBigRoundBracket( painter, style, leftRoundBracket, myX, myY, mySize );
        }
        break;
    case RightRoundBracket:
        if ( esstixChar != -1 ) {
            drawEsstixDelimiter( painter, style, myX, myY, mySize );
        }
        else {
            drawBigRoundBracket( painter, style, rightRoundBracket, myX, myY, mySize );
        }
        break;
    case EmptyBracket:
        break;
    case Integral:
        drawCharacter(painter, style, myX, myY, qRound( 1.5*mySize ), integralChar);
        break;
    case Sum:
        drawCharacter(painter, style, myX, myY, qRound( 1.5*mySize ), summationChar);
        break;
    case Product:
        drawCharacter(painter, style, myX, myY, qRound( 1.5*mySize ), productChar);
        break;
    }

    // debug
    //painter.setBrush(Qt::NoBrush);
    //painter.setPen(Qt::green);
    //painter.drawRect(myX, myY, getWidth(), getHeight());
}


bool EsstixArtwork::isNormalChar() const
{
    return Artwork::isNormalChar() && ( fontSizeFactor == 1 );
}


bool EsstixArtwork::calcEsstixDelimiterSize( const ContextStyle& context,
                                             char c,
                                             luPt fontSize,
                                             luPt parentSize )
{
    QFont f( "esstixseven" );

    for ( char i=1; c != 0; ++i ) {
        //f.setPointSizeFloat( context.layoutUnitToFontSize( i*fontSize, false ) );
        f.setPointSizeF( context.layoutUnitPtToPt( i*fontSize ) );
        QFontMetrics fm( f );
        LuPixelRect bound = fm.boundingRect( c );

        luPt height = context.ptToLayoutUnitPt( bound.height() );
        if ( height >= parentSize ) {
            luPt width = context.ptToLayoutUnitPt( fm.width( c ) );
            luPt baseline = context.ptToLayoutUnitPt( -bound.top() );

            esstixChar = c;
            fontSizeFactor = i;

            setHeight( height );
            setWidth( width );
            setBaseline( baseline );

            return true;
        }
        c = esstixseven_nextchar( c );
    }

    // Build it up from pieces.
    return false;
}


void EsstixArtwork::drawEsstixDelimiter( QPainter& painter, const ContextStyle& style,
                                         luPixel x, luPixel y,
                                         luPt height )
{
    QFont f( "esstixseven" );
    f.setPointSizeF( style.layoutUnitToFontSize( fontSizeFactor*height, false ) );

    painter.setFont( f );
    painter.drawText( style.layoutUnitToPixelX( x ),
                      style.layoutUnitToPixelY( y + getBaseline() ),
                      QString( QChar( esstixChar ) ) );
}


KFORMULA_NAMESPACE_END
