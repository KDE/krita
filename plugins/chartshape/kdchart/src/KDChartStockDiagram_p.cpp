/* -*- Mode: C++ -*-
   KDChart - a multi-platform charting engine
   */

/****************************************************************************
 ** Copyright (C) 2009 Klaralvdalens Datakonsult AB.  All rights reserved.
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

#include "KDChartStockDiagram_p.h"

using namespace KDChart;


class StockDiagram::Private::ThreeDPainter
{
public:
    struct ThreeDProperties {
        qreal depth;
        qreal angle;
        bool useShadowColors;
    };

    ThreeDPainter( QPainter *p )
        : painter( p ) {};

    QPolygonF drawTwoDLine( const QLineF &line, const QPen &pen,
                            const ThreeDProperties &props );
    QPolygonF drawThreeDLine( const QLineF &line, const QBrush &brush,
                              const QPen &pen, const ThreeDProperties &props );
    QPolygonF drawThreeDRect( const QRectF &rect, const QBrush &brush,
                              const QPen &pen, const ThreeDProperties &props );

private:
    QPointF projectPoint( const QPointF &point, qreal depth, qreal angle ) const;
    QColor calcShadowColor( const QColor &color, qreal angle ) const;

    QPainter *painter;
};

/**
 * Projects a point in 3D space
 *
 * @param depth The distance from the point and the projected point
 * @param angle The angle the projected point is rotated by around the original point
 */
QPointF StockDiagram::Private::ThreeDPainter::projectPoint( const QPointF &point, qreal depth, qreal angle ) const
{
    const qreal angleInRad = DEGTORAD( angle );
    const qreal distX = depth * cos( angleInRad );
    // Y coordinates are reversed on our coordinate plane
    const qreal distY = depth * -sin( angleInRad );

    return QPointF( point.x() + distX, point.y() + distY );
}

/**
 * Returns the shadow color for a given color, depending on the angle of rotation
 *
 * @param color The color to calculate the shadow color for
 * @param angle The angle that the colored area is rotated by
 */
QColor StockDiagram::Private::ThreeDPainter::calcShadowColor( const QColor &color, qreal angle ) const
{
    // The shadow factor determines to how many percent the brightness
    // of the color can be reduced. That is, the darkest shadow color
    // is color * shadowFactor.
    const qreal shadowFactor = 0.5;
    const qreal sinAngle = 1.0 - qAbs( sin( DEGTORAD( angle ) ) ) * shadowFactor;
    return QColor( qRound( color.red()   * sinAngle ),
                   qRound( color.green() * sinAngle ),
                   qRound( color.blue()  * sinAngle ) );
}

/**
 * Draws a 2D line in 3D space by painting it with a z-coordinate of props.depth / 2.0
 *
 * @param line The line to draw
 * @param pen The pen to use to draw the line
 * @param props The 3D properties to draw the line with
 * @return The drawn line, but with a width of 2px, as a polygon
 */
QPolygonF StockDiagram::Private::ThreeDPainter::drawTwoDLine( const QLineF &line, const QPen &pen,
                                                              const ThreeDProperties &props )
{
    // Restores the painting properties when destroyed
    PainterSaver painterSaver( painter );

    // The z coordinate to use (i.e., at what depth to draw the line)
    const qreal z = props.depth / 2.0;

    // Projec the 2D points of the line in 3D
    const QPointF deepP1 = projectPoint( line.p1(), z, props.angle );
    const QPointF deepP2 = projectPoint( line.p2(), z, props.angle );

    // The drawn line with a width of 2px
    QPolygonF threeDArea;
    // The offset of the line "borders" from the center to each side
    const QPointF offset( 0.0, 1.0 );
    threeDArea << deepP1 - offset << deepP2 - offset
               << deepP1 + offset << deepP2 + offset << deepP1 - offset;

    painter->setPen( pen );
    painter->drawLine( QLineF( deepP1, deepP2 ) );

    return threeDArea;
}

/**
 * Draws an ordinary line in 3D by expanding it in the z-axis by the given depth.
 *
 * @param line The line to draw
 * @param brush The brush to fill the resulting polygon with
 * @param pen The pen to paint the borders of the resulting polygon with
 * @param props The 3D properties to draw the line with
 * @return The 3D shape drawn
 */
