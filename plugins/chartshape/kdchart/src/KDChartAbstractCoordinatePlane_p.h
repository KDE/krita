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

#ifndef KDCHARTABSTRCOORDINATEPLANE_P_H
#define KDCHARTABSTRCOORDINATEPLANE_P_H

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

#include "KDChartAbstractArea_p.h"
#include <KDChartAbstractCoordinatePlane.h>
#include <KDChartGridAttributes.h>
#include <KDChartAbstractGrid.h>
#include <KDChartZoomParameters.h>

#include <KDABLibFakes>

#include <QStack>

class QRubberBand;

namespace KDChart {


/**
 * \internal
 */
class AbstractCoordinatePlane::Private : public AbstractArea::Private
{
    friend class AbstractCoordinatePlane;
protected:
    explicit Private();
    virtual ~Private(){
        delete grid;
    };

    virtual void initialize()
    {
        qDebug("ERROR: Calling AbstractCoordinatePlane::Private::initialize()");
        // can not call the base class: grid = new AbstractGrid();
    }

    virtual bool isVisiblePoint(
        const AbstractCoordinatePlane * plane,
        const QPointF& point ) const
    {
        Q_UNUSED( plane );
        Q_UNUSED( point );
        return true;
    }

    KDChart::Chart* parent;
    AbstractGrid* grid;
    QRect geometry;
    AbstractDiagramList diagrams;
    GridAttributes gridAttributes;
    AbstractCoordinatePlane *referenceCoordinatePlane;

    bool enableRubberBandZooming;
    QRubberBand* rubberBand;
    QPoint rubberBandOrigin;

    QStack< ZoomParameters > rubberBandZoomConfigHistory;
};


inline AbstractCoordinatePlane::AbstractCoordinatePlane( Private * p, KDChart::Chart* parent )
    : AbstractArea( p )
{
    if( p )
        p->parent = parent;
    init();
}
inline AbstractCoordinatePlane::Private * AbstractCoordinatePlane::d_func()
{
    return static_cast<Private*>( AbstractArea::d_func() );
}
inline const AbstractCoordinatePlane::Private * AbstractCoordinatePlane::d_func() const
{
    return static_cast<const Private*>( AbstractArea::d_func() );
}


}

#endif /* KDCHARTABSTRACTCOORDINATEPLANE_P_H*/
