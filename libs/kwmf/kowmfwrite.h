/* This file is part of the KDE libraries
   Copyright (c) 2003 thierry lorthiois (lorthioist@wanadoo.fr)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#ifndef _KOWMFWRITE_H_
#define _KOWMFWRITE_H_

#include <qpen.h>
#include <qbrush.h>
#include <qcolor.h>
#include <qfont.h>
#include <qrect.h>
#include <qbuffer.h>
#include <qregion.h>
#include <qstring.h>
#include <qmatrix.h>
#include <qimage.h>
#include <q3ptrlist.h>
#include <q3pointarray.h>
#include <koffice_export.h>
class KoWmfWritePrivate;

/**
 * KoWmfWrite allows to create a windows placeable meta file (WMF).
 * Most of the functions are compatible with QPainter format.
 *
 * sample of utilization:
 *   <pre>
 *   KoWmfWrite  wmf("/home/test.wmf");
 *   wmf.begin();
 *   wmf.setWindow(0, 0, 200, 200);
 *   wmf.drawRect(10, 20, 50, 120);
 *   wmf.end();
 *   </pre>
 */
class KOWMF_EXPORT KoWmfWrite
{
public:
    KoWmfWrite( const QString& fileName );
    virtual ~KoWmfWrite();


    // -------------------------------------------------------------------------
    // virtual Painter
    // for a good documentation : check QPainter documentation
    /**
     * Open the file. Returns true on success.
     */
    bool  begin();
    /**
     * Close the file. Returns true on success.
     */
    bool  end();
    void  save();
    void  restore();

    /**
     * Placeable WMF's use logical coordinates and have a default DPI.
     * This function set the dot per inch ratio.
     * If not specified the dpi is 1024.
     */
    void setDefaultDpi( int dpi );

    // Drawing tools
    void  setFont( const QFont& f );
    // the width of the pen is in logical coordinate
    void  setPen( const QPen& p );
    void  setBrush( const QBrush& b );

    // Drawing attributes/modes
    void  setBackgroundColor( const QColor& r );
    void  setBackgroundMode( Qt::BGMode );
    void  setCompositionMode( QPainter::CompositionMode );

    // Change logical Coordinate
    void  setWindow( int left, int top , int width, int height );

    // Clipping
    // the 'CoordinateMode' parameter is ommitted : always CoordPainter in wmf
    // not yet implemented
    void  setClipRegion( const QRegion& r );
    void  clipping( bool enable );

    // Graphics drawing functions
    void  moveTo( int left, int top );
    void  lineTo( int left, int top );
    void  drawRect( int left, int top, int width, int height );
    void  drawRoundRect( int left, int top, int width, int height, int = 25, int = 25 );
    void  drawEllipse( int left, int top, int width, int height );
    void  drawArc( int left, int top, int width, int height, int a, int alen );
    void  drawPie( int left, int top, int width, int height, int a, int alen );
    void  drawChord( int left, int top, int width, int height, int a, int alen );
    void  drawPolyline( const QPolygon& pa );
    void  drawPolygon( const QPolygon& pa, bool winding=FALSE );
    // drawPolyPolygon draw the XOR of a list of polygons
    // listPa : list of polygons
    void  drawPolyPolygon( Q3PtrList<QPolygon>& listPa, bool winding=FALSE );
    void  drawImage( int left, int top, const QImage &, int sx = 0, int sy = 0, int sw = -1, int sh = -1 );

    // Text drawing functions
    // rotation = the degrees of rotation in counterclockwise
    // not yet implemented
    void  drawText( int x, int y, int w, int h, int flags, const QString &s, double rotation );

private:
    //-----------------------------------------------------------------------------
    // Utilities and conversion Qt --> Wmf

    /** Convert QPointArray into qint16 position (result in mSt) */
    void pointArray( const QPolygon& pa );

    /** Convertion between windows color and QColor */
    quint32 winColor( QColor color );

    /** Convert angle a and alen in coordinate (xStart,yStart) and (xEnd, yEnd) */
    void angleToxy( int& xStart, int& yStart, int& xEnd, int& yEnd, int a, int alen );

    /** Convert windows rasterOp in QT rasterOp */
    quint16 qtRasterToWin16( QPainter::CompositionMode op ) const;
    quint32 qtRasterToWin32( QPainter::CompositionMode op ) const;


private:
    KoWmfWritePrivate *d;

};

#endif

