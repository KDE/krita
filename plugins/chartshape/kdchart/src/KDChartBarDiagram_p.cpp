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

#include "KDChartBarDiagram.h"
#include "KDChartDataValueAttributes.h"

#include "KDChartBarDiagram_p.h"

using namespace KDChart;

BarDiagram::Private::Private( const Private& rhs )
    : AbstractCartesianDiagram::Private( rhs )
{
}

Qt::Orientation BarDiagram::Private::abscissaOrientation() const
{
    if ( orientation == Qt::Horizontal )
        return Qt::Vertical;
    return Qt::Horizontal;
}

Qt::Orientation BarDiagram::Private::ordinateOrientation() const
{
    return orientation;
}

void BarDiagram::BarDiagramType::paintBars( PaintContext* ctx, const QModelIndex& index, const QRectF& bar, double& maxDepth )
{
    QRectF isoRect;
    QPolygonF topPoints, sidePoints;
    ThreeDBarAttributes threeDAttrs = diagram()->threeDBarAttributes( index );
    double usedDepth = 0;

    //Pending Michel: configure threeDBrush settings - shadowColor etc...
    QBrush indexBrush( diagram()->brush( index ) );
    QPen indexPen( diagram()->pen( index ) );
    PainterSaver painterSaver( ctx->painter() );
    if ( diagram()->antiAliasing() )
        ctx->painter()->setRenderHint ( QPainter::Antialiasing );
    if( threeDAttrs.isEnabled() )
        indexBrush = threeDAttrs.threeDBrush( indexBrush, bar );
    ctx->painter()->setBrush( indexBrush );
    ctx->painter()->setPen( PrintingParameters::scalePen( indexPen ) );

    if ( threeDAttrs.isEnabled() ) {
        bool stackedMode = false;
        bool percentMode = false;
        bool paintTop = true;
        if ( maxDepth )
            threeDAttrs.setDepth( -maxDepth );
        //fixme adjust the painting to reasonable depth value
        switch ( type() )
        {
        case BarDiagram::Normal:
            usedDepth = threeDAttrs.depth()/4;
            stackedMode = false;
            percentMode = false;
            break;
        case BarDiagram::Stacked:
            usedDepth = threeDAttrs.depth();
            stackedMode = true;
            percentMode = false;
            break;
        case BarDiagram::Percent:
            usedDepth = threeDAttrs.depth();
            stackedMode = false;
            percentMode = true;
            break;
        default:
            Q_ASSERT_X ( false, "dataBoundaries()",
                         "Type item does not match a defined bar chart Type." );
        }
        isoRect = bar.translated( usedDepth, -usedDepth );
        // we need to find out if the height is negative
        // and in this case paint it up and down
        //qDebug() << isoRect.height();
        if (  isoRect.height() < 0 ) {
          topPoints << isoRect.bottomLeft() << isoRect.bottomRight()
                    << bar.bottomRight() << bar.bottomLeft();
          if ( stackedMode ) {
              // fix it when several negative stacked values
              if (  index.column() == 0 ) {
                  paintTop = true;
              }
              else
                  paintTop = false;
          }

        } else {
            reverseMapper().addRect( index.row(), index.column(), isoRect );
            ctx->painter()->drawRect( isoRect );
            topPoints << bar.topLeft() << bar.topRight() << isoRect.topRight() << isoRect.topLeft();
        }

        if ( percentMode && isoRect.height() == 0 )
            paintTop = false;

        bool needToSetClippingOffForTop = false;
        if ( paintTop ){
            // Draw the top, if at least one of the top's points is
            // either inside or near at the edge of the coordinate plane:
            bool drawIt = false;
            bool hasPointOutside = false;
            const QRectF r( ctx->rectangle().adjusted(0,-1,1,0) );
            KDAB_FOREACH( QPointF pt, topPoints ) {
                if( r.contains( pt ) )
                    drawIt = true;
                else
                    hasPointOutside = true;
            }
            if( drawIt ){
                const PainterSaver p( ctx->painter() );
                needToSetClippingOffForTop = hasPointOutside && ctx->painter()->hasClipping();
                if( needToSetClippingOffForTop )
                    ctx->painter()->setClipping( false );
                reverseMapper().addPolygon( index.row(), index.column(), topPoints );
                ctx->painter()->drawPolygon( topPoints );
            }
        }



        sidePoints << bar.topRight() << isoRect.topRight() << isoRect.bottomRight() << bar.bottomRight();
        if (  bar.height() != 0 ){
            const PainterSaver p( ctx->painter() );
            if( needToSetClippingOffForTop )
                ctx->painter()->setClipping( false );
            reverseMapper().addPolygon( index.row(), index.column(), sidePoints );
            ctx->painter()->drawPolygon( sidePoints );
        }
    }

    if( bar.height() != 0 )
    {
        reverseMapper().addRect( index.row(), index.column(), bar );
        ctx->painter()->drawRect( bar );
    }
    // reset
    //diagram()->maxDepth = threeDAttrs.depth();
}

