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

#include "KDChartTernaryGrid.h"

#include <cmath>

#include <algorithm>
#include <limits>

#include <QtDebug>
#include <QApplication>

#include "TernaryPoint.h"
#include "TernaryConstants.h"
#include "KDChartPaintContext.h"
#include "KDChartPainterSaver_p.h"
#include "KDChartTernaryCoordinatePlane.h"
#include "KDChartPrintingParameters.h"

using namespace KDChart;

TickInfo::TickInfo( double _percentage, int _depth )
    : percentage ( _percentage )
    , depth( _depth )
{
}

bool KDChart::operator==(const TickInfo& left, const TickInfo& right)
{
    return fabs( left.percentage - right.percentage )
        <= std::numeric_limits<double>::epsilon()
        && left.depth == right.depth;
}

TernaryGrid::TernaryGrid()
    : AbstractGrid()
{
}

TernaryGrid::~TernaryGrid()
{
}

void TernaryGrid::drawGrid( PaintContext* context )
{
    static const int GridLineDistanceTreshold = 20; // <Treshold> pixels between each grid line

    QPainter& painter = *context->painter(); // recover from pointer madness
    PainterSaver s( &painter ); // can i have a reference based version of that?
    TernaryCoordinatePlane* plane = dynamic_cast<TernaryCoordinatePlane*>(context->coordinatePlane());
    Q_ASSERT_X ( plane, "TernaryGrid::drawGrid",
                 "Bad function call: PaintContext::coodinatePlane() NOT a ternary plane." );

    // translate the points and see how many grid lines we can draw:
    const int MaxDepth = 3;
    double xPixels = plane->translate( TriangleBottomRight ).x() -
                     plane->translate( TriangleBottomLeft ).x();
    int granularity = 20;
    if ( xPixels > 10 * GridLineDistanceTreshold ) granularity = 10;
    if ( xPixels > 20 * GridLineDistanceTreshold ) granularity = 5;

    m_tickInfo.clear();
    for ( int i = granularity; i < 100; i+=granularity )
    {
        TickInfo tick( ( 1.0 * i ) / 100.0, 2 );
        if ( i % 10 == 0 ) tick.depth = 1;
        if ( i % 20 == 0 ) tick.depth = 0;
        m_tickInfo.append( tick );
    }

    QVector<QLineF> lines[MaxDepth];
    {Q_FOREACH( const TickInfo& tick, m_tickInfo ) {
        const double& percent = tick.percentage;
        {   // draw parallels to B
            TernaryPoint ternaryStart( percent, 1.0 - percent );
            TernaryPoint ternaryEnd( 0.0, 1.0 - percent );
            QPointF start( translate( ternaryStart ) );
            QPointF end( translate( ternaryEnd ) );
            lines[tick.depth].append( QLineF( plane->translate( start ),
                                              plane->translate( end ) ) );
        }
        {   // draw parallels to C
            TernaryPoint ternaryStart( percent, 0.0 );
            TernaryPoint ternaryEnd( 0.0, percent );
            QPointF start( translate( ternaryStart ) );
            QPointF end( translate( ternaryEnd ) );
            lines[tick.depth].append( QLineF( plane->translate( start ),
                                              plane->translate( end ) ) );
        }
        {   // draw parallels to A
            TernaryPoint ternaryStart( percent, 1.0 - percent );
            TernaryPoint ternaryEnd( percent, 0.0 );
            QPointF start( translate( ternaryStart ) );
            QPointF end( translate( ternaryEnd ) );
            lines[tick.depth].append( QLineF( plane->translate( start ),
                                              plane->translate( end ) ) );
        }
    }}

    // now draw the lines:
    painter.setPen( PrintingParameters::scalePen( QPen( QColor( "lightgray" ), 1 ) ) );
    painter.setBrush( QColor( "lightgray" ) );
    painter.drawLines( lines[2] );
    painter.setPen( PrintingParameters::scalePen( QPen( QColor( "gray" ), 1 ) ) );
    painter.setBrush( QColor( "gray" ) );
    painter.drawLines( lines[1] );
    painter.setPen( PrintingParameters::scalePen( QPen( QColor( "darkslategray" ), 1 ) ) );
    painter.setBrush( QColor( "darkslategray" ) );
    painter.drawLines( lines[0] );

    // now draw the triangle (this could be part of the axis, in fact):
    painter.setPen( PrintingParameters::scalePen( QPen( Qt::black, 1 ) ) );
    // make sure this does not fill, otherwise it wipes the contents
    // of the triangle (doh!):
    painter.setBrush( Qt::NoBrush );
    QPolygonF points;
    points << plane->translate( TriangleBottomLeft )
           << plane->translate( TriangleBottomRight )
           << plane->translate( TriangleTop );
    painter.drawPolygon( points );

    // now draw the ticks:
    painter.setPen( PrintingParameters::scalePen( QPen( Qt::black ) ) );
    painter.setBrush( Qt::black );

    QVector<QLineF> ticks;
    // prepare list of percentages, then calculate lines:
    QVector<TickInfo> percentages( m_tickInfo );
    // I have commented those out, I think it looks ugly if they are
    // enabled:
    // percentages.prepend( 0.0 );
    // percentages.append( 1.0 );

    // FIXME this may need a predicate that takes eplison into account
    // (but it does not hurt, since it will not make the painter
    // paint two lines):
    percentages.erase( std::unique( percentages.begin(), percentages.end() ),
                       percentages.end() );

    {Q_FOREACH( const TickInfo& tick, percentages ) {
        const double& percent = tick.percentage;
        {   // BC axis markers:
            const QPointF markerDistance( FullMarkerDistanceBC
                                          / ( tick.depth + 1 ) );
            QPointF start( percent, 0.0 );
            ticks.append( QLineF( plane->translate( start ),
                                  plane->translate( start - markerDistance ) ) );
        }
        {   // AC axis markers:
            const QPointF markerDistance( FullMarkerDistanceAC
                                          / ( tick.depth + 1 ) );
            const QPointF start( TriangleBottomRight + percent * AxisVector_C_A );
            const QPointF end( start + markerDistance );
            ticks.append( QLineF( plane->translate( start ),
                                  plane->translate( end ) ) );
        }
        {
            // AB axis markers:
            const QPointF markerDistance( FullMarkerDistanceBA
                                          / ( tick.depth +1 ) );
            const QPointF start( percent * AxisVector_B_A );
            const QPointF end( start + markerDistance );
            ticks.append( QLineF( plane->translate( start ),
                                  plane->translate( end ) ) );
        }
    }}
    painter.drawLines( ticks );
}

