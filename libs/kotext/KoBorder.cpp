/* This file is part of the KDE project
   Copyright (C) 2000, 2001 Thomas Zander <zander@kde.org>

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

#include "KoBorder.h"
#include <qdom.h>
#include <QByteArray>
#include <kdebug.h>
#include "KoZoomHandler.h"
#include "KoTextFormat.h"
#include "KoRichText.h" // for KoTextFormat
#include "KoTextParag.h"
#include <float.h>

static const struct BorderStyle {
    Qt::PenStyle penStyle;
    QByteArray oasisName;
    QByteArray uiStringStyle;
} s_borderStyles[] = {
    { Qt::SolidLine, "solid", "_________" }, // SOLID
    { Qt::DashLine, "dashed", "___ ___ __" }, // DASH
    { Qt::DotLine, "dotted", "_ _ _ _ _ _" }, // DOT
    { Qt::DashDotLine, "dot-dash", "___ _ ___ _" }, // DASH_DOT
    { Qt::DashDotDotLine, "dot-dot-dash", "___ _ _ ___" }, // DASH_DOT_DOT
    { Qt::SolidLine, "double", "===========" } // DOUBLE_LINE
};

KoBorder::KoBorder()
    : color(), m_style( SOLID )
{
    setPenWidth( 1 );
}

KoBorder::KoBorder( const QColor & c, BorderStyle s, double width )
    : color( c ), m_style( s )
{
    setPenWidth( width );
}

bool KoBorder::operator==( const KoBorder _brd ) const {
    return ( m_style == _brd.m_style && color == _brd.color && ptPenWidth == _brd.ptPenWidth );
}

bool KoBorder::operator!=( const KoBorder _brd ) const {
    return ( m_style != _brd.m_style || color != _brd.color || ptPenWidth != _brd.ptPenWidth );
}

void KoBorder::setStyle(BorderStyle _style)
{
    m_style = _style;
    setPenWidth( ptPenWidth );
}

void KoBorder::setPenWidth(double _w)
{
    ptPenWidth = _w;
    if ( m_style == KoBorder::DOUBLE_LINE && _w > 0 )
        ptWidth = 2 * ptPenWidth + 1;
    else
        ptWidth = _w;
}

QPen KoBorder::borderPen( const KoBorder & _brd, int width, QColor defaultColor )
{
    QPen pen( _brd.color, width );
    if ( !_brd.color.isValid() )
        pen.setColor( defaultColor );

    pen.setStyle( s_borderStyles[ _brd.m_style ].penStyle );

    return pen;
}

// KOffice-1.3 file format (deprecated)
KoBorder KoBorder::loadBorder( const QDomElement & elem )
{
    KoBorder bd;
    if ( elem.hasAttribute("red") )
    {
        int r = elem.attribute("red").toInt();
        int g = elem.attribute("green").toInt();
        int b = elem.attribute("blue").toInt();
        bd.color.setRgb( r, g, b );
    }
    bd.m_style = static_cast<BorderStyle>( elem.attribute("style").toInt() );
    bd.setPenWidth( elem.attribute("width").toDouble() );
    return bd;
}

void KoBorder::loadFoBorder( const QString& border )
{
    //string like "0.088cm solid #800000"

    if (border.isEmpty() || border=="none" || border=="hidden") { // in fact no border
        setPenWidth( 0 );
        return;
    }

    // ## isn't it faster to use QStringList::split than parse it 3 times?
    QString _width = border.section(' ', 0, 0);
    QByteArray _style = border.section(' ', 1, 1).latin1();
    QString _color = border.section(' ', 2, 2);

    //TODO: let the user choose a more precise border width (in the current unit)
    double const penWidth = KoUnit::parseValue( _width, 1.0 );
    //kDebug() << "penWidth:" << penWidth << endl;
    if ( penWidth < 1.5 )
        setPenWidth( 1.0 );
    else if ( penWidth < 2.5 )
        setPenWidth( 2.0 );
    else if ( penWidth < 3.5 )
        setPenWidth( 3.0 );
    else if ( penWidth < 4.5 )
        setPenWidth( 4.0 );
    else if ( penWidth < 5.5 )
        setPenWidth( 5.0 );
    else if ( penWidth < 6.5 )
        setPenWidth( 6.0 );
    else if ( penWidth < 7.5 )
        setPenWidth( 7.0 );
    else if ( penWidth < 8.5 )
        setPenWidth( 8.0 );
    else if ( penWidth < 9.5 )
        setPenWidth( 9.0 );
    else
        setPenWidth( 10.0 );

    m_style = SOLID;
    for ( uint i = 0; i < sizeof( s_borderStyles ) / sizeof *s_borderStyles; ++i ) {
        if ( _style == s_borderStyles[i].oasisName )
            m_style = static_cast<BorderStyle>( i );
    }

    if ( _color.isEmpty() )
        color = QColor();
    else
        color.setNamedColor( _color );
}

QString KoBorder::saveFoBorder() const
{
    if ( QABS( ptPenWidth ) < 1E-10 ) // i.e. ptPenWidth == 0
        return "none";
    //string like "2pt solid #800000"
    QString str = QString::number( ptPenWidth, 'g', DBL_DIG );
    str += "pt ";
    str += s_borderStyles[ m_style ].oasisName;
    if ( color.isValid() ) {
        str += ' ';
        str += color.name();
    }
    return str;
}

// KOffice-1.3 file format (deprecated)
void KoBorder::save( QDomElement & elem ) const
{
    if (color.isValid()) {
        elem.setAttribute("red", color.red());
        elem.setAttribute("green", color.green());
        elem.setAttribute("blue", color.blue());
    }
    elem.setAttribute("style", static_cast<int>( m_style ));
    elem.setAttribute("width", ptPenWidth);
}

KoBorder::BorderStyle KoBorder::getStyle( const QString &style )
{
    for ( uint i = 0; i < sizeof( s_borderStyles ) / sizeof *s_borderStyles; ++i ) {
        if ( style == s_borderStyles[i].uiStringStyle.data() )
            return static_cast<BorderStyle>( i );
    }
    // default
    return KoBorder::SOLID;
}

QString KoBorder::getStyle( const BorderStyle &style )
{
    return s_borderStyles[style].uiStringStyle;
}

int KoBorder::zoomWidthX( double ptWidth, KoZoomHandler * zoomHandler, int minborder )
{
    // If a border was set, then zoom it and apply a minimum of 1, so that it's always visible.
    // If no border was set, apply minborder ( 0 for paragraphs, 1 for frames )
    return ptWidth > 0 ? qMax( 1, zoomHandler->zoomItX( ptWidth ) /*applies qRound*/ ) : minborder;
}

