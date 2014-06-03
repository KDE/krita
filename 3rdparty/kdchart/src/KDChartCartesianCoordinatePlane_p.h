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

#ifndef KDCHARTCARTESIANCOORDINATEPLANE_P_H
#define KDCHARTCARTESIANCOORDINATEPLANE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the KD Chart API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "KDChartAbstractCoordinatePlane_p.h"
#include "CartesianCoordinateTransformation.h"
#include "KDChartCartesianGrid.h"

#include <KDABLibFakes>


namespace KDChart {

/**
 * \internal
 */
class CartesianCoordinatePlane::Private : public AbstractCoordinatePlane::Private
{
    friend class CartesianCoordinatePlane;
public:
    explicit Private();
    virtual ~Private() {  }

    virtual void initialize()
    {
        bPaintIsRunning = false;
        coordinateTransformation.axesCalcModeX = Linear;
        coordinateTransformation.axesCalcModeY = Linear;
        grid = new CartesianGrid();
    }

    virtual bool isVisiblePoint(
        const AbstractCoordinatePlane * plane,
        const QPointF& point ) const
    {
        QPointF p = point;
        const CartesianCoordinatePlane* const ref =
            dynamic_cast< const CartesianCoordinatePlane* >( const_cast< AbstractCoordinatePlane* >( plane )->sharedAxisMasterPlane() );
        const CartesianCoordinatePlane* const cartPlane =
            dynamic_cast< const CartesianCoordinatePlane* >( plane );
        if( ref != 0 && ref != cartPlane )
        {
            const QPointF logical = ref->translateBack( point ) - cartPlane->visibleDataRange().topLeft()
                                                                      + ref->visibleDataRange().topLeft();
            p = ref->translate( logical );
        }
        const QRectF geo( plane->geometry() );
        return geo.contains( p );
    }


    // the coordinate plane will calculate the coordinate transformation:
    CoordinateTransformation coordinateTransformation;

    bool bPaintIsRunning;

    // true after setGridAttributes( Qt::Orientation ) was used,
    // false if resetGridAttributes( Qt::Orientation ) was called
    bool hasOwnGridAttributesHorizontal;
    bool hasOwnGridAttributesVertical;

    // true after the first resize event came in
    // bool initialResizeEventReceived;

    // true if the coordinate plane scales isometrically
    bool isometricScaling;

    GridAttributes gridAttributesHorizontal;
    GridAttributes gridAttributesVertical;

    qreal horizontalMin;
    qreal horizontalMax;
    qreal verticalMin;
    qreal verticalMax;

    qreal drawingAreaMarginLeft;
    qreal drawingAreaMarginTop;
    qreal drawingAreaMarginRight;
    qreal drawingAreaMarginBottom;

    // autoAdjustHorizontalRangeToData determines if and how much the horizontal range is adjusted.
    // A value of 100 means that the fixed horizontal range will be used (e.g. set by the user),
    // otherwise the value will be the percentage of the diagram's horizontal range that is to be
    // left empty (i.e., it resembles the 'gap' between the horizontal extrema and the border of the
    // diagram).
    unsigned int autoAdjustHorizontalRangeToData;

    // autoAdjustVerticalRangeToData determines if and how much the vertical range is adjusted.
    // A value of 100 means that the fixed vertical range will be used (e.g. set by the user),
    // otherwise the value will be the percentage of the diagram's vertical range that is to be
    // left empty (i.e., it resembles the 'gap' between the vertical extrema and the border of the
    // diagram).
    unsigned int autoAdjustVerticalRangeToData;
    bool autoAdjustGridToZoom;

    bool fixedDataCoordinateSpaceRelation;
    bool xAxisStartAtZero;
    QRectF fixedDataCoordinateSpaceRelationOldSize;

    DataDimensionsList dimensions;

    bool reverseVerticalPlane;
    bool reverseHorizontalPlane;
};


KDCHART_IMPL_DERIVED_PLANE(CartesianCoordinatePlane, AbstractCoordinatePlane)

}

#endif /* KDCHARTBARDIAGRAM_P_H */
