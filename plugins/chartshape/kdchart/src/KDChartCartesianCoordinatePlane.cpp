/****************************************************************************
 ** Copyright (C) 2007 Klaralvdalens Datakonsult AB.  All rights reserved.
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

#include "KDChartCartesianCoordinatePlane.h"
#include "KDChartCartesianCoordinatePlane_p.h"

#include <QFont>
#include <QList>
#include <QtDebug>
#include <QPainter>
#include <QApplication>

#include "KDChartAbstractDiagram.h"
#include "KDChartAbstractCartesianDiagram.h"
#include "CartesianCoordinateTransformation.h"
#include "KDChartGridAttributes.h"
#include "KDChartPaintContext.h"
#include "KDChartPainterSaver_p.h"
#include "KDChartBarDiagram.h"

#include <KDABLibFakes>


using namespace KDChart;

#define d d_func()

CartesianCoordinatePlane::Private::Private()
    : AbstractCoordinatePlane::Private()
    , bPaintIsRunning( false )
    , hasOwnGridAttributesHorizontal ( false )
    , hasOwnGridAttributesVertical ( false )
    // old: , initialResizeEventReceived ( false )
    , isometricScaling ( false )
    , horizontalMin(0)
    , horizontalMax(0)
    , verticalMin(0)
    , verticalMax(0)
    , autoAdjustHorizontalRangeToData(67)
    , autoAdjustVerticalRangeToData(  67)
    , autoAdjustGridToZoom( true )
    , fixedDataCoordinateSpaceRelation( false )
    , reverseVerticalPlane( false )
    , reverseHorizontalPlane( false )
{
}

CartesianCoordinatePlane::CartesianCoordinatePlane ( Chart* parent )
    : AbstractCoordinatePlane ( new Private(), parent )
{
    // this bloc left empty intentionally
}

CartesianCoordinatePlane::~CartesianCoordinatePlane()
{
    // this bloc left empty intentionally
}

void CartesianCoordinatePlane::init()
{
    // this bloc left empty intentionally
}


void CartesianCoordinatePlane::addDiagram ( AbstractDiagram* diagram )
{
    Q_ASSERT_X ( dynamic_cast<AbstractCartesianDiagram*> ( diagram ),
                 "CartesianCoordinatePlane::addDiagram", "Only cartesian "
                 "diagrams can be added to a cartesian coordinate plane!" );
    AbstractCoordinatePlane::addDiagram ( diagram );
    connect ( diagram,  SIGNAL ( layoutChanged ( AbstractDiagram* ) ),
              SLOT ( slotLayoutChanged ( AbstractDiagram* ) ) );

    connect( diagram, SIGNAL( propertiesChanged() ),this, SIGNAL( propertiesChanged() ) );
}


void CartesianCoordinatePlane::paint ( QPainter* painter )
{
    // prevent recursive call:
    //qDebug("attempt plane::paint()");
    if( d->bPaintIsRunning ){
        return;
    }
    d->bPaintIsRunning = true;

    //qDebug() << "start plane::paint()";

    AbstractDiagramList diags = diagrams();
    if ( !diags.isEmpty() )
    {
        PaintContext ctx;
        ctx.setPainter ( painter );
        ctx.setCoordinatePlane ( this );
        const QRectF drawArea( drawingArea() );
        ctx.setRectangle ( drawArea );

        // enabling clipping so that we're not drawing outside
        PainterSaver painterSaver( painter );
        QRect clipRect = drawArea.toRect().adjusted( -1, -1, 1, 1 );
        QRegion clipRegion( clipRect );
        painter->setClipRegion( clipRegion );

        // paint the coordinate system rulers:
        d->grid->drawGrid( &ctx );

        // paint the diagrams:
        for ( int i = 0; i < diags.size(); i++ )
        {
//qDebug("  start diags[i]->paint ( &ctx );");
            PainterSaver diagramPainterSaver( painter );
            diags[i]->paint ( &ctx );
//qDebug("  done: diags[i]->paint ( &ctx );");
        }

        //for debugging:
        //    painter->drawRect( drawArea.adjusted(4,4,-4,-4) );
        //    painter->drawRect( drawArea.adjusted(2,2,-2,-2) );
        //    painter->drawRect( drawArea );
    }
    d->bPaintIsRunning = false;
    //qDebug("done: plane::paint()");
}


void CartesianCoordinatePlane::slotLayoutChanged ( AbstractDiagram* )
{
    // old: if ( d->initialResizeEventReceived )
    layoutDiagrams();
}

QRectF CartesianCoordinatePlane::getRawDataBoundingRectFromDiagrams() const
{
    // determine unit of the rectangles of all involved diagrams:
    qreal minX, maxX, minY, maxY;
    bool bStarting = true;
    Q_FOREACH( const AbstractDiagram* diagram, diagrams() )
    {
        QPair<QPointF, QPointF> dataBoundariesPair = diagram->dataBoundaries();
        //qDebug() << "CartesianCoordinatePlane::getRawDataBoundingRectFromDiagrams()\ngets diagram->dataBoundaries: " << dataBoundariesPair.first << dataBoundariesPair.second;
        if ( bStarting || dataBoundariesPair.first.x()  < minX ) minX = dataBoundariesPair.first.x();
        if ( bStarting || dataBoundariesPair.first.y()  < minY ) minY = dataBoundariesPair.first.y();
        if ( bStarting || dataBoundariesPair.second.x() > maxX ) maxX = dataBoundariesPair.second.x();
        if ( bStarting || dataBoundariesPair.second.y() > maxY ) maxY = dataBoundariesPair.second.y();
        bStarting = false;
    }
    //qDebug() << "CartesianCoordinatePlane::getRawDataBoundingRectFromDiagrams()\nreturns data boundaries: " << QRectF( QPointF(minX, minY), QSizeF(maxX - minX, maxY - minY) );
    QRectF dataBoundingRect;
    dataBoundingRect.setBottomLeft( QPointF(minX, minY) );
    dataBoundingRect.setTopRight(   QPointF(maxX, maxY) );
    return dataBoundingRect;
}


QRectF CartesianCoordinatePlane::adjustedToMaxEmptyInnerPercentage(
        const QRectF& r, unsigned int percentX, unsigned int percentY ) const
{
    QRectF erg( r );
    if( ( axesCalcModeX() != Logarithmic || r.left() < 0.0 ) && (percentX > 0) && (percentX != 100) ) {
        const bool isPositive = (r.left() >= 0);
        if( (r.right() >= 0) == isPositive ){
            const qreal innerBound =
                    isPositive ? qMin(r.left(), r.right()) : qMax(r.left(), r.right());
            const qreal outerBound =
                    isPositive ? qMax(r.left(), r.right()) : qMin(r.left(), r.right());
            if( innerBound / outerBound * 100 <= percentX )
            {
                if( isPositive )
                    erg.setLeft( 0.0 );
                else
                    erg.setRight( 0.0 );
            }
        }
    }
    if( ( axesCalcModeY() != Logarithmic || r.bottom() < 0.0 ) && (percentY > 0) && (percentY != 100) ) {
        //qDebug() << erg.bottom() << erg.top();
        const bool isPositive = (r.bottom() >= 0);
        if( (r.top() >= 0) == isPositive ){
            const qreal innerBound =
                    isPositive ? qMin(r.top(), r.bottom()) : qMax(r.top(), r.bottom());
            const qreal outerBound =
                    isPositive ? qMax(r.top(), r.bottom()) : qMin(r.top(), r.bottom());
            //qDebug() << innerBound << outerBound;
            if( innerBound / outerBound * 100 <= percentY )
            {
                if( isPositive )
                    erg.setBottom( 0.0 );
                else
                    erg.setTop( 0.0 );
            }
        }
        //qDebug() << erg.bottom() << erg.top() << "!!";
    }
    return erg;
}


QRectF CartesianCoordinatePlane::calculateRawDataBoundingRect() const
{
    // are manually set ranges to be applied?
    const bool bAutoAdjustHorizontalRange = (d->autoAdjustHorizontalRangeToData < 100);
    const bool bAutoAdjustVerticalRange   = (d->autoAdjustVerticalRangeToData   < 100);

    const bool bHardHorizontalRange = (d->horizontalMin != d->horizontalMax) && ! bAutoAdjustHorizontalRange;
    const bool bHardVerticalRange   = (d->verticalMin   != d->verticalMax)   && ! bAutoAdjustVerticalRange;
    QRectF dataBoundingRect;

    // if custom boundaries are set on the plane, use them
    if ( bHardHorizontalRange && bHardVerticalRange ) {
        dataBoundingRect.setLeft(   d->horizontalMin );
        dataBoundingRect.setRight(  d->horizontalMax );
        dataBoundingRect.setBottom( d->verticalMin );
        dataBoundingRect.setTop(    d->verticalMax );
    }else{
        // determine unit of the rectangles of all involved diagrams:
        dataBoundingRect = getRawDataBoundingRectFromDiagrams();
        if ( bHardHorizontalRange ) {
            dataBoundingRect.setLeft(  d->horizontalMin );
            dataBoundingRect.setRight( d->horizontalMax );
        }
        if ( bHardVerticalRange ) {
            dataBoundingRect.setBottom( d->verticalMin );
            dataBoundingRect.setTop(    d->verticalMax );
        }
    }
    // recalculate the bounds, if automatic adjusting of ranges is desired AND
    //                         both bounds are at the same side of the zero line
    dataBoundingRect = adjustedToMaxEmptyInnerPercentage(
            dataBoundingRect, d->autoAdjustHorizontalRangeToData, d->autoAdjustVerticalRangeToData );
    if( bAutoAdjustHorizontalRange ){
        const_cast<CartesianCoordinatePlane::Private *>(d)->horizontalMin = dataBoundingRect.left();
        const_cast<CartesianCoordinatePlane::Private *>(d)->horizontalMax = dataBoundingRect.right();
    }
    if( bAutoAdjustVerticalRange ){
        const_cast<CartesianCoordinatePlane*>(this)->d->verticalMin = dataBoundingRect.bottom();
        const_cast<CartesianCoordinatePlane*>(this)->d->verticalMax = dataBoundingRect.top();
    }
    //qDebug() << "CartesianCoordinatePlane::calculateRawDataBoundingRect()\nreturns data boundaries: " << dataBoundingRect;
    return dataBoundingRect;
}


DataDimensionsList CartesianCoordinatePlane::getDataDimensionsList() const
{

    DataDimensionsList l;
    const AbstractCartesianDiagram* dgr
        = diagrams().isEmpty() ? 0 : dynamic_cast<const AbstractCartesianDiagram*> (diagrams().first() );
    if( dgr && dgr->referenceDiagram() )
    	dgr = dgr->referenceDiagram();
	const BarDiagram *barDiagram = qobject_cast< const BarDiagram* >( dgr );

	// note:
	// It does make sense to retrieve the orientation from the first diagram. This is because
	// a coordinate plane can either be for horizontal *or* for vertical diagrams. Both at the
	// same time won't work, and thus the orientation for all diagrams is the same as for the first one.
	const Qt::Orientation diagramOrientation = barDiagram != 0 ? barDiagram->orientation() : Qt::Vertical;

    const bool diagramIsVertical = diagramOrientation == Qt::Vertical;


    if( dgr ){
        const QRectF r( calculateRawDataBoundingRect() );
        // note:
        // We do *not* access d->gridAttributesHorizontal here, but
        // we use the getter function, to get the global attrs, if no
        // special ones have been set for the respective orientation.
        const GridAttributes gaH( gridAttributes( Qt::Horizontal ) );
        const GridAttributes gaV( gridAttributes( Qt::Vertical ) );
        // append the first dimension: for Abscissa axes
        l.append(
            DataDimension(
                r.left(), r.right(),
                diagramIsVertical ? ( dgr->datasetDimension() > 1 ) : true,
                axesCalcModeX(),
                gaH.gridGranularitySequence(),
                gaH.gridStepWidth(),
                gaH.gridSubStepWidth() ) );
        // append the second dimension: for Ordinate axes
        l.append(
            DataDimension(
                r.bottom(), r.top(),
                diagramIsVertical ? true : ( dgr->datasetDimension() > 1 ),
                axesCalcModeY(),
                gaV.gridGranularitySequence(),
                gaV.gridStepWidth(),
                gaV.gridSubStepWidth() ) );
    }else{
        l.append( DataDimension() ); // This gets us the default 1..0 / 1..0 grid
        l.append( DataDimension() ); // shown, if there is no diagram on this plane.
    }
    return l;
}

QRectF CartesianCoordinatePlane::drawingArea() const
{
    // the rectangle the diagrams cover in the *plane*:
    // (Why -3? We save 1px on each side for the antialiased drawing, and
    // respect the way QPainter calculates the width of a painted rect (the
    // size is the rectangle size plus the pen width). This way, most clipping
    // for regular pens should be avoided. When pens with a penWidth or larger
    // than 1 are used, this may not be sufficient.
    const QRect rect( areaGeometry() );
    return QRectF ( rect.left()+1, rect.top()+1, rect.width() - 3, rect.height() - 3 );
}


QRectF CartesianCoordinatePlane::logicalArea() const
{
    if ( d->dimensions.isEmpty() )
        return QRectF();

    const DataDimension dimX = d->dimensions.first();
    const DataDimension dimY = d->dimensions.last();
    const QPointF pt( qMin( dimX.start, dimX.end ), qMax( dimY.start, dimY.end ) );
    const QSizeF siz( qAbs( dimX.distance() ), -qAbs( dimY.distance() ) );
    const QRectF dataBoundingRect( pt, siz );

    // determine logical top left, taking the "reverse" option of
    // horizontal and vertical dimension into account
    QPointF topLeft;
    if( !d->reverseVerticalPlane && !d->reverseHorizontalPlane )
        topLeft = dataBoundingRect.topLeft();
    else if( d->reverseVerticalPlane && !d->reverseHorizontalPlane )
        topLeft = dataBoundingRect.bottomLeft();
    else if( d->reverseVerticalPlane && d->reverseHorizontalPlane )
        topLeft = dataBoundingRect.bottomRight();
    else if( !d->reverseVerticalPlane && d->reverseHorizontalPlane )
        topLeft = dataBoundingRect.topRight();

    const double width  = dataBoundingRect.width()  * ( d->reverseHorizontalPlane ? -1.0 : 1.0 );
    const double height = dataBoundingRect.height() * ( d->reverseVerticalPlane   ? -1.0 : 1.0 );

    return QRectF( topLeft, QSizeF( width, height ) );
}

QRectF CartesianCoordinatePlane::diagramArea() const
{
    const QRectF logArea( logicalArea() );
    QPointF physicalTopLeft = d->coordinateTransformation.translate( logArea.topLeft() );
    QPointF physicalBottomRight = d->coordinateTransformation.translate( logArea.bottomRight() );

    return QRectF( physicalTopLeft, physicalBottomRight ).normalized();
}

QRectF CartesianCoordinatePlane::visibleDiagramArea() const
{
    return diagramArea().intersected( drawingArea() );
}

void CartesianCoordinatePlane::layoutDiagrams()
{
    if ( diagrams().isEmpty() )
    {   // FIXME evaluate what can still be prepared
        // FIXME decide default dimension if no diagrams are present (to make empty planes useable)
    }

    d->dimensions = gridDimensionsList();
    // test for programming errors: critical
    Q_ASSERT_X ( d->dimensions.count() == 2, "CartesianCoordinatePlane::layoutDiagrams",
                 "Error: gridDimensionsList() did not return exactly two dimensions." );

    // physical area of the plane
    const QRectF physicalArea( drawingArea() );
    // .. in contrast to the logical area
    const QRectF logArea( logicalArea() );

    d->coordinateTransformation.unitVectorX = logArea.width()  != 0 ? physicalArea.width()  / logArea.width()  : 1.0;
    d->coordinateTransformation.unitVectorY = logArea.height() != 0 ? physicalArea.height() / logArea.height() : 1.0;

    const double diagramXUnitInCoordinatePlane = d->coordinateTransformation.unitVectorX;
    const double diagramYUnitInCoordinatePlane = d->coordinateTransformation.unitVectorY;

    double scaleX;
    double scaleY;

    // calculate isometric scaling factor to maxscale the diagram into
    // the coordinate system:
    if ( d->isometricScaling )
    {
        double scale = qMin ( qAbs ( diagramXUnitInCoordinatePlane ),
                              qAbs ( diagramYUnitInCoordinatePlane ) );

        scaleX = qAbs( scale / diagramXUnitInCoordinatePlane );
        scaleY = qAbs( scale / diagramYUnitInCoordinatePlane );
    } else {
        scaleX = 1.0;
        scaleY = 1.0;
    }

    const QPointF logicalTopLeft = logArea.topLeft();
    // calculate diagram origin in plane coordinates:
    QPointF coordinateOrigin = QPointF ( logicalTopLeft.x() * -diagramXUnitInCoordinatePlane,
                                         logicalTopLeft.y() * -diagramYUnitInCoordinatePlane );
    coordinateOrigin += physicalArea.topLeft();

    d->coordinateTransformation.originTranslation = coordinateOrigin;

    // As in the first quadrant of the coordinate system, the origin is the bottom left, not top left.
    // This origin is then the top left point of the resulting diagramRect for our coordinateTransformation.
    const QRectF normalizedLogArea = logArea.normalized();
    d->coordinateTransformation.diagramRect = QRectF( normalizedLogArea.bottomLeft(), normalizedLogArea.topRight() );

    d->coordinateTransformation.isoScaleX = scaleX;
    d->coordinateTransformation.isoScaleY = scaleY;

    // the plane area might have changed, so the zoom values might also be changed
    handleFixedDataCoordinateSpaceRelation( physicalArea );

    update();
}

void CartesianCoordinatePlane::setFixedDataCoordinateSpaceRelation( bool fixed )
{
    d->fixedDataCoordinateSpaceRelation = fixed;
    d->fixedDataCoordinateSpaceRelationOldSize = QRectF();
    /*
    //TODO(khz): We need to discuss if we want to do this:
    if( ! fixed ){
        bool bChanged = false;
        if( doneSetZoomFactorY( 1.0 ) )
            bChanged = true;
        if( doneSetZoomFactorX( 1.0 ) )
            bChanged = true;
        if( doneSetZoomCenter( QPointF(0.5, 0.5) ) )
            bChanged = true;
        if( bChanged ){
            emit propertiesChanged();
        }
    }
    */
}

