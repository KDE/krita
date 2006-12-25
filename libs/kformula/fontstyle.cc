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

#include <QPainter>
#include <QPen>
#include <QFontDatabase>
#include <QChar>
#include <QApplication>

#include <kstaticdeleter.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <kio/job.h>
#include <kmessagebox.h>

#include "fontstyle.h"


KFORMULA_NAMESPACE_BEGIN

#include "unicodetable.cc"

bool FontStyle::m_installed = false;

bool FontStyle::init( ContextStyle* style, bool install )
{
    if (!m_installed && install)
        installFonts();
    m_symbolTable.init( style->getMathFont() );

    return true;
}

// Cache the family list from QFontDatabase after fixing it up (no foundry, lowercase)
class FontList {
public:
    FontList() {
        QFontDatabase db;
        const QStringList lst = db.families();
        for ( QStringList::const_iterator it = lst.begin(), end = lst.end() ; it != end ; ++it ) {
            const QString name = *it;
            int i = name.indexOf('[');
            QString family = name;
            // Remove foundry
            if ( i > -1 ) {
                const int li = name.lastIndexOf(']');
                if (i < li) {
                    if (name[i - 1] == ' ')
                        i--;
                    family = name.left(i);
                }
            }
            m_fontNames.append( family.toLower() );
        }
    }
    bool hasFont( const QString& fontName ) const {
        return m_fontNames.contains( fontName );
    }
    QStringList m_fontNames;
};
static FontList* s_fontList = 0;
static KStaticDeleter<FontList> s_fontList_sd;

void FontStyle::testFont( QStringList& missing, const QString& fontName ) {
    if ( !s_fontList )
        s_fontList_sd.setObject( s_fontList, new FontList() );
    if ( !s_fontList->hasFont( fontName ) )  {
        kWarning(39001) << "Font '" << fontName << "' not found" << endl;
        missing.append( fontName );
    }
}


QStringList FontStyle::missingFonts( bool install )
{
    if (!m_installed && install)
        installFonts();

    QStringList missing = missingFontsInternal();
    return missing;
}

QStringList FontStyle::missingFontsInternal()
{
    QStringList missing;

    testFont( missing, "cmex10" );
    testFont( missing, "arev sans");

    return missing;
}

void FontStyle::installFonts()
{
    if (m_installed)
        return;
    QStringList missing = missingFontsInternal();
    if (!missing.isEmpty())
    {
        QStringList urlList;
        for (QStringList::iterator it = missing.begin(); it != missing.end(); ++it)
        {
            if ( *it == "arev sans" ) {
                if ( ! KIO::NetAccess::exists( KUrl( "fonts:/Personal/Arev.ttf" ), true, NULL) )
                    urlList.append( KStandardDirs::locate( "data", "kformula/fonts/Arev.ttf" ) );
                if ( ! KIO::NetAccess::exists( KUrl( "fonts:/Personal/ArevIt.ttf" ), true, NULL ) )
                    urlList.append( KStandardDirs::locate( "data", "kformula/fonts/ArevIt.ttf" ) );
                if ( ! KIO::NetAccess::exists( KUrl( "fonts:/Personal/ArevBd.ttf" ), true, NULL ) )
                    urlList.append( KStandardDirs::locate( "data", "kformula/fonts/ArevBd.ttf" ) );
                if ( ! KIO::NetAccess::exists( KUrl( "fonts:/Personal/ArevBI.ttf" ), true, NULL) )
                    urlList.append( KStandardDirs::locate( "data", "kformula/fonts/ArevBI.ttf" ) );
            }
            else {
                if ( ! KIO::NetAccess::exists( KUrl( "fonts:/Personal/" + *it + ".ttf" ), true, NULL ) )
                    urlList.append( KStandardDirs::locate( "data", "kformula/fonts/" + *it + ".ttf" ) );
            }
        }
        KIO::copy( urlList, KUrl( "fonts:/Personal/" ), false );
        KMessageBox::information(qApp->mainWidget(), 
                                 i18n("Some fonts have been installed to assure that symbols in formulas are properly visualized. You must restart the application in order so that changes take effect"));
    }
    m_installed = true;
}

Artwork* FontStyle::createArtwork( SymbolType type ) const
{
    return new Artwork( type );
}

// We claim that all chars come from the same font.
// It's up to the font tables to ensure this.
const uchar leftRoundBracket[] = {
    0x30, // uppercorner
    0x40, // lowercorner
    0x42  // line
};
const uchar leftSquareBracket[] = {
    0x32, // uppercorner
    0x34, // lowercorner
    0x36  // line
};
const uchar leftCurlyBracket[] = {
    0x38, // uppercorner
    0x3A, // lowercorner
    0x3E, // line
    0x3C  // middle
};