QPolygonF StockDiagram::Private::ThreeDPainter::drawThreeDLine( const QLineF &line, const QBrush &brush,
                                                                const QPen &pen, const ThreeDProperties &props )
{
    // Restores the painting properties when destroyed
    PainterSaver painterSaver( painter );

    const QPointF p1 = line.p1();
    const QPointF p2 = line.p2();

    // Project the 2D points of the line in 3D
    const QPointF deepP1 = projectPoint( p1, props.depth, props.angle );
    const QPointF deepP2 = projectPoint( p2, props.depth, props.angle );

    // The result is a 3D representation of the 2D line
    QPolygonF threeDArea;
    threeDArea << p1 << p2 << deepP2 << deepP1 << p1;

    // Use shadow colors if ThreeDProperties::useShadowColors is set
    // Note: Setting a new color on a brush or pen does not effect gradients or textures
    if ( props.useShadowColors ) {
        QBrush shadowBrush( brush );
        QPen shadowPen( pen );
        shadowBrush.setColor( calcShadowColor( brush.color(), props.angle ) );
        shadowPen.setColor( calcShadowColor( pen.color(), props.angle ) );
        painter->setBrush( shadowBrush );
        painter->setPen( shadowPen );
    } else {
        painter->setBrush( brush );
        painter->setPen( pen );
    }

    painter->drawPolygon( threeDArea );

    return threeDArea;
}

/**
 * Draws a 3D cuboid by extending a 2D rectangle in the z-axis
 *
 * @param rect The rectangle to draw
 * @param brush The brush fill the surfaces of the cuboid with
 * @param pen The pen to draw the edges with
 * @param props The 3D properties to use for drawing the cuboid
 * @return The drawn cuboid as a polygon
 */
QPolygonF StockDiagram::Private::ThreeDPainter::drawThreeDRect( const QRectF &rect, const QBrush &brush,
                                                                const QPen &pen, const ThreeDProperties &props )
{
    // Restores the painting properties when destroyed
    PainterSaver painterSaver( painter );

    // Make sure that the top really is the top
    const QRectF normalizedRect = rect.normalized();

    // Calculate all the four sides of the rectangle
    const QLineF topSide = QLineF( normalizedRect.topLeft(), normalizedRect.topRight() );
    const QLineF bottomSide = QLineF( normalizedRect.bottomLeft(), normalizedRect.bottomRight() );
    const QLineF leftSide = QLineF( normalizedRect.topLeft(), normalizedRect.bottomLeft() );
    const QLineF rightSide = QLineF( normalizedRect.topRight(), normalizedRect.bottomRight() );

    QPolygonF drawnPolygon;

    // Shorter names are easier on the eyes
    const qreal angle = props.angle;

    // Only top and right side is visible
    if ( angle >= 0.0 && angle < 90.0 ) {
        drawnPolygon = drawnPolygon.united( drawThreeDLine( topSide, brush, pen, props ) );
        drawnPolygon = drawnPolygon.united( drawThreeDLine( rightSide, brush, pen, props ) );
    // Only top and left side is visible
    } else if ( angle >= 90.0 && angle < 180.0 ) {
        drawnPolygon = drawnPolygon.united( drawThreeDLine( topSide, brush, pen, props ) );
        drawnPolygon = drawnPolygon.united( drawThreeDLine( leftSide, brush, pen, props ) );
    // Only bottom and left side is visible
    } else if ( angle >= 180.0 && angle < 270.0 ) {
        drawnPolygon = drawnPolygon.united( drawThreeDLine( bottomSide, brush, pen, props ) );
        drawnPolygon = drawnPolygon.united( drawThreeDLine( leftSide, brush, pen, props ) );
    // Only bottom and right side is visible
    } else if ( angle >= 270.0 && angle <= 360.0 ) {
        drawnPolygon = drawnPolygon.united( drawThreeDLine( bottomSide, brush, pen, props ) );
        drawnPolygon = drawnPolygon.united( drawThreeDLine( rightSide, brush, pen, props ) );
    }

    // Draw the front side
    painter->setPen( pen );
    painter->setBrush( brush );
    painter->drawRect( normalizedRect );

    return drawnPolygon;
}


StockDiagram::Private::Private()
    : AbstractCartesianDiagram::Private()
{
}

StockDiagram::Private::Private( const Private& r )
    : AbstractCartesianDiagram::Private( r )
{
}

StockDiagram::Private::~Private()
{
}