bool CartesianCoordinatePlane::hasFixedDataCoordinateSpaceRelation() const
{
    return d->fixedDataCoordinateSpaceRelation;
}

void CartesianCoordinatePlane::handleFixedDataCoordinateSpaceRelation( const QRectF& geometry )
{
    // is the feature enabled?
    if( !d->fixedDataCoordinateSpaceRelation )
        return;

    // is the new geometry ok?
    if( geometry.height() < 1 || geometry.width() < 1 )
        return;

    // if the size was changed, we calculate new zoom settings
    if( d->fixedDataCoordinateSpaceRelationOldSize != geometry && !d->fixedDataCoordinateSpaceRelationOldSize.isNull() )
    {
        const double newZoomX = zoomFactorX() * d->fixedDataCoordinateSpaceRelationOldSize.width() / geometry.width();
        const double newZoomY = zoomFactorY() * d->fixedDataCoordinateSpaceRelationOldSize.height() / geometry.height();

        const QPointF oldCenter = zoomCenter();
        const QPointF newCenter = QPointF( oldCenter.x() * geometry.width() / d->fixedDataCoordinateSpaceRelationOldSize.width(),
                                           oldCenter.y() * geometry.height() / d->fixedDataCoordinateSpaceRelationOldSize.height() );

        // Use these internal methods to avoid sending
        // the propertiesChanged signal three times:
        bool bChanged = false;
        if( doneSetZoomFactorY( newZoomY ) )
            bChanged = true;
        if( doneSetZoomFactorX( newZoomX ) )
            bChanged = true;
        if( doneSetZoomCenter( newCenter ) )
            bChanged = true;
        if( bChanged ){
            emit propertiesChanged();
        }
    }

    d->fixedDataCoordinateSpaceRelationOldSize = geometry;
}

