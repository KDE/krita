/****************************************************************************
** Copyright (C) 2001-2010 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Chart library.
**
** Licensees holding valid commercial KD Chart licenses may use this file in
** accordance with the KD Chart Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.GPL included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#ifndef KDCHARTRADARDIAGRAM_H
#define KDCHARTRADARDIAGRAM_H


#include "KDChartPosition.h"
#include "KDChartPolarDiagram.h"
#include "KDChartRadarCoordinatePlane.h"


class QPolygonF;


namespace KDChart {

/**
  * @brief RadarDiagram defines a common radar diagram
  */
class KDCHART_EXPORT RadarDiagram : public AbstractPolarDiagram
{
  Q_OBJECT

  Q_DISABLE_COPY( RadarDiagram )
  KDCHART_DECLARE_DERIVED_DIAGRAM( RadarDiagram, RadarCoordinatePlane )

public:
    explicit RadarDiagram(
        QWidget* parent = 0, RadarCoordinatePlane* plane = 0 );
    virtual ~RadarDiagram();

    virtual void paint ( PaintContext* paintContext,
                         bool calculateListAndReturnScale,
                         qreal& newZoomX, qreal& newZoomY );
    /** \reimpl */
    virtual void resize ( const QSizeF& area );
    
    /** \reimpl */
    virtual double valueTotals () const;
    /** \reimpl */
    virtual double numberOfValuesPerDataset() const;
    /** \reimpl */
    virtual double numberOfGridRings() const;
    
    /**
     * if val is true the diagram will mirror the diagram datapoints
     */
    void setReverseData( bool val );
    bool reverseData();
    
    virtual RadarDiagram * clone() const;
    
    /**
     * Close each of the data series by connecting the last point to its
     * respective start point
     */
    void setCloseDatasets( bool closeDatasets );
    bool closeDatasets() const;

    /**
     * Fill the areas of the radar chart with there respective color defined
     * via KDChart::DatasetBrushRole. The value defines the alpha of the
     * color to use. If set to 0.0 (the default) then the radar areas will
     * not be filled with any color. If set to 1.0 then the areas will be
     * solid filled and are not transparent.
     */
    qreal fillAlpha() const;
    void setFillAlpha(qreal alphaF);

protected:
    /** \reimpl */
    virtual const QPair<QPointF, QPointF> calculateDataBoundaries() const;
    void paintEvent ( QPaintEvent* );
    void resizeEvent ( QResizeEvent* );
    virtual void paint ( PaintContext* paintContext );

}; // End of class RadarDiagram

} 

#endif // KDCHARTRADARDIAGRAM_H