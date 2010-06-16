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

#include "KDChartStockDiagram.h"
#include "KDChartStockDiagram_p.h"

#include "KDChartPaintContext.h"

using namespace KDChart;

#define d d_func()

StockDiagram::StockDiagram( QWidget *parent, CartesianCoordinatePlane *plane )
    : AbstractCartesianDiagram( new Private(), parent, plane )
{
    init();
}

StockDiagram::~StockDiagram()
{
}

/**
  * Initializes the diagram
  */
void StockDiagram::init()
{
    d->diagram = this;
    d->compressor.setModel( attributesModel() );

    // Set properties to defaults
    d->type = HighLowClose;
    d->upTrendCandlestickBrush = QBrush( Qt::white );
    d->downTrendCandlestickBrush = QBrush( Qt::black );
    d->upTrendCandlestickPen = QPen( Qt::black );
    d->downTrendCandlestickPen = QPen( Qt::black );

    d->lowHighLinePen = QPen( Qt::black );

    setPen( QPen( Qt::black ) );
}

/**
  * Switches between the supported types of stock charts,
  * depending on \a type
  */
void StockDiagram::setType( Type type )
{
    d->type = type;
    emit propertiesChanged();
}

/**
  * @return the type of this diagram
  */
StockDiagram::Type StockDiagram::type() const
{
   return d->type;
}

void StockDiagram::setStockBarAttributes( const StockBarAttributes &attr )
{
    attributesModel()->setModelData(
            qVariantFromValue( attr ),
            StockBarAttributesRole );
    emit propertiesChanged();
}

StockBarAttributes StockDiagram::stockBarAttributes() const
{
    return qVariantValue<StockBarAttributes>(
        attributesModel()->modelData( StockBarAttributesRole ) );
}

void StockDiagram::setStockBarAttributes( int column, const StockBarAttributes &attr )
{
    attributesModel()->setHeaderData(
        column, Qt::Vertical,
        qVariantFromValue( attr ),
        StockBarAttributesRole );
    emit propertiesChanged();
}

StockBarAttributes StockDiagram::stockBarAttributes( int column ) const
{
    const QVariant attr( attributesModel()->headerData(
            column, Qt::Vertical, StockBarAttributesRole ) );
    if ( attr.isValid() )
        return qVariantValue<StockBarAttributes>( attr );
    return stockBarAttributes();
}

/**
 * Sets the 3D attributes for all bars (i.e. candlesticks)
 *
 * @param attr The 3D attributes to set
 */
void StockDiagram::setThreeDBarAttributes( const ThreeDBarAttributes &attr )
{
    attributesModel()->setModelData(
            qVariantFromValue( attr ),
            ThreeDBarAttributesRole );
    emit propertiesChanged();
}

/**
 * Returns the 3D attributes for all bars (i.e. candlesticks)
 *
 * @return the 3D bar attributes
 */
ThreeDBarAttributes StockDiagram::threeDBarAttributes() const
{
	return qVariantValue<ThreeDBarAttributes>(
			attributesModel()->modelData( ThreeDBarAttributesRole ) );
}

/**
 * Sets the 3D attributes for the bar (i.e. candlestick) in certain column
 * of the diagram
 *
 * Note: Every column in a StockDiagram is represented by a row in the model
 *
 * @param column The column to set the 3D bar attributes for
 * @param attr The 3D attributes to set
 */
void StockDiagram::setThreeDBarAttributes( int column, const ThreeDBarAttributes &attr )
{
    attributesModel()->setHeaderData(
            column, Qt::Vertical,
            qVariantFromValue( attr ),
            StockBarAttributesRole );
    emit propertiesChanged();
}

/**
 * Returns the 3D attributes for a bars (i.e. candlestick) in a certain column
 * of the diagram
 *
 * Note: Every column in a StockDiagram is represented by a row in the model
 *
 * @param column The column to get the 3D bar attributes for
 * @return The 3D attributes for the specified column
 */
ThreeDBarAttributes StockDiagram::threeDBarAttributes( int column ) const
{
    const QVariant attr( attributesModel()->headerData(
            column, Qt::Vertical, ThreeDBarAttributesRole ) );
    if ( attr.isValid() )
        return qVariantValue<ThreeDBarAttributes>( attr );
    return threeDBarAttributes();
}


void StockDiagram::setLowHighLinePen( const QPen &pen )
{
    d->lowHighLinePen = pen;
}

QPen StockDiagram::lowHighLinePen() const
{
    return d->lowHighLinePen;
}

void StockDiagram::setLowHighLinePen( int column, const QPen &pen )
{
    d->lowHighLinePens[column] = pen;
}

QPen StockDiagram::lowHighLinePen( int column ) const
{
    if ( d->lowHighLinePens.contains( column ) )
        return d->lowHighLinePens[column];
    return d->lowHighLinePen;
}

void StockDiagram::setUpTrendCandlestickBrush( const QBrush &brush )
{
    d->upTrendCandlestickBrush = brush;
}

QBrush StockDiagram::upTrendCandlestickBrush() const
{
    return d->upTrendCandlestickBrush;
}

void StockDiagram::setDownTrendCandlestickBrush( const QBrush &brush )
{
    d->downTrendCandlestickBrush = brush;
}

QBrush StockDiagram::downTrendCandlestickBrush() const
{
    return d->downTrendCandlestickBrush;
}

void StockDiagram::setUpTrendCandlestickBrush( int column, const QBrush &brush )
{
    d->upTrendCandlestickBrushes[column] = brush;
}

QBrush StockDiagram::upTrendCandlestickBrush( int column ) const
{
    if ( d->upTrendCandlestickBrushes.contains( column ) )
        return d->upTrendCandlestickBrushes[column];
    return d->upTrendCandlestickBrush;
}