const QPointF CartesianCoordinatePlane::translate( const QPointF& diagramPoint ) const
{
    // Note: We do not test if the point lays inside of the data area,
    //       but we just apply the transformation calculations to the point.
    //       This allows for basic calculations done by the user, see e.g.
    //       the file  examples/Lines/BubbleChart/mainwindow.cpp
    return  d->coordinateTransformation.translate ( diagramPoint );
}

const QPointF CartesianCoordinatePlane::translateBack( const QPointF& screenPoint ) const
{
    return  d->coordinateTransformation.translateBack ( screenPoint );
}

void CartesianCoordinatePlane::setIsometricScaling ( bool onOff )
{
    if ( d->isometricScaling != onOff )
    {
        d->isometricScaling = onOff;
        layoutDiagrams();
        emit propertiesChanged();
    }
}

bool CartesianCoordinatePlane::doesIsometricScaling () const
{
    return d->isometricScaling;
}

bool CartesianCoordinatePlane::doneSetZoomFactorX( double factor )
{
    const bool done = ( d->coordinateTransformation.zoom.xFactor != factor );
    if( done ){
        d->coordinateTransformation.zoom.xFactor = factor;
        if( d->autoAdjustGridToZoom )
            d->grid->setNeedRecalculate();
    }
    return done;
}

