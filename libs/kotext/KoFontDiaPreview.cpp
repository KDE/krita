/* This file is part of the KDE project
   Copyright (C)  2001, 2002 Montel Laurent <lmontel@mandrakesoft.com>

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

#include "KoFontDiaPreview.h"
#include "KoGlobal.h"
#include "KoTextFormat.h"

#include <klocale.h>

#include <qfontmetrics.h>
#include <qrect.h>
#include <qpainter.h>
#include <qfont.h>
#include <qstringlist.h>
#include <qstring.h>
#include <qregexp.h>
//Added by qt3to4:
#include <Q3Frame>

#include <math.h>

#include "KoFontDiaPreview.moc"

KoFontDiaPreview::KoFontDiaPreview( QWidget* parent, const char* name , Qt::WFlags fl )
        : Q3Frame( parent, name, fl )
        ,m_text( i18n( "The quick brown dog jumps over the lazy cat." ) )
        ,displayText( i18n( "The quick brown dog jumps over the lazy cat." ) )
        ,m_font( KoGlobal::defaultFont() )
        ,m_textColor( Qt::black )
        ,m_backgroundColor( Qt::white )
        ,m_shadowDistanceX( 0 )
        ,m_shadowDistanceY( 0 )
        ,m_shadowColor( Qt::black )
        ,m_underlining( 0 )
        ,m_underliningStyle( 0 )
        ,m_underliningColor( Qt::black )
        ,m_wordByWord( false )
        ,m_strikethrough( 0 )
        ,m_strikethroughStyle( 0 )
        ,m_capitalisation( 0 )
        ,m_subSuper( 0 )
        ,m_offset( 0 )
        ,m_relativeSize( 1 )

{
    setFrameStyle( Q3Frame::WinPanel | Q3Frame::Plain );
    setBackgroundMode( Qt::PaletteBase );
    setBackgroundColor( Qt::white );
    setMinimumSize( 400, 100 );
}

KoFontDiaPreview::~KoFontDiaPreview()
{
}

void KoFontDiaPreview::setText( const QString &text )
{
    m_text = text;
    update();
}

void KoFontDiaPreview::setFont( const QFont &font )
{
    m_font = font;
    m_fontSize = m_font.pointSize();
    update();
}

void KoFontDiaPreview::setFontColor( const QColor &textColor )
{
    m_textColor = textColor;
    update();
}

void KoFontDiaPreview::setBackgroundColor( const QColor &backgroundColor )
{
    m_backgroundColor = backgroundColor;
    update();
}

void KoFontDiaPreview::setShadow( double sdx, double sdy, QColor shadowColor )
{
    m_shadowDistanceX = sdx;
    m_shadowDistanceY = sdy;
    m_shadowColor = shadowColor;
    update();
}

void KoFontDiaPreview::setUnderlining( int underlining, int underliningStyle, const QColor underliningColor, bool wordByWord )
{
    m_underlining = underlining;
    m_underliningStyle = underliningStyle;
    m_underliningColor = underliningColor;
    m_wordByWord = wordByWord;
    update();
}

void KoFontDiaPreview::setWordByWord( bool wordByWord )
{
    m_wordByWord = wordByWord;
    update();
}

void KoFontDiaPreview::setStrikethrough( int strikethrough, int strikethroughStyle, bool wordByWord )
{
    m_strikethrough = strikethrough;
    m_strikethroughStyle = strikethroughStyle;
    m_wordByWord = wordByWord;
    update();
}

void KoFontDiaPreview::setCapitalisation( int capitalisation )
{
    m_capitalisation = capitalisation;
    update();
}

void KoFontDiaPreview::setSubSuperscript( int subSuper, int offset, double relativeSize )
{
    m_subSuper = subSuper;
    m_offset = offset;
    m_relativeSize = relativeSize;
    update();
}

QString KoFontDiaPreview::formatCapitalisation( const QString &string )
{
    switch ( m_capitalisation )
    {
    case KoTextFormat::ATT_NONE :
        return string;
    case KoTextFormat::ATT_UPPER :
        return string.upper();
    case KoTextFormat::ATT_LOWER :
        return string.toLower();
    case KoTextFormat::ATT_SMALL_CAPS :
        return string.upper();
    default:
        return string;
    }
}

void KoFontDiaPreview::drawContents( QPainter* p )
{
    p->save();

    // sort out the font to use

    //Capitalisation
    double capitalisationCoeff;
    QFontMetrics fmCapitalisation( m_font );

    switch ( m_capitalisation )
    {
    case KoTextFormat::ATT_NONE :
        capitalisationCoeff = 1.0;
        break;
    case KoTextFormat::ATT_UPPER :
        capitalisationCoeff = 1.0;
        break;
    case KoTextFormat::ATT_LOWER :
        capitalisationCoeff = 1.0;
        break;
    case KoTextFormat::ATT_SMALL_CAPS :
        capitalisationCoeff = ((double)fmCapitalisation.boundingRect("x").height()/(double)fmCapitalisation.boundingRect("X").height());
        break;
    default:
        capitalisationCoeff = 1.0;
        break;
    }
    //Set the display font. m_font is untouched by the modifications of capitalisation
    displayFont = m_font;
    displayFont.setPointSizeFloat( m_font.pointSize() * capitalisationCoeff );

// format the string in case Small Caps
    displayText = formatCapitalisation( m_text );

// draw the stuff
    QFontMetrics fm( displayFont );
    QRect br = fm.boundingRect( contentsRect().x(), contentsRect().y(), contentsRect().width(), contentsRect().height(), Qt::AlignCenter | Qt::TextWordWrap, displayText );

    if ( br.width() > contentsRect().width() || br.height() > contentsRect().height() ) {
        displayText = formatCapitalisation( i18n( "Font too large for the preview pane" ) );
        displayFont.setPointSizeFloat( 14 * capitalisationCoeff );
    }

    QFontMetrics fm1( displayFont );
    br = fm1.boundingRect( contentsRect().x(), contentsRect().y(), contentsRect().width(), contentsRect().height(), Qt::AlignCenter | Qt::TextWordWrap, displayText );

    int xorg = qRound( ( contentsRect().width() - br.width() ) / 2 ) + contentsRect().x() - fm1.leftBearing( displayText.at( 0 ) );

    // sub / superscript modifications
    int subSuperOffset = 0;
    switch ( m_subSuper ) {
        case 0: //normal
            displayFont.setPointSizeFloat( displayFont.pointSize() * m_relativeSize );
            subSuperOffset = -( m_offset );
            break;
        case 1: //subscript
            displayFont.setPointSizeFloat( displayFont.pointSize() * m_relativeSize );
            subSuperOffset = fm1.height() / 6;
            break;
        case 2: //superscript
            displayFont.setPointSizeFloat( displayFont.pointSize() * m_relativeSize );
            subSuperOffset = 0 - ( fm1.height() / 2 );
            break;
        default:
            displayFont.setPointSizeFloat( displayFont.pointSize() * m_relativeSize );
            subSuperOffset = 0 - m_offset;
            break;
    }

    QFontMetrics fm2( displayFont );
    br = fm2.boundingRect( contentsRect().x(), contentsRect().y(), contentsRect().width(), contentsRect().height(), Qt::AlignCenter | Qt::TextWordWrap, displayText );
    int yorg = qRound( ( contentsRect().height() - br.height() ) / 2 ) + fm1.ascent() + subSuperOffset;
    int sxorg = xorg + int( m_shadowDistanceX );
    int syorg = yorg + int( m_shadowDistanceY );
    QStringList textWords = QStringList::split( " ", displayText );
    int x = xorg;
    int y = yorg;
    int sx = sxorg;
    int sy = syorg;
    int bx= qMin( x, sx );
    int xend = bx;
    int yUnderline;
    int widthUnderline;
    int thicknessUnderline;
    int yStrikethrough;
    int widthStrikethrough;
    int thicknessStrikethrough;

    p->setFont(displayFont );
    p->setPen( m_textColor );
    int count = 1;

    for ( QStringList::iterator it = textWords.begin(); it != textWords.end(); ++it ) {
        int boffset = 0;
        if ( x + fm2.width( (*it) ) >  contentsRect().width() ) {
            y += fm1.lineSpacing();
            sy += fm1.lineSpacing();
            xend = x;
            x = xorg;
            sx = sxorg;
            bx= qMin( x, sx );
            count = 1;
        }
        QString textDraw;
        if ( (*it) == textWords.last() ) {
            textDraw = (*it);
        }
        else {
            textDraw = (*it) + " ";
        }
/*background*/
        if ( count == 1 ) boffset = QABS( int( m_shadowDistanceX ) );
        else boffset = 0;

        if ( bx < xend && (bx + fm2.width( textDraw ) + boffset ) < xend && ( qMin( y, sy ) - fm2.ascent() ) < ( qMin( yorg, syorg ) - fm2.ascent() + fm2.height() + QABS( m_shadowDistanceY ) ) ) {
            p->fillRect( bx, qMin( yorg, syorg ) - fm2.ascent() + fm2.height() + QABS( int( m_shadowDistanceY ) ), fm2.width( textDraw ) + boffset , fm2.height() + QABS( int( m_shadowDistanceY ) ) - ( qMin( yorg, syorg ) - qMin( y, sy ) + fm2.height() + QABS( int( m_shadowDistanceY ) ) ), m_backgroundColor );
        }
        else if ( bx < xend && (bx + fm2.width( textDraw ) + boffset ) >= xend && ( qMin( y, sy ) - fm2.ascent() ) < ( qMin( yorg, syorg ) - fm2.ascent() + fm2.height() + QABS( m_shadowDistanceY ) ) ) {
            p->fillRect( bx, qMin( yorg, syorg ) - fm2.ascent() + fm2.height() + QABS( int( m_shadowDistanceY ) ), xend - bx , fm2.height() + QABS( int( m_shadowDistanceY ) ) - ( qMin( yorg, syorg ) - qMin( y, sy ) + fm2.height() + QABS( int( m_shadowDistanceY ) ) ), m_backgroundColor );
            p->fillRect( xend, qMin( y, sy ) - fm2.ascent(), fm2.width( textDraw ) + boffset - xend + bx, fm2.height() + QABS( int( m_shadowDistanceY ) ), m_backgroundColor );
        }
        else {
            p->fillRect( bx, qMin( y, sy ) - fm2.ascent(), fm2.width( textDraw ) + boffset , fm2.height() + QABS( int( m_shadowDistanceY ) ), m_backgroundColor );
        }

        if ( count == 1 ) boffset = QABS( int( m_shadowDistanceX ) );
        else boffset = 0;
        bx += fm2.width( textDraw ) + boffset;//( count == 1 )?0:0;//QABS( m_shadowDistanceX ):0;
