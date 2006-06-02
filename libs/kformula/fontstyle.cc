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
#include <QFontDatabase>
#include <QChar>
#include <kstaticdeleter.h>

#include "fontstyle.h"


KFORMULA_NAMESPACE_BEGIN

#include "unicodenames.cc"

void FontStyle::fillNameTable( SymbolTable::NameTable& names )
{
    for ( int i=0; nameTable[i].unicode != 0; ++i ) {
        names[QChar( nameTable[i].unicode )] = nameTable[i].name;
    }
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



// We claim that all chars come from the same font.
// It's up to the font tables to ensure this.
const QChar leftRoundBracket[] = {
    0xF8EB, // uppercorner
    0xF8ED, // lowercorner
    0xF8EC  // line
};
const QChar leftSquareBracket[] = {
    0xF8EE, // uppercorner
    0xF8F0, // lowercorner
    0xF8EF  // line
};
const QChar leftCurlyBracket[] = {
    0xF8F1, // uppercorner
    0xF8F3, // lowercorner
    0xF8F4, // line
    0xF8F2  // middle
};

const QChar leftLineBracket[] = {
    0xF8EF, // line
    0xF8EF, // line
    0xF8EF  // line
};
const QChar rightLineBracket[] = {
    0xF8FA, // line
    0xF8FA, // line
    0xF8FA  // line
};

const QChar rightRoundBracket[] = {
    0xF8F6, // uppercorner
    0xF8F8, // lowercorner
    0xF8F7  // line
};
const QChar rightSquareBracket[] = {
    0xF8F9, // uppercorner
    0xF8FB, // lowercorner
    0xF8FA  // line
};
const QChar rightCurlyBracket[] = {
    0xF8FC, // uppercorner
    0xF8FE, // lowercorner
    0xF8F4, // line
    0xF8FD  // middle
};


Artwork::Artwork(SymbolType t)
    : baseline( -1 ), type(t)
{
}


void Artwork::calcSizes( const ContextStyle& style,
                         ContextStyle::TextStyle tstyle )
{
    luPt mySize = style.getAdjustedSize( tstyle );
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
                   const ContextStyle& style, ContextStyle::TextStyle tstyle,
                   const LuPixelPoint& parentOrigin)
{
    luPt mySize = style.getAdjustedSize( tstyle );
    luPixel myX = parentOrigin.x() + getX();
    luPixel myY = parentOrigin.y() + getY();
    /*
    if ( !LuPixelRect( myX, myY, getWidth(), getHeight() ).intersects( r ) )
        return;
    */

    painter.setPen(style.getDefaultColor());

    switch (type) {
    case LeftSquareBracket:
        drawCharacter(painter, style, myX, myY, mySize, leftSquareBracketChar);
        break;
    case RightSquareBracket:
        drawCharacter(painter, style, myX, myY, mySize, rightSquareBracketChar);
        break;
    case LeftCurlyBracket:
        drawCharacter(painter, style, myX, myY, mySize, leftCurlyBracketChar);
        break;
    case RightCurlyBracket:
        drawCharacter(painter, style, myX, myY, mySize, rightCurlyBracketChar);
        break;
    case LeftLineBracket:
    case RightLineBracket:
        drawCharacter(painter, style, myX, myY, mySize, verticalLineChar);
        break;
    case SlashBracket:
        drawCharacter(painter, style, myX, myY, mySize, slashChar);
        break;
    case BackSlashBracket:
        drawCharacter(painter, style, myX, myY, mySize, backSlashChar);
        break;
    case LeftCornerBracket:
        drawCharacter(painter, style, myX, myY, mySize, leftAngleBracketChar);
        break;
    case RightCornerBracket:
        drawCharacter(painter, style, myX, myY, mySize, rightAngleBracketChar);
        break;
    case LeftRoundBracket:
        drawCharacter(painter, style, myX, myY, mySize, leftParenthesisChar);
        break;
    case RightRoundBracket:
        drawCharacter(painter, style, myX, myY, mySize, rightParenthesisChar);
        break;
    case EmptyBracket:
        break;
    case Integral:
    case Sum:
    case Product:
        break;
    }
}


void Artwork::calcCharSize( const ContextStyle& style, luPt height, QChar ch )
{
    //QFont f = style.getSymbolFont();
    QChar c = style.symbolTable().character( ch );
    QFont f = style.symbolTable().font( ch );
    calcCharSize( style, f, height, c );
}


void Artwork::drawCharacter( QPainter& painter, const ContextStyle& style,
                             luPixel x, luPixel y,
                             luPt height, QChar ch )
{
    QChar c = style.symbolTable().character( ch );
    QFont f = style.symbolTable().font( ch );
    drawCharacter( painter, style, f, x, y, height, c );
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
                      style.layoutUnitToPixelY( y+getBaseline() ), QString( c ) );
}


