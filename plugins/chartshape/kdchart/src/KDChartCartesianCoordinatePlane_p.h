/****************************************************************************
 ** Copyright (C) 2007 Klarälvdalens Datakonsult AB.  All rights reserved.
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

    unsigned int autoAdjustHorizontalRangeToData;
    unsigned int autoAdjustVerticalRangeToData;
    bool autoAdjustGridToZoom;

    bool fixedDataCoordinateSpaceRelation;
    QRectF fixedDataCoordinateSpaceRelationOldSize;

    DataDimensionsList dimensions;

    bool reverseVerticalPlane;
    bool reverseHorizontalPlane;
};


KDCHART_IMPL_DERIVED_PLANE(CartesianCoordinatePlane, AbstractCoordinatePlane)

}

#endif /* KDCHARTBARDIAGRAM_P_H */
