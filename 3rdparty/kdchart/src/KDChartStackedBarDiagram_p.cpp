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

#include <QModelIndex>

#include "KDChartBarDiagram.h"
#include "KDChartTextAttributes.h"
#include "KDChartAttributesModel.h"
#include "KDChartAbstractCartesianDiagram.h"
#include "KDChartStackedBarDiagram_p.h"

using namespace KDChart;

StackedBarDiagram::StackedBarDiagram( BarDiagram* d )
    : BarDiagramType( d )
{
}

BarDiagram::BarType StackedBarDiagram::type() const
{
    return BarDiagram::Stacked;
}

const QPair<QPointF, QPointF> StackedBarDiagram::calculateDataBoundaries() const
{
    const int rowCount = compressor().modelDataRows();
    const int colCount = compressor().modelDataColumns();

    double xMin = 0;
    double xMax = diagram()->model() ? diagram()->model()->rowCount( diagram()->rootIndex() ) : 0;
    double yMin = 0, yMax = 0;

    bool bStarting = true;
    for( int row = 0; row < rowCount; ++row )
    {
        // calculate sum of values per column - Find out stacked Min/Max
        double stackedValues = 0.0;
        double negativeStackedValues = 0.0;
        for ( int col = 0; col < colCount ; ++col )
        {
            const CartesianDiagramDataCompressor::CachePosition position( row, col );
            const CartesianDiagramDataCompressor::DataPoint point = compressor().data( position );
            const double value = ISNAN( point.value ) ? 0.0 : point.value;

            if( value > 0.0 )
                stackedValues += value;
            else
                negativeStackedValues += value;

            // this is always true yMin can be 0 in case all values
            // are the same
            // same for yMax it can be zero if all values are negative
            if( bStarting ){
                yMin = negativeStackedValues < 0.0 ? negativeStackedValues : stackedValues;
                yMax = stackedValues > 0.0 ? stackedValues : negativeStackedValues;
                bStarting = false;
            }else{
                yMin = qMin( qMin( yMin, stackedValues ), negativeStackedValues );
                yMax = qMax( qMax( yMax, stackedValues ), negativeStackedValues );
            }
        }
    }
    // special cases
    if (  yMax == yMin ) {
        if ( yMin == 0.0 )
            yMax = 0.1; //we need at least a range
        else if( yMax < 0.0 )
            yMax = 0.0; // they are the same and negative
        else if( yMin > 0.0 )
            yMin = 0.0; // they are the same but positive
    }
    const QPointF bottomLeft ( QPointF( xMin, yMin ) );
    const QPointF topRight ( QPointF( xMax, yMax ) );

    return QPair< QPointF, QPointF >( bottomLeft,  topRight );
}

void StackedBarDiagram::paint(  PaintContext* ctx )
{
    reverseMapper().clear();

    const QPair<QPointF,QPointF> boundaries = diagram()->dataBoundaries(); // cached

    const QPointF boundLeft = ctx->coordinatePlane()->translate( boundaries.first ) ;
    const QPointF boundRight = ctx->coordinatePlane()->translate( boundaries.second );

    const int rowCount = compressor().modelDataRows();
    const int colCount = compressor().modelDataColumns();

    BarAttributes ba = diagram()->barAttributes( diagram()->model()->index( 0, 0, diagram()->rootIndex() ) );
    double barWidth = 0;
    double maxDepth = 0;
    double width = boundRight.x() - boundLeft.x();
    double groupWidth = width/ (rowCount + 2);
    double spaceBetweenBars = 0;
    double spaceBetweenGroups = 0;

    if ( ba.useFixedBarWidth() ) {
        barWidth = ba.fixedBarWidth();
        groupWidth += barWidth;

        // Pending Michel set a min and max value for the groupWidth
        // related to the area.width
        if ( groupWidth < 0 )
            groupWidth = 0;

        if ( groupWidth  * rowCount > width )
            groupWidth = width / rowCount;
    }

    // maxLimit: allow the space between bars to be larger until area.width()
    // is covered by the groups.
    double maxLimit = rowCount * (groupWidth + ((colCount-1) * ba.fixedDataValueGap()) );


    //Pending Michel: FixMe
    if ( ba.useFixedDataValueGap() ) {
        if ( width > maxLimit )
            spaceBetweenBars += ba.fixedDataValueGap();
        else
            spaceBetweenBars = ((width/rowCount) - groupWidth)/(colCount-1);
    }

    if ( ba.useFixedValueBlockGap() )
        spaceBetweenGroups += ba.fixedValueBlockGap();

    calculateValueAndGapWidths( rowCount, colCount,groupWidth,
                                barWidth, spaceBetweenBars, spaceBetweenGroups );

    DataValueTextInfoList list;
    for( int col = 0; col < colCount; ++col )
    {
        double offset = spaceBetweenGroups;
        if( ba.useFixedBarWidth() )
            offset -= ba.fixedBarWidth();
        
        if( offset < 0 )
            offset = 0;

        for( int row = 0; row < rowCount; ++row )
        {
            const CartesianDiagramDataCompressor::CachePosition position( row, col );
            const CartesianDiagramDataCompressor::DataPoint p = compressor().data( position );
 
            const QModelIndex index = attributesModel()->mapToSource( p.index );
            ThreeDBarAttributes threeDAttrs = diagram()->threeDBarAttributes( index );
            const double value = p.value;
            double stackedValues = 0.0;
            double key = 0.0;

            if ( threeDAttrs.isEnabled() ) {
                if ( barWidth > 0 )
                    barWidth =  (width - ((offset+(threeDAttrs.depth()))*rowCount))/ rowCount;
                if ( barWidth <= 0 ) {
                    barWidth = 0;
                    maxDepth = offset - (width/rowCount);
                }
            } else
                barWidth =  (width - (offset*rowCount))/ rowCount ;

            for ( int k = col; k >= 0; --k )
            {
                const CartesianDiagramDataCompressor::CachePosition position( row, k );
                const CartesianDiagramDataCompressor::DataPoint point = compressor().data( position );
                if( !ISNAN( point.value ) && (( p.value >= 0.0 && point.value >= 0.0 ) || ( p.value < 0.0 && point.value < 0.0 )) )
                    stackedValues += point.value;
                key = point.key;
            }
            if (!ISNAN( value )) {
                QPointF point = ctx->coordinatePlane()->translate( QPointF( key, stackedValues ) );
                point.rx() += offset / 2;
                const QPointF previousPoint = ctx->coordinatePlane()->translate( QPointF( key, stackedValues - value ) );
                const double barHeight = previousPoint.y() - point.y();

                const QRectF rect( point, QSizeF( barWidth , barHeight ) );
                appendDataValueTextInfoToList( diagram(), list, index, PositionPoints( rect ),
                                                Position::NorthWest, Position::SouthEast,
                                                value );
                paintBars( ctx, index, rect, maxDepth );
            }
        }
    }
    paintDataValueTextsAndMarkers( diagram(), ctx, list, false );
}
