/****************************************************************************
 ** Copyright (C) 2007 Klar�vdalens Datakonsult AB.  All rights reserved.
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

#include "KDChartPolarCoordinatePlane.h"
#include "KDChartPolarCoordinatePlane_p.h"

#include "KDChartPainterSaver_p.h"
#include "KDChartChart.h"
#include "KDChartPaintContext.h"
#include "KDChartAbstractDiagram.h"
#include "KDChartAbstractPolarDiagram.h"
#include "KDChartPolarDiagram.h"

#include <math.h>

#include <QFont>
#include <QList>
#include <QtDebug>
#include <QPainter>
#include <QTimer>

#include <KDABLibFakes>

using namespace KDChart;

#define d d_func()


/*
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define DEGTORAD(d) (d)*M_PI/180

struct PolarCoordinatePlane::CoordinateTransformation
{
    // represents the distance of the diagram coordinate origin to the
    // origin of the coordinate plane space:
    QPointF originTranslation;
    double radiusUnit;
    double angleUnit;

    ZoomParameters zoom;

    static QPointF polarToCartesian( double R, double theta )
    {
        return QPointF( R * cos( DEGTORAD( theta  ) ), R * sin( DEGTORAD( theta ) ) );
    }

    inline const QPointF translate( const QPointF& diagramPoint ) const
    {
        // calculate the polar coordinates
        const double x = diagramPoint.x() * radiusUnit;
        const double y = ( diagramPoint.y() * angleUnit) - 90;
        // convert to cartesian coordinates
        QPointF cartesianPoint = polarToCartesian( x, y );
        cartesianPoint.setX( cartesianPoint.x() * zoom.xFactor );
        cartesianPoint.setY( cartesianPoint.y() * zoom.yFactor );

        QPointF newOrigin = originTranslation;
        double minOrigin = qMin( newOrigin.x(), newOrigin.y() );
        newOrigin.setX( newOrigin.x() + minOrigin * ( 1 - zoom.xCenter * 2 ) * zoom.xFactor );
        newOrigin.setY( newOrigin.y() + minOrigin * ( 1 - zoom.yCenter * 2 ) * zoom.yFactor );

        return newOrigin + cartesianPoint;
    }

    inline const QPointF translatePolar( const QPointF& diagramPoint ) const
    {
        return QPointF( diagramPoint.x() * angleUnit, diagramPoint.y() * radiusUnit );
    }
};

class PolarCoordinatePlane::Private
{
public:
    Private()
        :currentTransformation(0),
        initialResizeEventReceived(false )
        {}


    // the coordinate plane will calculate coordinate transformations for all
    // diagrams and store them here:
    CoordinateTransformationList coordinateTransformations;
    // when painting, this pointer selects the coordinate transformation for
    // the current diagram:
    CoordinateTransformation* currentTransformation;
    // the reactangle occupied by the diagrams, in plane coordinates
    QRectF contentRect;
    // true after the first resize event came in
    bool initialResizeEventReceived;
};
*/

PolarCoordinatePlane::PolarCoordinatePlane ( Chart* parent )
    : AbstractCoordinatePlane ( new Private(), parent )
{
    // this bloc left empty intentionally
}

PolarCoordinatePlane::~PolarCoordinatePlane()
{
    // this bloc left empty intentionally
}

void PolarCoordinatePlane::init()
{
    // this bloc left empty intentionally
}

void PolarCoordinatePlane::addDiagram ( AbstractDiagram* diagram )
{
    Q_ASSERT_X ( dynamic_cast<AbstractPolarDiagram*> ( diagram ),
                 "PolarCoordinatePlane::addDiagram", "Only polar"
                 "diagrams can be added to a polar coordinate plane!" );
    AbstractCoordinatePlane::addDiagram ( diagram );
    connect ( diagram,  SIGNAL ( layoutChanged ( AbstractDiagram* ) ),
              SLOT ( slotLayoutChanged ( AbstractDiagram* ) ) );

}

