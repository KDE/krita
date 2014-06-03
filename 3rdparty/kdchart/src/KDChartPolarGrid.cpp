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

#include "KDChartPolarGrid.h"
#include "KDChartPaintContext.h"
#include "KDChartPolarDiagram.h"
#include "KDChartPieDiagram.h"
#include "KDChartPrintingParameters.h"

#include <QPainter>

#include <KDABLibFakes>

using namespace KDChart;


DataDimensionsList PolarGrid::calculateGrid(
    const DataDimensionsList& rawDataDimensions ) const
{
    qDebug("Calling PolarGrid::calculateGrid()");
    DataDimensionsList l;

    //FIXME(khz): do the real calculation

    l = rawDataDimensions;

    return l;
}


void PolarGrid::drawGrid( PaintContext* context )
{
//    if ( d->coordinateTransformations.size () <= 0 ) return;

    const QBrush backupBrush( context->painter()->brush() );
    context->painter()->setBrush( QBrush() );
    PolarCoordinatePlane* plane = dynamic_cast<PolarCoordinatePlane*>(context->coordinatePlane());
    Q_ASSERT_X ( plane, "PolarGrid::drawGrid",
                 "Bad function call: PaintContext::coodinatePlane() NOT a polar plane." );

    const GridAttributes gridAttrsCircular( plane->gridAttributes( true ) );
    const GridAttributes gridAttrsSagittal( plane->gridAttributes( false ) );

    //qDebug() << "OK:";
    if ( !gridAttrsCircular.isGridVisible() && !gridAttrsSagittal.isGridVisible() ) return;
    //qDebug() << "A";

    // FIXME: we paint the rulers to the settings of the first diagram for now:
    AbstractPolarDiagram* dgr = dynamic_cast<AbstractPolarDiagram*> (plane->diagrams().first() );
    Q_ASSERT ( dgr ); // only polar diagrams are allowed here


    // Do not draw a grid for pie diagrams
    if( dynamic_cast<PieDiagram*> (plane->diagrams().first() ) ) return;


    context->painter()->setPen ( PrintingParameters::scalePen( QColor ( Qt::lightGray ) ) );
    const double min = dgr->dataBoundaries().first.y();
    QPointF origin = plane->translate( QPointF( min, 0 ) ) + context->rectangle().topLeft();
    //qDebug() << "origin" << origin;

    const double r = qAbs( min ) + dgr->dataBoundaries().second.y(); // use the full extents

    if ( gridAttrsSagittal.isGridVisible() ){
        const int numberOfSpokes = ( int ) ( 360 / plane->angleUnit() );
        for ( int i = 0; i < numberOfSpokes ; ++i ) {
            context->painter()->drawLine( origin, plane->translate( QPointF( r - qAbs( min ), i ) ) + context->rectangle().topLeft() );
        }
    }

    if ( gridAttrsCircular.isGridVisible() )
    {
        const qreal startPos = plane->startPosition();
        plane->setStartPosition( 0.0 );
        const int numberOfGridRings = ( int )dgr->numberOfGridRings();
        for ( int j = 0; j < numberOfGridRings; ++j ) {
            const double rad = min - ( ( j + 1) * r / numberOfGridRings );
    
            if ( rad == 0 )
                continue;
    
            QRectF rect;
            QPointF topLeftPoint;
            QPointF bottomRightPoint;

            topLeftPoint = plane->translate( QPointF( rad, 0 ) );
            topLeftPoint.setX( plane->translate( QPointF( rad, 90 / plane->angleUnit() ) ).x() );
            bottomRightPoint = plane->translate( QPointF( rad, 180 / plane->angleUnit() ) );
            bottomRightPoint.setX( plane->translate( QPointF( rad, 270 / plane->angleUnit() ) ).x() );

            rect.setTopLeft( topLeftPoint + context->rectangle().topLeft() );
            rect.setBottomRight( bottomRightPoint + context->rectangle().topLeft() );

            context->painter()->drawEllipse( rect );
        }
        plane->setStartPosition( startPos );
    }
    context->painter()->setBrush( backupBrush );
}
