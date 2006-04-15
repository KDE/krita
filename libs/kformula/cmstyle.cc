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
#include <qapplication.h>

#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <kio/job.h>
#include <kmessagebox.h>

#include "kformuladefs.h"
#include "cmstyle.h"


KFORMULA_NAMESPACE_BEGIN

#include "cmmapping.cc"

bool CMStyle::m_installed = false;

bool CMStyle::init( ContextStyle* context, bool install )
{
    if (!m_installed && install)
        installFonts();
    SymbolTable* st = symbolTable();
    st->init( context );

    SymbolTable::NameTable tempNames;
    fillNameTable( tempNames );

    st->initFont( cmbx10Map, "cmbx10", tempNames );
    st->initFont( cmex10Map, "cmex10", tempNames );
    st->initFont( cmmi10Map, "cmmi10", tempNames );
    st->initFont( cmr10Map, "cmr10", tempNames );
    st->initFont( cmsy10Map, "cmsy10", tempNames );
    st->initFont( msam10Map, "msam10", tempNames );
    st->initFont( msbm10Map, "msbm10", tempNames );

    return true;
}


const AlphaTable* CMStyle::alphaTable() const
{
    return &m_alphaTable;
}


Artwork* CMStyle::createArtwork( SymbolType type ) const
{
    return new CMArtwork( type );
}

QStringList CMStyle::missingFonts( bool install )
{
    if (!m_installed && install)
        installFonts();

    QStringList missing = missingFontsInternal();
    return missing;
}

QStringList CMStyle::missingFontsInternal()
{
    QStringList missing;

    testFont( missing, "cmbx10" );
    testFont( missing, "cmex10" );
    testFont( missing, "cmmi10" );
    testFont( missing, "cmr10" );
    testFont( missing, "cmsy10" );
    testFont( missing, "msam10" );
    testFont( missing, "msbm10" );

    return missing;
}

void CMStyle::installFonts()
{
    if (m_installed)
        return;
    QStringList missing = missingFontsInternal();
    if (!missing.isEmpty())
    {
        QStringList urlList;
        for (QStringList::iterator it = missing.begin(); it != missing.end(); ++it)
        {
            if (!KIO::NetAccess::exists("fonts:/Personal/" + *it + ".ttf", true, NULL))
                urlList.append(locate("data", "kformula/fonts/" + *it + ".ttf"));
        }
        KIO::copy(urlList, KUrl("fonts:/Personal/"), false);
        KMessageBox::information(qApp->mainWidget(), 
                                 i18n("Some fonts have been installed to assure that symbols in formulas are properly visualized. You must restart the application in order so that changes take effect"));
    }
    m_installed = true;
}

CMAlphaTable::CMAlphaTable()
{
}


AlphaTableEntry CMAlphaTable::entry( short pos,
                                     CharFamily family,
                                     CharStyle /*style*/ ) const
{
    /*
    static short uppercase_greek[] = {
        0x0393, // Gamma
        0x0394, // Delta
        0x0398, // Theta
        0x039B, // Lambda
        0x039E, // Xi
        0x03A0, // Pi
        0x03A3, // Sigma
        0x03A6, // Phi
        0x03A8, // Psi
        0x03A9, // Omega
        0x03D2, // Upsilon
        0
    };
    */

    AlphaTableEntry entry;
    switch( family ) {
        //case normal:
    case scriptFamily:
        /*
        for ( int i=0; uppercase_greek[i] != 0; ++i ) {
            if ( pos == uppercase_greek[i] ) {
                entry.pos = pos;
                entry.font = QFont( "cmsl10" );
                return;
            }
        }
        */
        if ( ( pos >= 'A' ) && ( pos <= 'Z' ) ) {
            entry.pos = pos;
            entry.font = QFont( "cmsy10" );
        }
        break;
    case frakturFamily:
        break;
    case doubleStruckFamily:
        if ( ( pos >= 'A' ) && ( pos <= 'Z' ) ) {
            entry.pos = pos;
            entry.font = QFont( "msbm10" );
        }
        break;
    default:
        break;
    }
    return entry;
}