void PolarCoordinatePlane::paint ( QPainter* painter )
{
    AbstractDiagramList diags = diagrams();
    if ( d->coordinateTransformations.size() == diags.size() )
    {
        PaintContext ctx;
        ctx.setPainter ( painter );
        ctx.setCoordinatePlane ( this );
        ctx.setRectangle ( geometry() /*d->contentRect*/ );

        // 1. ask the diagrams if they need additional space for data labels / data comments
        const qreal oldZoomX = zoomFactorX();
        const qreal oldZoomY = zoomFactorY();
        d->newZoomX = oldZoomX;
        d->newZoomY = oldZoomY;
        for ( int i = 0; i < diags.size(); i++ )
        {
            d->currentTransformation = & ( d->coordinateTransformations[i] );
            qreal zoomX;
            qreal zoomY;
            PolarDiagram* polarDia = dynamic_cast<PolarDiagram*> ( diags[i] );
            if( polarDia ){
                polarDia->paint ( &ctx, true, zoomX, zoomY );
                d->newZoomX = qMin(d->newZoomX, zoomX);
                d->newZoomY = qMin(d->newZoomY, zoomY);
            }
        }
        d->currentTransformation = 0;

        // if re-scaling is needed start the timer and bail out
        if( d->newZoomX != oldZoomX || d->newZoomY != oldZoomY ){
            //qDebug()<<"new zoom:"<<d->newZoomY<<"  old zoom"<<oldZoomY;
            QTimer::singleShot(10, this, SLOT(adjustZoomAndRepaint()));
            return;
        }

        // 2. there was room enough for the labels, so we start drawing

        // paint the coordinate system rulers:
        d->currentTransformation = & ( d->coordinateTransformations.first() );

        d->grid->drawGrid( &ctx );

        // paint the diagrams which will re-use their DataValueTextInfoList(s) filled in step 1:
        for ( int i = 0; i < diags.size(); i++ )
        {
            d->currentTransformation = & ( d->coordinateTransformations[i] );
            PainterSaver painterSaver( painter );
            PolarDiagram* polarDia = dynamic_cast<PolarDiagram*> ( diags[i] );
            if( polarDia ){
                qreal dummy1, dummy2;
                polarDia->paint ( &ctx, false, dummy1, dummy2 );
            }else{
                diags[i]->paint ( &ctx );
            }
        }
        d->currentTransformation = 0;
    } // else: diagrams have not been set up yet
}


void PolarCoordinatePlane::adjustZoomAndRepaint()
{
    const qreal newZoom = qMin(d->newZoomX, d->newZoomY);
    setZoomFactors(newZoom, newZoom);
    update();
}


void PolarCoordinatePlane::resizeEvent ( QResizeEvent* )
{
    d->initialResizeEventReceived = true;
    layoutDiagrams();
}

void PolarCoordinatePlane::layoutDiagrams()
{
    // the rectangle the diagrams cover in the *plane*:
    // (Why -3? We save 1px on each side for the antialiased drawing, and
    // respect the way QPainter calculates the width of a painted rect (the
    // size is the rectangle size plus the pen width). This way, most clipping
    // for regular pens should be avoided. When pens with a penWidth or larger
    // than 1 are used, this may not b sufficient.
    const QRect rect( areaGeometry() );
    d->contentRect = QRectF ( 1, 1, rect.width() - 3, rect.height() - 3 );

    const ZoomParameters zoom = d->coordinateTransformations.isEmpty() ? ZoomParameters() 
                                                                       : d->coordinateTransformations.front().zoom;
    // FIXME distribute space according to options:
    const qreal oldStartPosition = startPosition();
    d->coordinateTransformations.clear();
    Q_FOREACH( AbstractDiagram* diagram, diagrams() )
        {
            AbstractPolarDiagram *polarDiagram = dynamic_cast<AbstractPolarDiagram*>( diagram );
            Q_ASSERT( polarDiagram );
            QPair<QPointF, QPointF> dataBoundariesPair = polarDiagram->dataBoundaries();

            const double angleUnit = 360 / polarDiagram->valueTotals();
//qDebug() << "--------------------------------------------------------";
            const double radius = qAbs( dataBoundariesPair.first.y() ) + dataBoundariesPair.second.y();
//qDebug() << radius <<"="<<dataBoundariesPair.second.y();
            const double diagramWidth = radius * 2; // == height
            const double planeWidth = d->contentRect.width();
            const double planeHeight = d->contentRect.height();
            const double radiusUnit = qMin( planeWidth, planeHeight ) / diagramWidth;
//qDebug() << radiusUnit <<"=" << "qMin( "<<planeWidth<<","<< planeHeight <<") / "<<diagramWidth;
            QPointF coordinateOrigin = QPointF ( planeWidth / 2, planeHeight / 2 );
            coordinateOrigin += d->contentRect.topLeft();

            CoordinateTransformation diagramTransposition;
            diagramTransposition.originTranslation = coordinateOrigin;
            diagramTransposition.radiusUnit = radiusUnit;
            diagramTransposition.angleUnit = angleUnit;
            diagramTransposition.startPosition = oldStartPosition;
            diagramTransposition.zoom = zoom;
            diagramTransposition.minValue = dataBoundariesPair.first.y() < 0 ? dataBoundariesPair.first.y() : 0.0;
            d->coordinateTransformations.append( diagramTransposition );
        }
}

const QPointF PolarCoordinatePlane::translate( const QPointF& diagramPoint ) const
{
    Q_ASSERT_X ( d->currentTransformation != 0, "PolarCoordinatePlane::translate",
                 "Only call translate() from within paint()." );
    return  d->currentTransformation->translate ( diagramPoint );
}

const QPointF PolarCoordinatePlane::translatePolar( const QPointF& diagramPoint ) const
{
    Q_ASSERT_X ( d->currentTransformation != 0, "PolarCoordinatePlane::translate",
                 "Only call translate() from within paint()." );
    return  d->currentTransformation->translatePolar ( diagramPoint );
}

