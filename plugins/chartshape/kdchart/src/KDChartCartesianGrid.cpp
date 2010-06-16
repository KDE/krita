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

#include "KDChartCartesianGrid.h"
#include "KDChartAbstractCartesianDiagram.h"
#include "KDChartPaintContext.h"
#include "KDChartPainterSaver_p.h"
#include "KDChartPrintingParameters.h"

#include <QPainter>

#include <KDABLibFakes>

#include <limits>

using namespace KDChart;


void CartesianGrid::drawGrid( PaintContext* context )
{
    //qDebug() << "KDChart::CartesianGrid::drawGrid( PaintContext* context ) called";

    CartesianCoordinatePlane* plane = dynamic_cast<CartesianCoordinatePlane*>(context->coordinatePlane());
   
    // This plane is used for tranlating the coordinates - not for the data boundaries
    PainterSaver p( context->painter() );
    plane = dynamic_cast< CartesianCoordinatePlane* >( plane->sharedAxisMasterPlane( context->painter() ) );

    Q_ASSERT_X ( plane, "CartesianGrid::drawGrid",
                 "Bad function call: PaintContext::coodinatePlane() NOT a cartesian plane." );


    const GridAttributes gridAttrsX( plane->gridAttributes( Qt::Horizontal ) );
    const GridAttributes gridAttrsY( plane->gridAttributes( Qt::Vertical ) );

    //qDebug() << "OK:";
    if ( !gridAttrsX.isGridVisible() && !gridAttrsY.isGridVisible() ) return;
    //qDebug() << "A";

    // important: Need to update the calculated mData,
    //            before we may use it!
    updateData( context->coordinatePlane() );

    if( plane->axesCalcModeX() == KDChart::AbstractCoordinatePlane::Logarithmic && mData.first().stepWidth == 0.0 )
            mData.first().stepWidth = 1.0;
    if( plane->axesCalcModeY() == KDChart::AbstractCoordinatePlane::Logarithmic && mData.last().stepWidth == 0.0 )
            mData.last().stepWidth = 1.0;

    // test for programming errors: critical
    Q_ASSERT_X ( mData.count() == 2, "CartesianGrid::drawGrid",
                 "Error: updateData did not return exactly two dimensions." );

    // test for invalid boundaries: non-critical
    if( !isBoundariesValid( mData ) ) return;
    //qDebug() << "B";

    DataDimension dimX = mData.first();
    const DataDimension& dimY = mData.last();
    // test for other programming errors: critical
    Q_ASSERT_X ( dimX.stepWidth, "CartesianGrid::drawGrid",
                 "Error: updateData returned a Zero step width for the X grid." );
    Q_ASSERT_X ( dimY.stepWidth, "CartesianGrid::drawGrid",
                 "Error: updateData returned a Zero step width for the Y grid." );


    qreal numberOfUnitLinesX =
        qAbs( dimX.distance() / dimX.stepWidth )
        + (dimX.isCalculated ? 1.0 : 0.0);
    qreal numberOfUnitLinesY =
        qAbs( dimY.distance() / dimY.stepWidth )
        + (dimY.isCalculated ? 1.0 : 0.0);
    //qDebug("numberOfUnitLinesX: %f    numberOfUnitLinesY: %f",numberOfUnitLinesX,numberOfUnitLinesY);

    // do not draw a Zero size grid (and do not divide by Zero)
    if( numberOfUnitLinesX <= 0.0 || numberOfUnitLinesY <= 0.0 ) return;
    //qDebug() << "C";

    const QPointF p1 = plane->translate( QPointF(dimX.start, dimY.start) );
    const QPointF p2 = plane->translate( QPointF(dimX.end, dimY.end) );
//qDebug() << "dimX.isCalculated:" << dimX.isCalculated << "dimY.isCalculated:" << dimY.isCalculated;
//qDebug() << "dimX.start: " << dimX.start << "dimX.end: " << dimX.end;
//qDebug() << "dimY.start: " << dimY.start << "dimY.end: " << dimY.end;
//qDebug() << "p1:" << p1 << "  p2:" << p2;

    const qreal screenRangeX = qAbs ( p1.x() - p2.x() );
    const qreal screenRangeY = qAbs ( p1.y() - p2.y() );

    /*
     * let us paint the grid at a smaller resolution
     * the user can disable at any time
     * by setting the grid attribute to false
     * Same Value as for Cartesian Axis
     */
    static const qreal GridLineDistanceTreshold = 4.0; // <Treshold> pixels between each grid line
    const qreal MinimumPixelsBetweenLines =
            GridLineDistanceTreshold;
    //qDebug() << "x step " << dimX.stepWidth << "  y step " << dimY.stepWidth;

    //qreal unitFactorX = 1.0;
//    qreal unitFactorY = 1.0;

    //FIXME(khz): Remove this code, and do the calculation in the grid calc function
    if( ! dimX.isCalculated ){

        while( screenRangeX / numberOfUnitLinesX <= MinimumPixelsBetweenLines ){
            dimX.stepWidth *= 10.0;
            dimX.subStepWidth *= 10.0;
            numberOfUnitLinesX = qAbs( dimX.distance() / dimX.stepWidth );
        }
    }
    if( dimX.subStepWidth && (screenRangeX / (dimX.distance() / dimX.subStepWidth) <= MinimumPixelsBetweenLines) ){
        dimX.subStepWidth = 0.0;
        //qDebug() << "de-activating grid sub steps: not enough space";
    }

    const bool drawUnitLinesX = gridAttrsX.isGridVisible() &&
            (screenRangeX / numberOfUnitLinesX > MinimumPixelsBetweenLines);
    const bool drawUnitLinesY = gridAttrsY.isGridVisible() &&
            (screenRangeY / numberOfUnitLinesY > MinimumPixelsBetweenLines);

    const bool isLogarithmicX = dimX.isCalculated && (dimX.calcMode == AbstractCoordinatePlane::Logarithmic );
    const bool isLogarithmicY = (dimY.calcMode == AbstractCoordinatePlane::Logarithmic );
/*
    while ( !drawUnitLinesX ) {
        unitFactorX *= 10.0;
        drawUnitLinesX = screenRangeX / (numberOfUnitLinesX / unitFactorX) > MinimumPixelsBetweenLines;
    }
    while ( !drawUnitLinesY ) {
        unitFactorY *= 10.0;
        drawUnitLinesY = screenRangeY / (numberOfUnitLinesY / unitFactorY) > MinimumPixelsBetweenLines;
    }
*/

    const bool drawSubGridLinesX = isLogarithmicX ||
        ((dimX.subStepWidth != 0.0) &&
        (screenRangeX / (numberOfUnitLinesX / dimX.stepWidth * dimX.subStepWidth) > MinimumPixelsBetweenLines) &&
        gridAttrsX.isSubGridVisible());

    const bool drawSubGridLinesY = isLogarithmicY ||
        ((dimY.subStepWidth != 0.0) &&
        (screenRangeY / (numberOfUnitLinesY / dimY.stepWidth * dimY.subStepWidth) > MinimumPixelsBetweenLines) &&
        gridAttrsY.isSubGridVisible());

    qreal minValueX = qMin( dimX.start, dimX.end );
    qreal maxValueX = qMax( dimX.start, dimX.end );
    qreal minValueY = qMin( dimY.start, dimY.end );
    qreal maxValueY = qMax( dimY.start, dimY.end );
    AbstractGrid::adjustLowerUpperRange( minValueX, maxValueX, dimX.stepWidth, true, true );
    AbstractGrid::adjustLowerUpperRange( minValueY, maxValueY, dimY.stepWidth, true, true );

    if ( drawSubGridLinesX ) {
        context->painter()->setPen( PrintingParameters::scalePen( gridAttrsX.subGridPen() ) );
        qreal f = minValueX;
        qreal fLogSubstep = minValueX;

        int logSubstep = 0;
        while ( f <= maxValueX ) {
            QPointF topPoint( f, maxValueY );
            QPointF bottomPoint( f, minValueY );
            topPoint = plane->translate( topPoint );
            bottomPoint = plane->translate( bottomPoint );
            context->painter()->drawLine( topPoint, bottomPoint );
            if ( isLogarithmicX ){
                if( logSubstep == 9 ){
                    fLogSubstep *= ( fLogSubstep > 0.0 ) ? 10.0 : 0.1;
                    if( fLogSubstep == 0.0 )
                        fLogSubstep = pow( 10.0, floor( log10( dimX.start ) ) );

                    logSubstep = 0;
                    f = fLogSubstep;
                }
                else
                {
                    f += fLogSubstep;
                }
                ++logSubstep;
            }else{
                f += dimX.subStepWidth;
            }
        }
    }

    if ( drawSubGridLinesY ) {
        context->painter()->setPen( PrintingParameters::scalePen( gridAttrsY.subGridPen() ) );
        qreal f = minValueY;
        qreal fLogSubstep = minValueY;

        int logSubstep = 0;
        while ( f <= maxValueY ) {
            //qDebug() << "sub grid line Y at" << f;
            QPointF leftPoint( minValueX, f );
            QPointF rightPoint( maxValueX, f );
            leftPoint = plane->translate( leftPoint );
            rightPoint = plane->translate( rightPoint );
            context->painter()->drawLine( leftPoint, rightPoint );
            if ( isLogarithmicY ){
                if( logSubstep == 9 ){
                    fLogSubstep *= ( fLogSubstep > 0.0 ) ? 10.0 : 0.1;
                    if( fLogSubstep == 0.0 )
                        fLogSubstep = pow( 10.0, floor( log10( dimY.start ) ) );

                    logSubstep = 0;
                    f = fLogSubstep;
                }
                else
                {
                    f += fLogSubstep;
                }
                ++logSubstep;
            }else{
                f += dimY.subStepWidth;
            }
        }
    }

    const bool drawXZeroLineX
        = dimX.isCalculated &&
        gridAttrsX.zeroLinePen().style() != Qt::NoPen;

    const bool drawZeroLineY
        = gridAttrsY.zeroLinePen().style() != Qt::NoPen;

    if ( drawUnitLinesX || drawXZeroLineX ) {
        //qDebug() << "E";
        if ( drawUnitLinesX )
            context->painter()->setPen( PrintingParameters::scalePen( gridAttrsX.gridPen() ) );
//        const qreal minX = dimX.start;

        qreal f = minValueX;

        while ( f <= maxValueX ) {
            // PENDING(khz) FIXME: make draving/not drawing of Zero line more sophisticated?:
            const bool zeroLineHere = drawXZeroLineX && (f == 0.0);
            if ( drawUnitLinesX || zeroLineHere ){
                //qDebug("main grid line X at: %f --------------------------",f);
                QPointF topPoint( f, maxValueY );
                QPointF bottomPoint( f, minValueY );
                topPoint = plane->translate( topPoint );
                bottomPoint = plane->translate( bottomPoint );
                if ( zeroLineHere )
                    context->painter()->setPen( PrintingParameters::scalePen( gridAttrsX.zeroLinePen() ) );
                context->painter()->drawLine( topPoint, bottomPoint );
                if ( zeroLineHere )
                    context->painter()->setPen( PrintingParameters::scalePen( gridAttrsX.gridPen() ) );
            }
            if ( isLogarithmicX ) {
                f *= ( f > 0.0 ) ? 10.0 : 0.1;
                if( f == 0.0 )
                    f = pow( 10.0, floor( log10( dimX.start ) ) );
            }
            else
                f += dimX.stepWidth;
        }
        // draw the last line if not logarithmic calculation
        // we need the in order to get the right grid line painted
        // when f + dimX.stepWidth jump over maxValueX
        if (  ! isLogarithmicX )
        context->painter()->drawLine( plane->translate( QPointF(  maxValueX, maxValueY ) ),
                                      plane->translate( QPointF( maxValueX, minValueY ) ) );

    }
    if ( drawUnitLinesY || drawZeroLineY ) {
        //qDebug() << "F";
        if ( drawUnitLinesY )
            context->painter()->setPen( PrintingParameters::scalePen( gridAttrsY.gridPen() ) );
        //const qreal minY = dimY.start;
        //qDebug("minY: %f   maxValueY: %f   dimY.stepWidth: %f",minY,maxValueY,dimY.stepWidth);
        qreal f = minValueY;

        while ( f <= maxValueY ) {
            // PENDING(khz) FIXME: make draving/not drawing of Zero line more sophisticated?:
            //qDebug("main grid line Y at: %f",f);
            const bool zeroLineHere = (f == 0.0);
            if ( drawUnitLinesY || zeroLineHere ){
                QPointF leftPoint(  minValueX, f );
                QPointF rightPoint( maxValueX, f );
                leftPoint  = plane->translate( leftPoint );
                rightPoint = plane->translate( rightPoint );
                if ( zeroLineHere )
                    context->painter()->setPen( PrintingParameters::scalePen( gridAttrsY.zeroLinePen() ) );
                context->painter()->drawLine( leftPoint, rightPoint );
                if ( zeroLineHere )
                    context->painter()->setPen( PrintingParameters::scalePen( gridAttrsY.gridPen() ) );
            }
            if ( isLogarithmicY ) {
                f *= ( f > 0.0 ) ? 10.0 : 0.1;
                if( f == 0.0 )
                    f = pow( 10.0, floor( log10( dimY.start ) ) );
            }
            else
                f += dimY.stepWidth;
        }
    }
    //qDebug() << "Z";
}