void StockDiagram::setDownTrendCandlestickBrush( int column, const QBrush &brush )
{
    d->downTrendCandlestickBrushes[column] = brush;
}

QBrush StockDiagram::downTrendCandlestickBrush( int column ) const
{
    if ( d->downTrendCandlestickBrushes.contains( column ) )
        return d->downTrendCandlestickBrushes[column];
    return d->downTrendCandlestickBrush;
}


void StockDiagram::setUpTrendCandlestickPen( const QPen &pen )
{
    d->upTrendCandlestickPen = pen;
}

QPen StockDiagram::upTrendCandlestickPen() const
{
    return d->upTrendCandlestickPen;
}

void StockDiagram::setDownTrendCandlestickPen( const QPen &pen )
{
    d->downTrendCandlestickPen = pen;
}

QPen StockDiagram::downTrendCandlestickPen() const
{
    return d->downTrendCandlestickPen;
}

void StockDiagram::setUpTrendCandlestickPen( int column, const QPen &pen )
{
    d->upTrendCandlestickPens[column] = pen;
}

QPen StockDiagram::upTrendCandlestickPen( int column ) const
{
    if ( d->upTrendCandlestickPens.contains( column ) )
        return d->upTrendCandlestickPens[column];
    return d->upTrendCandlestickPen;
}

void StockDiagram::setDownTrendCandlestickPen( int column, const QPen &pen )
{
    d->downTrendCandlestickPens[column] = pen;
}

QPen StockDiagram::downTrendCandlestickPen( int column ) const
{
    if ( d->downTrendCandlestickPens.contains( column ) )
        return d->downTrendCandlestickPens[column];
    return d->downTrendCandlestickPen;
}

#if QT_VERSION < 0x040400 || defined(Q_COMPILER_MANGLES_RETURN_TYPE)
const
#endif
int StockDiagram::numberOfAbscissaSegments() const { return 1; }

#if QT_VERSION < 0x040400 || defined(Q_COMPILER_MANGLES_RETURN_TYPE)
const
#endif
int StockDiagram::numberOfOrdinateSegments() const { return 1; }

void StockDiagram::paint( PaintContext *context )
{
    // Clear old reverse mapping data and create new
    // reverse mapping scene
    d->reverseMapper.clear();

    PainterSaver painterSaver( context->painter() );
    int rowCount = attributesModel()->rowCount( attributesModelRootIndex() );
    for ( int row = 0; row < rowCount; row++ ) {
        CartesianDiagramDataCompressor::DataPoint low;
        CartesianDiagramDataCompressor::DataPoint high;
        CartesianDiagramDataCompressor::DataPoint open;
        CartesianDiagramDataCompressor::DataPoint close;
        CartesianDiagramDataCompressor::DataPoint volume;

        if ( d->type == HighLowClose ) {
            const CartesianDiagramDataCompressor::CachePosition highPos( row, 0 );
            const CartesianDiagramDataCompressor::CachePosition lowPos( row, 1 );
            const CartesianDiagramDataCompressor::CachePosition closePos( row, 2 );
            low = d->compressor.data( lowPos );
            high = d->compressor.data( highPos );
            close = d->compressor.data( closePos );
        } else if ( d->type == OpenHighLowClose || d->type == Candlestick ) {
            const CartesianDiagramDataCompressor::CachePosition openPos( row, 0 );
            const CartesianDiagramDataCompressor::CachePosition highPos( row, 1 );
            const CartesianDiagramDataCompressor::CachePosition lowPos( row, 2 );
            const CartesianDiagramDataCompressor::CachePosition closePos( row, 3 );
            open = d->compressor.data( openPos );
            low = d->compressor.data( lowPos );
            high = d->compressor.data( highPos );
            close = d->compressor.data( closePos );
        }

        switch( d->type ) {
        case HighLowClose:
            open.hidden = true;
            // Fall-through intended!
        case OpenHighLowClose:
            d->drawOHLCBar( open, high, low, close, context );
            break;
        case Candlestick:
            d->drawCandlestick( open, high, low, close, context );
            break;
        }
    }
}

void StockDiagram::resize( const QSizeF &size )
{
    d->compressor.setResolution( static_cast< int >( size.width() * coordinatePlane()->zoomFactorX() ),
                                 static_cast< int >( size.height() * coordinatePlane()->zoomFactorY() ) );
    setDataBoundariesDirty();
}

double StockDiagram::threeDItemDepth( int column ) const
{
    Q_UNUSED( column );
    //FIXME: Implement threeD functionality
    return 1.0;
}

double StockDiagram::threeDItemDepth( const QModelIndex &index ) const
{
    Q_UNUSED( index );
    //FIXME: Implement threeD functionality
    return 1.0;
}

const QPair<QPointF, QPointF> StockDiagram::calculateDataBoundaries() const
{
    const int rowCount = attributesModel()->rowCount( attributesModelRootIndex() );
    const int colCount = attributesModel()->columnCount( attributesModelRootIndex() );
    qreal xMin = 0.0;
    qreal xMax = rowCount;
    qreal yMin = 0.0;
    qreal yMax = 0.0;
    for ( int row = 0; row < rowCount; row++ ) {
        for ( int col = 0; col < colCount; col++ ) {
            const CartesianDiagramDataCompressor::CachePosition pos( row, col );
            const CartesianDiagramDataCompressor::DataPoint point = d->compressor.data( pos );
            yMax = qMax( yMax, point.value );
            yMin = qMin( yMin, point.value ); // FIXME: Can stock charts really have negative values?
        }
    }
    return QPair<QPointF, QPointF>( QPointF( xMin, yMin ), QPointF( xMax, yMax ) );
}

