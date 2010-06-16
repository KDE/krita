/****************************************************************************
 ** Copyright (C) 2007 Klar�vdalens Datakonsult AB.  All rights reserved.
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

#include "KDChartPolarDiagram.h"
#include "KDChartPolarDiagram_p.h"

#include <QPainter>
#include "KDChartAttributesModel.h"
#include "KDChartPaintContext.h"
#include "KDChartPainterSaver_p.h"
#include "KDChartDataValueAttributes.h"

#include <KDABLibFakes>

using namespace KDChart;

PolarDiagram::Private::Private() :
    rotateCircularLabels( false ),
    closeDatasets( false )
{
}

PolarDiagram::Private::~Private() {}

#define d d_func()

PolarDiagram::PolarDiagram( QWidget* parent, PolarCoordinatePlane* plane ) :
    AbstractPolarDiagram( new Private( ), parent, plane )
{
    //init();
}

PolarDiagram::~PolarDiagram()
{
}


void PolarDiagram::init()
{
    setShowDelimitersAtPosition( Position::Unknown, false );
    setShowDelimitersAtPosition( Position::Center, false );
    setShowDelimitersAtPosition( Position::NorthWest, false );
    setShowDelimitersAtPosition( Position::North, true );
    setShowDelimitersAtPosition( Position::NorthEast, false );
    setShowDelimitersAtPosition( Position::West, false );
    setShowDelimitersAtPosition( Position::East, false );
    setShowDelimitersAtPosition( Position::SouthWest, false );
    setShowDelimitersAtPosition( Position::South, true );
    setShowDelimitersAtPosition( Position::SouthEast, false );
    setShowDelimitersAtPosition( Position::Floating, false );

    setShowLabelsAtPosition( Position::Unknown, false );
    setShowLabelsAtPosition( Position::Center, false );
    setShowLabelsAtPosition( Position::NorthWest, false );
    setShowLabelsAtPosition( Position::North, true );
    setShowLabelsAtPosition( Position::NorthEast, false );
    setShowLabelsAtPosition( Position::West, false );
    setShowLabelsAtPosition( Position::East, false );
    setShowLabelsAtPosition( Position::SouthWest, false );
    setShowLabelsAtPosition( Position::South, true );
    setShowLabelsAtPosition( Position::SouthEast, false );
    setShowLabelsAtPosition( Position::Floating, false );
}

/**
  * Creates an exact copy of this diagram.
  */
PolarDiagram * PolarDiagram::clone() const
{
    PolarDiagram* newDiagram = new PolarDiagram( new Private( *d ) );
    // This needs to be copied after the fact
    newDiagram->d->showDelimitersAtPosition = d->showDelimitersAtPosition;
    newDiagram->d->showLabelsAtPosition = d->showLabelsAtPosition;
    newDiagram->d->rotateCircularLabels = d->rotateCircularLabels;
    newDiagram->d->closeDatasets = d->closeDatasets;
    return newDiagram;
}

const QPair<QPointF, QPointF> PolarDiagram::calculateDataBoundaries () const
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



void PolarDiagram::paintEvent ( QPaintEvent*)
{
    QPainter painter ( viewport() );
    PaintContext ctx;
    ctx.setPainter ( &painter );
    ctx.setRectangle( QRectF ( 0, 0, width(), height() ) );
    paint ( &ctx );
}

void PolarDiagram::resizeEvent ( QResizeEvent*)
{
}

void PolarDiagram::paintPolarMarkers( PaintContext* ctx, const QPolygonF& polygon )
{
    Q_UNUSED(ctx);
    Q_UNUSED(polygon);
    // obsolete, since we are using real markers now!
}

void PolarDiagram::paint( PaintContext* ctx )
{
    qreal dummy1, dummy2;
    paint( ctx, true,  dummy1, dummy2 );
    paint( ctx, false, dummy1, dummy2 );
}