DataDimensionsList CartesianGrid::calculateGrid(
    const DataDimensionsList& rawDataDimensions ) const
{
    Q_ASSERT_X ( rawDataDimensions.count() == 2, "CartesianGrid::calculateGrid",
                 "Error: calculateGrid() expects a list with exactly two entries." );

    CartesianCoordinatePlane* plane = dynamic_cast<CartesianCoordinatePlane*>( mPlane );
    Q_ASSERT_X ( plane, "CartesianGrid::calculateGrid",
                 "Error: PaintContext::calculatePlane() called, but no cartesian plane set." );

    DataDimensionsList l( rawDataDimensions );
    // rule:  Returned list is either empty, or it is providing two
    //        valid dimensions, complete with two non-Zero step widths.
    if( isBoundariesValid( l ) ) {
        const QPointF translatedBottomLeft( plane->translateBack( plane->geometry().bottomLeft() ) );
        const QPointF translatedTopRight(   plane->translateBack( plane->geometry().topRight() ) );
        //qDebug() << "CartesianGrid::calculateGrid()         first:" << l.first().start << l.first().end <<                   "   last:" << l.last().start << l.last().end;
        //qDebug() << "CartesianGrid::calculateGrid()  translated x:" << translatedBottomLeft.x() << translatedTopRight.x() << "      y:" << translatedBottomLeft.y() << translatedTopRight.y();
        //qDebug() << "CartesianGrid::calculateGrid()  raw data y-range  :" << l.last().end - l.last().start;
        //qDebug() << "CartesianGrid::calculateGrid()  translated y-range:" << translatedTopRight.y() - translatedBottomLeft.y();

        /* Code is obsolete. The dataset dimension of the diagram should *never* be > 1.
        if( l.first().isCalculated
            && plane->autoAdjustGridToZoom()
            && plane->axesCalcModeX() == CartesianCoordinatePlane::Linear
            && plane->zoomFactorX() > 1.0 )
        {
            l.first().start = translatedBottomLeft.x();
            l.first().end   = translatedTopRight.x();
        }
        */

        const GridAttributes gridAttrsX( plane->gridAttributes( Qt::Horizontal ) );
        const GridAttributes gridAttrsY( plane->gridAttributes( Qt::Vertical ) );

        const DataDimension dimX
                = calculateGridXY( l.first(), Qt::Horizontal,
                                   gridAttrsX.adjustLowerBoundToGrid(),
                                   gridAttrsX.adjustUpperBoundToGrid() );
        if( dimX.stepWidth ){
            //qDebug("CartesianGrid::calculateGrid()   l.last().start:  %f   l.last().end:  %f", l.last().start, l.last().end);
            //qDebug("                                 l.first().start: %f   l.first().end: %f", l.first().start, l.first().end);

            // one time for the min/max value
            const DataDimension minMaxY
                    = calculateGridXY( l.last(), Qt::Vertical,
                                       gridAttrsY.adjustLowerBoundToGrid(),
                                       gridAttrsY.adjustUpperBoundToGrid() );

            if( plane->autoAdjustGridToZoom()
                && plane->axesCalcModeY() == CartesianCoordinatePlane::Linear
                && plane->zoomFactorY() > 1.0 )
            {
                l.last().start = translatedBottomLeft.y();
                l.last().end   = translatedTopRight.y();
            }
            // and one other time for the step width
            const DataDimension dimY
                    = calculateGridXY( l.last(), Qt::Vertical,
                                       gridAttrsY.adjustLowerBoundToGrid(),
                                       gridAttrsY.adjustUpperBoundToGrid() );
            if( dimY.stepWidth ){
                l.first().start        = dimX.start;
                l.first().end          = dimX.end;
                l.first().stepWidth    = dimX.stepWidth;
                l.first().subStepWidth = dimX.subStepWidth;
                l.last().start        = minMaxY.start;
                l.last().end          = minMaxY.end;
                l.last().stepWidth    = dimY.stepWidth;
                l.last().subStepWidth    = dimY.subStepWidth;
                //qDebug() << "CartesianGrid::calculateGrid()  final grid y-range:" << l.last().end - l.last().start << "   step width:" << l.last().stepWidth << endl;
                // calculate some reasonable subSteps if the
                // user did not set the sub grid but did set
                // the stepWidth.
                
                // FIXME (Johannes)
                // the last (y) dimension is not always the dimension for the ordinate!
                // since there's no way to check for the orientation of this dimension here,
                // we cannot automatically assume substep values
                //if ( dimY.subStepWidth == 0 )
                //    l.last().subStepWidth = dimY.stepWidth/2;
                //else
                //    l.last().subStepWidth = dimY.subStepWidth;
            }
        }
    }
    //qDebug() << "CartesianGrid::calculateGrid()  final grid Y-range:" << l.last().end - l.last().start << "   substep width:" << l.last().subStepWidth;
    //qDebug() << "CartesianGrid::calculateGrid()  final grid X-range:" << l.first().end - l.first().start << "   substep width:" << l.first().subStepWidth;

    return l;
}


