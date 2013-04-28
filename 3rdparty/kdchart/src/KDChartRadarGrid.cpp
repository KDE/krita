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

#include "KDChartRadarGrid.h"
#include "KDChartPaintContext.h"
#include "KDChartRadarDiagram.h"
#include "KDChartPieDiagram.h"
#include "KDChartPrintingParameters.h"

#include <QPainter>

#include <KDABLibFakes>

using namespace KDChart;


DataDimensionsList RadarGrid::calculateGrid(
    const DataDimensionsList& rawDataDimensions ) const
{
    qDebug("Calling PolarGrid::calculateGrid()");
    DataDimensionsList l;

    //FIXME(khz): do the real calculation

    l = rawDataDimensions;

    return l;
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

QPointF scaleToRealPosition( const QPointF& origin, const QRectF& sourceRect, const QRectF& destRect, const AbstractCoordinatePlane& plane )
{
    QPointF result = plane.translate( origin );
    result -= sourceRect.topLeft();
    result.setX( result.x() / sourceRect.width() * destRect.width() );
    result.setY( result.y() / sourceRect.height() * destRect.height() );
    result += destRect.topLeft();
    return result;
}

QPointF scaleToRect( const QPointF& origin, const QRectF& sourceRect, const QRectF& destRect )
{
    QPointF result( origin );
    result -= sourceRect.topLeft();
    result.setX( result.x() / sourceRect.width() * destRect.width() );
    result.setY( result.y() / sourceRect.height() * destRect.height() );
    result += destRect.topLeft();
    return result;
}

void RadarGrid::drawGrid( PaintContext* context )
{
    const QBrush backupBrush( context->painter()->brush() );
    context->painter()->setBrush( QBrush() );
    RadarCoordinatePlane* plane = dynamic_cast< RadarCoordinatePlane* >( context->coordinatePlane() );
    Q_ASSERT( plane );
    Q_ASSERT( plane->diagram() );
    QPair< QPointF, QPointF >  boundaries = plane->diagram()->dataBoundaries();
    Q_ASSERT_X ( plane, "PolarGrid::drawGrid",
                 "Bad function call: PaintContext::coodinatePlane() NOT a polar plane." );

    const GridAttributes gridAttrsCircular( plane->gridAttributes( true ) );
    const GridAttributes gridAttrsSagittal( plane->gridAttributes( false ) );

    //qDebug() << "OK:";
    if ( !gridAttrsCircular.isGridVisible() && !gridAttrsSagittal.isGridVisible() ) return;
    //qDebug() << "A";

    // FIXME: we paint the rulers to the settings of the first diagram for now:
    AbstractPolarDiagram* dgr = dynamic_cast<AbstractPolarDiagram*> (plane->diagrams().first() );
    Q_ASSERT ( dgr ); // only polar diagrams are allowed here


    // Do not draw a grid for pie diagrams
    if( dynamic_cast<PieDiagram*> (plane->diagrams().first() ) ) return;


    context->painter()->setPen ( PrintingParameters::scalePen( QColor ( Qt::lightGray ) ) );
    const double min = dgr->dataBoundaries().first.y();
    QPointF origin = plane->translate( QPointF( min, 0 ) ) + context->rectangle().topLeft();
    //qDebug() << "origin" << origin;

    const double r = qAbs( min ) + dgr->dataBoundaries().second.y(); // use the full extents
    
    // distance between two axis lines
    const double step = ( r - qAbs( min ) ) / ( dgr->numberOfGridRings() );
    
    // calculate the height needed for text to be displayed at the bottom and top of the chart
    QPointF topLeft = context->rectangle().topLeft();
    Q_ASSERT( plane->diagram()->model() );
    TextAttributes ta = plane->textAttributes();
    const int numberOfSpokes = ( int ) ( 360 / plane->angleUnit() );
    const qreal stepWidth = boundaries.second.y() / ( dgr->numberOfGridRings()  );
    QRectF destRect = context->rectangle();
    if (ta.isVisible() )
    {
        QAbstractItemModel* model = plane->diagram()->model();
        QRectF fontRect = context->rectangle();    
        fontRect.setSize( QSizeF( fontRect.width(), step / 2.0 ) );
        const qreal labelFontSize = fitFontSizeToGeometry( "TestXYWQgqy", ta.font(), fontRect, ta );
        QFont labelFont = ta.font();
        context->painter()->setPen( ta.pen() );
        labelFont.setPointSizeF( labelFontSize );
        const QFontMetricsF metric( labelFont );
        const qreal labelHeight = metric.height();
        QPointF offset;
        destRect.setY( destRect.y() + 2 * labelHeight );
        destRect.setHeight( destRect.height() - 4 * labelHeight );
        offset.setY( labelHeight );
        offset.setX( 0 );
        topLeft += offset;
        origin += offset;
        origin = scaleToRealPosition( QPointF( min, 0 ), context->rectangle(), destRect, *plane );
        
        const qreal aWidth = metric.width( "A" );
        const QLineF startLine( origin, scaleToRealPosition( QPointF( r - qAbs( min ), 0 ), context->rectangle(), destRect, *plane ) );
        for ( int i = 0; i < model->rowCount(); ++i )
        {
            const QLineF currentLine( origin, scaleToRealPosition( QPointF( r - qAbs( min ), i ), context->rectangle(), destRect, *plane ) );
            const int angle = ( int ) startLine.angleTo( currentLine ) % 360;
            const qreal angleTest = qAbs( angle - 180 );
            const QString data = model->headerData( i, Qt::Vertical ).toString();
            const qreal xOffset = metric.width( data ) / 2.0;
            if ( angleTest < 5.0 )
                context->painter()->drawText( currentLine.pointAt( 1 ) + QPointF( -xOffset, labelHeight + qAbs( min ) ) , data );
            else if ( qAbs( angleTest - 180 ) < 5.0 )
                context->painter()->drawText( currentLine.pointAt( 1 ) - QPointF( xOffset, labelHeight + qAbs( min ) ) , data );
            else if ( angle < 175 && angle > 5 )
                context->painter()->drawText( currentLine.pointAt( 1 ) - QPointF( xOffset * 2 + qAbs( min ) + aWidth, -labelHeight/ 2.0 + qAbs( min ) ) , data );
            else if ( angle < 355 && angle > 185 )
                context->painter()->drawText( currentLine.pointAt( 1 ) + QPointF( qAbs( min ) + aWidth, labelHeight/ 2.0 + qAbs( min ) ) , data );
            
        }
    }
    context->painter()->setPen ( PrintingParameters::scalePen( QColor ( Qt::lightGray ) ) );
    if ( plane->globalGridAttributes().isGridVisible() )
    {
        for ( int j = 1; j < dgr->numberOfGridRings() + 1; ++j )
        {
            QPointF oldPoint( scaleToRealPosition( QPointF( j * step - qAbs( min ), numberOfSpokes - 1 ), context->rectangle(), destRect, *plane ) );
            for ( int i = 0; i < numberOfSpokes ; ++i ) {                
                const QPointF newPoint = scaleToRealPosition( QPointF( j * step - qAbs( min ), i ), context->rectangle(), destRect, *plane );
                context->painter()->drawLine( oldPoint, newPoint );
                oldPoint = newPoint;
                
                context->painter()->drawLine( origin, newPoint );
            }
        }
        context->painter()->setPen( ta.pen() );
        qreal fontSize = 0;
        for ( int i = 0; i < dgr->numberOfGridRings() + 1; ++i )
        {            
            const QString text = QString::number( i * stepWidth );
            const QPointF translatedPoint = scaleToRealPosition( QPointF( i * step - qAbs( min ), 0 ), context->rectangle(), destRect, *plane );
            const QFontMetrics metric( ta.font()/*QFont( "Arial", 10 )*/ );
            const double textLength = metric.width( text );
            const double textHeight = metric.height() / 2.0;
            QPointF textOffset( textLength, -textHeight );
            textOffset = scaleToRect( textOffset, context->rectangle(), destRect );
            QPointF _topLeft = topLeft;
            _topLeft.setY( translatedPoint.y() );
            QRectF boundary( _topLeft, ( translatedPoint + QPointF( 0, step / 2.0 ) ) );
            const qreal calcFontSize = fitFontSizeToGeometry( text, ta.font(), boundary, ta );
            if ( fontSize != calcFontSize )
            {
                QFont paintFont( ta.font() );
                paintFont.setPointSizeF( calcFontSize );
                ta.setFont( paintFont );
                ta.setFontSize( calcFontSize );
                const double textHeight2 = QFontMetricsF( paintFont ).height() / 2.0;
                textOffset.setY( - textHeight2 );
                textOffset = scaleToRect( textOffset, context->rectangle(), destRect );
                context->painter()->setFont( paintFont );
                fontSize = calcFontSize;
            }
            context->painter()->drawText( translatedPoint + destRect.topLeft() - textOffset, text );
            
        }
    }
    plane->setTextAttributes( ta );
    context->painter()->setPen ( PrintingParameters::scalePen( QColor ( Qt::lightGray ) ) );
    context->painter()->setBrush( backupBrush );
}