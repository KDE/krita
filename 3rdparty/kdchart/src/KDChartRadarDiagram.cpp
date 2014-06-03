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

#include "KDChartRadarDiagram.h"
#include "KDChartRadarDiagram_p.h"

#include <QPainter>
#include "KDChartAttributesModel.h"
#include "KDChartPaintContext.h"
#include "KDChartPainterSaver_p.h"
#include "KDChartDataValueAttributes.h"

#include <KDABLibFakes>

using namespace KDChart;

RadarDiagram::Private::Private() :
    closeDatasets( false ),
    reverseData( false ),
    fillAlpha( 0.0 )
{
}

RadarDiagram::Private::~Private() {}

#define d d_func()

RadarDiagram::RadarDiagram( QWidget* parent, RadarCoordinatePlane* plane ) :
    AbstractPolarDiagram( new Private( ), parent, plane )
{
    //init();
}

RadarDiagram::~RadarDiagram()
{

}

void RadarDiagram::init()
{
}


/**
  * Creates an exact copy of this diagram.
  */
RadarDiagram * RadarDiagram::clone() const
{
    RadarDiagram* newDiagram = new RadarDiagram( new Private( *d ) );
    // This needs to be copied after the fact
    newDiagram->d->closeDatasets = d->closeDatasets;
    return newDiagram;
}

const QPair<QPointF, QPointF> RadarDiagram::calculateDataBoundaries () const
{
    if ( !checkInvariants(true) ) return QPair<QPointF, QPointF>( QPointF( 0, 0 ), QPointF( 0, 0 ) );
    const int rowCount = model()->rowCount(rootIndex());
    const int colCount = model()->columnCount(rootIndex());
    double xMin = 0.0;
    double xMax = colCount;
    double yMin = 0, yMax = 0;
    for ( int iCol=0; iCol<colCount; ++iCol ) {
        for ( int iRow=0; iRow< rowCount; ++iRow ) {
            double value = model()->data( model()->index( iRow, iCol, rootIndex() ) ).toDouble();
            yMax = qMax( yMax, value );
            yMin = qMin( yMin, value );
        }
    }
    QPointF bottomLeft ( QPointF( xMin, yMin ) );
    QPointF topRight ( QPointF( xMax, yMax ) );
    return QPair<QPointF, QPointF> ( bottomLeft,  topRight );
}



void RadarDiagram::paintEvent ( QPaintEvent*)
{
    QPainter painter ( viewport() );
    PaintContext ctx;
    ctx.setPainter ( &painter );
    ctx.setRectangle( QRectF ( 0, 0, width(), height() ) );
    paint ( &ctx );
}

void RadarDiagram::paint( PaintContext* ctx )
{
    qreal dummy1, dummy2;
    paint( ctx, true,  dummy1, dummy2 );
    paint( ctx, false, dummy1, dummy2 );
}

static qreal fitFontSizeToGeometry( const QString& text, const QFont& font, const QRectF& geometry, const TextAttributes& ta )
{
    QFont f = font;
    const qreal origResult = f.pointSizeF();
    qreal result = origResult;
    const QSizeF mySize = geometry.size();
    if( mySize.isNull() )
        return result;

    const QString t = text;
    QFontMetrics fm( f );
    while( true )
    {
        const QSizeF textSize = rotatedRect( fm.boundingRect( t ), ta.rotation() ).normalized().size();

        if( textSize.height() <= mySize.height() && textSize.width() <= mySize.width() )
            return result;

        result -= 0.5;
        if( result <= 0.0 )
            return origResult;
        f.setPointSizeF( result );
        fm = QFontMetrics( f );
    }
}

static QPointF scaleToRealPosition( const QPointF& origin, const QRectF& sourceRect, const QRectF& destRect, const AbstractCoordinatePlane& plane )
{
    QPointF result = plane.translate( origin );
    result -= sourceRect.topLeft();
    result.setX( result.x() / sourceRect.width() * destRect.width() );
    result.setY( result.y() / sourceRect.height() * destRect.height() );
    result += destRect.topLeft();
    return result;
}