qreal PolarCoordinatePlane::angleUnit() const
{
    Q_ASSERT_X ( d->currentTransformation != 0, "PolarCoordinatePlane::angleUnit",
                 "Only call angleUnit() from within paint()." );
    return  d->currentTransformation->angleUnit;
}

qreal PolarCoordinatePlane::radiusUnit() const
{
    Q_ASSERT_X ( d->currentTransformation != 0, "PolarCoordinatePlane::radiusUnit",
                 "Only call radiusUnit() from within paint()." );
    return  d->currentTransformation->radiusUnit;
}

void PolarCoordinatePlane::slotLayoutChanged ( AbstractDiagram* )
{
    if ( d->initialResizeEventReceived ) layoutDiagrams();
}

void PolarCoordinatePlane::setStartPosition( qreal degrees )
{
    Q_ASSERT_X ( diagram(), "PolarCoordinatePlane::setStartPosition",
                 "setStartPosition() needs a diagram to be associated to the plane." );
    for( CoordinateTransformationList::iterator it = d->coordinateTransformations.begin(); 
                                                it != d->coordinateTransformations.end();
                                                ++it )
    {
        CoordinateTransformation& trans = *it;
        trans.startPosition = degrees;
    }
}

qreal PolarCoordinatePlane::startPosition() const
{
    return d->coordinateTransformations.isEmpty()
        ? 0.0
        :  d->coordinateTransformations.first().startPosition;
}

double PolarCoordinatePlane::zoomFactorX() const
{
    return d->coordinateTransformations.isEmpty()
        ? 1.0
        : d->coordinateTransformations.first().zoom.xFactor;
}

double PolarCoordinatePlane::zoomFactorY() const
{
    return d->coordinateTransformations.isEmpty()
        ? 1.0
        : d->coordinateTransformations.first().zoom.yFactor;
}

void PolarCoordinatePlane::setZoomFactors( double factorX, double factorY )
{
    setZoomFactorX( factorX );
    setZoomFactorY( factorY );
}

void PolarCoordinatePlane::setZoomFactorX( double factor )
{
    for( CoordinateTransformationList::iterator it = d->coordinateTransformations.begin(); 
                                                it != d->coordinateTransformations.end();
                                                ++it )
    {
        CoordinateTransformation& trans = *it;
        trans.zoom.xFactor = factor;
    }
}

void PolarCoordinatePlane::setZoomFactorY( double factor )
{
    for( CoordinateTransformationList::iterator it = d->coordinateTransformations.begin(); 
                                                it != d->coordinateTransformations.end();
                                                ++it )
    {
        CoordinateTransformation& trans = *it;
        trans.zoom.yFactor = factor;
    }
}

QPointF PolarCoordinatePlane::zoomCenter() const
{
    return d->coordinateTransformations.isEmpty()
        ? QPointF( 0.5, 0.5 )
        : QPointF( d->coordinateTransformations.first().zoom.xCenter, d->coordinateTransformations.first().zoom.yCenter );
}

void PolarCoordinatePlane::setZoomCenter( const QPointF& center )
{
    for( CoordinateTransformationList::iterator it = d->coordinateTransformations.begin(); 
                                                it != d->coordinateTransformations.end();
                                                ++it )
    {
        CoordinateTransformation& trans = *it;
        trans.zoom.xCenter = center.x();
        trans.zoom.yCenter = center.y();
    }
}

DataDimensionsList PolarCoordinatePlane::getDataDimensionsList() const
{
    DataDimensionsList l;

    //FIXME(khz): do the real calculation

    return l;
}

void KDChart::PolarCoordinatePlane::setGridAttributes(
    bool circular,
    const GridAttributes& a )
{
    if( circular )
        d->gridAttributesCircular = a;
    else
        d->gridAttributesSagittal = a;
    setHasOwnGridAttributes( circular, true );
    update();
    emit propertiesChanged();
}

void KDChart::PolarCoordinatePlane::resetGridAttributes(
    bool circular )
{
    setHasOwnGridAttributes( circular, false );
    update();
}

const GridAttributes KDChart::PolarCoordinatePlane::gridAttributes(
    bool circular ) const
{
    if( hasOwnGridAttributes( circular ) ){
        if( circular )
            return d->gridAttributesCircular;
        else
            return d->gridAttributesSagittal;
    }else{
        return globalGridAttributes();
    }
}

void KDChart::PolarCoordinatePlane::setHasOwnGridAttributes(
    bool circular, bool on )
{
    if( circular )
        d->hasOwnGridAttributesCircular = on;
    else
        d->hasOwnGridAttributesSagittal = on;
    emit propertiesChanged();
}

bool KDChart::PolarCoordinatePlane::hasOwnGridAttributes(
    bool circular ) const
{
    return
        ( circular )
        ? d->hasOwnGridAttributesCircular
        : d->hasOwnGridAttributesSagittal;
}