bool CartesianCoordinatePlane::doneSetZoomFactorY( double factor )
{
    const bool done = ( d->coordinateTransformation.zoom.yFactor != factor );
    if( done ){
        d->coordinateTransformation.zoom.yFactor = factor;
        if( d->autoAdjustGridToZoom )
            d->grid->setNeedRecalculate();
    }
    return done;
}

bool CartesianCoordinatePlane::doneSetZoomCenter( const QPointF& point )
{
    const bool done = ( d->coordinateTransformation.zoom.center() != point );
    if( done ){
        d->coordinateTransformation.zoom.setCenter( point );
        if( d->autoAdjustGridToZoom )
            d->grid->setNeedRecalculate();
    }
    return done;
}

void CartesianCoordinatePlane::setZoomFactors( double factorX, double factorY )
{
    if( doneSetZoomFactorX( factorX ) || doneSetZoomFactorY( factorY ) ){
        emit propertiesChanged();
    }
}

void CartesianCoordinatePlane::setZoomFactorX( double factor )
{
    if( doneSetZoomFactorX( factor ) ){
        emit propertiesChanged();
    }
}

void CartesianCoordinatePlane::setZoomFactorY( double factor )
{
    if( doneSetZoomFactorY( factor ) ){
        emit propertiesChanged();
    }
}

