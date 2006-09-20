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
#ifndef _KOWMFREAD_H_
#define _KOWMFREAD_H_

#include <QPen>
#include <QBrush>
#include <QFont>
#include <QColor>
#include <QRect>
#include <qregion.h>
#include <QImage>
#include <QMatrix>
#include <QString>
#include <q3ptrlist.h>
#include <q3pointarray.h>
#include <QPainter>

#include <koffice_export.h>
class KoWmfReadPrivate;

/**
 * KoWmfRead allows the redirection of the actions stored in a WMF file.
 * Most of the virtuals functions are compatible with QPainter format.
 *
 * How to use :
 *   inherit this class and define abstract functions
 *   then create an object and call @ref load() and @ref play()
 *
 */

class KOWMF_EXPORT KoWmfRead
{
public:
    KoWmfRead();
    virtual ~KoWmfRead();

    /**
     * Load WMF file. Returns true on success.
     */
    virtual bool load( const QString& fileName );
    virtual bool load( const QByteArray& array );

    /**
     * play the WMF file => call virtuals functions
     */
    virtual bool play(  );

    /**
     * Returns true if the metafile is standard / placeable / enhanced / valid
     */
    bool isStandard( void ) const;
    bool isPlaceable( void ) const;
    bool isEnhanced( void ) const;
    bool isValid( void ) const;

    /**
     * Returns the bounding rectangle
     * Standard Meta File : return the bounding box from setWindowOrg and setWindowExt (slow)
     * Placeable Meta File : return the bounding box from header
     * always in logical coordinate
     */
    virtual QRect boundingRect( void ) const;

    /**
     * Returns the default DotPerInch for placeable meta file,
     * return 0 for Standard meta file
     */
    int defaultDpi( void ) const;

    /**
     * Activate debug mode.
     * nbFunc : number of functions to draw
     * nbFunc!=0 switch to debug mode with trace
     */
    void setDebug( int nbFunc );

    // -------------------------------------------------------------------------
    // A virtual QPainter : inherit those virtuals functions
    // for a good documentation : check QPainter documentation
    virtual bool  begin() = 0;
    virtual bool  end() = 0;
    virtual void  save() = 0;
    virtual void  restore() = 0;

    // Drawing tools
    virtual void  setFont( const QFont & ) = 0;
    // the pen : the width of the pen is in logical coordinate
    virtual void  setPen( const QPen &p ) = 0;
    virtual const QPen &pen() const = 0;
    virtual void  setBrush( const QBrush & ) = 0;

    // Drawing attributes/modes
    virtual void  setBackgroundColor( const QColor & ) = 0;
    virtual void  setBackgroundMode( Qt::BGMode ) = 0;
    virtual void  setCompositionMode( QPainter::CompositionMode ) = 0;

    // Change logical Coordinate
    // some wmf files call those functions several times in the middle of a drawing
    // others doesn't call setWindow* at all
    virtual void  setWindowOrg( int left, int top ) = 0;
    virtual void  setWindowExt( int width, int height ) = 0;

    // Clipping
    // the 'CoordinateMode' parameter is ommitted : always CoordPainter in wmf
    // setClipRegion() is often used with save() and restore() => implement all or none
    virtual void  setClipRegion( const QRegion & ) = 0;
    virtual QRegion clipRegion() = 0;

    // Graphics drawing functions
    virtual void  moveTo( int x, int y ) = 0;
    virtual void  lineTo( int x, int y ) = 0;
    virtual void  drawRect( int x, int y, int w, int h ) = 0;
    virtual void  drawRoundRect( int x, int y, int w, int h, int = 25, int = 25 ) = 0;
    virtual void  drawEllipse( int x, int y, int w, int h ) = 0;
    virtual void  drawArc( int x, int y, int w, int h, int a, int alen ) = 0;
    virtual void  drawPie( int x, int y, int w, int h, int a, int alen ) = 0;
    virtual void  drawChord( int x, int y, int w, int h, int a, int alen ) = 0;
    virtual void  drawPolyline( const QPolygon &pa ) = 0;
    virtual void  drawPolygon( const QPolygon &pa, bool winding=false ) = 0;
    // drawPolyPolygon draw the XOR of a list of polygons
    // listPa : list of polygons
    virtual void  drawPolyPolygon( Q3PtrList<QPolygon>& listPa, bool winding=false ) = 0;
    virtual void  drawImage( int x, int y, const QImage &, int sx = 0, int sy = 0, int sw = -1, int sh = -1 ) = 0;

    // Text drawing functions
    // rotation = the degrees of rotation in counterclockwise
    // not yet implemented in KWinMetaFile
    virtual void  drawText( int x, int y, int w, int h, int flags, const QString &s, double rotation ) = 0;

    // matrix transformation : only used for bitmap manipulation
    virtual void  setMatrix( const QMatrix &, bool combine=false ) = 0;

private:
    KoWmfReadPrivate  *mKwmf;

};

#endif