qreal fastPow10( int x )
{
    qreal res = 1.0;
    if( 0 <= x ){
        for( int i = 1; i <= x; ++i )
            res *= 10.0;
    }else{
        for( int i = -1; i >= x; --i )
            res /= 10.0;
    }
    return res;
}

#if defined ( Q_WS_WIN)
#define trunc(x) ((int)(x))
#endif

DataDimension CartesianGrid::calculateGridXY(
    const DataDimension& rawDataDimension,
    Qt::Orientation orientation,
    bool adjustLower, bool adjustUpper ) const
{
    CartesianCoordinatePlane* const plane = dynamic_cast<CartesianCoordinatePlane*>( mPlane );
    if(    ((orientation == Qt::Vertical)   && (plane->autoAdjustVerticalRangeToData()   >= 100))
        || ((orientation == Qt::Horizontal) && (plane->autoAdjustHorizontalRangeToData() >= 100)) )
    {
        adjustLower = false;
        adjustUpper = false;
    }

    DataDimension dim( rawDataDimension );
    if( dim.isCalculated && dim.start != dim.end ){
        if( dim.calcMode == AbstractCoordinatePlane::Linear ){
            // linear ( == not-logarithmic) calculation
            if( dim.stepWidth == 0.0 ){
                QList<qreal> granularities;
                switch( dim.sequence ){
                    case KDChartEnums::GranularitySequence_10_20:
                        granularities << 1.0 << 2.0;
                        break;
                    case KDChartEnums::GranularitySequence_10_50:
                        granularities << 1.0 << 5.0;
                        break;
                    case KDChartEnums::GranularitySequence_25_50:
                        granularities << 2.5 << 5.0;
                        break;
                    case KDChartEnums::GranularitySequence_125_25:
                        granularities << 1.25 << 2.5;
                        break;
                    case KDChartEnums::GranularitySequenceIrregular:
                        granularities << 1.0 << 1.25 << 2.0 << 2.5 << 5.0;
                        break;
                    default:
                        break;
                }
                //qDebug("CartesianGrid::calculateGridXY()   dim.start: %f   dim.end: %f", dim.start, dim.end);
                calculateStepWidth(
                    dim.start, dim.end, granularities, orientation,
                    dim.stepWidth, dim.subStepWidth,
                    adjustLower, adjustUpper );
            }
            // if needed, adjust start/end to match the step width:
            //qDebug() << "CartesianGrid::calculateGridXY() has 1st linear range: min " << dim.start << " and max" << dim.end;

            AbstractGrid::adjustLowerUpperRange( dim.start, dim.end, dim.stepWidth,
                    adjustLower, adjustUpper );
            //qDebug() << "CartesianGrid::calculateGridXY() returns linear range: min " << dim.start << " and max" << dim.end;
        }else{
            // logarithmic calculation with negative values
            if( dim.end <= 0 )
            {
                qreal min;
                const qreal minRaw = qMin( dim.start, dim.end );
                const int minLog = -static_cast<int>(trunc( log10( -minRaw ) ) );
                if( minLog >= 0 )
                    min = qMin( minRaw, -std::numeric_limits< qreal >::epsilon() );
                else
                    min = -fastPow10( -(minLog-1) );
            
                qreal max;
                const qreal maxRaw = qMin( -std::numeric_limits< qreal >::epsilon(), qMax( dim.start, dim.end ) );
                const int maxLog = -static_cast<int>(ceil( log10( -maxRaw ) ) );
                if( maxLog >= 0 )
                    max = -1;
                else if( fastPow10( -maxLog ) < maxRaw )
                    max = -fastPow10( -(maxLog+1) );
                else
                    max = -fastPow10( -maxLog );
                if( adjustLower )
                    dim.start = min;
                if( adjustUpper )
                    dim.end   = max;
                dim.stepWidth = -pow( 10.0, ceil( log10( qAbs( max - min ) / 10.0 ) ) );
            }
            // logarithmic calculation (ignoring all negative values)
            else
            {
                qreal min;
                const qreal minRaw = qMax( qMin( dim.start, dim.end ), qreal( 0.0 ) );
                const int minLog = static_cast<int>(trunc( log10( minRaw ) ) );
                if( minLog <= 0 && dim.end < 1.0 )
                    min = qMax( minRaw, std::numeric_limits< qreal >::epsilon() );
                else if( minLog <= 0 )
                    min = qMax( qreal(0.00001), dim.start );
                else
                    min = fastPow10( minLog-1 );

                // Uh oh. Logarithmic scaling doesn't work with a lower or upper
                // bound being 0.
                const bool zeroBound = dim.start == 0.0 || dim.end == 0.0;

                qreal max;
                const qreal maxRaw = qMax( qMax( dim.start, dim.end ), qreal( 0.0 ) );
                const int maxLog = static_cast<int>(ceil( log10( maxRaw ) ) );
                if( maxLog <= 0 )
                    max = 1;
                else if( fastPow10( maxLog ) < maxRaw )
                    max = fastPow10( maxLog+1 );
                else
                    max = fastPow10( maxLog );
                if( adjustLower || zeroBound )
                    dim.start = min;
                if( adjustUpper || zeroBound )
                    dim.end   = max;
                dim.stepWidth = pow( 10.0, ceil( log10( qAbs( max - min ) / 10.0 ) ) );
            }
        }
    }else{
        //qDebug() << "CartesianGrid::calculateGridXY() returns stepWidth 1.0  !!";
        // Do not ignore the user configuration
        dim.stepWidth = dim.stepWidth ? dim.stepWidth : 1.0;
    }
    return dim;
}


