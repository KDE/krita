/* -*- Mode: C++ -*-
   KDChart - a multi-platform charting engine
   */

/****************************************************************************
 ** Copyright (C) 2005-2007 Klaralvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Chart library.
 **
 ** This file may be used under the terms of the GNU General Public
 ** License versions 2.0 or 3.0 as published by the Free Software
 ** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
 ** included in the packaging of this file.  Alternatively you may (at
 ** your option) use any later version of the GNU General Public
 ** License if such license has been publicly approved by
 ** Klarälvdalens Datakonsult AB (or its successors, if any).
 ** 
 ** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
 ** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE. Klarälvdalens Datakonsult AB reserves all rights
 ** not expressly granted herein.
 ** 
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 **********************************************************************/

#ifndef KDCHARTLEVEYJENNINGSDIAGRAM_H
#define KDCHARTLEVEYJENNINGSDIAGRAM_H

#include "../KDChartLineDiagram.h"
#include "KDChartLeveyJenningsCoordinatePlane.h"

class QPainter;
class QPolygonF;
class QSvgRenderer;

namespace KDChart {

    class ThreeDLineAttributes;

/**
 * @brief LeveyDiagram defines a Levey Jennings chart.
 * 
 * It provides different subtypes which are set using \a setType.
 */
class KDCHART_EXPORT LeveyJenningsDiagram : public LineDiagram
{
    Q_OBJECT

    Q_DISABLE_COPY( LeveyJenningsDiagram )
//    KDCHART_DECLARE_PRIVATE_DERIVED_PARENT( LineDiagram, CartesianCoordinatePlane * )

    KDCHART_DECLARE_DERIVED_DIAGRAM( LeveyJenningsDiagram, LeveyJenningsCoordinatePlane )


public:
    explicit LeveyJenningsDiagram( QWidget* parent = 0, LeveyJenningsCoordinatePlane* plane = 0 );
    virtual ~LeveyJenningsDiagram();

    virtual LineDiagram * clone() const;

    enum Symbol
    {
        OkDataPoint,
        NotOkDataPoint,
        LotChanged,
        SensorChanged,
        FluidicsPackChanged
    };

    /**
     * Returns true if both diagrams have the same settings.
     */
    bool compare( const LeveyJenningsDiagram* other ) const;

    void setLotChangedSymbolPosition( Qt::Alignment pos );
    Qt::Alignment lotChangedSymbolPosition() const;

    void setFluidicsPackChangedSymbolPosition( Qt::Alignment pos );
    Qt::Alignment fluidicsPackChangedSymbolPosition() const;

    void setSensorChangedSymbolPosition( Qt::Alignment pos );
    Qt::Alignment sensorChangedSymbolPosition() const;

    void setExpectedMeanValue( float meanValue );
    float expectedMeanValue() const;

    void setExpectedStandardDeviation( float sd );
    float expectedStandardDeviation() const; 

    float calculatedMeanValue() const;
    float calculatedStandardDeviation() const;

    void setFluidicsPackChanges( const QVector< QDateTime >& changes );
    QVector< QDateTime > fluidicsPackChanges() const;

    void setSensorChanges( const QVector< QDateTime >& changes );
    QVector< QDateTime > sensorChanges() const;

    void setScanLinePen( const QPen& pen );
    QPen scanLinePen() const;

    void setSymbol( Symbol symbol, const QString& filename );
    QString symbol( Symbol symbol ) const;

    /* \reimpl */
    void setModel( QAbstractItemModel* model );

    QPair< QDateTime, QDateTime > timeRange() const;
    void setTimeRange( const QPair< QDateTime, QDateTime >& timeRange );

protected:
    void paint( PaintContext* paintContext );
    void drawChanges( PaintContext* paintContext );

    virtual void drawDataPointSymbol( PaintContext* paintContext, const QPointF& pos, bool ok );
    virtual void drawLotChangeSymbol( PaintContext* paintContext, const QPointF& pos );
    virtual void drawSensorChangedSymbol( PaintContext* paintContext, const QPointF& pos );
    virtual void drawFluidicsPackChangedSymbol( PaintContext* paintContext, const QPointF& pos );

    virtual QRectF iconRect() const;

    QSvgRenderer* iconRenderer( Symbol symbol );

    /** \reimpl */
    const QPair<QPointF, QPointF> calculateDataBoundaries() const;

protected Q_SLOTS:
    void calculateMeanAndStandardDeviation() const;
}; // End of class KDChartLineDiagram

}

#endif // KDCHARTLINEDIAGRAM_H
