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
}