AttributesModel* BarDiagram::BarDiagramType::attributesModel() const
{
    return m_private->attributesModel;
}

QModelIndex BarDiagram::BarDiagramType::attributesModelRootIndex() const
{
    return m_private->diagram->attributesModelRootIndex();
}

BarDiagram* BarDiagram::BarDiagramType::diagram() const
{
    return m_private->diagram;
}

void BarDiagram::BarDiagramType::appendDataValueTextInfoToList(
            AbstractDiagram * diagram,
            DataValueTextInfoList & list,
            const QModelIndex & index,
            const PositionPoints& points,
            const Position& autoPositionPositive,
            const Position& autoPositionNegative,
            const qreal value )
{
    m_private->appendDataValueTextInfoToList( diagram, list, index, 0,
                                              points,
                                              autoPositionPositive, autoPositionNegative, value );
}

void BarDiagram::BarDiagramType::paintDataValueTextsAndMarkers(
    AbstractDiagram* diagram,
    PaintContext* ctx,
    const DataValueTextInfoList & list,
    bool paintMarkers )
{
    m_private->paintDataValueTextsAndMarkers( diagram, ctx, list, paintMarkers );
}


void BarDiagram::BarDiagramType::calculateValueAndGapWidths( int rowCount,int colCount,
                                             double groupWidth,
                                             double& outBarWidth,
                                             double& outSpaceBetweenBars,
                                             double& outSpaceBetweenGroups )
{

    Q_UNUSED( rowCount );

    BarAttributes ba = diagram()->barAttributes( diagram()->model()->index( 0, 0, diagram()->rootIndex() ) );

    // Pending Michel Fixme
    /* We are colCount groups to paint. Each group is centered around the
     * horizontal point position on the grid. The full area covers the
     * values -1 to colCount + 1. A bar has a relative width of one unit,
     * the gaps between bars are 0.5 wide, and the gap between groups is
     * also one unit, by default. */

    double units;
    if( type() == Normal )
        units = colCount // number of bars in group * 1.0
                + (colCount-1) * ba.barGapFactor() // number of bar gaps
                + 1 * ba.groupGapFactor(); // number of group gaps
    else
        units = 1 + 1 * ba.groupGapFactor();

    double unitWidth = groupWidth / units;
    outBarWidth = unitWidth;
    outSpaceBetweenBars += unitWidth * ba.barGapFactor();

    // Pending Michel - minLimit: allow space between bars to be reduced until the bars are displayed next to each other.
    // is that what we want?
    // sebsauer; in the case e.g. CartesianCoordinatePlane::setHorizontalRangeReversed(true) was
    // used to reverse the values, we deal with negative outSpaceBetweenBars and unitWidth here
    // and since that's correct we don't like to lose e.g. the spacing here.
    //if ( outSpaceBetweenBars < 0 )
    //    outSpaceBetweenBars = 0;

    outSpaceBetweenGroups += unitWidth * ba.groupGapFactor();
}

ReverseMapper& BarDiagram::BarDiagramType::reverseMapper()
{
    return m_private->reverseMapper;
}

CartesianDiagramDataCompressor& BarDiagram::BarDiagramType::compressor() const
{
    return m_private->compressor;
}
