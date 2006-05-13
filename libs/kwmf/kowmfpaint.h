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
#ifndef _KOWMFPAINT_H_
#define _KOWMFPAINT_H_

#include <QPainter>
//Added by qt3to4:
#include <QPolygon>
#include <Q3PtrList>

#include "kowmfread.h"
#include <koffice_export.h>
/**
 * KoWmfPaint inherits the abstract class KoWmfRead
 * and redirects WMF actions onto a QPaintDevice.
 * Uses relative or absolute coordinate.
 *
 * how to use:
 * <pre>
 *   QPixmap pix( 100, 100 );
 *   KoWmfPaint wmf;
 *   if ( wmf.load( "/home/test.wmf" ) ) {
 *      wmf.play( pix );
 *   }
 *   paint.drawPixmap( 0, 0, pix );
 * </pre>
 *
 */

class KOWMF_EXPORT KoWmfPaint : public KoWmfRead
{
public:
    KoWmfPaint();
    ~KoWmfPaint() { }

    /**
     * play WMF file on a QPaintDevice. Return true on success.
     * Use absolute or relative coordinate :
     *   absolute coord. reset the world transfomation Matrix (by default)
     *   relative coord. use the existing world transfomation Matrix
     */
    bool play( QPaintDevice& target, bool relativeCoord=false );


private:
    // -------------------------------------------------------------------------
    // A virtual QPainter
    bool  begin();
    bool  end();
    void  save();
    void  restore();

    // Drawing tools
    void  setFont( const QFont& font );
    // the pen : the width of the pen is in logical coordinate
    void  setPen( const QPen& pen );
    const QPen& pen() const;
    void  setBrush( const QBrush& brush );

    // Drawing attributes/modes
    void  setBackgroundColor( const QColor& c );
    void  setBackgroundMode( Qt::BGMode mode );
    void  setCompositionMode( QPainter::CompositionMode mode );

    /**
     * Change logical Coordinate
     * some wmf files call those functions several times in the middle of a drawing
     * others wmf files doesn't call setWindow* at all
     * negative width and height are possible
     */
    void  setWindowOrg( int left, int top );
    void  setWindowExt( int width, int height );

    // Clipping
    // the 'CoordinateMode' is ommitted : always CoordPainter in wmf
    // setClipRegion() is often used with save() and restore() => implement all or none
    void  setClipRegion( const QRegion &rec );
    QRegion clipRegion();

    // Graphics drawing functions
    void  moveTo( int x, int y );
    void  lineTo( int x, int y );
    void  drawRect( int x, int y, int w, int h );
    void  drawRoundRect( int x, int y, int w, int h, int = 25, int = 25 );
    void  drawEllipse( int x, int y, int w, int h );
    void  drawArc( int x, int y, int w, int h, int a, int alen );
    void  drawPie( int x, int y, int w, int h, int a, int alen );
    void  drawChord( int x, int y, int w, int h, int a, int alen );
    void  drawPolyline( const QPolygon& pa );
    void  drawPolygon( const QPolygon& pa, bool winding=FALSE );
    /**
     * drawPolyPolygon draw the XOR of a list of polygons
     * listPa : list of polygons
     */
    void  drawPolyPolygon( Q3PtrList<QPolygon>& listPa, bool winding=FALSE );
    void  drawImage( int x, int y, const QImage &, int sx = 0, int sy = 0, int sw = -1, int sh = -1 );

    // Text drawing functions
    // rotation = the degrees of rotation in counterclockwise
    // not yet implemented in KWinMetaFile
    void  drawText( int x, int y, int w, int h, int flags, const QString &s, double rotation );

    // matrix transformation : only used in some bitmap manipulation
    void  setMatrix( const QMatrix &, bool combine=FALSE );

private:
    QPainter mPainter;
    QPaintDevice *mTarget;
    bool  mRelativeCoord;
    // memorisation of WMF matrix transformation (in relative coordinate)
    QMatrix  mInternalWorldMatrix;
    QPoint mLastPos;

};

#endif