void CartesianCoordinatePlane::setZoomCenter( const QPointF& point )
{
    if( doneSetZoomCenter( point ) ){
        emit propertiesChanged();
    }
}

QPointF CartesianCoordinatePlane::zoomCenter() const
{
    return d->coordinateTransformation.zoom.center();
}

double CartesianCoordinatePlane::zoomFactorX() const
{
    return d->coordinateTransformation.zoom.xFactor;
}

double CartesianCoordinatePlane::zoomFactorY() const
{
    return d->coordinateTransformation.zoom.yFactor;
}


CartesianCoordinatePlane::AxesCalcMode CartesianCoordinatePlane::axesCalcModeY() const
{
    return d->coordinateTransformation.axesCalcModeY;
}

CartesianCoordinatePlane::AxesCalcMode CartesianCoordinatePlane::axesCalcModeX() const
{
    return d->coordinateTransformation.axesCalcModeX;
}

void CartesianCoordinatePlane::setAxesCalcModes( AxesCalcMode mode )
{
    if( d->coordinateTransformation.axesCalcModeY != mode ||
        d->coordinateTransformation.axesCalcModeX != mode ){
        d->coordinateTransformation.axesCalcModeY = mode;
        d->coordinateTransformation.axesCalcModeX = mode;
        emit propertiesChanged();
    }
}