int KoBorder::zoomWidthY( double ptWidth, KoZoomHandler * zoomHandler, int minborder )
{
    // If a border was set, then zoom it and apply a minimum of 1, so that it's always visible.
    // If no border was set, apply minborder ( 0 for paragraphs, 1 for frames )
    return ptWidth > 0 ? qMax( 1, zoomHandler->zoomItY( ptWidth ) /*applies qRound*/ ) : minborder;
}

void KoBorder::drawBorders( QPainter& painter, KoZoomHandler * zoomHandler, const QRect& rect, const KoBorder& leftBorder, const KoBorder& rightBorder, const KoBorder& topBorder, const KoBorder& bottomBorder, int minborder, const QPen& defaultPen, bool drawTopBorder /* = true */, bool drawBottomBorder /* = true */)
{
    int topBorderWidth = zoomWidthY( topBorder.width(), zoomHandler, minborder );
    int bottomBorderWidth = zoomWidthY( bottomBorder.width(), zoomHandler, minborder );
    int leftBorderWidth = zoomWidthX( leftBorder.width(), zoomHandler, minborder );
    int rightBorderWidth = zoomWidthX( rightBorder.width(), zoomHandler, minborder );

    int topBorderPenWidth = zoomWidthY( topBorder.penWidth(), zoomHandler, minborder );
    int bottomBorderPenWidth = zoomWidthY( bottomBorder.penWidth(), zoomHandler, minborder );
    int leftBorderPenWidth = zoomWidthX( leftBorder.penWidth(), zoomHandler, minborder );
    int rightBorderPenWidth = zoomWidthX( rightBorder.penWidth(), zoomHandler, minborder );

    // Wide pen don't draw the last pixel, so add one to the bottom and right coords
    int lastPixelAdj = 1;

    //kDebug(32500) << "KoBorder::drawBorders widths: top=" << topBorderWidth << " bottom=" << bottomBorderWidth
    //               << " left=" << leftBorderWidth << " right=" << rightBorderWidth << endl;

    //kDebug(32500) << "                   penWidths: top=" << topBorderPenWidth << " bottom=" << bottomBorderPenWidth
    //               << " left=" << leftBorderPenWidth << " right=" << rightBorderPenWidth << endl;

    QColor defaultColor = KoTextFormat::defaultTextColor( &painter );

    if ( topBorderWidth > 0 && drawTopBorder )
    {
        if ( topBorder.penWidth() > 0 )
            painter.setPen( KoBorder::borderPen( topBorder, topBorderPenWidth, defaultColor ) );
        else
            painter.setPen( defaultPen );
        int y = rect.top() - topBorderWidth + topBorderPenWidth/2;
        if ( topBorder.m_style==KoBorder::DOUBLE_LINE)
        {
            painter.drawLine( rect.left()-leftBorderWidth, y, rect.right()+2*(rightBorderPenWidth+lastPixelAdj), y );
            y += topBorderPenWidth + 1;
            painter.drawLine( rect.left()-leftBorderPenWidth, y, rect.right()+rightBorderPenWidth+lastPixelAdj, y );
        }
        else
        {
            painter.drawLine( rect.left()-leftBorderWidth, y, rect.right()+rightBorderWidth+lastPixelAdj, y );
        }
    }
    if ( bottomBorderWidth > 0 && drawBottomBorder )
    {
        if ( bottomBorder.penWidth() > 0 )
            painter.setPen( KoBorder::borderPen( bottomBorder, bottomBorderPenWidth, defaultColor ) );
        else
            painter.setPen( defaultPen );
	//kDebug(32500) << "bottomBorderWidth=" << bottomBorderWidth << " bottomBorderWidth/2=" << (int)bottomBorderWidth/2 << endl;
        int y = rect.bottom() + bottomBorderPenWidth/2 + 1;
	//kDebug(32500) << "   -> bottom=" << rect.bottom() << " y=" << y << endl;
        if ( bottomBorder.m_style==KoBorder::DOUBLE_LINE)
        {
            painter.drawLine( rect.left()-leftBorderPenWidth, y, rect.right()+rightBorderPenWidth+lastPixelAdj, y );
            y += bottomBorderPenWidth + 1;
            painter.drawLine( rect.left()-leftBorderWidth, y, rect.right()+2*(rightBorderPenWidth+lastPixelAdj), y );
        }
        else
        {
            painter.drawLine( rect.left()-leftBorderWidth, y, rect.right()+rightBorderWidth+lastPixelAdj, y );
        }
    }
    if ( leftBorderWidth > 0 )
    {
        if ( leftBorder.penWidth() > 0 )
            painter.setPen( KoBorder::borderPen( leftBorder, leftBorderPenWidth, defaultColor ) );
        else
            painter.setPen( defaultPen );
        int x = rect.left() - leftBorderWidth + leftBorderPenWidth/2;
        if ( leftBorder.m_style==KoBorder::DOUBLE_LINE)
        {
            painter.drawLine( x, rect.top()-topBorderWidth, x, rect.bottom()+2*(bottomBorderPenWidth+lastPixelAdj) );
            x += leftBorderPenWidth + 1;
            painter.drawLine( x, rect.top()-topBorderPenWidth, x, rect.bottom()+bottomBorderPenWidth+lastPixelAdj );
        }
        else
        {
            int yTop = rect.top() - topBorderWidth;
            int yBottom = rect.bottom() + bottomBorderWidth;
            /*kDebug(32500) << " pen=" << painter.pen() << " rect=" << rect << " topBorderWidth=" << topBorderWidth
                           << " painting from " << x << "," << yTop
                           << " to " << x << "," << yBottom << endl;*/
            painter.drawLine( x, yTop, x, yBottom+lastPixelAdj );
        }
    }
    if ( rightBorderWidth > 0 )
    {
        if ( rightBorder.penWidth() > 0 )
            painter.setPen( KoBorder::borderPen( rightBorder, rightBorderPenWidth, defaultColor ) );
        else
            painter.setPen( defaultPen );
        int x = rect.right() + rightBorderPenWidth/2 + 1;
        if ( rightBorder.m_style==KoBorder::DOUBLE_LINE)
        {
            painter.drawLine( x, rect.top()-topBorderPenWidth, x, rect.bottom()+bottomBorderPenWidth+lastPixelAdj );
            x += rightBorderPenWidth + 1;
            painter.drawLine( x, rect.top()-topBorderWidth, x, rect.bottom()+2*(bottomBorderPenWidth+lastPixelAdj) );

        }
        else
        {
            int yTop = rect.top()-topBorderWidth;
            int yBottom = rect.bottom()+bottomBorderWidth+lastPixelAdj;
            /*kDebug(32500) << " pen=" << painter.pen() << " rect=" << rect << " topBorderWidth=" << topBorderWidth
                           << " painting from " << x << "," << yTop
                           << " to " << x << "," << yBottom << endl;*/
            painter.drawLine( x, yTop, x, yBottom );
        }
    }
}