/*shadow*/
        if ( m_shadowDistanceX || m_shadowDistanceY )
        {
            p->save();
            p->setPen( m_shadowColor );
            p->drawText( sx, sy, textDraw );
            p->restore();
        }
/*text*/	
        p->drawText( x, y, textDraw );
/*underline*/	
        switch ( m_underlining ) {
            case KoTextFormat::U_NONE:
                break;
            case KoTextFormat::U_SIMPLE:
                yUnderline = y + fm2.descent();
                ( m_wordByWord )? widthUnderline = fm2.width( (*it) ): widthUnderline = fm2.width( textDraw );
                thicknessUnderline = 1;
                drawUnderline( x, yUnderline, widthUnderline, thicknessUnderline, m_underliningColor, p );
                break;
            case KoTextFormat::U_DOUBLE:
                yUnderline = y + fm2.descent();
                ( m_wordByWord )? widthUnderline = fm2.width( (*it) ): widthUnderline = fm2.width( textDraw );
                thicknessUnderline = 1;
                drawUnderline( x, yUnderline, widthUnderline, thicknessUnderline, m_underliningColor, p  );
                yUnderline = y + qRound( fm2.descent() / 2 );
                drawUnderline( x, yUnderline, widthUnderline, thicknessUnderline, m_underliningColor, p  );
                break;
            case KoTextFormat::U_SIMPLE_BOLD:
                yUnderline = y + fm2.descent();
                ( m_wordByWord )? widthUnderline = fm2.width( (*it) ): widthUnderline = fm2.width( textDraw );
                thicknessUnderline = qRound( displayFont.pointSize() / 10 ) + 1;
                drawUnderline( x, yUnderline, widthUnderline, thicknessUnderline, m_underliningColor, p );
                break;
            case KoTextFormat::U_WAVE:
                yUnderline = y + fm2.descent();
                ( m_wordByWord )? widthUnderline = fm2.width( (*it) ): widthUnderline = fm2.width( textDraw );
                thicknessUnderline = 1;
                drawUnderlineWave( x, yUnderline, widthUnderline, thicknessUnderline, m_underliningColor, p );
                break;
            default:
                break;
        }