void CartesianCoordinatePlane::setAxesCalcModeY( AxesCalcMode mode )
{
    if( d->coordinateTransformation.axesCalcModeY != mode ){
        d->coordinateTransformation.axesCalcModeY = mode;
        emit propertiesChanged();
    }
}

void CartesianCoordinatePlane::setAxesCalcModeX( AxesCalcMode mode )
{
    if( d->coordinateTransformation.axesCalcModeX != mode ){
        d->coordinateTransformation.axesCalcModeX = mode;
        emit propertiesChanged();
    }
}

void CartesianCoordinatePlane::setHorizontalRange( const QPair< qreal, qreal > & range )
{
    if ( d->horizontalMin != range.first || d->horizontalMax != range.second ) {
        d->autoAdjustHorizontalRangeToData = 100;
        d->horizontalMin = range.first;
        d->horizontalMax = range.second;
        layoutDiagrams();
        emit propertiesChanged();
    }
}

void CartesianCoordinatePlane::setVerticalRange( const QPair< qreal, qreal > & range )
{

    if ( d->verticalMin != range.first || d->verticalMax != range.second ) {
        d->autoAdjustVerticalRangeToData = 100;
        d->verticalMin = range.first;
        d->verticalMax = range.second;
        layoutDiagrams();
        emit propertiesChanged();
    }
}