void RadarDiagram::setReverseData( bool val )
{
    d->reverseData = val;
}
bool RadarDiagram::reverseData()
{
    return d->reverseData;
}

// local structure to remember the settings of a polygon inclusive the used color and pen.
struct Polygon {
    QPolygonF polygon;
    QBrush brush;
    QPen pen;
    Polygon(const QPolygonF &polygon, const QBrush &brush, const QPen &pen) : polygon(polygon), brush(brush), pen(pen) {}
};

void RadarDiagram::paint( PaintContext* ctx,
                          bool calculateListAndReturnScale,
                          qreal& newZoomX, qreal& newZoomY )
{
    // note: Not having any data model assigned is no bug
    //       but we can not draw a diagram then either.
    if ( !checkInvariants(true) )
        return;
    d->reverseMapper.clear();

    const int rowCount = model()->rowCount( rootIndex() );
    const int colCount = model()->columnCount( rootIndex() );

    int iRow, iCol;

    const double min = dataBoundaries().first.y();
    const double r = qAbs( min ) + dataBoundaries().second.y();
    const double step = ( r - qAbs( min ) ) / ( numberOfGridRings() );
    
    RadarCoordinatePlane* plane = dynamic_cast<RadarCoordinatePlane*>(ctx->coordinatePlane());
    TextAttributes ta = plane->textAttributes();
    QRectF fontRect = ctx->rectangle();    
    fontRect.setSize( QSizeF( fontRect.width(), step / 2.0 ) );
    const qreal labelFontSize = fitFontSizeToGeometry( "TestXYWQgqy", ta.font(), fontRect, ta );
    QFont labelFont = ta.font();
    ctx->painter()->setPen( ta.pen() );
    labelFont.setPointSizeF( labelFontSize );
    const QFontMetricsF metric( labelFont );
    const qreal labelHeight = metric.height();
    QPointF offset;
    QRectF destRect = ctx->rectangle();    
    if ( ta.isVisible() )
    {        
        destRect.setY( destRect.y() + 2 * labelHeight );
        destRect.setHeight( destRect.height() - 4 * labelHeight );
    }
    
    if( calculateListAndReturnScale ){
        ctx->painter()->save();
        // Check if all of the data value texts / data comments will fit
        // into the available space:
        d->dataValueInfoList.clear();
        ctx->painter()->save();
        for ( iCol=0; iCol < colCount; ++iCol ) {
            for ( iRow=0; iRow < rowCount; ++iRow ) {
                QModelIndex index = model()->index( iRow, iCol, rootIndex() );
                const double value = model()->data( index ).toDouble();
                QPointF point = scaleToRealPosition( QPointF( value, iRow ), ctx->rectangle(), destRect, *ctx->coordinatePlane() );
                d->appendDataValueTextInfoToList(
                        this, d->dataValueInfoList, index, 0,
                        PositionPoints( point ), Position::Center, Position::Center,
                        value );
            }
        }
        ctx->painter()->restore();
        const qreal oldZoomX = coordinatePlane()->zoomFactorX();
        const qreal oldZoomY = coordinatePlane()->zoomFactorY();
        newZoomX = oldZoomX;
        newZoomY = oldZoomY;
        if( d->dataValueInfoList.count() ){
            QRectF txtRectF;
            d->paintDataValueTextsAndMarkers( this, ctx, d->dataValueInfoList, true, true, &txtRectF );
            const QRect txtRect = txtRectF.toRect();
            const QRect curRect = coordinatePlane()->geometry();
            const qreal gapX = qMin( txtRect.left() - curRect.left(), curRect.right()  - txtRect.right() );
            const qreal gapY = qMin( txtRect.top()  - curRect.top(),  curRect.bottom() - txtRect.bottom() );
            newZoomX = oldZoomX;
            newZoomY = oldZoomY;
            if( gapX < 0.0 )
                newZoomX *= 1.0 + (gapX-1.0) / curRect.width();
            if( gapY < 0.0 )
                newZoomY *= 1.0 + (gapY-1.0) / curRect.height();
        }
        ctx->painter()->restore();

    }else{
        // Iterate through data sets and create a list of polygons out of them.
        QList<Polygon> polygons;
        for ( iCol=0; iCol < colCount; ++iCol ) {
            //TODO(khz): As of yet RadarDiagram can not show per-segment line attributes
            //           but it draws every polyline in one go - using one color.
            //           This needs to be enhanced to allow for cell-specific settings
            //           in the same way as LineDiagram does it.
            QPolygonF polygon;
            QPointF point0;
            for ( iRow=0; iRow < rowCount; ++iRow ) {
                QModelIndex index = model()->index( iRow, iCol, rootIndex() );
                const double value = model()->data( index ).toDouble();
                QPointF point = scaleToRealPosition( QPointF( value, d->reverseData ? ( rowCount - iRow ) : iRow ), ctx->rectangle(), destRect, *ctx->coordinatePlane() );
                polygon.append( point );
                if( ! iRow )
                    point0= point;
            }
            if( closeDatasets() && rowCount )
                polygon.append( point0 );

            QBrush brush = qVariantValue<QBrush>( d->datasetAttrs( iCol, KDChart::DatasetBrushRole ) );
            QPen p( model()->headerData( iCol, Qt::Horizontal, KDChart::DatasetPenRole ).value< QPen >() );
            if ( p.style() != Qt::NoPen )
            {
                polygons.append( Polygon(polygon, brush, PrintingParameters::scalePen( p )) );
            }
        }

        // first fill the areas with the brush-color and the defined alpha-value.
        if (d->fillAlpha > 0.0) {
            foreach(const Polygon& p, polygons) {
                PainterSaver painterSaver( ctx->painter() );
                ctx->painter()->setRenderHint ( QPainter::Antialiasing );
                QBrush br = p.brush;
                QColor c = br.color();
                c.setAlphaF(d->fillAlpha);
                br.setColor(c);
                ctx->painter()->setBrush( br );
                ctx->painter()->setPen( p.pen );
                ctx->painter()->drawPolygon( p.polygon );
            }
        }

        // then draw the poly-lines.
        foreach(const Polygon& p, polygons) {
            PainterSaver painterSaver( ctx->painter() );
            ctx->painter()->setRenderHint ( QPainter::Antialiasing );
            ctx->painter()->setBrush( p.brush );
            ctx->painter()->setPen( p.pen );
            ctx->painter()->drawPolyline( p.polygon );
        }

        d->paintDataValueTextsAndMarkers( this, ctx, d->dataValueInfoList, true );
    }
}

void RadarDiagram::resize ( const QSizeF& size )
{
    d->diagramSize = size;
}

/*virtual*/
double RadarDiagram::valueTotals () const
{
    return model()->rowCount(rootIndex());
}

/*virtual*/
double RadarDiagram::numberOfValuesPerDataset() const
{
    return model() ? model()->rowCount(rootIndex()) : 0.0;
}

/*virtual*/
double RadarDiagram::numberOfGridRings() const
{
    return 5; // FIXME
}

void RadarDiagram::setCloseDatasets( bool closeDatasets )
{
    d->closeDatasets = closeDatasets;
}

bool RadarDiagram::closeDatasets() const
{
    return d->closeDatasets;
}

qreal RadarDiagram::fillAlpha() const
{
    return d->fillAlpha;
}

void RadarDiagram::setFillAlpha(qreal alphaF)
{
    d->fillAlpha = alphaF;
}

void RadarDiagram::resizeEvent ( QResizeEvent*)
{
}