const uchar leftLineBracket[] = {
    0x36, // line
    0x36, // line
    0x36  // line
};
const uchar rightLineBracket[] = {
    0x37, // line
    0x37, // line
    0x37  // line
};

const uchar rightRoundBracket[] = {
    0x31, // uppercorner
    0x41, // lowercorner
    0x43  // line
};
const uchar rightSquareBracket[] = {
    0x33, // uppercorner
    0x35, // lowercorner
    0x37  // line
};
const uchar rightCurlyBracket[] = {
    0x39, // uppercorner
    0x3B, // lowercorner
    0x3E, // line
    0x3D  // middle
};


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

bool Artwork::calcCMDelimiterSize( const ContextStyle& context,
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


void Artwork::calcLargest( const ContextStyle& context,
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


void Artwork::drawCMDelimiter( QPainter& painter, const ContextStyle& style,
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


Artwork::Artwork(SymbolType t)
    : baseline( -1 ), type(t)
{
}


void Artwork::calcSizes( const ContextStyle& style,
                           ContextStyle::TextStyle tstyle,
                           double factor,
                           luPt parentSize )
{
    setBaseline( -1 );
    cmChar = -1;
    luPt mySize = style.getAdjustedSize( tstyle, factor );
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
        calcCharSize( style, style.getBracketFont(), mySize, cmex_Int );
        break;
    case Sum:
        calcCharSize( style, style.getBracketFont(), mySize, cmex_Sum );
        break;
    case Product:
        calcCharSize( style, style.getBracketFont(), mySize, cmex_Prod );
        break;
    }
}

void Artwork::calcSizes( const ContextStyle& style,
                         ContextStyle::TextStyle tstyle,
                         double factor )
{
    luPt mySize = style.getAdjustedSize( tstyle, factor );
    switch (type) {
    case LeftSquareBracket:
        calcCharSize(style, mySize, leftSquareBracketChar);
        break;
    case RightSquareBracket:
        calcCharSize(style, mySize, rightSquareBracketChar);
        break;
    case LeftLineBracket:
    case RightLineBracket:
        calcCharSize(style, mySize, verticalLineChar);
        break;
    case SlashBracket:
        calcCharSize(style, mySize, slashChar);
        break;
    case BackSlashBracket:
        calcCharSize(style, mySize, backSlashChar);
        break;
    case LeftCornerBracket:
        calcCharSize(style, mySize, leftAngleBracketChar);
        break;
    case RightCornerBracket:
        calcCharSize(style, mySize, rightAngleBracketChar);
        break;
    case LeftRoundBracket:
        calcCharSize(style, mySize, leftParenthesisChar);
        break;
    case RightRoundBracket:
        calcCharSize(style, mySize, rightParenthesisChar);
        break;
    case EmptyBracket:
        //calcCharSize(style, mySize, spaceChar);
        setHeight(0);
        //setWidth(style.getEmptyRectWidth());
        setWidth(0);
        break;
    case LeftCurlyBracket:
        calcCharSize(style, mySize, leftCurlyBracketChar);
        break;
    case RightCurlyBracket:
        calcCharSize(style, mySize, rightCurlyBracketChar);
        break;
    case Integral:
    case Sum:
    case Product:
        break;
    }
}


void Artwork::draw(QPainter& painter, const LuPixelRect& /*r*/,
                   const ContextStyle& context, ContextStyle::TextStyle tstyle,
                   StyleAttributes& style, const LuPixelPoint& parentOrigin)
{
    luPt mySize = context.getAdjustedSize( tstyle, style.sizeFactor() );
    luPixel myX = parentOrigin.x() + getX();
    luPixel myY = parentOrigin.y() + getY();
    /*
    if ( !LuPixelRect( myX, myY, getWidth(), getHeight() ).intersects( r ) )
        return;
    */

    painter.setPen(context.getDefaultColor());

    switch (type) {
    case LeftSquareBracket:
        drawCharacter(painter, context, myX, myY, mySize, leftSquareBracketChar);
        break;
    case RightSquareBracket:
        drawCharacter(painter, context, myX, myY, mySize, rightSquareBracketChar);
        break;
    case LeftCurlyBracket:
        drawCharacter(painter, context, myX, myY, mySize, leftCurlyBracketChar);
        break;
    case RightCurlyBracket:
        drawCharacter(painter, context, myX, myY, mySize, rightCurlyBracketChar);
        break;
    case LeftLineBracket:
    case RightLineBracket:
        drawCharacter(painter, context, myX, myY, mySize, verticalLineChar);
        break;
    case SlashBracket:
        drawCharacter(painter, context, myX, myY, mySize, slashChar);
        break;
    case BackSlashBracket:
        drawCharacter(painter, context, myX, myY, mySize, backSlashChar);
        break;
    case LeftCornerBracket:
        drawCharacter(painter, context, myX, myY, mySize, leftAngleBracketChar);
        break;
    case RightCornerBracket:
        drawCharacter(painter, context, myX, myY, mySize, rightAngleBracketChar);
        break;
    case LeftRoundBracket:
        drawCharacter(painter, context, myX, myY, mySize, leftParenthesisChar);
        break;
    case RightRoundBracket:
        drawCharacter(painter, context, myX, myY, mySize, rightParenthesisChar);
        break;
    case EmptyBracket:
        break;
    case Integral:
    case Sum:
    case Product:
        break;
    }
}


void Artwork::draw(QPainter& painter, const LuPixelRect& ,
                     const ContextStyle& context, ContextStyle::TextStyle tstyle,
                     StyleAttributes& style, luPt , const LuPixelPoint& origin)
{
    luPt mySize = context.getAdjustedSize( tstyle, style.sizeFactor() );
    luPixel myX = origin.x() + getX();
    luPixel myY = origin.y() + getY();
    /*
    if ( !LuPixelRect( myX, myY, getWidth(), getHeight() ).intersects( r ) )
        return;
    */

    painter.setPen(context.getDefaultColor());

    switch (getType()) {
    case LeftSquareBracket:
        if ( cmChar != -1 ) {
            drawCMDelimiter( painter, context, myX, myY, mySize );
        }
        else {
            drawBigRoundBracket( painter, context, leftSquareBracket, myX, myY, mySize );
        }
        break;
    case RightSquareBracket:
        if ( cmChar != -1 ) {
            drawCMDelimiter( painter, context, myX, myY, mySize );
        }
        else {
            drawBigRoundBracket( painter, context, rightSquareBracket, myX, myY, mySize );
        }
        break;
    case LeftCurlyBracket:
        if ( cmChar != -1 ) {
            drawCMDelimiter( painter, context, myX, myY, mySize );
        }
        else {
            drawBigCurlyBracket( painter, context, leftCurlyBracket, myX, myY, mySize );
        }
        break;
    case RightCurlyBracket:
        if ( cmChar != -1 ) {
            drawCMDelimiter( painter, context, myX, myY, mySize );
        }
        else {
            drawBigCurlyBracket( painter, context, rightCurlyBracket, myX, myY, mySize );
        }
        break;
    case LeftLineBracket: {
        luPixel halfWidth = getWidth()/2;
        drawBigRoundBracket( painter, context, leftLineBracket,
                             myX-halfWidth, myY, mySize );
    }
        break;
    case RightLineBracket: {
        luPixel halfWidth = getWidth()/2;
        drawBigRoundBracket( painter, context, rightLineBracket,
                             myX-halfWidth, myY, mySize );
    }
        break;
    case SlashBracket:
        if ( cmChar != -1 ) {
            drawCMDelimiter( painter, context, myX, myY, mySize );
        }
        break;
    case BackSlashBracket:
        if ( cmChar != -1 ) {
            drawCMDelimiter( painter, context, myX, myY, mySize );
        }
        break;
    case LeftCornerBracket:
        if ( cmChar != -1 ) {
            drawCMDelimiter( painter, context, myX, myY, mySize );
        }
        else drawCharacter(painter, context, myX, myY, mySize, leftAngleBracketChar);
        break;
    case RightCornerBracket:
        if ( cmChar != -1 ) {
            drawCMDelimiter( painter, context, myX, myY, mySize );
        }
        else drawCharacter(painter, context, myX, myY, mySize, rightAngleBracketChar);
        break;
    case LeftRoundBracket:
        if ( cmChar != -1 ) {
            drawCMDelimiter( painter, context, myX, myY, mySize );
        }
        else {
            drawBigRoundBracket( painter, context, leftRoundBracket, myX, myY, mySize );
        }
        break;
    case RightRoundBracket:
        if ( cmChar != -1 ) {
            drawCMDelimiter( painter, context, myX, myY, mySize );
        }
        else {
            drawBigRoundBracket( painter, context, rightRoundBracket, myX, myY, mySize );
        }
        break;
    case EmptyBracket:
        break;
    case Integral:
        drawCharacter(painter, context, QFont( "cmex10" ), myX, myY, mySize, cmex_Int);
        break;
    case Sum:
        drawCharacter(painter, context, QFont( "cmex10" ), myX, myY, mySize, cmex_Sum);
        break;
    case Product:
        drawCharacter(painter, context, QFont( "cmex10" ), myX, myY, mySize, cmex_Prod);
        break;
    }

    // debug
//     painter.setBrush(Qt::NoBrush);
//     painter.setPen(Qt::green);
//     painter.drawRect( context.layoutUnitToPixelX( myX ),
//                       context.layoutUnitToPixelY( myY ),
//                       context.layoutUnitToPixelX( getWidth() ),
//                       context.layoutUnitToPixelY( getHeight() ) );
}

void Artwork::calcCharSize( const ContextStyle& style, luPt height, QChar ch )
{
    calcCharSize( style, style.getMathFont(), height, ch );
}


void Artwork::drawCharacter( QPainter& painter, const ContextStyle& style,
                             luPixel x, luPixel y,
                             luPt height, QChar ch )
{
    drawCharacter( painter, style, style.getMathFont(), x, y, height, ch );
}


void Artwork::calcCharSize( const ContextStyle& style, QFont f,
                            luPt height, QChar c )
{
    f.setPointSizeF( style.layoutUnitPtToPt( height ) );
    //f.setPointSize( height );
    QFontMetrics fm(f);
    setWidth( style.ptToLayoutUnitPt( fm.width( c ) ) );
    LuPixelRect bound = fm.boundingRect( c );
    setHeight( style.ptToLayoutUnitPt( bound.height() ) );
    setBaseline( style.ptToLayoutUnitPt( -bound.top() ) );
}


void Artwork::drawCharacter( QPainter& painter, const ContextStyle& style,
                             QFont f,
                             luPixel x, luPixel y, luPt height, QChar c )
{
    f.setPointSizeF( style.layoutUnitToFontSize( height, false ) );

    painter.setFont( f );
    painter.drawText( style.layoutUnitToPixelX( x ),
                      style.layoutUnitToPixelY( y+getBaseline() ), 
                      QString( c ) );
}


void Artwork::calcRoundBracket( const ContextStyle& style, const uchar chars[],
                                luPt height, luPt charHeight )
{
    uchar uppercorner = chars[0];
    uchar lowercorner = chars[1];
    //uchar line = style.symbolTable().character( chars[2] );

    QFont f = style.getBracketFont();
    f.setPointSizeF( style.layoutUnitPtToPt( charHeight ) );
    QFontMetrics fm( f );
    LuPtRect upperBound = fm.boundingRect( uppercorner );
    LuPtRect lowerBound = fm.boundingRect( lowercorner );
    //LuPtRect lineBound = fm.boundingRect( line );

    setWidth( style.ptToLayoutUnitPt( fm.width( QChar ( uppercorner ) ) ) );
    luPt edgeHeight = style.ptToLayoutUnitPt( upperBound.height()+lowerBound.height() );
    //luPt lineHeight = style.ptToLayoutUnitPt( lineBound.height() );

    //setHeight( edgeHeight + ( ( height-edgeHeight-1 ) / lineHeight + 1 ) * lineHeight );
    setHeight( qMax( edgeHeight, height ) );
}

void Artwork::drawBigRoundBracket( QPainter& p, const ContextStyle& style, const uchar chars[],
                                   luPixel x, luPixel y, luPt charHeight )
{
    uchar uppercorner = chars[0];
    uchar lowercorner = chars[1];
    uchar line = chars[2];

    QFont f = style.getBracketFont();
    f.setPointSizeF( style.layoutUnitToFontSize( charHeight, false ) );
    p.setFont(f);

    QFontMetrics fm(f);
    QRect upperBound = fm.boundingRect(uppercorner);
    QRect lowerBound = fm.boundingRect(lowercorner);
    QRect lineBound = fm.boundingRect(line);

    pixel ptX = style.layoutUnitToPixelX( x );
    pixel ptY = style.layoutUnitToPixelY( y );
    pixel height = style.layoutUnitToPixelY( getHeight() );

//     p.setPen( Qt::red );
//     //p.drawRect( ptX, ptY, upperBound.width(), upperBound.height() + lowerBound.height() );
//     p.drawRect( ptX, ptY, style.layoutUnitToPixelX( getWidth() ),
//                 style.layoutUnitToPixelY( getHeight() ) );

//     p.setPen( Qt::black );
    p.drawText( ptX, ptY-upperBound.top(), QString( QChar( uppercorner ) ) );
    p.drawText( ptX, ptY+height-lowerBound.top()-lowerBound.height(),
                QString( QChar( lowercorner ) ) );

    // for printing
    //pt safety = lineBound.height() / 10.0;
    pixel safety = 0;

    pixel gap = height - upperBound.height() - lowerBound.height();
    pixel lineHeight = lineBound.height() - safety;
    int lineCount = qRound( static_cast<double>( gap ) / lineHeight );
    pixel start = upperBound.height()-lineBound.top() - safety;

    for (int i = 0; i < lineCount; i++) {
        p.drawText( ptX, ptY+start+i*lineHeight, QString(QChar(line)));
    }
    pixel remaining = gap - lineCount*lineHeight;
    pixel dist = ( lineHeight - remaining ) / 2;
    p.drawText( ptX, ptY+height-upperBound.height()+dist-lineBound.height()-lineBound.top(),
                QString( QChar( line ) ) );
}

void Artwork::calcCurlyBracket( const ContextStyle& style, const uchar chars[],
                                luPt height, luPt charHeight )
{
    uchar uppercorner = chars[0];
    uchar lowercorner = chars[1];
    //uchar line = style.symbolTable().character( chars[2] );
    uchar middle = chars[3];

    QFont f = style.getBracketFont();
    f.setPointSizeF( style.layoutUnitPtToPt( charHeight ) );
    QFontMetrics fm( f );
    LuPtRect upperBound = fm.boundingRect( uppercorner );
    LuPtRect lowerBound = fm.boundingRect( lowercorner );
    //LuPtRect lineBound = fm.boundingRect( line );
    LuPtRect middleBound = fm.boundingRect( middle );

    setWidth( style.ptToLayoutUnitPt( fm.width( QChar( uppercorner ) ) ) );
    luPt edgeHeight = style.ptToLayoutUnitPt( upperBound.height()+
                                              lowerBound.height()+
                                              middleBound.height() );
    //luPt lineHeight = style.ptToLayoutUnitPt( lineBound.height() );

    //setHeight( edgeHeight + ( ( height-edgeHeight-1 ) / lineHeight + 1 ) * lineHeight );
    setHeight( qMax( edgeHeight, height ) );
}

void Artwork::drawBigCurlyBracket( QPainter& p, const ContextStyle& style, const uchar chars[],
                                   luPixel x, luPixel y, luPt charHeight )
{
    //QFont f = style.getSymbolFont();
    QFont f = style.getBracketFont();
    f.setPointSizeF( style.layoutUnitToFontSize( charHeight, false ) );
    p.setFont(f);

    uchar uppercorner = chars[0];
    uchar lowercorner = chars[1];
    uchar line = chars[2];
    uchar middle = chars[3];

    QFontMetrics fm(p.fontMetrics());
    QRectF upperBound = fm.boundingRect(uppercorner);
    QRectF lowerBound = fm.boundingRect(lowercorner);
    QRectF middleBound = fm.boundingRect(middle);
    QRectF lineBound = fm.boundingRect(line);

    pixel ptX = style.layoutUnitToPixelX( x );
    pixel ptY = style.layoutUnitToPixelY( y );
    pixel height = style.layoutUnitToPixelY( getHeight() );

    //p.setPen(Qt::gray);
    //p.drawRect(x, y, upperBound.width() + offset, height);

    p.drawText( ptX, ptY-upperBound.top(), QString( QChar ( uppercorner ) ) );
    p.drawText( ptX, ptY+(height-middleBound.height())/2-middleBound.top(),
                QString( QChar( middle ) ) );
    p.drawText( ptX, ptY+height-lowerBound.top()-lowerBound.height(),
                QString( QChar( lowercorner ) ) );

    // for printing
    // If the world was perfect and the urw-symbol font correct
    // this could be 0.
    //lu safety = lineBound.height() / 10;
    double safety = 0;

    double lineHeight = lineBound.height() - safety;
    double gap = height/2 - upperBound.height() - middleBound.height() / 2;

    if (gap > 0) {
        QString ch = QString(QChar(line));
        int lineCount = qRound( gap / lineHeight ) + 1;

        double start = (height - middleBound.height()) / 2 + safety;
        for (int i = 0; i < lineCount; i++) {
            p.drawText( ptX, ptY-lineBound.top()+qMax( start-(i+1)*lineHeight,
                                                       upperBound.width() ),
                        ch );
        }

        start = (height + middleBound.height()) / 2 - safety;
        for (int i = 0; i < lineCount; i++) {
            p.drawText( ptX, ptY-lineBound.top()+qMin( start+i*lineHeight,
                                                       height-upperBound.width()-lineBound.height() ),
                        ch );
        }
    }
}

KFORMULA_NAMESPACE_END
