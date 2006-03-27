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

#ifndef kotextzoomhandler_h
#define kotextzoomhandler_h

#include <KoZoomHandler.h>

/**
 * This class extends KoZoomHandler to add support for WYSIWYG text layouting.
 */
class KOTEXT_EXPORT KoTextZoomHandler : public KoZoomHandler
{
public:
    KoTextZoomHandler() {}
    virtual ~KoTextZoomHandler() {}

    //// Support for WYSIWYG text layouting /////

    /** The "[zoomed] view pixel" -> "layout unit pixel" conversions. */
    int pixelToLayoutUnitX( int x ) const;
    int pixelToLayoutUnitY( int y ) const;
    QPoint pixelToLayoutUnit( const QPoint &p ) const
    { return QPoint( pixelToLayoutUnitX( p.x() ),
                     pixelToLayoutUnitY( p.y() ) ); }
    QRect pixelToLayoutUnit( const QRect &r ) const
    { return QRect( pixelToLayoutUnit( r.topLeft() ),
                    pixelToLayoutUnit( r.bottomRight() ) ); }

    /** The "layout unit pixel" -> "[zoomed] view pixel" conversions. */
    int layoutUnitToPixelX( int lupix ) const;
    int layoutUnitToPixelY( int lupix ) const;

    /** This variant converts a width, using a reference X position.
     * This prevents rounding problems. */
    int layoutUnitToPixelX( int x, int w ) const;
    /** This variant converts a height, using a reference Y position.
     * This prevents rounding problems. */
    int layoutUnitToPixelY( int y, int h ) const;

    QPoint layoutUnitToPixel( const QPoint &p ) const
    { return QPoint( layoutUnitToPixelX( p.x() ),
                     layoutUnitToPixelY( p.y() ) ); }
    QRect layoutUnitToPixel( const QRect &r ) const
    { return QRect( layoutUnitToPixel( r.topLeft() ),
                    layoutUnitToPixel( r.bottomRight() ) ); }

    /** Basic pt to pixel and pixel to pt conversions, valid at any zoom level,
        as well as at the Layout Unit level (and mostly useful for Layout Units).
        Don't confuse with zoomIt, which also converts pt to pixels, but applying the zoom! */
    int ptToPixelX( double pt ) const
    { return qRound( pt * m_resolutionX ); }
    int ptToPixelY( double pt ) const
    { return qRound( pt * m_resolutionY ); }
    QPoint ptToPixel( const KoPoint & p ) const {
        return QPoint( ptToPixelX( p.x() ), ptToPixelY( p.y() ) );
    }
    double pixelXToPt( int x ) const
    { return static_cast<double>(x) / m_resolutionX; }
    double pixelYToPt( int y ) const
    { return static_cast<double>(y) / m_resolutionY; }
    KoPoint pixelToPt( const QPoint& p ) const {
        return KoPoint( pixelXToPt( p.x() ), pixelYToPt( p.y() ) );
    }

    /** The "document pt" -> "Layout Unit pixels" conversions, for convenience */
    int ptToLayoutUnitPixX( double x_pt ) const
    { return ptToPixelX( ptToLayoutUnitPt( x_pt ) ); }
    int ptToLayoutUnitPixY( double y_pt ) const
    { return ptToPixelY( ptToLayoutUnitPt( y_pt ) ); }
    QPoint ptToLayoutUnitPix( const KoPoint & p ) const {
        return QPoint( ptToLayoutUnitPixX( p.x() ), ptToLayoutUnitPixY( p.y() ) );
    }

    /**
     * Given the font size for the font in layout units, in pt (use pointSize())
     * this returns the font size to use on screen the current zoom, in pt (use setPointSizeFloat()),
     */
    double layoutUnitToFontSize( int luSize, bool /*forPrint*/ ) const;

    // Note: For converting fontsizes from/to layout units and zoom-independent
    // pt sizes (like the one the user sees, e.g. 12pt),
    // use ptToLayoutUnit and layoutUnitToPt, not layoutToFontSize.


    // The conversions below convert between an internal text layout resolution of
    // ~1440 DPI (by default) and the point-size for the fonts (those known by the user).
    // Those conversions don't depend on the zoom level.

    /** Change the factor that converts between pointsizes
     * and layout units (by default 20 - for 1440 DPI at 72 DPI) */
    static void setPtToLayoutUnitFactor( int factor ) { m_layoutUnitFactor = factor; }

    /** Not zoom dependent. Simply convert a pt value (e.g. a frame)
     * to high-resolution layout unit coordinates (in pt). */
    static double ptToLayoutUnitPt( double pt )
    { return pt * static_cast<double>( m_layoutUnitFactor ); }
    /** Same thing for integer values, e.g. a font size in pt */
    static int ptToLayoutUnitPt( int ptSize )
    { return ptSize * m_layoutUnitFactor; }

    static KoPoint ptToLayoutUnitPt( const KoPoint &p )
    { return KoPoint( ptToLayoutUnitPt( p.x() ),
                      ptToLayoutUnitPt( p.y() ) ); }
    static KoRect ptToLayoutUnitPt( const KoRect &r )
    { return KoRect( ptToLayoutUnitPt( r.topLeft() ),
                     ptToLayoutUnitPt( r.bottomRight() ) ); }

    static double layoutUnitPtToPt( double lupt )
    { return lupt / static_cast<double>( m_layoutUnitFactor ); }
    static KoPoint layoutUnitPtToPt( const KoPoint& p )
    { return KoPoint( layoutUnitPtToPt( p.x() ),
                      layoutUnitPtToPt( p.y() ) ); }

protected:
    /** This being static ensures that the same value is used by all KoTextZoomHandler instances */
    static int m_layoutUnitFactor;
};

#endif