/**
 * Projects a point onto the coordinate plane
 *
 * @param context The context to paint the point in
 * @point The point to project onto the coordinate plane
 * @return The projected point
 */
QPointF StockDiagram::Private::projectPoint( PaintContext *context, const QPointF &point ) const
{
    return context->coordinatePlane()->translate( QPointF( point.x() + 0.5, point.y() ) );
}

/**
 * Projects a candlestick onto the coordinate plane
 *
 * @param context The context to paint the candlestick in
 * @param low The
 */
QRectF StockDiagram::Private::projectCandlestick( PaintContext *context, const QPointF &open, const QPointF &close, qreal width ) const
{
    const QPointF leftHighPoint = context->coordinatePlane()->translate( QPointF( close.x() + 0.5 - width / 2.0, close.y() ) );
    const QPointF rightLowPoint = context->coordinatePlane()->translate( QPointF( open.x() + 0.5 + width / 2.0, open.y() ) );
    const QPointF rightHighPoint = context->coordinatePlane()->translate( QPointF( close.x() + 0.5 + width / 2.0, close.y() ) );

    return QRectF( leftHighPoint, QSizeF( rightHighPoint.x() - leftHighPoint.x(),
                                          rightLowPoint.y() - leftHighPoint.y() ) );
}

void StockDiagram::Private::drawOHLCBar( const CartesianDiagramDataCompressor::DataPoint &open,
        const CartesianDiagramDataCompressor::DataPoint &high,
        const CartesianDiagramDataCompressor::DataPoint &low,
        const CartesianDiagramDataCompressor::DataPoint &close,
        PaintContext *context )
{
    // Note: A row in the model is a column in a StockDiagram
    const int col = low.index.row();

    StockBarAttributes attr = diagram->stockBarAttributes( col );
    ThreeDBarAttributes threeDAttr = diagram->threeDBarAttributes( col );
    const qreal tickLength = attr.tickLength();

    const QPointF leftOpenPoint( open.key + 0.5 - tickLength, open.value );
    const QPointF rightOpenPoint( open.key + 0.5, open.value );
    const QPointF highPoint( high.key + 0.5, high.value );
    const QPointF lowPoint( low.key + 0.5, low.value );
    const QPointF leftClosePoint( close.key + 0.5, close.value );
    const QPointF rightClosePoint( close.key + 0.5 + tickLength, close.value );

    bool reversedOrder = false;
    // If 3D mode is enabled, we have to make sure the z-order is right
    if ( threeDAttr.isEnabled() ) {
        const int angle = threeDAttr.angle();
        // Z-order is from right to left
        if ( (angle >= 0 && angle < 90) || (angle >= 180 && angle < 270) )
            reversedOrder = true;
        // Z-order is from left to right
        if ( (angle >= 90 && angle < 180) || (angle >= 270 && angle < 0) )
            reversedOrder = false;
    }

    if ( reversedOrder ) {
        if ( !open.hidden )
            drawLine( col, leftOpenPoint, rightOpenPoint, context ); // Open marker
        if ( !low.hidden && !high.hidden )
            drawLine( col, lowPoint, highPoint, context ); // Low-High line
        if ( !close.hidden )
            drawLine( col, leftClosePoint, rightClosePoint, context ); // Close marker
    } else {
        if ( !close.hidden )
            drawLine( col, leftClosePoint, rightClosePoint, context ); // Close marker
        if ( !low.hidden && !high.hidden )
            drawLine( col, lowPoint, highPoint, context ); // Low-High line
        if ( !open.hidden )
            drawLine( col, leftOpenPoint, rightOpenPoint, context ); // Open marker
    }

    DataValueTextInfoList list;
    if ( !open.hidden )
        appendDataValueTextInfoToList( diagram, list, diagram->attributesModel()->mapToSource( open.index ), 0,
                                       PositionPoints( leftOpenPoint ), Position::South, Position::South, open.value );
    if ( !high.hidden )
        appendDataValueTextInfoToList( diagram, list, diagram->attributesModel()->mapToSource( high.index ), 0,
                                       PositionPoints( highPoint ), Position::South, Position::South, high.value );
    if ( !low.hidden )
        appendDataValueTextInfoToList( diagram, list, diagram->attributesModel()->mapToSource( low.index ), 0,
                                       PositionPoints( lowPoint ), Position::South, Position::South, low.value );
    if ( !close.hidden )
        appendDataValueTextInfoToList( diagram, list, diagram->attributesModel()->mapToSource( close.index ), 0,
                                       PositionPoints( rightClosePoint ), Position::South, Position::South, close.value );
    paintDataValueTextsAndMarkers( diagram, context, list, false );
}

