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

#include "KDChartTernaryLineDiagram.h"
#include "KDChartTernaryLineDiagram_p.h"

#include <limits>

#include <QPainter>

#include <KDChartPaintContext>

#include "KDChartLineAttributes.h"
#include "KDChartDataValueAttributes.h"
#include "KDChartMarkerAttributes.h"
#include "TernaryPoint.h"
#include "TernaryConstants.h"
#include "KDChartPainterSaver_p.h"

using namespace KDChart;

#define d d_func()

TernaryLineDiagram::Private::Private()
    : AbstractTernaryDiagram::Private()
{
}

TernaryLineDiagram::TernaryLineDiagram ( QWidget* parent,
                                         TernaryCoordinatePlane* plane )
    : AbstractTernaryDiagram( new Private(), parent, plane )
{
    init();
    setDatasetDimensionInternal( 3 ); // the third column is implicit

    DataValueAttributes dataValueAttributes;
    dataValueAttributes.setVisible( true );
    MarkerAttributes markerAttributes;
    markerAttributes.setMarkerStyle( MarkerAttributes::MarkerCircle );
    markerAttributes.setVisible( true );
    dataValueAttributes.setMarkerAttributes( markerAttributes );
    attributesModel()->setDefaultForRole(
        KDChart::DataValueLabelAttributesRole,
        qVariantFromValue( dataValueAttributes ) );
}

TernaryLineDiagram::~TernaryLineDiagram()
{
}

void TernaryLineDiagram::init()
{
}

void  TernaryLineDiagram::resize (const QSizeF& area)
{
    Q_UNUSED( area );
}

void  TernaryLineDiagram::paint (PaintContext *paintContext)
{
    d->reverseMapper.clear();

    d->paint( paintContext );
    // sanity checks:
    if ( model() == 0 ) return;

    QPainter* p = paintContext->painter();
    PainterSaver s( p );

    TernaryCoordinatePlane* plane =
        (TernaryCoordinatePlane*) paintContext->coordinatePlane();
    Q_ASSERT( plane );

    double x, y, z;


    // for some reason(?) TernaryPointDiagram is using per-diagram DVAs only:
    const DataValueAttributes attrs( dataValueAttributes() );


    d->clearListOfAlreadyDrawnDataValueTexts();

    int columnCount = model()->columnCount( rootIndex() );
    QPointF start;
    for(int column=0; column<columnCount; column+=datasetDimension() )
    {
        int numrows = model()->rowCount( rootIndex() );
        for( int row = 0; row < numrows; row++ )
        {
            // see if there is data otherwise skip
            QModelIndex base = model()->index( row, column );
            if( ! model()->data( base ).isNull() )
            {
                p->setPen( PrintingParameters::scalePen( pen( base ) ) );
                p->setBrush( brush( base ) );

                // retrieve data
                x = qMax( model()->data( model()->index( row, column, rootIndex() ) ).toDouble(),
                          0.0 );
                y = qMax( model()->data( model()->index( row, column+1, rootIndex() ) ).toDouble(),
                          0.0 );
                z = qMax( model()->data( model()->index( row, column+2, rootIndex() ) ).toDouble(),
                          0.0 );

                double total = x + y + z;
                if ( fabs( total ) > 3 * std::numeric_limits<double>::epsilon() ) {
                    TernaryPoint tPunkt( x / total, y / total );
                    QPointF diagramLocation = translate( tPunkt );
                    QPointF widgetLocation = plane->translate( diagramLocation );

                    if ( row > 0 ) {
                        p->drawLine( start, widgetLocation );
                    }
                    paintMarker( p, model()->index( row, column, rootIndex() ), widgetLocation );
                    start = widgetLocation;
                    // retrieve text and data value attributes
                    // FIXME use data model DisplayRole text
                    QString text = tr( "(%1, %2, %3)" )
                                   .arg( x * 100, 0, 'f', 0 )
                                   .arg( y * 100, 0, 'f', 0 )
                                   .arg( z * 100, 0, 'f', 0 );
                    d->paintDataValueText( this, p, attrs, widgetLocation, text, true );
                } else {
                    // ignore and do not paint this point, garbage data
                    qDebug() << "TernaryPointDiagram::paint: data point x/y/z:"
                             << x << "/" << y << "/" << z << "ignored, unusable.";
                }
            }
        }
    }
}

const QPair< QPointF, QPointF >  TernaryLineDiagram::calculateDataBoundaries () const
{
    // this is a constant, because we defined it to be one:
    static QPair<QPointF, QPointF> Boundaries(
        TriangleBottomLeft,
        QPointF( TriangleBottomRight.x(), TriangleHeight ) );
    return Boundaries;
}
