/*
  Copyright 2008 Brad Hards  <bradh@frogmouth.net>
  Copyright 2009 Inge Wallin <inge@lysator.liu.se>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either 
  version 2.1 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public 
  License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef EMFOUTPUTDEBUGSTRATEGY_H
#define EMFOUTPUTDEBUGSTRATEGY_H

#include "emf_export.h"

#include <QList>
#include <QPainter>
#include <QPixmap>
#include <QRect> // also provides QSize, QPoint
#include <QString>
#include <QVariant>

#include "EmfEnums.h"
#include "EmfHeader.h"
#include "EmfRecords.h"
#include "EmfOutput.h"

/**
   \file

   Contains definitions for an EMF debug output strategy
*/

/**
   Namespace for Enhanced Metafile (EMF) classes
*/
namespace Libemf
{


/**
    Debug (text dump) output strategy for EMF Parser
*/
class EMF_EXPORT OutputDebugStrategy : public AbstractOutput
{
public:
    OutputDebugStrategy();
    ~OutputDebugStrategy();

    void init( const Header *header );
    void cleanup( const Header *header );
    void eof();

    void createPen( quint32 ihPen, quint32 penStyle, quint32 x, quint32 y,
		    quint8 red, quint8 green, quint8 blue, quint8 reserved );
    void createBrushIndirect( quint32 ihBrush, quint32 BrushStyle, quint8 red,
			      quint8 green, quint8 blue, quint8 reserved, 
			      quint32 BrushHatch );
    void selectObject( const quint32 ihObject );
    void deleteObject( const quint32 ihObject );
    void arc( const QRect &box, const QPoint &start, const QPoint &end );
    void chord( const QRect &box, const QPoint &start, const QPoint &end );
    void pie( const QRect &box, const QPoint &start, const QPoint &end );
    void ellipse( const QRect &box );
    void rectangle( const QRect &box );
    void setMapMode( const quint32 mapMode );
    void setMetaRgn();
    void setWindowOrgEx( const QPoint &origin );
    void setWindowExtEx( const QSize &size );
    void setViewportOrgEx( const QPoint &origin );
    void setViewportExtEx( const QSize &size );
    void beginPath();
    void closeFigure();
    void endPath();
    void setBkMode( const quint32 backgroundMode );
    void setPolyFillMode( const quint32 polyFillMode );
    void setLayout( const quint32 layoutMode );
    void extCreateFontIndirectW( const ExtCreateFontIndirectWRecord &extCreateFontIndirectW );
    void setTextAlign( const quint32 textAlignMode );
    void setTextColor( const quint8 red, const quint8 green, const quint8 blue,
		       const quint8 reserved );
    void setBkColor( const quint8 red, const quint8 green, const quint8 blue,
                     const quint8 reserved );
    void setPixelV( QPoint &point, quint8 red, quint8 green, quint8 blue, quint8 reserved );
    void modifyWorldTransform( quint32 mode, float M11, float M12,
			       float M21, float M22, float Dx, float Dy );
    void setWorldTransform( float M11, float M12, float M21,
			    float M22, float Dx, float Dy );
    void extTextOutA( const ExtTextOutARecord &extTextOutA );
    void extTextOutW( const QPoint &referencePoint, const QString &textString );
    void moveToEx( const quint32 x, const quint32 y );
    void saveDC();
    void restoreDC( const qint32 savedDC );
    void lineTo( const QPoint &finishPoint );
    void arcTo( const QRect &box, const QPoint &start, const QPoint &end );
    void polygon16( const QRect &bounds, const QList<QPoint> points );
    void polyLine( const QRect &bounds, const QList<QPoint> points );
    void polyLine16( const QRect &bounds, const QList<QPoint> points );
    void polyPolygon16( const QRect &bounds, const QList< QVector< QPoint > > &points );
    void polyPolyLine16( const QRect &bounds, const QList< QVector< QPoint > > &points );
    void polyLineTo16( const QRect &bounds, const QList<QPoint> points );
    void polyBezier16( const QRect &bounds, const QList<QPoint> points );
    void polyBezierTo16( const QRect &bounds, const QList<QPoint> points );
    void fillPath( const QRect &bounds );
    void strokeAndFillPath( const QRect &bounds );
    void strokePath( const QRect &bounds );
    void setClipPath( const quint32 regionMode );
    void bitBlt( BitBltRecord bitBltRecord );
    void setStretchBltMode( const quint32 stretchMode );
    void stretchDiBits( StretchDiBitsRecord stretchDiBitsRecord );
};


}

#endif