/**
  * Draws a line connecting the low and the high value of an OHLC chart
  *
  * @param low The low data point
  * @param high The high data point
  * @param context The context to draw the candlestick in
  */
void StockDiagram::Private::drawCandlestick( const CartesianDiagramDataCompressor::DataPoint &open,
                                             const CartesianDiagramDataCompressor::DataPoint &high,
                                             const CartesianDiagramDataCompressor::DataPoint &low,
                                             const CartesianDiagramDataCompressor::DataPoint &close,
                                             PaintContext *context )
{
    PainterSaver painterSaver( context->painter() );

    // Note: A row in the model is a column in a StockDiagram, and the other way around
    const int row = low.index.row();
    const int col = low.index.column();

    QPointF bottomCandlestickPoint;
    QPointF topCandlestickPoint;
    QBrush brush;
    QPen pen;
    bool drawLowerLine;
    bool drawCandlestick = !open.hidden && !close.hidden;
    bool drawUpperLine;

    // Find out if we need to paint a down-trend or up-trend candlestick
    // and set brush and pen accordingly
    // Also, determine what the top and bottom points of the candlestick are
    if ( open.value <= close.value ) {
        pen = diagram->upTrendCandlestickPen( row );
        brush = diagram->upTrendCandlestickBrush( row );
        bottomCandlestickPoint = QPointF( open.key, open.value );
        topCandlestickPoint = QPointF( close.key, close.value );
        drawLowerLine = !low.hidden && !open.hidden;
        drawUpperLine = !low.hidden && !close.hidden;
    } else {
        pen = diagram->downTrendCandlestickPen( row );
        brush = diagram->downTrendCandlestickBrush( row );
        bottomCandlestickPoint = QPointF( close.key, close.value );
        topCandlestickPoint = QPointF( open.key, open.value );
        drawLowerLine = !low.hidden && !close.hidden;
        drawUpperLine = !low.hidden && !open.hidden;
    }

    StockBarAttributes attr = diagram->stockBarAttributes( col );
    ThreeDBarAttributes threeDAttr = diagram->threeDBarAttributes( col );

    const QPointF lowPoint = projectPoint( context, QPointF( low.key, low.value ) );
    const QPointF highPoint = projectPoint( context, QPointF( high.key, high.value ) );
    const QLineF lowerLine = QLineF( lowPoint, projectPoint( context, bottomCandlestickPoint ) );
    const QLineF upperLine = QLineF( projectPoint( context, topCandlestickPoint ), highPoint );

    // Convert the data point into coordinates on the coordinate plane
    QRectF candlestick = projectCandlestick( context, bottomCandlestickPoint,
                                             topCandlestickPoint, attr.candlestickWidth() );

    // Remember the drawn polygon to add it to the ReverseMapper later
    QPolygonF drawnPolygon;

    // Use the ThreeDPainter class to draw a 3D candlestick
    if ( threeDAttr.isEnabled() ) {
        ThreeDPainter threeDPainter( context->painter() );

        ThreeDPainter::ThreeDProperties threeDProps;
        threeDProps.depth = threeDAttr.depth();
        threeDProps.angle = threeDAttr.angle();
        threeDProps.useShadowColors = threeDAttr.useShadowColors();

        // If the perspective angle is within [0,180], we paint from bottom to top,
        // otherwise from top to bottom to ensure the correct z order
        if ( threeDProps.angle > 0.0 && threeDProps.angle < 180.0 ) {
            if ( drawLowerLine )
                drawnPolygon = threeDPainter.drawTwoDLine( lowerLine, pen, threeDProps );
            if ( drawCandlestick )
                drawnPolygon = threeDPainter.drawThreeDRect( candlestick, brush, pen, threeDProps );
            if ( drawUpperLine )
            drawnPolygon = threeDPainter.drawTwoDLine( upperLine, pen, threeDProps );
        } else {
            if ( drawUpperLine )
                drawnPolygon = threeDPainter.drawTwoDLine( upperLine, pen, threeDProps );
            if ( drawCandlestick )
                drawnPolygon = threeDPainter.drawThreeDRect( candlestick, brush, pen, threeDProps );
            if ( drawLowerLine )
                drawnPolygon = threeDPainter.drawTwoDLine( lowerLine, pen, threeDProps );
        }
    } else {
        QPainter *const painter = context->painter();
        painter->setBrush( brush );
        painter->setPen( pen );
        if ( drawLowerLine )
            painter->drawLine( lowerLine );
        if ( drawUpperLine )
            painter->drawLine( upperLine );
        if ( drawCandlestick )
            painter->drawRect( candlestick );

        // The 2D representation is the projected candlestick itself
        drawnPolygon = candlestick;

        // FIXME: Add lower and upper line to reverse mapper
    }

    DataValueTextInfoList list;

    if ( !low.hidden )
        appendDataValueTextInfoToList( diagram, list, diagram->attributesModel()->mapToSource( low.index ), 0,
                                       PositionPoints( lowPoint ), Position::South, Position::South, low.value );
    if ( drawCandlestick ) {
        // Both, the open as well as the close value are represented by this candlestick
        reverseMapper.addPolygon( row, openValueColumn(), drawnPolygon );
        reverseMapper.addPolygon( row, closeValueColumn(), drawnPolygon );

        appendDataValueTextInfoToList( diagram, list, diagram->attributesModel()->mapToSource( open.index ), 0,
                                       PositionPoints( candlestick.bottomRight() ), Position::South, Position::South, open.value );
        appendDataValueTextInfoToList( diagram, list, diagram->attributesModel()->mapToSource( close.index ), 0,
                                       PositionPoints( candlestick.topRight() ), Position::South, Position::South, close.value );
    }
    if ( !high.hidden )
        appendDataValueTextInfoToList( diagram, list, diagram->attributesModel()->mapToSource( high.index ), 0,
                                       PositionPoints( highPoint ), Position::South, Position::South, high.value );

    paintDataValueTextsAndMarkers( diagram, context, list, false );
}

