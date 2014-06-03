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

#include "KDChartLeveyJenningsGrid.h"
#include "KDChartLeveyJenningsDiagram.h"
#include "KDChartPaintContext.h"
#include "KDChartPainterSaver_p.h"
#include "KDChartPrintingParameters.h"

#include <QPainter>

#include <KDABLibFakes>
#include <limits>


using namespace KDChart;

qreal fastPow10( int x );

DataDimensionsList LeveyJenningsGrid::calculateGrid( const DataDimensionsList& rawDataDimensions ) const
{
    Q_ASSERT_X ( rawDataDimensions.count() == 2, "CartesianGrid::calculateGrid",
                 "Error: calculateGrid() expects a list with exactly two entries." );

    LeveyJenningsCoordinatePlane* plane = dynamic_cast< LeveyJenningsCoordinatePlane*>( mPlane );
    Q_ASSERT_X ( plane, "LeveyJenningsGrid::calculateGrid",
                 "Error: PaintContext::calculatePlane() called, but no cartesian plane set." );

    DataDimensionsList l( rawDataDimensions );
    // rule:  Returned list is either empty, or it is providing two
    //        valid dimensions, complete with two non-Zero step widths.
    if( isBoundariesValid( l ) ) {
        const QPointF translatedBottomLeft( plane->translateBack( plane->geometry().bottomLeft() ) );
        const QPointF translatedTopRight(   plane->translateBack( plane->geometry().topRight() ) );

        if( l.first().isCalculated
            && plane->autoAdjustGridToZoom()
            && plane->axesCalcModeX() == CartesianCoordinatePlane::Linear
            && plane->zoomFactorX() > 1.0 )
        {
            l.first().start = translatedBottomLeft.x();
            l.first().end   = translatedTopRight.x();
        }

        const DataDimension dimX
                = calculateGridXY( l.first(), Qt::Horizontal, false, false );
        if( dimX.stepWidth ){
            // one time for the min/max value
            const DataDimension minMaxY
                    = calculateGridXY( l.last(), Qt::Vertical, false, false );

            if( plane->autoAdjustGridToZoom()
                && plane->axesCalcModeY() == CartesianCoordinatePlane::Linear
                && plane->zoomFactorY() > 1.0 )
            {
                l.last().start = translatedBottomLeft.y();
                l.last().end   = translatedTopRight.y();
            }
            // and one other time for the step width
            const DataDimension dimY
                    = calculateGridXY( l.last(), Qt::Vertical, false, false );
            if( dimY.stepWidth ){
                l.first().start        = dimX.start;
                l.first().end          = dimX.end;
                l.first().stepWidth    = dimX.stepWidth;
                l.first().subStepWidth = dimX.subStepWidth;
                l.last().start        = minMaxY.start;
                l.last().end          = minMaxY.end;
                l.last().stepWidth    = dimY.stepWidth;
                //qDebug() << "CartesianGrid::calculateGrid()  final grid y-range:" << l.last().end - l.last().start << "   step width:" << l.last().stepWidth << endl;
                // calculate some reasonable subSteps if the
                // user did not set the sub grid but did set
                // the stepWidth.
                if ( dimY.subStepWidth == 0 )
                    l.last().subStepWidth = dimY.stepWidth/2;
                else
                    l.last().subStepWidth = dimY.subStepWidth;
            }
        }
    }
    //qDebug() << "CartesianGrid::calculateGrid()  final grid Y-range:" << l.last().end - l.last().start << "   step width:" << l.last().stepWidth;
    //qDebug() << "CartesianGrid::calculateGrid()  final grid X-range:" << l.first().end - l.first().start << "   step width:" << l.first().stepWidth;

    return l;
}

#if defined ( Q_WS_WIN)
#define trunc(x) ((int)(x))
#endif