static const char cmex_LeftSquareBracket = 163;
static const char cmex_RightSquareBracket = 164;
static const char cmex_LeftCurlyBracket = 169;
static const char cmex_RightCurlyBracket = 170;
static const char cmex_LeftCornerBracket = 173;
static const char cmex_RightCornerBracket = 174;
static const char cmex_LeftRoundBracket = 161;
static const char cmex_RightRoundBracket = 162;
static const char cmex_SlashBracket = 177;
static const char cmex_BackSlashBracket = 178;
//static const char cmex_LeftLineBracket = 0x4b;
//static const char cmex_RightLineBracket = 0x4b;

// use the big symbols here
static const char cmex_Int = 90;
static const char cmex_Sum = 88;
static const char cmex_Prod = 89;


// cmex is a special font with symbols in four sizes.
static short cmex_nextchar( short ch )
{
    switch ( ch ) {
    case 161: return 179;
    case 162: return 180;
    case 163: return 104;
    case 164: return 105;
    case 169: return 110;
    case 170: return 111;
    case 165: return 106;
    case 166: return 107;
    case 167: return 108;
    case 168: return 109;
    case 173: return 68;
    case 174: return 69;
    case 177: return 46;
    case 178: return 47;

    case 179: return 181;
    case 180: return 182;
    case 104: return 183;
    case 105: return 184;
    case 110: return 189;
    case 111: return 190;
    case 106: return 185;
    case 107: return 186;
    case 108: return 187;
    case 109: return 188;
    case 68: return 191;
    case 69: return 192;
    case 46: return 193;
    case 47: return 194;

    case 181: return 195;
    case 182: return 33;
    case 183: return 34;
    case 184: return 35;
    case 189: return 40;
    case 190: return 41;
    case 185: return 36;
    case 186: return 37;
    case 187: return 38;
    case 188: return 39;
    case 191: return 42;
    case 192: return 43;
    case 193: return 44;
    case 194: return 45;
    }
    return 0;
}

CMArtwork::CMArtwork( SymbolType t )
    : Artwork( t ), cmChar( -1 )
{
}


