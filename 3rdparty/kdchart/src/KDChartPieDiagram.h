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

#ifndef KDCHARTPIEDIAGRAM_H
#define KDCHARTPIEDIAGRAM_H

#include "KDChartAbstractPieDiagram.h"

namespace KDChart {

    class DataValueTextInfo;
    typedef class QVector<DataValueTextInfo> DataValueTextInfoList;

/**
  * @brief PieDiagram defines a common pie diagram
  */
class KDCHART_EXPORT PieDiagram : public AbstractPieDiagram
{
    Q_OBJECT

    Q_DISABLE_COPY( PieDiagram )
    KDCHART_DECLARE_DERIVED_DIAGRAM( PieDiagram, PolarCoordinatePlane )

public:
    explicit PieDiagram(
        QWidget* parent = 0, PolarCoordinatePlane* plane = 0 );
    virtual ~PieDiagram();

protected:
    // Implement AbstractDiagram
    /** \reimpl */
    virtual void paint ( PaintContext* paintContext );
    void paintInternal(PaintContext* paintContext, QRectF& textBoundingRect);

public:
    /** \reimpl */
    virtual void resize ( const QSizeF& area );

    // Implement AbstractPolarDiagram
    /** \reimpl */
    virtual double valueTotals () const;
    /** \reimpl */
    virtual double numberOfValuesPerDataset() const;
    /** \reimpl */
    virtual double numberOfGridRings() const;

    virtual PieDiagram * clone() const;

protected:
    /** \reimpl */
    virtual const QPair<QPointF, QPointF> calculateDataBoundaries() const;
    void paintEvent( QPaintEvent* );
    void resizeEvent( QResizeEvent* );

private:
    QRectF piePosition( uint dataset, uint pie ) const;
    void drawOnePie( QPainter* painter,
        DataValueTextInfoList* list,
        uint dataset, uint pie,
        qreal granularity,
        qreal threeDPieHeight );
    void drawPieSurface( QPainter* painter,
        DataValueTextInfoList* list,
        uint dataset, uint pie,
        qreal granularity );
    void draw3DEffect( QPainter* painter,
        const QRectF& drawPosition,
        uint dataset, uint pie,
        qreal granularity,
        const ThreeDPieAttributes& threeDAttrs,
        bool /*explode*/ );
    void drawStraightEffectSegment( QPainter* painter,
        const QRectF& rect,
        qreal threeDHeight,
        qreal angle );
    void drawUpperBrinkEffect( QPainter* painter,
        const QRectF& rect,
        qreal angle );
    void drawArcEffectSegment( QPainter* painter,
        const QRectF& rect,
        qreal threeDHeight,
        qreal startAngle,
        qreal endAngle,
        qreal granularity );
    void drawArcUpperBrinkEffectSegment( QPainter* painter,
        const QRectF& rect,
        qreal startAngle,
        qreal endAngle,
        qreal granularity );
    uint findPieAt( qreal angle, int columnCount );
    uint findLeftPie( uint pie, int columnCount );
    uint findRightPie( uint pie, int columnCount );
    QPointF pointOnCircle( const QRectF& rect, qreal angle );
}; // End of class KDChartPieDiagram

}
#endif // KDCHARTPIEDIAGRAM_H