void PolarDiagram::paint( PaintContext* ctx,
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

    if( calculateListAndReturnScale ){

        // Check if all of the data value texts / data comments will fit
        // into the available space:
        d->dataValueInfoList.clear();
        for ( iCol=0; iCol < colCount; ++iCol ) {
            for ( iRow=0; iRow < rowCount; ++iRow ) {
                QModelIndex index = model()->index( iRow, iCol, rootIndex() );
                const double value = model()->data( index ).toDouble();
                QPointF point = coordinatePlane()->translate(
                        QPointF( value, iRow ) ) + ctx->rectangle().topLeft();
                //qDebug() << point;
                d->appendDataValueTextInfoToList(
                        this, d->dataValueInfoList, index, 0,
                        PositionPoints( point ), Position::Center, Position::Center,
                        value );
            }
        }
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

    }else{

        for ( iCol=0; iCol < colCount; ++iCol ) {
            //TODO(khz): As of yet PolarDiagram can not show per-segment line attributes
            //           but it draws every polyline in one go - using one color.
            //           This needs to be enhanced to allow for cell-specific settings
            //           in the same way as LineDiagram does it.
            QBrush brush = qVariantValue<QBrush>( attributesModel()->headerData( iCol, Qt::Vertical, KDChart::DatasetBrushRole ) );
            QPolygonF polygon;
            QPointF point0;
            for ( iRow=0; iRow < rowCount; ++iRow ) {
                QModelIndex index = model()->index( iRow, iCol, rootIndex() );
                const double value = model()->data( index ).toDouble();
                QPointF point = coordinatePlane()->translate(
                        QPointF( value, iRow ) ) + ctx->rectangle().topLeft();
                polygon.append( point );
                //qDebug() << point;
                if( ! iRow )
                    point0= point;
            }
            if( closeDatasets() && rowCount )
                polygon.append( point0 );

            PainterSaver painterSaver( ctx->painter() );
            ctx->painter()->setRenderHint ( QPainter::Antialiasing );
            ctx->painter()->setBrush( brush );
            QPen p( ctx->painter()->pen() );
            p.setColor( brush.color() ); // FIXME use DatasetPenRole
            p.setWidth( 2 );// FIXME properties
            ctx->painter()->setPen( PrintingParameters::scalePen( p ) );
            ctx->painter()->drawPolyline( polygon );
        }
        d->paintDataValueTextsAndMarkers( this, ctx, d->dataValueInfoList, true );
    }
}

void PolarDiagram::resize ( const QSizeF& )
{
}

/*virtual*/
double PolarDiagram::valueTotals () const
{
    return model()->rowCount(rootIndex());
}

/*virtual*/
double PolarDiagram::numberOfValuesPerDataset() const
{
    return model() ? model()->rowCount(rootIndex()) : 0.0;
}

/*virtual*/
double PolarDiagram::numberOfGridRings() const
{
    return 5; // FIXME
}

void PolarDiagram::setZeroDegreePosition( int degrees )
{
    Q_UNUSED( degrees );
    qWarning() << "Deprecated PolarDiagram::setZeroDegreePosition() called, setting ignored.";
}

int PolarDiagram::zeroDegreePosition() const
{
    qWarning() << "Deprecated PolarDiagram::zeroDegreePosition() called.";
    return 0;
}

void PolarDiagram::setRotateCircularLabels( bool rotateCircularLabels )
{
    d->rotateCircularLabels = rotateCircularLabels;
}

bool PolarDiagram::rotateCircularLabels() const
{
    return d->rotateCircularLabels;
}

void PolarDiagram::setCloseDatasets( bool closeDatasets )
{
    d->closeDatasets = closeDatasets;
}

bool PolarDiagram::closeDatasets() const
{
    return d->closeDatasets;
}

void PolarDiagram::setShowDelimitersAtPosition( Position position,
                                                       bool showDelimiters )
{
    d->showDelimitersAtPosition[position.value()] = showDelimiters;
}

void PolarDiagram::setShowLabelsAtPosition( Position position,
                                                   bool showLabels )
{
    d->showLabelsAtPosition[position.value()] = showLabels;
}

bool PolarDiagram::showDelimitersAtPosition( Position position ) const
{
    return d->showDelimitersAtPosition[position.value()];
}

bool PolarDiagram::showLabelsAtPosition( Position position ) const
{
    return d->showLabelsAtPosition[position.value()];
}