/**
  * Draws a line connecting two points
  *
  * @param col The column of the diagram to paint the line in
  * @param point1 The first point
  * @param point2 The second point
  * @param context The context to draw the low-high line in
  */
void StockDiagram::Private::drawLine( int col, const QPointF &point1, const QPointF &point2, PaintContext *context )
{
    PainterSaver painterSaver( context->painter() );

    // A row in the model is a column in the diagram
    const int modelRow = col;
    const int modelCol = 0;

    const QPen pen = diagram->pen( col );
    const QBrush brush = diagram->brush( col );
    const ThreeDBarAttributes threeDBarAttr = diagram->threeDBarAttributes( col );

    QPointF transP1 = context->coordinatePlane()->translate( point1 );
    QPointF transP2 = context->coordinatePlane()->translate( point2 );
    QLineF line = QLineF( transP1, transP2 );

    if ( threeDBarAttr.isEnabled() ) {
        ThreeDPainter::ThreeDProperties threeDProps;
        threeDProps.angle = threeDBarAttr.angle();
        threeDProps.depth = threeDBarAttr.depth();
        threeDProps.useShadowColors = threeDBarAttr.useShadowColors();

        ThreeDPainter painter( context->painter() );
        reverseMapper.addPolygon( modelCol, modelRow, painter.drawThreeDLine( line, brush, pen, threeDProps ) );
    } else {
        context->painter()->setPen( pen );
        //context->painter()->setBrush( brush );
        reverseMapper.addLine( modelCol, modelRow, transP1, transP2 );
        context->painter()->drawLine( line );
    }
}

/**
 * Returns the column of the open value in the model
 *
 * @return The column of the open value
 */
int StockDiagram::Private::openValueColumn() const
{
    // Return an invalid column if diagram has no open values
    return type == HighLowClose ? -1 : 0;
}

/**
 * Returns the column of the high value in the model
 *
 * @return The column of the high value
 */
int StockDiagram::Private::highValueColumn() const
{
    return type == HighLowClose ? 0 : 1;
}

/**
 * Returns the column of the low value in the model
 *
 * @return The column of the low value
 */
int StockDiagram::Private::lowValueColumn() const
{
    return type == HighLowClose ? 1 : 2;
}

/**
 * Returns the column of the close value in the model
 *
 * @return The column of the close value
 */
int StockDiagram::Private::closeValueColumn() const
{
    return type == HighLowClose ? 2 : 3;
}