QPair< qreal, qreal > CartesianCoordinatePlane::horizontalRange( ) const
{
    return QPair<qreal, qreal>( d->horizontalMin, d->horizontalMax );
}

QPair< qreal, qreal > CartesianCoordinatePlane::verticalRange( ) const
{
    return QPair<qreal, qreal>( d->verticalMin, d->verticalMax );
}

void CartesianCoordinatePlane::adjustRangesToData()
{
    const QRectF dataBoundingRect( getRawDataBoundingRectFromDiagrams() );
    d->horizontalMin = dataBoundingRect.left();
    d->horizontalMax = dataBoundingRect.right();
    d->verticalMin = dataBoundingRect.top();
    d->verticalMax = dataBoundingRect.bottom();
    layoutDiagrams();
    emit propertiesChanged();
}

void CartesianCoordinatePlane::adjustHorizontalRangeToData()
{
    const QRectF dataBoundingRect( getRawDataBoundingRectFromDiagrams() );
    d->horizontalMin = dataBoundingRect.left();
    d->horizontalMax = dataBoundingRect.right();
    layoutDiagrams();
    emit propertiesChanged();
}

void CartesianCoordinatePlane::adjustVerticalRangeToData()
{
    const QRectF dataBoundingRect( getRawDataBoundingRectFromDiagrams() );
    d->verticalMin = dataBoundingRect.bottom();
    d->verticalMax = dataBoundingRect.top();
    layoutDiagrams();
    emit propertiesChanged();
}

void CartesianCoordinatePlane::setAutoAdjustHorizontalRangeToData( unsigned int percentEmpty )
{
    d->autoAdjustHorizontalRangeToData = percentEmpty;
    d->horizontalMin = 0.0;
    d->horizontalMax = 0.0;
    layoutDiagrams();
    emit propertiesChanged();
}

void CartesianCoordinatePlane::setAutoAdjustVerticalRangeToData( unsigned int percentEmpty )
{
    d->autoAdjustVerticalRangeToData = percentEmpty;
    d->verticalMin = 0.0;
    d->verticalMax = 0.0;
    layoutDiagrams();
    emit propertiesChanged();
}

unsigned int CartesianCoordinatePlane::autoAdjustHorizontalRangeToData() const
{
    return d->autoAdjustHorizontalRangeToData;
}

unsigned int CartesianCoordinatePlane::autoAdjustVerticalRangeToData() const
{
    return d->autoAdjustVerticalRangeToData;
}

void CartesianCoordinatePlane::setGridAttributes(
    Qt::Orientation orientation,
    const GridAttributes& a )
{
    if( orientation == Qt::Horizontal )
        d->gridAttributesHorizontal = a;
    else
        d->gridAttributesVertical = a;
    setHasOwnGridAttributes( orientation, true );
    update();
    emit propertiesChanged();
}

void CartesianCoordinatePlane::resetGridAttributes(
    Qt::Orientation orientation )
{
    setHasOwnGridAttributes( orientation, false );
    update();
}

const GridAttributes CartesianCoordinatePlane::gridAttributes(
    Qt::Orientation orientation ) const
{
    if( hasOwnGridAttributes( orientation ) ){
        if( orientation == Qt::Horizontal )
            return d->gridAttributesHorizontal;
        else
            return d->gridAttributesVertical;
    }else{
        return globalGridAttributes();
    }
}

void CartesianCoordinatePlane::setHasOwnGridAttributes(
    Qt::Orientation orientation, bool on )
{
    if( orientation == Qt::Horizontal )
        d->hasOwnGridAttributesHorizontal = on;
    else
        d->hasOwnGridAttributesVertical = on;
    emit propertiesChanged();
}