/*Strikethrough*/
        switch ( m_strikethrough ) {
            case KoTextFormat::S_NONE:
                break;
            case KoTextFormat::S_SIMPLE:
                yStrikethrough = y - qRound( fm2.ascent() / 3 );
                ( m_wordByWord )? widthStrikethrough = fm2.width( (*it) ): widthStrikethrough = fm2.width( textDraw );
                thicknessStrikethrough = 1;
                drawStrikethrough( x, yStrikethrough, widthStrikethrough, thicknessStrikethrough, p );
                break;
            case KoTextFormat::S_DOUBLE:
                yStrikethrough = y - qRound( fm2.ascent() / 4 );
                ( m_wordByWord )? widthStrikethrough = fm2.width( (*it) ): widthStrikethrough = fm2.width( textDraw );
                thicknessStrikethrough = 1;
                drawStrikethrough( x, yStrikethrough, widthStrikethrough, thicknessStrikethrough, p  );
                yStrikethrough = y - 2 * qRound( fm2.ascent() / 4 );
                drawStrikethrough( x, yStrikethrough, widthStrikethrough, thicknessStrikethrough, p  );
                break;
            case KoTextFormat::S_SIMPLE_BOLD:
                yStrikethrough = y - qRound( fm2.ascent() / 3 );
                ( m_wordByWord )? widthStrikethrough = fm2.width( (*it) ): widthStrikethrough = fm2.width( textDraw );
                thicknessStrikethrough = qRound( displayFont.pointSize() / 10 ) + 1;
                drawStrikethrough( x, yStrikethrough, widthStrikethrough, thicknessStrikethrough, p );
                break;
            default:
                break;
        }
        x += fm2.width( textDraw );
        sx += fm2.width( textDraw );
        count++;
    }

    p->restore();
}