void CMArtwork::calcSizes( const ContextStyle& style,
                           ContextStyle::TextStyle tstyle,
                           luPt parentSize )
{
    setBaseline( -1 );
    cmChar = -1;
    luPt mySize = style.getAdjustedSize( tstyle );
    switch (getType()) {
    case LeftSquareBracket:
        if ( calcCMDelimiterSize( style, cmex_LeftSquareBracket,
                                  mySize, parentSize ) ) {
            return;
        }
        calcRoundBracket( style, leftSquareBracket, parentSize, mySize );
        break;
    case RightSquareBracket:
        if ( calcCMDelimiterSize( style, cmex_RightSquareBracket,
                                  mySize, parentSize ) ) {
            return;
        }
        calcRoundBracket( style, rightSquareBracket, parentSize, mySize );
        break;
    case LeftLineBracket:
        calcRoundBracket( style, leftLineBracket, parentSize, mySize );
        setWidth( getWidth()/2 );
        break;
    case RightLineBracket:
        calcRoundBracket( style, rightLineBracket, parentSize, mySize );
        setWidth( getWidth()/2 );
        break;
    case SlashBracket:
        if ( calcCMDelimiterSize( style, cmex_SlashBracket,
                                  mySize, parentSize ) ) {
            return;
        }
        calcLargest( style, cmex_SlashBracket, mySize );
        break;
    case BackSlashBracket:
        if ( calcCMDelimiterSize( style, cmex_BackSlashBracket,
                                  mySize, parentSize ) ) {
            return;
        }
        calcLargest( style, cmex_BackSlashBracket, mySize );
        break;
    case LeftCornerBracket:
        if ( calcCMDelimiterSize( style, cmex_LeftCornerBracket,
                                  mySize, parentSize ) ) {
            return;
        }
        calcLargest( style, cmex_LeftCornerBracket, mySize );
        break;
    case RightCornerBracket:
        if ( calcCMDelimiterSize( style, cmex_RightCornerBracket,
                                  mySize, parentSize ) ) {
            return;
        }
        calcLargest( style, cmex_RightCornerBracket, mySize );
        break;
    case LeftRoundBracket:
        if ( calcCMDelimiterSize( style, cmex_LeftRoundBracket,
                                  mySize, parentSize ) ) {
            return;
        }
        calcRoundBracket( style, leftRoundBracket, parentSize, mySize );
        break;
    case RightRoundBracket:
        if ( calcCMDelimiterSize( style, cmex_RightRoundBracket,
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
        if ( calcCMDelimiterSize( style, cmex_LeftCurlyBracket,
                                  mySize, parentSize ) ) {
            return;
        }
        calcCurlyBracket( style, leftCurlyBracket, parentSize, mySize );
        break;
    case RightCurlyBracket:
        if ( calcCMDelimiterSize( style, cmex_RightCurlyBracket,
                                  mySize, parentSize ) ) {
            return;
        }
        calcCurlyBracket( style, rightCurlyBracket, parentSize, mySize );
        break;
    case Integral:
        calcCharSize( style, QFont( "cmex10" ), mySize, cmex_Int );
        break;
    case Sum:
        calcCharSize( style, QFont( "cmex10" ), mySize, cmex_Sum );
        break;
    case Product:
        calcCharSize( style, QFont( "cmex10" ), mySize, cmex_Prod );
        break;
    }
}


void CMArtwork::calcSizes( const ContextStyle& style,
                           ContextStyle::TextStyle tstyle )
{
    luPt mySize = style.getAdjustedSize( tstyle );
    switch (getType()) {
    case LeftLineBracket:
    case RightLineBracket:
        calcCharSize(style, mySize, 0x2223);
        break;
    default:
        Artwork::calcSizes( style, tstyle );
        break;
    }
}


void CMArtwork::draw( QPainter& painter, const LuPixelRect& r,
                      const ContextStyle& style,
                      ContextStyle::TextStyle tstyle,
                      const LuPixelPoint& parentOrigin )
{
    luPt mySize = style.getAdjustedSize( tstyle );
    luPixel myX = parentOrigin.x() + getX();
    luPixel myY = parentOrigin.y() + getY();
    /*
    if ( !LuPixelRect( myX, myY, getWidth(), getHeight() ).intersects( r ) )
        return;
    */

    painter.setPen(style.getDefaultColor());

    switch (getType()) {
    case LeftLineBracket:
    case RightLineBracket:
        drawCharacter(painter, style, myX, myY, mySize, 0x2223);
        break;
    default:
        Artwork::draw( painter, r, style, tstyle, parentOrigin );
        break;
    }
}


void CMArtwork::draw(QPainter& painter, const LuPixelRect& /*r*/,
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

    switch (getType()) {
    case LeftSquareBracket:
        if ( cmChar != -1 ) {
            drawCMDelimiter( painter, style, myX, myY, mySize );
        }
        else {
            drawBigRoundBracket( painter, style, leftSquareBracket, myX, myY, mySize );
        }
        break;
    case RightSquareBracket:
        if ( cmChar != -1 ) {
            drawCMDelimiter( painter, style, myX, myY, mySize );
        }
        else {
            drawBigRoundBracket( painter, style, rightSquareBracket, myX, myY, mySize );
        }
        break;
    case LeftCurlyBracket:
        if ( cmChar != -1 ) {
            drawCMDelimiter( painter, style, myX, myY, mySize );
        }
        else {
            drawBigCurlyBracket( painter, style, leftCurlyBracket, myX, myY, mySize );
        }
        break;
    case RightCurlyBracket:
        if ( cmChar != -1 ) {
            drawCMDelimiter( painter, style, myX, myY, mySize );
        }
        else {
            drawBigCurlyBracket( painter, style, rightCurlyBracket, myX, myY, mySize );
        }
        break;
    case LeftLineBracket: {
        luPixel halfWidth = getWidth()/2;
        drawBigRoundBracket( painter, style, leftLineBracket,
                             myX-halfWidth, myY, mySize );
    }
        break;
    case RightLineBracket: {
        luPixel halfWidth = getWidth()/2;
        drawBigRoundBracket( painter, style, rightLineBracket,
                             myX-halfWidth, myY, mySize );
    }
        break;
    case SlashBracket:
        if ( cmChar != -1 ) {
            drawCMDelimiter( painter, style, myX, myY, mySize );
        }
        break;
    case BackSlashBracket:
        if ( cmChar != -1 ) {
            drawCMDelimiter( painter, style, myX, myY, mySize );
        }
        break;
    case LeftCornerBracket:
        if ( cmChar != -1 ) {
            drawCMDelimiter( painter, style, myX, myY, mySize );
        }
        else drawCharacter(painter, style, myX, myY, mySize, leftAngleBracketChar);
        break;
    case RightCornerBracket:
        if ( cmChar != -1 ) {
            drawCMDelimiter( painter, style, myX, myY, mySize );
        }
        else drawCharacter(painter, style, myX, myY, mySize, rightAngleBracketChar);
        break;
    case LeftRoundBracket:
        if ( cmChar != -1 ) {
            drawCMDelimiter( painter, style, myX, myY, mySize );
        }
        else {
            drawBigRoundBracket( painter, style, leftRoundBracket, myX, myY, mySize );
        }
        break;
    case RightRoundBracket:
        if ( cmChar != -1 ) {
            drawCMDelimiter( painter, style, myX, myY, mySize );
        }
        else {
            drawBigRoundBracket( painter, style, rightRoundBracket, myX, myY, mySize );
        }
        break;
    case EmptyBracket:
        break;
    case Integral:
        drawCharacter(painter, style, QFont( "cmex10" ), myX, myY, mySize, cmex_Int);
        break;
    case Sum:
        drawCharacter(painter, style, QFont( "cmex10" ), myX, myY, mySize, cmex_Sum);
        break;
    case Product:
        drawCharacter(painter, style, QFont( "cmex10" ), myX, myY, mySize, cmex_Prod);
        break;
    }

    // debug
//     painter.setBrush(Qt::NoBrush);
//     painter.setPen(Qt::green);
//     painter.drawRect( style.layoutUnitToPixelX( myX ),
//                       style.layoutUnitToPixelY( myY ),
//                       style.layoutUnitToPixelX( getWidth() ),
//                       style.layoutUnitToPixelY( getHeight() ) );
}


bool CMArtwork::isNormalChar() const
{
    return Artwork::isNormalChar() && ( cmChar == -1 );
}

double CMArtwork::slant() const
{
    if ( getType() == Integral ) {
        return 0.25;
    }
    return 0;
}

bool CMArtwork::calcCMDelimiterSize( const ContextStyle& context,
                                     uchar c,
                                     luPt fontSize,
                                     luPt parentSize )
{
    QFont f( "cmex10" );
    f.setPointSizeFloat( context.layoutUnitPtToPt( fontSize ) );
    QFontMetrics fm( f );

    for ( char i=1; c != 0; ++i ) {
        LuPixelRect bound = fm.boundingRect( c );

        luPt height = context.ptToLayoutUnitPt( bound.height() );
        if ( height >= parentSize ) {
            luPt width = context.ptToLayoutUnitPt( fm.width( c ) );
            luPt baseline = context.ptToLayoutUnitPt( -bound.top() );

            cmChar = c;

            setHeight( height );
            setWidth( width );
            setBaseline( baseline );

            return true;
        }
        c = cmex_nextchar( c );
    }

    // Build it up from pieces.
    return false;
}


void CMArtwork::calcLargest( const ContextStyle& context,
                             uchar c, luPt fontSize )
{
    QFont f( "cmex10" );
    f.setPointSizeFloat( context.layoutUnitPtToPt( fontSize ) );
    QFontMetrics fm( f );

    cmChar = c;
    for ( ;; ) {
        c = cmex_nextchar( c );
        if ( c == 0 ) {
            break;
        }
        cmChar = c;
    }

    LuPixelRect bound = fm.boundingRect( cmChar );

    luPt height = context.ptToLayoutUnitPt( bound.height() );
    luPt width = context.ptToLayoutUnitPt( fm.width( cmChar ) );
    luPt baseline = context.ptToLayoutUnitPt( -bound.top() );

    setHeight( height );
    setWidth( width );
    setBaseline( baseline );
}


void CMArtwork::drawCMDelimiter( QPainter& painter, const ContextStyle& style,
                                 luPixel x, luPixel y,
                                 luPt height )
{
    QFont f( "cmex10" );
    f.setPointSizeFloat( style.layoutUnitToFontSize( height, false ) );

    painter.setFont( f );
    painter.drawText( style.layoutUnitToPixelX( x ),
                      style.layoutUnitToPixelY( y + getBaseline() ),
                      QString( QChar( cmChar ) ) );

    // Debug
#if 0
    QFontMetrics fm( f );
    LuPixelRect bound = fm.boundingRect( cmChar );
    painter.setBrush(Qt::NoBrush);
    painter.setPen(Qt::green);
    painter.drawRect( style.layoutUnitToPixelX( x ),
                      style.layoutUnitToPixelY( y ),
                      fm.width( cmChar ),
                      bound.height() );
#endif
}


KFORMULA_NAMESPACE_END
