/* -*- Mode: C++ -*-
   KDChart - a multi-platform charting engine
   */

/****************************************************************************
 ** Copyright (C) 2005-2007 Klarälvdalens Datakonsult AB.  All rights reserved.
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

#include "KDChartLineDiagram.h"
#include "KDChartDataValueAttributes.h"

#include "KDChartLineDiagram_p.h"

using namespace KDChart;
using namespace std;

LineDiagram::Private::Private( const Private& rhs )
    : AbstractCartesianDiagram::Private( rhs )
{
}

void LineDiagram::Private::paintPolyline(
    PaintContext* ctx,
    const QBrush& brush, const QPen& pen,
    const QPolygonF& points ) const
{
    ctx->painter()->setBrush( brush );
    ctx->painter()->setPen( PrintingParameters::scalePen(
        QPen( pen.color(),
              pen.width(),
              pen.style(),
              Qt::FlatCap,
              Qt::MiterJoin ) ) );
#if QT_VERSION > 0x040299
    ctx->painter()->drawPolyline( points );
#else
    // FIXME (Mirko) verify, this sounds reverse-logical
    // For Qt versions older than 4.3 drawPolyline is VERY slow
    // so we use traditional line segments drawing instead then.
    for (int i = 0; i < points.size()-1; ++i)
        ctx->painter()->drawLine( points.at(i), points.at(i+1) );
#endif
}

/*!
  Projects a point in a space defined by its x, y, and z coordinates
  into a point onto a plane, given two rotation angles around the x
  resp. y axis.
*/
const QPointF LineDiagram::LineDiagramType::project(
    QPointF point, QPointF maxLimits,
    double z, const QModelIndex& index ) const
{
    Q_UNUSED( maxLimits );
    ThreeDLineAttributes td = diagram()->threeDLineAttributes( index );

    //Pending Michel FIXME - the rotation does not work as expected atm
    double xrad = DEGTORAD( td.lineXRotation() );
    double yrad = DEGTORAD( td.lineYRotation() );
    QPointF ret = QPointF(point.x()*cos( yrad ) + z * sin( yrad ) ,  point.y()*cos( xrad ) - z * sin( xrad ) );
    return ret;
}

void LineDiagram::LineDiagramType::paintThreeDLines(
    PaintContext* ctx, const QModelIndex& index,
    const QPointF& from, const QPointF& to, const double depth  )
{
    // retrieve the boundaries
    const QPair< QPointF, QPointF > boundaries = diagram()->dataBoundaries();
    const QPointF& maxLimits = boundaries.second;
    const QPointF topLeft = project( from, maxLimits, depth, index  );
    const QPointF topRight = project ( to, maxLimits, depth, index  );

    const QPolygonF segment = QPolygonF() << from << topLeft << topRight << to;
    const QBrush indexBrush ( diagram()->brush( index ) );
    const PainterSaver painterSaver( ctx->painter() );

    if( diagram()->antiAliasing() )
        ctx->painter()->setRenderHint( QPainter::Antialiasing );

    ctx->painter()->setBrush( indexBrush );
    ctx->painter()->setPen( PrintingParameters::scalePen( diagram()->pen( index ) ) );

    reverseMapper().addPolygon( index.row(), index.column(), segment );
    ctx->painter()->drawPolygon( segment );
}

