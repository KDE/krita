/* This file is part of the KDE project
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

// Own
#include "ChartShapeFactory.h"

// Qt
#include <QStringList>
#include <QStandardItemModel>

// KDE
#include <kiconloader.h>
#include <kgenericfactory.h>
#include <klocale.h>

// KOffice
#include <KoProperties.h>
#include <KoToolRegistry.h>
#include <KoShapeRegistry.h>

// Chart shape
#include "ChartToolFactory.h"
#include "ChartConfigWidget.h"
#include "ChartProxyModel.h"
#include "PlotArea.h"
#include "Axis.h"
#include "Legend.h"


using namespace KChart;

K_EXPORT_COMPONENT_FACTORY( chartshape, KGenericFactory<ChartShapePlugin>( "ChartShape" ) )

ChartShapePlugin::ChartShapePlugin( QObject * parent,  const QStringList& )
{
    // Register the chart shape factory.
    KoShapeRegistry::instance()->add( new ChartShapeFactory( parent ) );

    // Register all tools for the chart shape.
    KoToolRegistry::instance()->add( new ChartToolFactory( parent ) );
}


ChartShapeFactory::ChartShapeFactory( QObject* parent )
    : KoShapeFactoryBase( parent, ChartShapeId, i18n( "Chart" ) )
{
    setOdfElementNames( "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0", QStringList( "object" ) );
    setToolTip( i18n( "Business charts" ) );

    KIconLoader::global()->addAppDir("kchart");
    setIcon( "kchart" );

    // Default 'app specific' config pages i.e. unless an app defines
    // other config pages, these are used.
    QList<KoShapeConfigFactoryBase*> panelFactories;
    // panelFactories.append( new ChartDataConfigFactory() );
    setOptionPanels( panelFactories );
}


bool ChartShapeFactory::supports( const KoXmlElement &element ) const
{
    return element.namespaceURI() == "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0"
        && element.tagName() == "object";
}

KoShape *ChartShapeFactory::createDefaultShape(KoResourceManager *documentResources) const
{
    ChartShape* shape = new ChartShape(documentResources);

    // Fill cells with data.
    QStandardItemModel  *m_chartData = new QStandardItemModel();
    m_chartData->setRowCount( 4 );
    m_chartData->setColumnCount( 5 );

    // Vertical header data
    m_chartData->setData( m_chartData->index( 1, 0 ), i18n( "January" ) );
    m_chartData->setData( m_chartData->index( 2, 0 ), i18n( "July" ) );
    m_chartData->setData( m_chartData->index( 3, 0 ), i18n( "December" ) );

    // Horizontal header data
    m_chartData->setData( m_chartData->index( 0, 1 ), i18n( "Column %1", 1 ) );
    m_chartData->setData( m_chartData->index( 0, 2 ), i18n( "Column %1", 2 ) );
    m_chartData->setData( m_chartData->index( 0, 3 ), i18n( "Column %1", 3 ) );
    m_chartData->setData( m_chartData->index( 0, 4 ), i18n( "Column %1", 4 ) );

    // First row
    m_chartData->setData( m_chartData->index( 1, 1 ), 5.7 );
    m_chartData->setData( m_chartData->index( 1, 2 ), 3.4 );
    m_chartData->setData( m_chartData->index( 1, 3 ), 1.2 );
    m_chartData->setData( m_chartData->index( 1, 4 ), 8.4 );

    // Second row
    m_chartData->setData( m_chartData->index( 2, 1 ), 2.1 );
    m_chartData->setData( m_chartData->index( 2, 2 ), 6.5 );
    m_chartData->setData( m_chartData->index( 2, 3 ), 0.9 );
    m_chartData->setData( m_chartData->index( 2, 4 ), 1.5 );

    // Third row
    m_chartData->setData( m_chartData->index( 3, 1 ), 7.9 );
    m_chartData->setData( m_chartData->index( 3, 2 ), 3.5 );
    m_chartData->setData( m_chartData->index( 3, 3 ), 8.6 );
    m_chartData->setData( m_chartData->index( 3, 4 ), 4.3 );

    // We want the chart shape to take over and handle this model itself
    shape->setModel( m_chartData, true );
    shape->setFirstRowIsLabel( true );
    shape->setFirstColumnIsLabel( true );

    const QSizeF shapeSize = shape->size();

    QPointF  plotAreaPos( 0.0, 0.0 );
    QSizeF   plotAreaSize( shapeSize );
    Legend *legend = shape->legend();
    legend->rebuild();          // Implies update()

    QPointF  legendPos( 0.0, 0.0 );
    QSizeF   legendSize = legend->size();
    legendPos.ry() = shapeSize.height() / 2.0 - legendSize.height() / 2.0;
    plotAreaSize.rwidth() -= legendSize.width();

    Axis    *xAxis      = shape->plotArea()->xAxis();
    KoShape *xAxisTitle = xAxis->title();
    if ( xAxis ) {
        xAxis->setTitleText( i18n( "Month" ) );
        xAxisTitle->setPosition( QPointF( shapeSize.width() / 2.0 - xAxisTitle->size().width() / 2.0,
                                          shapeSize.height() - xAxisTitle->size().height() ) );
        plotAreaSize.rheight() -= xAxisTitle->size().height();
    }

    Axis    *yAxis      = shape->plotArea()->yAxis();
    KoShape *yAxisTitle = yAxis->title();
    if ( yAxis ) {
        yAxis->setTitleText( i18n( "Growth in %") );
        yAxisTitle->setPosition( QPointF( -yAxisTitle->size().width() / 2.0 + yAxisTitle->size().height() / 2.0,
                                          shapeSize.height() / 2.0 - yAxisTitle->size().height() / 2.0 ) );

        plotAreaPos.rx() += yAxisTitle->size().height();
        legendPos.rx() += yAxisTitle->size().height();
        plotAreaSize.rwidth() -= yAxisTitle->size().height();
    }

    if ( legend ) {
        legendPos.rx() += plotAreaSize.width();
        legend->setPosition( legendPos );
    }

    shape->plotArea()->setPosition( plotAreaPos );
    shape->plotArea()->setSize( plotAreaSize );

    return shape;
}

QList<KoShapeConfigWidgetBase*> ChartShapeFactory::createShapeOptionPanels()
{
    return QList<KoShapeConfigWidgetBase*>();
}

void ChartShapeFactory::newDocumentResourceManager(KoResourceManager *manager)
{
    Q_UNUSED(manager);
}

#include "ChartShapeFactory.moc"
