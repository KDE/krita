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

#include <qpainter.h>
#include <qpen.h>

#include <kdebug.h>
#include <klocale.h>

#include "kformuladefs.h"
#include "symbolfontstyle.h"


KFORMULA_NAMESPACE_BEGIN

#include "symbolfontmapping.cc"


SymbolFontHelper::SymbolFontHelper()
    : greek("abgdezhqiklmnxpvrstufjcywGDQLXPSUFYVW")
{
    for ( uint i = 0; symbolMap[ i ].unicode != 0; i++ ) {
        compatibility[ symbolMap[ i ].pos ] = symbolMap[ i ].unicode;
    }
}


bool SymbolFontStyle::init( ContextStyle* context, bool /* install */)
{
    // We require the symbol font to be there as it's the last resort
    // anyway.
    symbolTable()->init( context );

    SymbolTable::NameTable names;
    fillNameTable( names );
    symbolTable()->initFont( symbolMap, "symbol", names );

    return true;
}


Artwork* SymbolFontStyle::createArtwork( SymbolType type ) const
{
    return new SymbolArtwork( type );
}

QStringList SymbolFontStyle::missingFonts()
{
    QStringList missing;

    testFont( missing, "symbol" );

    return missing;
}

inline bool doSimpleRoundBracket( luPt height, luPt baseHeight )
{
    return height < 1.5*baseHeight;
}

inline bool doSimpleSquareBracket( luPt height, luPt baseHeight )
{
    return height < 1.5*baseHeight;
}

inline bool doSimpleCurlyBracket( luPt height, luPt baseHeight )
{
    return height < 2*baseHeight;
}


void SymbolArtwork::calcSizes( const ContextStyle& style,
                               ContextStyle::TextStyle tstyle,
                               luPt parentSize )
{
    setBaseline( -1 );
    luPt mySize = style.getAdjustedSize( tstyle );
    switch (getType()) {
    case LeftSquareBracket:
        if ( doSimpleSquareBracket( parentSize, mySize ) ) {
            calcCharSize( style, mySize, leftSquareBracketChar );
            return;
        }
        calcRoundBracket( style, leftSquareBracket, parentSize, mySize );
        break;
    case RightSquareBracket:
        if ( doSimpleSquareBracket( parentSize, mySize ) ) {
            calcCharSize(style, mySize, rightSquareBracketChar);
            return;
        }
        calcRoundBracket( style, rightSquareBracket, parentSize, mySize );
        break;
    case LeftLineBracket:
        if ( doSimpleSquareBracket( parentSize, mySize ) ) {
            calcCharSize(style, mySize, verticalLineChar);
            return;
        }
        calcRoundBracket( style, leftLineBracket, parentSize, mySize );
        break;
    case RightLineBracket:
        if ( doSimpleSquareBracket( parentSize, mySize ) ) {
            calcCharSize(style, mySize, verticalLineChar);
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
        calcCharSize(style, mySize, leftAngleBracketChar);
        break;
    case RightCornerBracket:
        calcCharSize(style, mySize, rightAngleBracketChar);
        break;
    case LeftRoundBracket:
        if ( doSimpleRoundBracket( parentSize, mySize ) ) {
            calcCharSize(style, mySize, leftParenthesisChar);
            return;
        }
        calcRoundBracket( style, leftRoundBracket, parentSize, mySize );
        break;
    case RightRoundBracket:
        if ( doSimpleRoundBracket( parentSize, mySize ) ) {
            calcCharSize(style, mySize, rightParenthesisChar);
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
        if ( doSimpleCurlyBracket( parentSize, mySize ) ) {
            calcCharSize(style, mySize, leftCurlyBracketChar);
            return;
        }
        calcCurlyBracket( style, leftCurlyBracket, parentSize, mySize );
        break;
    case RightCurlyBracket:
        if ( doSimpleCurlyBracket( parentSize, mySize ) ) {
            calcCharSize(style, mySize, rightCurlyBracketChar);
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


void SymbolArtwork::draw(QPainter& painter, const LuPixelRect& /*r*/,
                         const ContextStyle& style, ContextStyle::TextStyle tstyle,
                         luPt parentSize, const LuPixelPoint& origin)
{
    luPt mySize = style.getAdjustedSize( tstyle );
    luPixel myX = origin.x() + getX();
    luPixel myY = origin.y() + getY();
    /*
    if ( !LuPixelRect( myX, myY, getWidth(), getHeight() ).intersects( r ) )
        return;
    */

    painter.setPen(style.getDefaultColor());

    switch (getType()) {
    case LeftSquareBracket:
        if ( !doSimpleSquareBracket( parentSize, mySize ) ) {
            drawBigRoundBracket( painter, style, leftSquareBracket, myX, myY, mySize );
        }
        else {
            drawCharacter(painter, style, myX, myY, mySize, leftSquareBracketChar);
        }
        break;
    case RightSquareBracket:
        if ( !doSimpleSquareBracket( parentSize, mySize ) ) {
            drawBigRoundBracket( painter, style, rightSquareBracket, myX, myY, mySize );
        }
        else {
            drawCharacter(painter, style, myX, myY, mySize, rightSquareBracketChar);
        }
        break;
    case LeftCurlyBracket:
        if ( !doSimpleCurlyBracket( parentSize, mySize ) ) {
            drawBigCurlyBracket( painter, style, leftCurlyBracket, myX, myY, mySize );
        }
        else {
            drawCharacter(painter, style, myX, myY, mySize, leftCurlyBracketChar);
        }
        break;
    case RightCurlyBracket:
        if ( !doSimpleCurlyBracket( parentSize, mySize ) ) {
            drawBigCurlyBracket( painter, style, rightCurlyBracket, myX, myY, mySize );
        }
        else {
            drawCharacter(painter, style, myX, myY, mySize, rightCurlyBracketChar);
        }
        break;
    case LeftLineBracket:
        if ( !doSimpleSquareBracket( parentSize, mySize ) ) {
            drawBigRoundBracket( painter, style, leftLineBracket, myX, myY, mySize );
        }
        else {
            drawCharacter(painter, style, myX, myY, mySize, verticalLineChar);
        }
        break;
    case RightLineBracket:
        if ( !doSimpleSquareBracket( parentSize, mySize ) ) {
            drawBigRoundBracket( painter, style, rightLineBracket, myX, myY, mySize );
        }
        else {
            drawCharacter(painter, style, myX, myY, mySize, verticalLineChar);
        }
        break;
    case SlashBracket:
        //drawCharacter(painter, style, myX, myY, mySize, '/');
        break;
    case BackSlashBracket:
        //drawCharacter(painter, style, myX, myY, mySize, '\\');
        break;
    case LeftCornerBracket:
        drawCharacter(painter, style, myX, myY, mySize, leftAngleBracketChar);
        break;
    case RightCornerBracket:
        drawCharacter(painter, style, myX, myY, mySize, rightAngleBracketChar);
        break;
    case LeftRoundBracket:
        if ( !doSimpleRoundBracket( parentSize, mySize ) ) {
            drawBigRoundBracket( painter, style, leftRoundBracket, myX, myY, mySize );
        }
        else {
            drawCharacter(painter, style, myX, myY, mySize, leftParenthesisChar);
        }
        break;
    case RightRoundBracket:
        if ( !doSimpleRoundBracket( parentSize, mySize ) ) {
            drawBigRoundBracket( painter, style, rightRoundBracket, myX, myY, mySize );
        }
        else {
            drawCharacter(painter, style, myX, myY, mySize, rightParenthesisChar);
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


KFORMULA_NAMESPACE_END