bool CartesianCoordinatePlane::hasOwnGridAttributes(
    Qt::Orientation orientation ) const
{
    return
        ( orientation == Qt::Horizontal )
        ? d->hasOwnGridAttributesHorizontal
        : d->hasOwnGridAttributesVertical;
}

void CartesianCoordinatePlane::setAutoAdjustGridToZoom( bool autoAdjust )
{
    if( d->autoAdjustGridToZoom != autoAdjust ){
        d->autoAdjustGridToZoom = autoAdjust;
        d->grid->setNeedRecalculate();
        emit propertiesChanged();
    }
}

#if QT_VERSION < 0x040400 || defined(Q_COMPILER_MANGLES_RETURN_TYPE)
const
#endif
bool CartesianCoordinatePlane::autoAdjustGridToZoom() const
{
    return d->autoAdjustGridToZoom;
}

AbstractCoordinatePlane* CartesianCoordinatePlane::sharedAxisMasterPlane( QPainter* painter )
{
    CartesianCoordinatePlane* plane = this;
    AbstractCartesianDiagram* diag = dynamic_cast< AbstractCartesianDiagram* >( plane->diagram() );
    const CartesianAxis* sharedAxis = 0;
    if( diag != 0 )
    {
        const CartesianAxisList axes = diag->axes();
        KDAB_FOREACH( const CartesianAxis* a, axes )
        {
            CartesianCoordinatePlane* p = const_cast< CartesianCoordinatePlane* >(
                                              dynamic_cast< const CartesianCoordinatePlane* >( a->coordinatePlane() ) );
            if( p != 0 && p != this )
            {
                plane = p;
                sharedAxis = a;
            }
        }
    }

    if( plane == this || painter == 0 )
        return plane;

    const QPointF zero = QPointF( 0, 0 );
    const QPointF tenX = QPointF( 10, 0 );
    const QPointF tenY = QPointF( 0, 10 );


    if( sharedAxis->isOrdinate() )
    {
        painter->translate( translate( zero ).x(), 0.0 );
        const qreal factor = (translate( tenX ) - translate( zero ) ).x() / ( plane->translate( tenX ) - plane->translate( zero ) ).x();
        painter->scale( factor, 1.0 );
        painter->translate( -plane->translate( zero ).x(), 0.0 );
    }
    if( sharedAxis->isAbscissa() )
    {
        painter->translate( 0.0, translate( zero ).y() );
        const qreal factor = (translate( tenY ) - translate( zero ) ).y() / ( plane->translate( tenY ) - plane->translate( zero ) ).y();
        painter->scale( 1.0, factor );
        painter->translate( 0.0, -plane->translate( zero ).y() );
    }


    return plane;
}

void CartesianCoordinatePlane::setHorizontalRangeReversed( bool reverse )
{
    if( d->reverseHorizontalPlane == reverse )
        return;

    d->reverseHorizontalPlane = reverse;
    layoutDiagrams();
    emit propertiesChanged();
}

bool CartesianCoordinatePlane::isHorizontalRangeReversed() const
{
    return d->reverseHorizontalPlane;
}

void CartesianCoordinatePlane::setVerticalRangeReversed( bool reverse )
{
    if( d->reverseVerticalPlane == reverse )
        return;

    d->reverseVerticalPlane = reverse;
    layoutDiagrams();
    emit propertiesChanged();
}

bool CartesianCoordinatePlane::isVerticalRangeReversed() const
{
    return d->reverseVerticalPlane;
}

QRectF CartesianCoordinatePlane::visibleDataRange() const
{
    QRectF result;

    const QRectF drawArea = drawingArea();

    result.setTopLeft( translateBack( drawArea.topLeft() ) );
    result.setBottomRight( translateBack( drawArea.bottomRight() ) );

    return result;
}

void CartesianCoordinatePlane::setGeometry( const QRect& rectangle )
{
    if( rectangle == geometry() )
        return;

    AbstractCoordinatePlane::setGeometry( rectangle );
    Q_FOREACH( AbstractDiagram* diagram, diagrams() ) {
        diagram->resize( drawingArea().size() );
    }
}