void KoFontDiaPreview::drawUnderline( int x, int y, int width, int thickness, QColor & color, QPainter *p )
{
    p->save();
    switch ( m_underliningStyle ) {
        case KoTextFormat::U_SOLID:
            p->setPen( QPen( color, thickness, Qt::SolidLine ) );
            break;
        case KoTextFormat::U_DASH:
            p->setPen( QPen( color, thickness, Qt::DashLine ) );
            break;
        case KoTextFormat::U_DOT:
            p->setPen( QPen( color, thickness, Qt::DotLine ) );
            break;
        case KoTextFormat::U_DASH_DOT:
            p->setPen( QPen( color, thickness, Qt::DashDotLine ) );
            break;
        case KoTextFormat::U_DASH_DOT_DOT:
            p->setPen( QPen( color, thickness, Qt::DashDotDotLine ) );
            break;
        default:
            p->setPen( QPen( color, thickness, Qt::SolidLine ) );
    }
    p->drawLine( x, y, x+ width, y );
    p->restore();
}

void KoFontDiaPreview::drawUnderlineWave( int x, int y, int width, int thickness, QColor & color, QPainter *p )
{
    p->save();
    int offset = 2 * thickness;
    QPen pen(color, thickness, Qt::SolidLine);
    pen.setCapStyle(Qt::RoundCap);
    p->setPen(pen);
    double anc=acos(1.0-2*(static_cast<double>(offset-(x)%offset)/static_cast<double>(offset)))/3.1415*180;
    int pos=1;
    //set starting position
    if(2*((x/offset)/2)==x/offset)
        pos*=-1;
    //draw first part of wave
    p->drawArc( (x/offset)*offset, y, offset, offset, 0, -qRound(pos*anc*16) );
    //now the main part
    int zigzag_x = (x/offset+1)*offset;
    for ( ; zigzag_x + offset <= width+x; zigzag_x += offset)
    {
        p->drawArc( zigzag_x, y, offset, offset, 0, pos*180*16 );
        pos*=-1;
    }
    //and here we finish
    anc=acos(1.0-2*(static_cast<double>((x+width)%offset)/static_cast<double>(offset)))/3.1415*180;
    p->drawArc( zigzag_x, y, offset, offset, 180*16, -qRound(pos*anc*16) );
    p->restore();
}

void KoFontDiaPreview::drawStrikethrough( int x, int y, int width, int thickness, QPainter *p )
{
    p->save();
    switch ( m_strikethroughStyle ) {
        case KoTextFormat::S_SOLID:
            p->setPen( QPen( Qt::black, thickness, Qt::SolidLine ) );
            break;
        case KoTextFormat::S_DASH:
            p->setPen( QPen( Qt::black, thickness, Qt::DashLine ) );
            break;
        case KoTextFormat::S_DOT:
            p->setPen( QPen( Qt::black, thickness, Qt::DotLine ) );
            break;
        case KoTextFormat::S_DASH_DOT:
            p->setPen( QPen( Qt::black, thickness, Qt::DashDotLine ) );
            break;
        case KoTextFormat::S_DASH_DOT_DOT:
            p->setPen( QPen( Qt::black, thickness, Qt::DashDotDotLine ) );
            break;
        default:
            p->setPen( QPen( Qt::black, thickness, Qt::SolidLine ) );
    }
     p->drawLine( x, y, x+ width, y );
    p->restore();
}

