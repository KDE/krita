/* This file is part of the KDE libraries
 * Copyright (c) 2003 thierry lorthiois (lorthioist@wanadoo.fr)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <kdebug.h>

#include "kowmfpaint.h"
//Added by qt3to4:
#include <QPolygon>
#include <Q3PtrList>
#include <QPrinter>

KoWmfPaint::KoWmfPaint() : KoWmfRead() {
    mTarget = 0;
}


bool KoWmfPaint::play( QPaintDevice& target, bool relativeCoord )
{
    if ( mPainter.isActive() ) return false;
    mTarget = &target;
    mRelativeCoord = relativeCoord;

    // Play the wmf file
    return KoWmfRead::play( );
}


//-----------------------------------------------------------------------------
// Virtual Painter

bool KoWmfPaint::begin() {
    bool ret = mPainter.begin( mTarget );

    if ( ret ) {
        if ( mRelativeCoord ) {
            mInternalWorldMatrix.reset();
        }
        else {
            // some wmf files doesn't call setwindowOrg and setWindowExt, so it's better to do :
            QRect rec = boundingRect();
            mPainter.setWindow( rec.left(), rec.top(), rec.width(), rec.height() );
        }
    }
    return ret;
}


bool KoWmfPaint::end() {
    if ( mRelativeCoord ) {
       QRect rec = boundingRect();

        // Draw 2 invisible points
        // because QPicture::setBoundingRect() doesn't give expected result (QT3.1.2)
        // setBoundingRect( boundingRect() );
//        mPainter.setPen( Qt::NoPen );
//        mPainter.drawPoint( rec.left(), rec.top() );
//        mPainter.drawPoint( rec.right(), rec.bottom() );
    }
    return mPainter.end();
}


void KoWmfPaint::save() {
    mPainter.save();
}


void KoWmfPaint::restore() {
    mPainter.restore();
}


void KoWmfPaint::setFont( const QFont &font ) {
    mPainter.setFont( font );
}


void KoWmfPaint::setPen( const QPen &pen ) {
    QPen p = pen;
    int width = pen.width();

    if ( dynamic_cast<QPrinter *>( mTarget ) ) {
        width = 0;
    }
    else {
        // WMF spec : width of pen in logical coordinate
        // => width of pen proportional with device context width
        QRect rec = mPainter.window();
        // QPainter documentation says this is equivalent of xFormDev, but it doesn't compile. Bug reported.
#if 0
        QRect devRec = rec * mPainter.matrix();
        if ( rec.width() != 0 )
            width = ( width * devRec.width() ) / rec.width() ;
        else
            width = 0;
#endif
    }

    p.setWidth( width );
    mPainter.setPen( p );
}


const QPen &KoWmfPaint::pen() const {
    return mPainter.pen();
}


void KoWmfPaint::setBrush( const QBrush &brush ) {
    mPainter.setBrush( brush );
}


void KoWmfPaint::setBackgroundColor( const QColor &c ) {
    mPainter.setBackground( QBrush( c ) );
}


void KoWmfPaint::setBackgroundMode( Qt::BGMode mode ) {
    mPainter.setBackgroundMode( mode );
}


void KoWmfPaint::setCompositionMode( QPainter::CompositionMode mode ) {
    mPainter.setCompositionMode( mode );
}


// ---------------------------------------------------------------------
// To change those functions it's better to have
// a large set of WMF files. WMF special case includes :
// - without call to setWindowOrg and setWindowExt
// - change the origin or the scale in the middle of the drawing
// - negative width or height
// and relative/absolute coordinate
void KoWmfPaint::setWindowOrg( int left, int top ) {
    if ( mRelativeCoord ) {
        double dx = mInternalWorldMatrix.dx();
        double dy = mInternalWorldMatrix.dy();

        // translation : Don't use setWindow()
        mInternalWorldMatrix.translate( -dx, -dy );
        mPainter.translate( -dx, -dy );
        mInternalWorldMatrix.translate( -left, -top );
        mPainter.translate( -left, -top );
    }
    else {
        QRect rec = mPainter.window();
        mPainter.setWindow( left, top, rec.width(), rec.height() );
    }
}


void KoWmfPaint::setWindowExt( int w, int h ) {
    if ( mRelativeCoord ) {
        QRect r = mPainter.window();
        double dx = mInternalWorldMatrix.dx();
        double dy = mInternalWorldMatrix.dy();
        double sx = mInternalWorldMatrix.m11();
        double sy = mInternalWorldMatrix.m22();

        // scale : don't use setWindow()
        mInternalWorldMatrix.translate( -dx, -dy );
        mPainter.translate( -dx, -dy );
        mInternalWorldMatrix.scale( 1/sx, 1/sy );
        mPainter.scale( 1/sx, 1/sy );

        sx = (double)r.width() / (double)w;
        sy = (double)r.height() / (double)h;

        mInternalWorldMatrix.scale( sx, sy );
        mPainter.scale( sx, sy );
        mInternalWorldMatrix.translate( dx, dy );
        mPainter.translate( dx, dy );
    }
    else {
        QRect rec = mPainter.window();
        mPainter.setWindow( rec.left(), rec.top(), w, h );
    }
}


void KoWmfPaint::setMatrix( const QMatrix &wm, bool combine ) {
    mPainter.setMatrix( wm, combine );
}


void KoWmfPaint::setClipRegion( const QRegion &rec ) {
    mPainter.setClipRegion( rec );
}


QRegion KoWmfPaint::clipRegion() {
    return mPainter.clipRegion();
}


void KoWmfPaint::moveTo( int x, int y ) {
    mLastPos = QPoint( x, y );
}


void KoWmfPaint::lineTo( int x, int y ) {
    mPainter.drawLine( mLastPos, QPoint( x, y ) );
}


void KoWmfPaint::drawRect( int x, int y, int w, int h ) {
    mPainter.drawRect( x, y, w, h );
}


void KoWmfPaint::drawRoundRect( int x, int y, int w, int h, int roudw, int roudh ) {
    mPainter.drawRoundRect( x, y, w, h, roudw, roudh );
}


void KoWmfPaint::drawEllipse( int x, int y, int w, int h ) {
    mPainter.drawEllipse( x, y, w, h );
}


void KoWmfPaint::drawArc( int x, int y, int w, int h, int a, int alen ) {
    mPainter.drawArc( x, y, w, h, a, alen );
}


void KoWmfPaint::drawPie( int x, int y, int w, int h, int a, int alen ) {
    mPainter.drawPie( x, y, w, h, a, alen );
}


void KoWmfPaint::drawChord( int x, int y, int w, int h, int a, int alen ) {
    mPainter.drawChord( x, y, w, h, a, alen );
}


void KoWmfPaint::drawPolyline( const QPolygon &pa ) {
    mPainter.drawPolyline( pa );
}


void KoWmfPaint::drawPolygon( const QPolygon &pa, bool winding ) {
    if( winding )
	mPainter.drawPolygon( pa, Qt::WindingFill );
    else
	mPainter.drawPolygon( pa, Qt::OddEvenFill );
}


void KoWmfPaint::drawPolyPolygon( Q3PtrList<QPolygon>& listPa, bool winding ) {
    QPolygon *pa;

    mPainter.save();
    QBrush brush = mPainter.brush();

    // define clipping region
    QRegion region;
    for ( pa = listPa.first() ; pa ; pa = listPa.next() ) {
        region = region.eor( *pa );
    }
    mPainter.setClipRegion( region );

    // fill polygons
    if ( brush != Qt::NoBrush ) {
        mPainter.fillRect( region.boundingRect(), brush );
    }

    // draw polygon's border
    mPainter.setClipping( false );
    if ( mPainter.pen().style() != Qt::NoPen ) {
        mPainter.setBrush( Qt::NoBrush );
        for ( pa = listPa.first() ; pa ; pa = listPa.next() ) 
	{
	    if( winding )
	        mPainter.drawPolygon( *pa, Qt::WindingFill );
	    else
		mPainter.drawPolygon( *pa, Qt::OddEvenFill );
        }
    }

    // restore previous state
    mPainter.restore();
}


void KoWmfPaint::drawImage( int x, int y, const QImage &img, int sx, int sy, int sw, int sh ) {
    mPainter.drawImage( x, y, img, sx, sy, sw, sh );
}


void KoWmfPaint::drawText( int x, int y, int w, int h, int flags, const QString& s, double ) {
    mPainter.drawText( x, y, w, h, flags, s );
}