// this method is factored out from LineDiagram::paint, and contains
// the common parts of the method that  previously implemented all
// chart types in one
void LineDiagram::LineDiagramType::paintElements(
    PaintContext* ctx,
    DataValueTextInfoList& list,
    LineAttributesInfoList& lineList,
    LineAttributes::MissingValuesPolicy policy )
{
    Q_UNUSED( policy );
    // paint all lines and their attributes
    const PainterSaver painterSaver( ctx->painter() );
    if ( diagram()->antiAliasing() )
        ctx->painter()->setRenderHint ( QPainter::Antialiasing );
    LineAttributesInfoListIterator itline ( lineList );

    QBrush curBrush;
    QPen curPen;
    QPolygonF points;
    while ( itline.hasNext() ) {
        const LineAttributesInfo& lineInfo = itline.next();
        const QModelIndex& index = lineInfo.index;
        const ThreeDLineAttributes td = diagram()->threeDLineAttributes( index );
        const ValueTrackerAttributes vt = diagram()->valueTrackerAttributes( index );

        if( td.isEnabled() ){
            paintThreeDLines( ctx, index, lineInfo.value, lineInfo.nextValue, td.depth() );
        } else {
            const QBrush br( diagram()->brush( index ) );
            const QPen pn( diagram()->pen( index ) );
            if( points.count() && points.last() == lineInfo.value && curBrush == br && curPen == pn ) {
                // line goes from last value in points to lineInfo.nextValue
                reverseMapper().addLine( lineInfo.index.row(), lineInfo.index.column(), points.last(), lineInfo.nextValue );
                points << lineInfo.nextValue;
            } else {
                if( points.count() )
                    paintPolyline( ctx, curBrush, curPen, points );
                curBrush = br;
                curPen   = pn;
                points.clear();
                // line goes from lineInfo.value to lineInfo,nextValue
                reverseMapper().addLine( lineInfo.index.row(), lineInfo.index.column(), lineInfo.value, lineInfo.nextValue );
                points << lineInfo.value << lineInfo.nextValue;
            }
        }

        if( vt.isEnabled() )
            paintValueTracker( ctx, vt, lineInfo.value );
    }
    if( points.count() )
        paintPolyline( ctx, curBrush, curPen, points );
    // paint all data value texts and the point markers
    paintDataValueTextsAndMarkers( diagram(), ctx, list, true );
}

AttributesModel* LineDiagram::LineDiagramType::attributesModel() const
{
    return m_private->attributesModel;
}

QModelIndex LineDiagram::LineDiagramType::attributesModelRootIndex() const
{
    return m_private->diagram->attributesModelRootIndex();
}

int LineDiagram::LineDiagramType::datasetDimension() const
{
    return m_private->datasetDimension;
}

ReverseMapper& LineDiagram::LineDiagramType::reverseMapper()
{
    return m_private->reverseMapper;
}

LineAttributes::MissingValuesPolicy LineDiagram::LineDiagramType::getCellValues(
    int row, int column,
    bool shiftCountedXValuesByHalfSection,
    double& valueX, double& valueY ) const
{
    return m_private->diagram->getCellValues( row, column, shiftCountedXValuesByHalfSection,
                                              valueX, valueY );
}

double LineDiagram::LineDiagramType::valueForCellTesting(
    int row, int column,
    bool& bOK,
    bool showHiddenCellsAsInvalid) const
{
    return m_private->diagram->valueForCellTesting( row, column, bOK, showHiddenCellsAsInvalid );
}

LineDiagram* LineDiagram::LineDiagramType::diagram() const
{
    return m_private->diagram;
}

void LineDiagram::LineDiagramType::paintAreas(
    PaintContext* ctx,
    const QModelIndex& index, const QList< QPolygonF >& areas,
    const uint transparency )
{
    QColor trans = diagram()->brush( index ).color();
    trans.setAlpha( transparency );
    QPen indexPen = diagram()->pen(index);
    indexPen.setColor( trans );
    const PainterSaver painterSaver( ctx->painter() );

    if( diagram()->antiAliasing() )
        ctx->painter()->setRenderHint( QPainter::Antialiasing );

    ctx->painter()->setPen( PrintingParameters::scalePen( indexPen ) );
    ctx->painter()->setBrush( trans );

    QPainterPath path;
    for( int i = 0; i < areas.count(); ++i )
    {
        const QPolygonF& p = areas[ i ];
        path.addPolygon( p );
        reverseMapper().addPolygon( index.row(), index.column(), p );
        path.closeSubpath();
    }
    ctx->painter()->drawPath( path );
}

double LineDiagram::LineDiagramType::valueForCell( int row, int column )
{
    return diagram()->valueForCell( row, column );
}

void LineDiagram::LineDiagramType::appendDataValueTextInfoToList(
            AbstractDiagram * diagram,
            DataValueTextInfoList & list,
            const QModelIndex & index,
            const CartesianDiagramDataCompressor::CachePosition * position,
            const PositionPoints& points,
            const Position& autoPositionPositive,
            const Position& autoPositionNegative,
            const qreal value )
{
    Q_UNUSED( autoPositionNegative );
    m_private->appendDataValueTextInfoToList( diagram, list, index, position, points,
                                              autoPositionPositive, autoPositionPositive, value );
}