static void calculateSteps(
    qreal start_, qreal end_, const QList<qreal>& list,
    int minSteps, int maxSteps,
    int power,
    qreal& steps, qreal& stepWidth,
    bool adjustLower, bool adjustUpper )
{
    //qDebug("-----------------------------------\nstart: %f   end: %f   power-of-ten: %i", start_, end_, power);

    qreal distance;
    steps = 0.0;

    const int lastIdx = list.count()-1;
    for( int i = 0;  i <= lastIdx;  ++i ){
        const qreal testStepWidth = list.at(lastIdx - i) * fastPow10( power );
        //qDebug( "testing step width: %f", testStepWidth);
        qreal start = qMin( start_, end_ );
        qreal end   = qMax( start_, end_ );
        //qDebug("pre adjusting    start: %f   end: %f", start, end);
        AbstractGrid::adjustLowerUpperRange( start, end, testStepWidth, adjustLower, adjustUpper );
        //qDebug("post adjusting   start: %f   end: %f", start, end);

        const qreal testDistance = qAbs(end - start);
        const qreal testSteps    = testDistance / testStepWidth;

        //qDebug() << "testDistance:" << testDistance << "  distance:" << distance;
        if( (minSteps <= testSteps) && (testSteps <= maxSteps)
              && ( (steps == 0.0) || (testDistance <= distance) ) ){
            steps     = testSteps;
            stepWidth = testStepWidth;
            distance  = testDistance;
            //qDebug( "start: %f   end: %f   step width: %f   steps: %f   distance: %f", start, end, stepWidth, steps, distance);
        }
    }
}


