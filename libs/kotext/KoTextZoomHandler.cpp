/* This file is part of the KDE project
   Copyright (C) 2001-2005 David Faure <faure@kde.org>

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

#include "KoTextZoomHandler.h"
#include <kdebug.h>
#include <qpaintdevice.h>
#include <KoUnit.h>
#include <KoGlobal.h>

// Layout text at 1440 DPI
// Well, not really always 1440 DPI, but always 20 times the point size
// This is constant, no need to litterally apply 1440 DPI at all resolutions.
int KoTextZoomHandler::m_layoutUnitFactor = 20;

#if 0
int KoTextZoomHandler::fontSizeToLayoutUnit( double ptSizeFloat, bool forPrint ) const
{
    return ptToLayoutUnit( ptSizeFloat / ( m_zoomedResolutionY *
        ( forPrint ? 1.0 : (72.0 / KoGlobal::dpiY()) ) ) );
}
#endif

double KoTextZoomHandler::layoutUnitToFontSize( int luSize, bool /*forPrint*/ ) const
{
    // Qt will use QPaintDevice::x11AppDpiY() to go from pt to pixel for fonts
    return layoutUnitPtToPt( luSize ) * m_zoomedResolutionY
#ifdef Q_WS_X11
        / POINT_TO_INCH(QPaintDevice::x11AppDpiY())
#endif
        ;
}

int KoTextZoomHandler::layoutUnitToPixelX( int x, int w ) const
{
    // We call layoutUnitToPixelX on the right value, i.e. x+w-1,
    // and then determine the height from the result (i.e. right-left+1).
    // Calling layoutUnitToPixelX(w) leads to rounding problems.
    return layoutUnitToPixelY( x + w - 1 ) - layoutUnitToPixelY( x ) + 1;
}

int KoTextZoomHandler::layoutUnitToPixelY( int y, int h ) const
{
    // We call layoutUnitToPixelY on the bottom value, i.e. y+h-1,
    // and then determine the height from the result (i.e. bottom-top+1).
    // Calling layoutUnitToPixelY(h) leads to rounding problems.
    return layoutUnitToPixelY( y + h - 1 ) - layoutUnitToPixelY( y ) + 1;
}

int KoTextZoomHandler::layoutUnitToPixelX( int lupix ) const
{
    return int( static_cast<double>( lupix * m_zoomedResolutionX )
                / ( static_cast<double>( m_layoutUnitFactor ) * m_resolutionX ) );
}

int KoTextZoomHandler::layoutUnitToPixelY( int lupix ) const
{
    // qRound replaced with a truncation, too many problems (e.g. bottom of parags)
    return int( static_cast<double>( lupix * m_zoomedResolutionY )
                / ( static_cast<double>( m_layoutUnitFactor ) * m_resolutionY ) );
}

int KoTextZoomHandler::pixelToLayoutUnitX( int x ) const
{
    return qRound( static_cast<double>( x * m_layoutUnitFactor * m_resolutionX )
                   / m_zoomedResolutionX );
}

int KoTextZoomHandler::pixelToLayoutUnitY( int y ) const
{
    return qRound( static_cast<double>( y * m_layoutUnitFactor * m_resolutionY )
                   / m_zoomedResolutionY );
}