void Artwork::calcRoundBracket( const ContextStyle& style, const QChar chars[],
                                luPt height, luPt charHeight )
{
    QChar uppercorner = style.symbolTable().character( chars[0] );
    QChar lowercorner = style.symbolTable().character( chars[1] );
    //uchar line = style.symbolTable().character( chars[2] );

    QFont f = style.symbolTable().font( chars[0] );
    f.setPointSizeF( style.layoutUnitPtToPt( charHeight ) );
    QFontMetrics fm( f );
    LuPtRect upperBound = fm.boundingRect( uppercorner );
    LuPtRect lowerBound = fm.boundingRect( lowercorner );
    //LuPtRect lineBound = fm.boundingRect( line );

    setWidth( style.ptToLayoutUnitPt( fm.width( uppercorner ) ) );
    luPt edgeHeight = style.ptToLayoutUnitPt( upperBound.height()+lowerBound.height() );
    //luPt lineHeight = style.ptToLayoutUnitPt( lineBound.height() );

    //setHeight( edgeHeight + ( ( height-edgeHeight-1 ) / lineHeight + 1 ) * lineHeight );
    setHeight( qMax( edgeHeight, height ) );
}

void Artwork::drawBigRoundBracket( QPainter& p, const ContextStyle& style, const QChar chars[],
                                   luPixel x, luPixel y, luPt charHeight )
{
    QChar uppercorner = style.symbolTable().character( chars[0] );
    QChar lowercorner = style.symbolTable().character( chars[1] );
    QChar line = style.symbolTable().character( chars[2] );

    QFont f = style.symbolTable().font( chars[0] );
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

void Artwork::calcCurlyBracket( const ContextStyle& style, const QChar chars[],
                                luPt height, luPt charHeight )
{
    QChar uppercorner = style.symbolTable().character( chars[0] );
    QChar lowercorner = style.symbolTable().character( chars[1] );
    //uchar line = style.symbolTable().character( chars[2] );
    QChar middle = style.symbolTable().character( chars[3] );

    QFont f = style.symbolTable().font( chars[0] );
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

void Artwork::drawBigCurlyBracket( QPainter& p, const ContextStyle& style, const QChar chars[],
                                   luPixel x, luPixel y, luPt charHeight )
{
    //QFont f = style.getSymbolFont();
    QFont f = style.symbolTable().font( chars[0] );
    f.setPointSizeF( style.layoutUnitToFontSize( charHeight, false ) );
    p.setFont(f);

    QChar uppercorner = style.symbolTable().character( chars[0] );
    QChar lowercorner = style.symbolTable().character( chars[1] );
    QChar line = style.symbolTable().character( chars[2] );
    QChar middle = style.symbolTable().character( chars[3] );

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

    p.drawText( ptX, ptY-upperBound.top(), QString( uppercorner ) );
    p.drawText( ptX, ptY+(height-middleBound.height())/2-middleBound.top(),
                QString( middle ) );
    p.drawText( ptX, ptY+height-lowerBound.top()-lowerBound.height(),
                QString( lowercorner ) );

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