void CartesianGrid::calculateStepWidth(
    qreal start_, qreal end_,
    const QList<qreal>& granularities,
    Qt::Orientation orientation,
    qreal& stepWidth, qreal& subStepWidth,
    bool adjustLower, bool adjustUpper ) const
{
    Q_UNUSED( orientation );

    Q_ASSERT_X ( granularities.count(), "CartesianGrid::calculateStepWidth",
                 "Error: The list of GranularitySequence values is empty." );
    QList<qreal> list( granularities );
    qSort( list );

    const qreal start = qMin( start_, end_);
    const qreal end   = qMax( start_, end_);
    const qreal distance = end - start;
    //qDebug( "raw data start: %f   end: %f", start, end);

    //FIXME(khz): make minSteps and maxSteps configurable by the user.
    const int minSteps = 2;
    const int maxSteps = 12;

    qreal steps;
    int power = 0;
    while( list.last() * fastPow10( power ) < distance ){
        ++power;
    };
    // We have the sequence *two* times in the calculation test list,
    // so we will be sure to find the best match:
    const int count = list.count();
    QList<qreal> testList;

    for( int dec = -1; dec == -1 || fastPow10( dec + 1 ) >= distance; --dec )
        for( int i = 0;  i < count;  ++i )
            testList << list.at(i) * fastPow10( dec );

    testList << list;

    do{
        //qDebug() << "list:" << testList;
        //qDebug( "calculating steps: power: %i", power);
        calculateSteps( start, end, testList, minSteps, maxSteps, power,
                        steps, stepWidth,
                        adjustLower, adjustUpper );
        --power;
    }while( steps == 0.0 );
    ++power;
    //qDebug( "steps calculated:  stepWidth: %f   steps: %f", stepWidth, steps);

    // find the matching sub-grid line width in case it is
    // not set by the user

    if (  subStepWidth == 0.0 ) {
        if( stepWidth == list.first() * fastPow10( power ) ){
            subStepWidth = list.last() * fastPow10( power-1 );
            //qDebug("A");
        }else if( stepWidth == list.first() * fastPow10( power-1 ) ){
            subStepWidth = list.last() * fastPow10( power-2 );
            //qDebug("B");
        }else{
            qreal smallerStepWidth = list.first();
            for( int i = 1;  i < list.count();  ++i ){
                if( stepWidth == list.at( i ) * fastPow10( power ) ){
                    subStepWidth = smallerStepWidth * fastPow10( power );
                    break;
                }
                if( stepWidth == list.at( i ) * fastPow10( power-1 ) ){
                    subStepWidth = smallerStepWidth * fastPow10( power-1 );
                    break;
                }
                smallerStepWidth = list.at( i );
            }

            //qDebug("C");
        }
    }
    //qDebug("CartesianGrid::calculateStepWidth() found stepWidth %f (%f steps) and sub-stepWidth %f", stepWidth, steps, subStepWidth);
}