DataDimension LeveyJenningsGrid::calculateGridXY(
    const DataDimension& rawDataDimension,
    Qt::Orientation orientation,
    bool adjustLower, bool adjustUpper ) const
{
    DataDimension dim( rawDataDimension );
    if( dim.isCalculated && dim.start != dim.end ){
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

    qreal distance=std::numeric_limits<qreal>::max();
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

void LeveyJenningsGrid::calculateStepWidth(
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
    for( int i = 0;  i < count;  ++i )
        testList << list.at(i) * 0.1;
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
    //qDebug("LeveyJenningsGrid::calculateStepWidth() found stepWidth %f (%f steps) and sub-stepWidth %f", stepWidth, steps, subStepWidth);
}

void LeveyJenningsGrid::drawGrid( PaintContext* context )
{
    LeveyJenningsCoordinatePlane* plane = dynamic_cast<LeveyJenningsCoordinatePlane*>(context->coordinatePlane());
   
    // This plane is used for tranlating the coordinates - not for the data boundaries
    PainterSaver p( context->painter() );
    plane = dynamic_cast< LeveyJenningsCoordinatePlane* >( plane->sharedAxisMasterPlane( context->painter() ) );

    Q_ASSERT_X ( plane, "LeveyJenningsGrid::drawGrid",
                 "Bad function call: PaintContext::coodinatePlane() NOT a Levey Jennings plane." );

    LeveyJenningsDiagram* diag = dynamic_cast<LeveyJenningsDiagram*>( plane->diagram() );

    if( diag == 0 )
        return;
    
    const LeveyJenningsGridAttributes gridAttrs( plane->gridAttributes() );

    // important: Need to update the calculated mData,
    //            before we may use it!
    updateData( context->coordinatePlane() );

    // test for programming errors: critical
    Q_ASSERT_X ( mData.count() == 2, "CartesianGrid::drawGrid",
                 "Error: updateData did not return exactly two dimensions." );

    // test for invalid boundaries: non-critical
    if( !isBoundariesValid( mData ) ) return;
    //qDebug() << "B";

    DataDimension dimX = mData.first();
    // this happens if there's only one data point
    if( dimX.start == 0.0 && dimX.end == 0.0 )
        dimX.end += plane->geometry().width();

    // first we draw the expected lines
    // draw the "mean" line
    const float meanValue = diag->expectedMeanValue();
    const float standardDeviation = diag->expectedStandardDeviation();

    // then the calculated ones
    const float calcMeanValue = diag->calculatedMeanValue();
    const float calcStandardDeviation = diag->calculatedStandardDeviation();


    // draw the normal range
    QPointF topLeft = plane->translate( QPointF( dimX.start, meanValue - 2 * standardDeviation ) );
    QPointF bottomRight = plane->translate( QPointF( dimX.end, meanValue + 2 * standardDeviation ) );
    context->painter()->fillRect( QRectF( topLeft, QSizeF( bottomRight.x() - topLeft.x(), bottomRight.y() - topLeft.y() ) ),
                                  gridAttrs.rangeBrush( LeveyJenningsGridAttributes::NormalRange ) ); 

    // draw the critical range
    topLeft = plane->translate( QPointF( dimX.start, meanValue + 2 * standardDeviation ) );
    bottomRight = plane->translate( QPointF( dimX.end, meanValue + 3 * standardDeviation ) );
    context->painter()->fillRect( QRectF( topLeft, QSizeF( bottomRight.x() - topLeft.x(), bottomRight.y() - topLeft.y() ) ),
                                  gridAttrs.rangeBrush( LeveyJenningsGridAttributes::CriticalRange ) );

    topLeft = plane->translate( QPointF( dimX.start, meanValue - 2 * standardDeviation ) );
    bottomRight = plane->translate( QPointF( dimX.end, meanValue - 3 * standardDeviation ) );
    context->painter()->fillRect( QRectF( topLeft, QSizeF( bottomRight.x() - topLeft.x(), bottomRight.y() - topLeft.y() ) ),
                                  gridAttrs.rangeBrush( LeveyJenningsGridAttributes::CriticalRange ) );

    // draw the "out of range" range
    topLeft = plane->translate( QPointF( dimX.start, meanValue + 3 * standardDeviation ) );
    bottomRight = plane->translate( QPointF( dimX.end, meanValue + 4 * standardDeviation ) );
    context->painter()->fillRect( QRectF( topLeft, QSizeF( bottomRight.x() - topLeft.x(), bottomRight.y() - topLeft.y() ) ),
                                  gridAttrs.rangeBrush( LeveyJenningsGridAttributes::OutOfRange ) );

    topLeft = plane->translate( QPointF( dimX.start, meanValue - 3 * standardDeviation ) );
    bottomRight = plane->translate( QPointF( dimX.end, meanValue - 4 * standardDeviation ) );
    context->painter()->fillRect( QRectF( topLeft, QSizeF( bottomRight.x() - topLeft.x(), bottomRight.y() - topLeft.y() ) ),
                                  gridAttrs.rangeBrush( LeveyJenningsGridAttributes::OutOfRange ) );

    // the "expected" grid
    if( gridAttrs.isGridVisible( LeveyJenningsGridAttributes::Expected ) )
    {
        context->painter()->setPen( gridAttrs.gridPen( LeveyJenningsGridAttributes::Expected ) );
        context->painter()->drawLine( plane->translate( QPointF( dimX.start, meanValue ) ), 
                                      plane->translate( QPointF( dimX.end,   meanValue ) ) );
        context->painter()->drawLine( plane->translate( QPointF( dimX.start, meanValue + 2 * standardDeviation ) ), 
                                      plane->translate( QPointF( dimX.end,   meanValue + 2 * standardDeviation ) ) );
        context->painter()->drawLine( plane->translate( QPointF( dimX.start, meanValue + 3 * standardDeviation ) ), 
                                      plane->translate( QPointF( dimX.end,   meanValue + 3 * standardDeviation ) ) );
        context->painter()->drawLine( plane->translate( QPointF( dimX.start, meanValue + 4 * standardDeviation ) ), 
                                      plane->translate( QPointF( dimX.end,   meanValue + 4 * standardDeviation ) ) );
        context->painter()->drawLine( plane->translate( QPointF( dimX.start, meanValue - 2 * standardDeviation ) ), 
                                      plane->translate( QPointF( dimX.end,   meanValue - 2 * standardDeviation ) ) );
        context->painter()->drawLine( plane->translate( QPointF( dimX.start, meanValue - 3 * standardDeviation ) ), 
                                      plane->translate( QPointF( dimX.end,   meanValue - 3 * standardDeviation ) ) );
        context->painter()->drawLine( plane->translate( QPointF( dimX.start, meanValue - 4 * standardDeviation ) ), 
                                      plane->translate( QPointF( dimX.end,   meanValue - 4 * standardDeviation ) ) );
    }
    
    // the "calculated" grid
    if( gridAttrs.isGridVisible( LeveyJenningsGridAttributes::Calculated ) )
    {
        context->painter()->setPen( gridAttrs.gridPen( LeveyJenningsGridAttributes::Calculated ) );
        context->painter()->drawLine( plane->translate( QPointF( dimX.start, calcMeanValue ) ), 
                                      plane->translate( QPointF( dimX.end,   calcMeanValue ) ) );
        context->painter()->drawLine( plane->translate( QPointF( dimX.start, calcMeanValue + 2 * calcStandardDeviation ) ), 
                                      plane->translate( QPointF( dimX.end,   calcMeanValue + 2 * calcStandardDeviation ) ) );
        context->painter()->drawLine( plane->translate( QPointF( dimX.start, calcMeanValue + 3 * calcStandardDeviation ) ), 
                                      plane->translate( QPointF( dimX.end,   calcMeanValue + 3 * calcStandardDeviation ) ) );
        context->painter()->drawLine( plane->translate( QPointF( dimX.start, calcMeanValue - 2 * calcStandardDeviation ) ), 
                                      plane->translate( QPointF( dimX.end,   calcMeanValue - 2 * calcStandardDeviation ) ) );
        context->painter()->drawLine( plane->translate( QPointF( dimX.start, calcMeanValue - 3 * calcStandardDeviation ) ), 
                                      plane->translate( QPointF( dimX.end,   calcMeanValue - 3 * calcStandardDeviation ) ) );
    }
}