DataDimensionsList TernaryGrid::calculateGrid( const DataDimensionsList& ) const
{
    return DataDimensionsList();
}

QPair<QSizeF, QSizeF> TernaryGrid::requiredMargins() const
{
//    double topMargin = ( FullMarkerDistanceBA * RelMarkerLength ).x();
    double topMargin = 0.0; // no markers on tip of triangle
    double leftMargin = fabs( FullMarkerDistanceBA.x() );
    double bottomMargin = fabs( FullMarkerDistanceBC.y() );
//     qDebug() << "TernaryGrid::requiredMargins: leftMargin:" << leftMargin
//              << ", bottomMargin:" << bottomMargin
//              << ", topMargin:" << topMargin
//              << ", FullMarkerDistanceBC:" << FullMarkerDistanceBC
//              << ", FullMarkerDistanceBA:" << FullMarkerDistanceBA
//              << ", FullMarkerDistanceAC:" << FullMarkerDistanceAC
//              << ", RelMarkerLength:" << RelMarkerLength;
    return QPair<QSizeF, QSizeF>
        ( QSizeF( leftMargin, topMargin ),
          QSizeF( leftMargin, bottomMargin ) );
}

const QVector<TickInfo>& TernaryGrid::tickInfo() const
{
    return m_tickInfo;
}
