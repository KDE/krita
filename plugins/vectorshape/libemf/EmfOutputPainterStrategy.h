/*
  Copyright 2008        Brad Hards  <bradh@frogmouth.net>
  Copyright 2009 - 2010 Inge Wallin <inge@lysator.liu.se>

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

#ifndef EMFOUTPUTPAINTERSTRATEGY_H
#define EMFOUTPUTPAINTERSTRATEGY_H

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

   Primary definitions for EMF output strategies
*/

/**
   Namespace for Enhanced Metafile (EMF) classes
*/
namespace Libemf
{


/**
    QPainter based output strategy for EMF Parser.

    This class allows rendering of an EMF file to a QPixmap or any other QPaintDevice.
*/
class EMF_EXPORT OutputPainterStrategy : public AbstractOutput
{
public:
    /**
       Constructor.

       This will probably need to take an enum to say what sort of output
       we want.
    */
    OutputPainterStrategy();
    OutputPainterStrategy( QPainter &painter, QSize &size, 
                           bool keepAspectRatio = false );
    ~OutputPainterStrategy();

    void init( const Header *header );
    void cleanup( const Header *header );
    void eof();

    /**
       The image that has been rendered to.
    */
    QImage *image();

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
    void modifyWorldTransform( const quint32 mode, float M11, float M12,
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
    void polyLine16( const QRect &bounds, const QList<QPoint> points );
    void polyPolygon16( const QRect &bounds, const QList< QVector< QPoint > > &points );
    void polyPolyLine16( const QRect &bounds, const QList< QVector< QPoint > > &points );
    void polyLine( const QRect &bounds, const QList<QPoint> points );
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

private:
    /**
       Select a stock object.

       See [MS-EMF] Section 2.1.31.

       \param ihObject the stock object value
    */
    void selectStockObject( const quint32 ihObject );

    /**
       Test if we are currently building a path
    */
    bool currentlyBuildingPath() const;

    
    /**
       Helper routine to convert the EMF angle (centrepoint + radial endpoint) into
       the Qt format (in degress - may need to multiply by 16 for some purposes)
    */
    qreal angleFromArc( const QPoint &centrePoint, const QPoint &radialPoint );

    /**
      Calculate the anglular difference (span) between two angles
      
      This should always be positive.
    */
    qreal angularSpan( const qreal startAngle, const qreal endAngle );

    /**
       Convert the EMF font weight scale (0..1000) to Qt equivalent.
       
       This is a bit rough - the EMF spec only says 400 is normal, and 
       700 is bold.
    */
    int convertFontWeight( quint32 emfWeight );

    QPainter                *m_painter;
    int                      m_painterSaves; // The number of times that save() was called.
    QSize                    m_outputSize;
    bool                     m_keepAspectRatio;

    QMap<quint32, QVariant>  m_objectTable;

    QPainterPath *m_path;
    bool m_currentlyBuildingPath;

    /**
       The image we are painting to
    */
    QImage *m_image;

    /**
       The current text pen
    */
    QPen m_textPen;

    /**
       The current fill rule
    */
    enum Qt::FillRule m_fillRule;

    /**
        The current text alignment mode
    */
    quint32 m_textAlignMode;

    /**
       The current coordinates
    */
    QPoint  m_currentCoords;
};

}

#endif
