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

#include "kritavectorimage_export.h"

#include <QList>
#include <QPainter>
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
class KRITAVECTORIMAGE_EXPORT OutputDebugStrategy : public AbstractOutput
{
public:
    OutputDebugStrategy();
    ~OutputDebugStrategy() override;

    void init( const Header *header ) override;
    void cleanup( const Header *header ) override;
    void eof() override;

    void createPen( quint32 ihPen, quint32 penStyle, quint32 x, quint32 y,
		    quint8 red, quint8 green, quint8 blue, quint8 reserved ) override;
    void createBrushIndirect( quint32 ihBrush, quint32 BrushStyle, quint8 red,
			      quint8 green, quint8 blue, quint8 reserved, 
			      quint32 BrushHatch ) override;
    void createMonoBrush( quint32 ihBrush, Bitmap *bitmap ) override;
    void selectObject( const quint32 ihObject ) override;
    void deleteObject( const quint32 ihObject ) override;
    void arc( const QRect &box, const QPoint &start, const QPoint &end ) override;
    void chord( const QRect &box, const QPoint &start, const QPoint &end ) override;
    void pie( const QRect &box, const QPoint &start, const QPoint &end ) override;
    void ellipse( const QRect &box ) override;
    void rectangle( const QRect &box ) override;
    void setMapMode( const quint32 mapMode ) override;
    void setMetaRgn() override;
    void setWindowOrgEx( const QPoint &origin ) override;
    void setWindowExtEx( const QSize &size ) override;
    void setViewportOrgEx( const QPoint &origin ) override;
    void setViewportExtEx( const QSize &size ) override;
    void beginPath() override;
    void closeFigure() override;
    void endPath() override;
    void setBkMode( const quint32 backgroundMode ) override;
    void setPolyFillMode( const quint32 polyFillMode ) override;
    void setLayout( const quint32 layoutMode ) override;
    void extCreateFontIndirectW( const ExtCreateFontIndirectWRecord &extCreateFontIndirectW ) override;
    void setTextAlign( const quint32 textAlignMode ) override;
    void setTextColor( const quint8 red, const quint8 green, const quint8 blue,
		       const quint8 reserved ) override;
    void setBkColor( const quint8 red, const quint8 green, const quint8 blue,
                     const quint8 reserved ) override;
    void setPixelV( QPoint &point, quint8 red, quint8 green, quint8 blue, quint8 reserved ) override;
    void modifyWorldTransform( quint32 mode, float M11, float M12,
			       float M21, float M22, float Dx, float Dy ) override;
    void setWorldTransform( float M11, float M12, float M21,
			    float M22, float Dx, float Dy ) override;
    void extTextOut( const QRect &bounds, const EmrTextObject &textObject ) override;
    void moveToEx( const qint32 x, const qint32 y ) override;
    void saveDC() override;
    void restoreDC( const qint32 savedDC ) override;
    void lineTo( const QPoint &finishPoint ) override;
    void arcTo( const QRect &box, const QPoint &start, const QPoint &end ) override;
    void polygon16( const QRect &bounds, const QList<QPoint> points ) override;
    void polyLine( const QRect &bounds, const QList<QPoint> points ) override;
    void polyLine16( const QRect &bounds, const QList<QPoint> points ) override;
    void polyPolygon16( const QRect &bounds, const QList< QVector< QPoint > > &points ) override;
    void polyPolyLine16( const QRect &bounds, const QList< QVector< QPoint > > &points ) override;
    void polyLineTo16( const QRect &bounds, const QList<QPoint> points ) override;
    void polyBezier16( const QRect &bounds, const QList<QPoint> points ) override;
    void polyBezierTo16( const QRect &bounds, const QList<QPoint> points ) override;
    void fillPath( const QRect &bounds ) override;
    void strokeAndFillPath( const QRect &bounds ) override;
    void strokePath( const QRect &bounds ) override;
    void setClipPath( const quint32 regionMode ) override;
    void bitBlt( BitBltRecord &bitBltRecord ) override;
    void setStretchBltMode( const quint32 stretchMode ) override;
    void stretchDiBits( StretchDiBitsRecord &stretchDiBitsRecord ) override;
};


}

#endif