void LineDiagram::LineDiagramType::paintValueTracker( PaintContext* ctx, const ValueTrackerAttributes& vt, const QPointF& at )
{
    CartesianCoordinatePlane* plane = qobject_cast<CartesianCoordinatePlane*>( ctx->coordinatePlane() );
    if( !plane )
        return;

    DataDimensionsList gridDimensions = ctx->coordinatePlane()->gridDimensionsList();
    const QPointF bottomLeft( ctx->coordinatePlane()->translate(
                              QPointF( plane->isHorizontalRangeReversed() ?
                                           gridDimensions.at( 0 ).end :
                                           gridDimensions.at( 0 ).start,
                                       plane->isVerticalRangeReversed() ?
                                           gridDimensions.at( 1 ).end :
                                           gridDimensions.at( 1 ).start ) ) );
    const QPointF markerPoint = at;
    const QPointF ordinatePoint( bottomLeft.x(), at.y() );
    const QPointF abscissaPoint( at.x(), bottomLeft.y() );

    const QSizeF markerSize = vt.markerSize();
    const QRectF ellipseMarker = QRectF( at.x() - markerSize.width() / 2,
                                         at.y() - markerSize.height() / 2,
                                         markerSize.width(), markerSize.height() );

    const QPointF ordinateMarker[3] = {
        QPointF( ordinatePoint.x(), at.y() + markerSize.height() / 2 ),
        QPointF( ordinatePoint.x() + markerSize.width() / 2, at.y() ),
        QPointF( ordinatePoint.x(), at.y() - markerSize.height() / 2 )
    };

    const QPointF abscissaMarker[3] = {
        QPointF( at.x() + markerSize.width() / 2, abscissaPoint.y() ),
        QPointF( at.x(), abscissaPoint.y() - markerSize.height() / 2 ),
        QPointF( at.x() - markerSize.width() / 2, abscissaPoint.y() )
    };

    QPointF topLeft = ordinatePoint;
    QPointF bottomRightOffset = abscissaPoint - topLeft;
    QSizeF size( bottomRightOffset.x(), bottomRightOffset.y() );
    QRectF area( topLeft, size );

    PainterSaver painterSaver( ctx->painter() );
    ctx->painter()->setPen( PrintingParameters::scalePen( vt.pen() ) );
    ctx->painter()->setBrush( QBrush() );

    ctx->painter()->drawLine( markerPoint, ordinatePoint );
    ctx->painter()->drawLine( markerPoint, abscissaPoint );

    ctx->painter()->fillRect( area, vt.areaBrush() );

    ctx->painter()->drawEllipse( ellipseMarker );

    ctx->painter()->setBrush( vt.pen().color() );
    ctx->painter()->drawPolygon( ordinateMarker, 3 );
    ctx->painter()->drawPolygon( abscissaMarker, 3 );
}

CartesianDiagramDataCompressor& LineDiagram::LineDiagramType::compressor() const
{
    return m_private->compressor;
}

double LineDiagram::LineDiagramType::interpolateMissingValue( const CartesianDiagramDataCompressor::CachePosition& pos ) const
{
    double leftValue = std::numeric_limits< double >::quiet_NaN();
    double rightValue = std::numeric_limits< double >::quiet_NaN();
    int missingCount = 1;

    const int column = pos.second;
    const int row = pos.first;
    const int rowCount = compressor().modelDataRows();

    // iterate back and forth to find valid values
    for( int r1 = row - 1; r1 > 0; --r1 )
    {
        const CartesianDiagramDataCompressor::CachePosition position( r1, column );
        const CartesianDiagramDataCompressor::DataPoint point = compressor().data( position );
        leftValue = point.value;
        if( !ISNAN( point.value ) )
            break;
        ++missingCount;
    }
    for( int r2 = row + 1; r2 < rowCount; ++r2 )
    {
        const CartesianDiagramDataCompressor::CachePosition position( r2, column );
        const CartesianDiagramDataCompressor::DataPoint point = compressor().data( position );
        rightValue = point.value;
        if( !ISNAN( point.value ) )
            break;
        ++missingCount;
    }
    if( !ISNAN( leftValue ) && !ISNAN( rightValue ) )
        return leftValue + ( rightValue - leftValue ) / ( missingCount + 1 );
    else
        return std::numeric_limits< double >::quiet_NaN();
}
