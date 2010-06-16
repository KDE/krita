/* This file is part of the KDE project

   Copyright 2007 Johannes Simon <johannes.simon@gmail.com>
   Copyright 2009 Inge Wallin    <inge@lysator.liu.se>

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
#include "Axis.h"

// Qt
#include <QList>
#include <QString>
#include <QTextDocument>

// KOffice
#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoShapeRegistry.h>
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoGenStyles.h>
#include <KoXmlNS.h>
#include <KoTextShapeData.h>
#include <KoOdfStylesReader.h>
#include <KoUnit.h>
#include <KoStyleStack.h>
#include <KoOdfLoadingContext.h>
#include <KoCharacterStyle.h>
#include <KoOdfGraphicStyles.h>
#include <KoOdfWorkaround.h>

// KDChart
#include <KDChartChart>
#include <KDChartLegend>
#include <KDChartCartesianAxis>
#include <KDChartCartesianCoordinatePlane>
#include <KDChartGridAttributes>
#include <KDChartBarDiagram>
#include <KDChartLineDiagram>
#include <KDChartPieDiagram>
#include <KDChartPlotter>
#include <KDChartStockDiagram>
#include <KDChartRingDiagram>
#include <KDChartPolarDiagram>
#include <KDChartBarAttributes>
#include <KDChartPieAttributes>
#include <KDChartThreeDBarAttributes>
#include <KDChartThreeDPieAttributes>
#include <KDChartThreeDLineAttributes>
#include <KDChartDataValueAttributes>

// KChart
#include "PlotArea.h"
#include "KDChartModel.h"
#include "DataSet.h"
#include "Legend.h"
#include "KDChartConvertions.h"
#include "ChartProxyModel.h"
#include "TextLabelDummy.h"
#include "Layout.h"


using namespace KChart;

class Axis::Private
{
public:
    Private( Axis *axis );
    ~Private();

    void adjustAllDiagrams();

    void registerDiagram( KDChart::AbstractDiagram *diagram );
    void deregisterDiagram( KDChart::AbstractDiagram *diagram );

    KDChart::AbstractDiagram *createDiagramIfNeeded( ChartType chartType );
    KDChart::AbstractDiagram *getDiagram( ChartType chartType );
    void deleteDiagram( ChartType chartType );

    void createBarDiagram();
    void createLineDiagram();
    void createAreaDiagram();
    void createCircleDiagram();
    void createRingDiagram();
    void createRadarDiagram();
    void createScatterDiagram();
    void createStockDiagram();
    void createBubbleDiagram();
    void createSurfaceDiagram();
    void createGanttDiagram();

    // Pointer to Axis that owns this Private instance
    Axis * const q;

    PlotArea *plotArea;

    AxisPosition  position;
    AxisDimension dimension;

    KoShape *title;
    TextLabelData *titleData;

    QString id;
    QList<DataSet*> dataSets;
    qreal majorInterval;
    int minorIntervalDivisor;
    bool showInnerMinorTicks;
    bool showOuterMinorTicks;
    bool showInnerMajorTicks;
    bool showOuterMajorTicks;
    bool logarithmicScaling;
    bool showMajorGrid;
    bool showMinorGrid;
    bool useAutomaticMajorInterval;
    bool useAutomaticMinorInterval;

    QFont font;

    KDChart::CartesianAxis            *kdAxis;
    KDChart::CartesianCoordinatePlane *kdPlane;
    KDChart::PolarCoordinatePlane     *kdPolarPlane;

    KDChart::BarDiagram   *kdBarDiagram;
    KDChart::LineDiagram  *kdLineDiagram;
    KDChart::LineDiagram  *kdAreaDiagram;
    KDChart::PieDiagram   *kdCircleDiagram;
    KDChart::RingDiagram  *kdRingDiagram;
    KDChart::PolarDiagram *kdRadarDiagram;
    KDChart::Plotter      *kdScatterDiagram;
    KDChart::StockDiagram *kdStockDiagram;
    KDChart::Plotter      *kdBubbleDiagram;
    // FIXME BUG: Somehow we need to visualize something for these
    //            missing chart types.  We have some alternatives:
    //            1. Show an empty area
    //            2. Show a text "unsupported chart type"
    //            3. Exchange for something else, e.g. a bar chart.
    //            ... More?
    //
    // NOTE: Whatever we do, we should always store the data so that
    //       it can be saved back into the file for a perfect
    //       roundtrip.
    KDChart::BarDiagram   *kdSurfaceDiagram;
    KDChart::BarDiagram   *kdGanttDiagram;

    KDChartModel *kdBarDiagramModel;
    KDChartModel *kdLineDiagramModel;
    KDChartModel *kdAreaDiagramModel;
    KDChartModel *kdCircleDiagramModel;
    KDChartModel *kdRingDiagramModel;
    KDChartModel *kdRadarDiagramModel;
    KDChartModel *kdScatterDiagramModel;
    KDChartModel *kdStockDiagramModel;
    KDChartModel *kdBubbleDiagramModel;
    KDChartModel *kdSurfaceDiagramModel;
    KDChartModel *kdGanttDiagramModel;

    ChartType     plotAreaChartType;
    ChartSubtype  plotAreaChartSubType;

    QString categoryDataRegionString;

    // If KDChart::LineDiagram::centerDataPoints() property is set to true,
    // the data points drawn in a line (i.e., also an area) diagram start at
    // an offset of 0.5, that is, in the middle of a column in the diagram.
    // Set flag to true if at least one dataset is attached to this axis
    // that belongs to a horizontal bar chart
    bool centerDataPoints;

    // TODO: Save to ODF
    int gapBetweenBars;
    // TODO: Save to ODF
    int gapBetweenSets;

    // TODO: Save
    // See ODF v1.2 $19.12 (chart:display-label)
    bool showLabels;

    bool isVisible;
};


Axis::Private::Private( Axis *axis )
    : q( axis )
{
    position = LeftAxisPosition;
    centerDataPoints = false;

    gapBetweenBars = 0;
    gapBetweenSets = 100;

    isVisible = true;

    useAutomaticMajorInterval = true;
    useAutomaticMinorInterval = true;

    majorInterval = 2;
    minorIntervalDivisor = 1;
    
    showMajorGrid = false;
    showMinorGrid = false;

    kdBarDiagram     = 0;
    kdLineDiagram    = 0;
    kdAreaDiagram    = 0;
    kdCircleDiagram  = 0;
    kdRingDiagram    = 0;
    kdRadarDiagram   = 0;
    kdScatterDiagram = 0;
    kdStockDiagram   = 0;
    kdBubbleDiagram  = 0;
    kdSurfaceDiagram = 0;
    kdGanttDiagram   = 0;

    kdBarDiagramModel     = 0;
    kdLineDiagramModel    = 0;
    kdAreaDiagramModel    = 0;
    kdCircleDiagramModel  = 0;
    kdRingDiagramModel    = 0;
    kdRadarDiagramModel   = 0;
    kdScatterDiagramModel = 0;
    kdStockDiagramModel   = 0;
    kdBubbleDiagramModel  = 0;
    kdSurfaceDiagramModel = 0;
    kdGanttDiagramModel   = 0;
}

Axis::Private::~Private()
{
    Q_ASSERT( plotArea );

    if ( kdPlane ) {
        plotArea->kdChart()->takeCoordinatePlane( kdPlane );
        delete kdPlane;
    }

    if ( kdPolarPlane ) {
        plotArea->kdChart()->takeCoordinatePlane( kdPolarPlane );
        delete kdPolarPlane;
    }

    if ( kdBarDiagram ) {
        plotArea->parent()->legend()->kdLegend()->removeDiagram( kdBarDiagram );
        delete kdBarDiagram;
        delete kdBarDiagramModel;
    }

    if ( kdAreaDiagram ) {
        plotArea->parent()->legend()->kdLegend()->removeDiagram( kdAreaDiagram );
        delete kdAreaDiagram;
        delete kdAreaDiagramModel;
    }

    if ( kdCircleDiagram ) {
        plotArea->parent()->legend()->kdLegend()->removeDiagram( kdCircleDiagram );
        delete kdCircleDiagram;
        delete kdCircleDiagramModel;
    }

    if ( kdRingDiagram ) {
        plotArea->parent()->legend()->kdLegend()->removeDiagram( kdRingDiagram );
        delete kdRingDiagram;
        delete kdRingDiagramModel;
    }

    if ( kdRadarDiagram ) {
        plotArea->parent()->legend()->kdLegend()->removeDiagram( kdRadarDiagram );
        delete kdRadarDiagram;
        delete kdRadarDiagramModel;
    }

    if ( kdScatterDiagram ) {
        plotArea->parent()->legend()->kdLegend()->removeDiagram( kdScatterDiagram );
        delete kdScatterDiagram;
        delete kdScatterDiagramModel;
    }

    if ( kdStockDiagram ) {
        plotArea->parent()->legend()->kdLegend()->removeDiagram( kdStockDiagram );
        delete kdStockDiagram;
        delete kdStockDiagramModel;
    }

    if ( kdBubbleDiagram ) {
        plotArea->parent()->legend()->kdLegend()->removeDiagram( kdBubbleDiagram );
        delete kdBubbleDiagram;
        delete kdBubbleDiagramModel;
    }

    if ( kdSurfaceDiagram ) {
        plotArea->parent()->legend()->kdLegend()->removeDiagram( kdSurfaceDiagram );
        delete kdSurfaceDiagram;
        delete kdSurfaceDiagramModel;
    }

    if ( kdGanttDiagram ) {
        plotArea->parent()->legend()->kdLegend()->removeDiagram( kdGanttDiagram );
        delete kdGanttDiagram;
        delete kdGanttDiagramModel;
    }
}

void Axis::Private::registerDiagram( KDChart::AbstractDiagram *diagram )
{
    KDChartModel *model = dynamic_cast<KDChartModel*>( diagram->model() );
    Q_ASSERT( model );

    QObject::connect( plotArea->proxyModel(), SIGNAL( columnsInserted( const QModelIndex&, int, int ) ),
                      model,                  SLOT( slotColumnsInserted( const QModelIndex&, int, int ) ) );

    QObject::connect( diagram, SIGNAL( propertiesChanged() ),
                      plotArea, SLOT( plotAreaUpdate() ) );
    QObject::connect( diagram, SIGNAL( layoutChanged( AbstractDiagram* ) ),
                      plotArea, SLOT( plotAreaUpdate() ) );
    QObject::connect( diagram, SIGNAL( modelsChanged() ),
                      plotArea, SLOT( plotAreaUpdate() ) );
    QObject::connect( diagram, SIGNAL( dataHidden() ),
                      plotArea, SLOT( plotAreaUpdate() ) );
}

void Axis::Private::deregisterDiagram( KDChart::AbstractDiagram *diagram )
{
    KDChartModel *model = dynamic_cast<KDChartModel*>( diagram->model() );
    Q_ASSERT( model );

    QObject::disconnect( plotArea->proxyModel(), SIGNAL( columnsInserted( const QModelIndex&, int, int ) ),
                         model,                  SLOT( slotColumnsInserted( const QModelIndex&, int, int ) ) );

    QObject::disconnect( diagram, SIGNAL( propertiesChanged() ),
                         plotArea, SLOT( plotAreaUpdate() ) );
    QObject::disconnect( diagram, SIGNAL( layoutChanged( AbstractDiagram* ) ),
                         plotArea, SLOT( plotAreaUpdate() ) );
    QObject::disconnect( diagram, SIGNAL( modelsChanged() ),
                         plotArea, SLOT( plotAreaUpdate() ) );
    QObject::disconnect( diagram, SIGNAL( dataHidden() ),
                         plotArea, SLOT( plotAreaUpdate() ) );
}

KDChart::AbstractDiagram *Axis::Private::createDiagramIfNeeded( ChartType chartType )
{
    KDChart::AbstractDiagram *diagram = 0;
    KDChartModel *model = 0;

    switch ( chartType ) {
    case BarChartType:
        if ( !kdBarDiagram )
            createBarDiagram();
        model   = kdBarDiagramModel;
        diagram = kdBarDiagram;
        break;
    case LineChartType:
        if ( !kdLineDiagram )
            createLineDiagram();
        model   = kdLineDiagramModel;
        diagram = kdLineDiagram;
        break;
    case AreaChartType:
        if ( !kdAreaDiagram )
            createAreaDiagram();
        model   = kdAreaDiagramModel;
        diagram = kdAreaDiagram;
        break;
    case CircleChartType:
        if ( !kdCircleDiagram )
            createCircleDiagram();
        model   = kdCircleDiagramModel;
        diagram = kdCircleDiagram;
        break;
    case RingChartType:
        if ( !kdRingDiagram )
            createRingDiagram();
        model   = kdRingDiagramModel;
        diagram = kdRingDiagram;
        break;
    case RadarChartType:
        if ( !kdRadarDiagram )
            createRadarDiagram();
        model   = kdRadarDiagramModel;
        diagram = kdRadarDiagram;
        break;
    case ScatterChartType:
        if ( !kdScatterDiagram )
            createScatterDiagram();
        model   = kdScatterDiagramModel;
        diagram = kdScatterDiagram;
        break;
    case StockChartType:
        if ( !kdStockDiagram )
            createStockDiagram();
        model   = kdStockDiagramModel;
        diagram = kdStockDiagram;
        break;
    case BubbleChartType:
        if ( !kdBubbleDiagram )
            createBubbleDiagram();
        model   = kdBubbleDiagramModel;
        diagram = kdBubbleDiagram;
        break;
    case SurfaceChartType:
        if ( !kdSurfaceDiagram )
            createSurfaceDiagram();
        model   = kdSurfaceDiagramModel;
        diagram = kdSurfaceDiagram;
        break;
    case GanttChartType:
        if ( !kdGanttDiagram )
            createGanttDiagram();
        model   = kdGanttDiagramModel;
        diagram = kdGanttDiagram;
        break;
    default:
        ;
    }

    if(diagram)
        diagram->setModel( model );

    adjustAllDiagrams();

    return diagram;
}

/**
 * Returns currently used internal KDChart diagram for the specified chart type
 */
KDChart::AbstractDiagram *Axis::Private::getDiagram( ChartType chartType )
{
    KDChart::AbstractDiagram *diagram = 0;
        switch ( chartType ) {
        case BarChartType:
            diagram = (KDChart::AbstractDiagram*)kdBarDiagram;
            break;
        case LineChartType:
            diagram = (KDChart::AbstractDiagram*)kdLineDiagram;
            break;
        case AreaChartType:
            diagram = (KDChart::AbstractDiagram*)kdAreaDiagram;
            break;
        case CircleChartType:
            diagram = (KDChart::AbstractDiagram*)kdCircleDiagram;
            break;
        case RingChartType:
            diagram = (KDChart::AbstractDiagram*)kdRingDiagram;
            break;
        case RadarChartType:
            diagram = (KDChart::AbstractDiagram*)kdRadarDiagram;
            break;
        case ScatterChartType:
            diagram = (KDChart::AbstractDiagram*)kdScatterDiagram;
            break;
        case StockChartType:
            diagram = (KDChart::AbstractDiagram*)kdStockDiagram;
            break;
        case BubbleChartType:
            diagram = (KDChart::AbstractDiagram*)kdBubbleDiagram;
            break;
        case SurfaceChartType:
            diagram = (KDChart::AbstractDiagram*)kdSurfaceDiagram;
            break;
        case GanttChartType:
            diagram = (KDChart::AbstractDiagram*)kdGanttDiagram;
            break;
        default:;
        }
    return diagram;
}


void Axis::Private::deleteDiagram( ChartType chartType )
{
    KDChart::AbstractDiagram **diagram = 0;
    KDChartModel **model = 0;

    switch ( chartType ) {
    case BarChartType:
        diagram = (KDChart::AbstractDiagram**)&kdBarDiagram;
        model   = &kdBarDiagramModel;
        break;
    case LineChartType:
        diagram = (KDChart::AbstractDiagram**)&kdLineDiagram;
        model   = &kdLineDiagramModel;
        break;
    case AreaChartType:
        diagram = (KDChart::AbstractDiagram**)&kdAreaDiagram;
        model   = &kdAreaDiagramModel;
        break;
    case CircleChartType:
        diagram = (KDChart::AbstractDiagram**)&kdCircleDiagram;
        model   = &kdCircleDiagramModel;
        break;
    case RingChartType:
        diagram = (KDChart::AbstractDiagram**)&kdRingDiagram;
        model   = &kdRingDiagramModel;
        break;
    case RadarChartType:
        diagram = (KDChart::AbstractDiagram**)&kdRadarDiagram;
        model   = &kdRadarDiagramModel;
        break;
    case ScatterChartType:
        diagram = (KDChart::AbstractDiagram**)&kdScatterDiagram;
        model   = &kdScatterDiagramModel;
        break;
    case StockChartType:
        diagram = (KDChart::AbstractDiagram**)&kdStockDiagram;
        model   = &kdStockDiagramModel;
        break;
    case BubbleChartType:
        diagram = (KDChart::AbstractDiagram**)&kdBubbleDiagram;
        model   = &kdBubbleDiagramModel;
        break;
    case SurfaceChartType:
        diagram = (KDChart::AbstractDiagram**)&kdSurfaceDiagram;
        model   = &kdSurfaceDiagramModel;
        break;
    case GanttChartType:
        diagram = (KDChart::AbstractDiagram**)&kdGanttDiagram;
        model   = &kdGanttDiagramModel;
        break;
    default:;
        // FIXME: Error handling?
    }

    Q_ASSERT( model );
    Q_ASSERT( *model );
    Q_ASSERT( diagram );
    Q_ASSERT( *diagram );

    deregisterDiagram( *diagram );
    delete *diagram;
    delete *model;

    *model = 0;
    *diagram = 0;

    adjustAllDiagrams();
}


void Axis::Private::createBarDiagram()
{
    Q_ASSERT( kdBarDiagramModel == 0 );
    Q_ASSERT( kdBarDiagram == 0 );

    kdBarDiagramModel = new KDChartModel;

    kdBarDiagram = new KDChart::BarDiagram( plotArea->kdChart(), kdPlane );
    kdBarDiagram->setModel( kdBarDiagramModel );
    // By 'vertical', KDChart means the orientation of a chart's bars,
    // not the orientation of the x axis.
    kdBarDiagram->setOrientation( plotArea->isVertical() ? Qt::Horizontal : Qt::Vertical );
    registerDiagram( kdBarDiagram );
    kdBarDiagram->setPen( QPen( Qt::black, 0.0 ) );

    if ( plotAreaChartSubType == StackedChartSubtype )
        kdBarDiagram->setType( KDChart::BarDiagram::Stacked );
    else if ( plotAreaChartSubType == PercentChartSubtype )
        kdBarDiagram->setType( KDChart::BarDiagram::Percent );

    if ( isVisible )
        kdBarDiagram->addAxis( kdAxis );
    kdPlane->addDiagram( kdBarDiagram );

    if ( !plotArea->kdChart()->coordinatePlanes().contains( kdPlane ) )
        plotArea->kdChart()->addCoordinatePlane( kdPlane );

    Q_ASSERT( plotArea );
    foreach ( Axis *axis, plotArea->axes() )
    {
        if ( axis->isVisible() && axis->dimension() == XAxisDimension )
            kdBarDiagram->addAxis( axis->kdAxis() );
    }

    // Set default bar diagram attributes
    q->setGapBetweenBars( 0 );
    q->setGapBetweenSets( 100 );

    // Propagate existing settings
    KDChart::ThreeDBarAttributes attributes( kdBarDiagram->threeDBarAttributes() );
    attributes.setEnabled( plotArea->isThreeD() );
    kdBarDiagram->setThreeDBarAttributes( attributes );

    plotArea->parent()->legend()->kdLegend()->addDiagram( kdBarDiagram );
}

void Axis::Private::createLineDiagram()
{
    Q_ASSERT( kdLineDiagramModel == 0 );
    Q_ASSERT( kdLineDiagram == 0 );

    kdLineDiagramModel = new KDChartModel;

    kdLineDiagram = new KDChart::LineDiagram( plotArea->kdChart(), kdPlane );
    kdLineDiagram->setModel( kdLineDiagramModel );
    registerDiagram( kdLineDiagram );

    if ( plotAreaChartSubType == StackedChartSubtype )
        kdLineDiagram->setType( KDChart::LineDiagram::Stacked );
    else if ( plotAreaChartSubType == PercentChartSubtype )
        kdLineDiagram->setType( KDChart::LineDiagram::Percent );

    if ( isVisible )
        kdLineDiagram->addAxis( kdAxis );
    kdPlane->addDiagram( kdLineDiagram );

    if ( !plotArea->kdChart()->coordinatePlanes().contains( kdPlane ) )
        plotArea->kdChart()->addCoordinatePlane( kdPlane );

    Q_ASSERT( plotArea );
    foreach ( Axis *axis, plotArea->axes() ) {
        if ( axis->dimension() == XAxisDimension )
            if ( axis->isVisible() )
                kdLineDiagram->addAxis( axis->kdAxis() );
    }

    // Propagate existing settings
    KDChart::ThreeDLineAttributes attributes( kdLineDiagram->threeDLineAttributes() );
    attributes.setEnabled( plotArea->isThreeD() );
    kdLineDiagram->setThreeDLineAttributes( attributes );

    plotArea->parent()->legend()->kdLegend()->addDiagram( kdLineDiagram );
}

void Axis::Private::createAreaDiagram()
{
    Q_ASSERT( kdAreaDiagramModel == 0 );
    Q_ASSERT( kdAreaDiagram == 0 );

    kdAreaDiagramModel = new KDChartModel;

    kdAreaDiagram = new KDChart::LineDiagram( plotArea->kdChart(), kdPlane );
    KDChart::LineAttributes attr = kdAreaDiagram->lineAttributes();
    // Draw the area under the lines. This makes this diagram an area chart.
    attr.setDisplayArea( true );
    kdAreaDiagram->setLineAttributes( attr );
    kdAreaDiagram->setModel( kdAreaDiagramModel );
    registerDiagram( kdAreaDiagram );
    kdAreaDiagram->setPen( QPen( Qt::black, 0.0 ) );

    if ( plotAreaChartSubType == StackedChartSubtype )
        kdAreaDiagram->setType( KDChart::LineDiagram::Stacked );
    else if ( plotAreaChartSubType == PercentChartSubtype )
        kdAreaDiagram->setType( KDChart::LineDiagram::Percent );

    if ( isVisible )
        kdAreaDiagram->addAxis( kdAxis );
    kdPlane->addDiagram( kdAreaDiagram );

    if ( !plotArea->kdChart()->coordinatePlanes().contains( kdPlane ) )
        plotArea->kdChart()->addCoordinatePlane( kdPlane );

    Q_ASSERT( plotArea );
    foreach ( Axis *axis, plotArea->axes() ) {
        if ( axis->dimension() == XAxisDimension )
            if ( axis->isVisible() )
                kdAreaDiagram->addAxis( axis->kdAxis() );
    }

    // Propagate existing settings
    KDChart::ThreeDLineAttributes attributes( kdAreaDiagram->threeDLineAttributes() );
    attributes.setEnabled( plotArea->isThreeD() );
    kdAreaDiagram->setThreeDLineAttributes( attributes );

    plotArea->parent()->legend()->kdLegend()->addDiagram( kdAreaDiagram );
}

void Axis::Private::createCircleDiagram()
{
    Q_ASSERT( kdCircleDiagramModel == 0);
    Q_ASSERT( kdCircleDiagram == 0 );

    kdCircleDiagramModel = new KDChartModel;
    kdCircleDiagramModel->setDataDirection( Qt::Horizontal );

    kdCircleDiagram = new KDChart::PieDiagram( plotArea->kdChart(), kdPolarPlane );
    kdCircleDiagram->setModel( kdCircleDiagramModel );
    registerDiagram( kdCircleDiagram );

    plotArea->parent()->legend()->kdLegend()->addDiagram( kdCircleDiagram );
    kdPolarPlane->addDiagram( kdCircleDiagram );

    if ( !plotArea->kdChart()->coordinatePlanes().contains( kdPolarPlane ) )
        plotArea->kdChart()->addCoordinatePlane( kdPolarPlane );

    // Propagate existing settings
    KDChart::ThreeDPieAttributes attributes( kdCircleDiagram->threeDPieAttributes() );
    attributes.setEnabled( plotArea->isThreeD() );
    kdCircleDiagram->setThreeDPieAttributes( attributes );

    // Initialize with default values that are specified in PlotArea
    // Note: KDChart takes an int here, though ODF defines the offset to be a double.
    kdPolarPlane->setStartPosition( (int)plotArea->pieAngleOffset() );
}

void Axis::Private::createRingDiagram()
{
    Q_ASSERT( kdRingDiagramModel == 0 );
    Q_ASSERT( kdRingDiagram == 0 );

    kdRingDiagramModel = new KDChartModel;
    kdRingDiagramModel->setDataDirection( Qt::Horizontal );

    kdRingDiagram = new KDChart::RingDiagram( plotArea->kdChart(), kdPolarPlane );
    kdRingDiagram->setModel( kdRingDiagramModel );
    registerDiagram( kdRingDiagram );

    plotArea->parent()->legend()->kdLegend()->addDiagram( kdRingDiagram );
    kdPolarPlane->addDiagram( kdRingDiagram );

    if ( !plotArea->kdChart()->coordinatePlanes().contains( kdPolarPlane ) )
        plotArea->kdChart()->addCoordinatePlane( kdPolarPlane );

    // Propagate existing settings
    KDChart::ThreeDPieAttributes attributes( kdRingDiagram->threeDPieAttributes() );
    attributes.setEnabled( plotArea->isThreeD() );
    kdRingDiagram->setThreeDPieAttributes( attributes );

    // Initialize with default values that are specified in PlotArea
    // Note: KDChart takes an int here, though ODF defines the offset to be a double.
    kdPolarPlane->setStartPosition( (int)plotArea->pieAngleOffset() );
}

void Axis::Private::createRadarDiagram()
{
    Q_ASSERT( kdRadarDiagramModel == 0 );
    Q_ASSERT( kdRadarDiagram == 0 );

    kdRadarDiagramModel = new KDChartModel;
    //kdRadarDiagramModel->setDataDimensions( 2 );
    //kdRadarDiagramModel->setDataDirection( Qt::Horizontal );

    kdRadarDiagram = new KDChart::PolarDiagram( plotArea->kdChart(), kdPolarPlane );
    kdRadarDiagram->setModel( kdRadarDiagramModel );
    kdRadarDiagram->setCloseDatasets(true);
    registerDiagram( kdRadarDiagram );

#if 0  // Stacked and Percent not supported by KDChart.
    if ( plotAreaChartSubType == StackedChartSubtype )
        kdRadarDiagram->setType( KDChart::PolarDiagram::Stacked );
    else if ( plotAreaChartSubType == PercentChartSubtype )
        kdRadarDiagram->setType( KDChart::PolarDiagram::Percent );
#endif
    plotArea->parent()->legend()->kdLegend()->addDiagram( kdRadarDiagram );
    kdPolarPlane->addDiagram( kdRadarDiagram );

    if ( !plotArea->kdChart()->coordinatePlanes().contains( kdPolarPlane ) )
        plotArea->kdChart()->addCoordinatePlane( kdPolarPlane );
}

void Axis::Private::createScatterDiagram()
{
    Q_ASSERT( kdScatterDiagramModel == 0 );
    Q_ASSERT( kdScatterDiagram == 0 );

    kdScatterDiagramModel = new KDChartModel;
    kdScatterDiagramModel->setDataDimensions( 2 );

    kdScatterDiagram = new KDChart::Plotter( plotArea->kdChart(), kdPlane );
    kdScatterDiagram->setModel( kdScatterDiagramModel );
    registerDiagram( kdScatterDiagram );

    if ( isVisible )
        kdScatterDiagram->addAxis( kdAxis );
    kdPlane->addDiagram( kdScatterDiagram );

    if ( !plotArea->kdChart()->coordinatePlanes().contains( kdPlane ) )
        plotArea->kdChart()->addCoordinatePlane( kdPlane );

    Q_ASSERT( plotArea );
    foreach ( Axis *axis, plotArea->axes() ) {
        if ( axis->dimension() == XAxisDimension )
            if ( axis->isVisible() )
                kdScatterDiagram->addAxis( axis->kdAxis() );
    }

    // Propagate existing settings
    KDChart::ThreeDLineAttributes attributes( kdScatterDiagram->threeDLineAttributes() );
    attributes.setEnabled( plotArea->isThreeD() );
    kdScatterDiagram->setThreeDLineAttributes( attributes );

    plotArea->parent()->legend()->kdLegend()->addDiagram( kdScatterDiagram );
}

void Axis::Private::createStockDiagram()
{
    Q_ASSERT( kdStockDiagramModel == 0 );
    Q_ASSERT( kdStockDiagram == 0 );

    kdStockDiagramModel = new KDChartModel;

    kdStockDiagram = new KDChart::StockDiagram( plotArea->kdChart(), kdPlane );
    kdStockDiagram->setModel( kdStockDiagramModel );
    registerDiagram( kdStockDiagram );

#if 0  // Stacked and Percent not supported by KDChart.
    if ( plotAreaChartSubType == StackedChartSubtype )
        kdStockDiagram->setType( KDChart::StockDiagram::Stacked );
    else if ( plotAreaChartSubType == PercentChartSubtype )
        kdStockDiagram->setType( KDChart::StockDiagram::Percent );
#endif

    if ( isVisible )
        kdStockDiagram->addAxis( kdAxis );
    kdPlane->addDiagram( kdStockDiagram );

    if ( !plotArea->kdChart()->coordinatePlanes().contains( kdPlane ) )
        plotArea->kdChart()->addCoordinatePlane( kdPlane );

    Q_ASSERT( plotArea );
    foreach ( Axis *axis, plotArea->axes() ) {
        if ( axis->dimension() == XAxisDimension )
            if ( axis->isVisible() )
                kdStockDiagram->addAxis( axis->kdAxis() );
    }

    plotArea->parent()->legend()->kdLegend()->addDiagram( kdStockDiagram );

}

void Axis::Private::createBubbleDiagram()
{
    Q_ASSERT( kdBubbleDiagramModel == 0 );
    Q_ASSERT( kdBubbleDiagram == 0 );

    kdBubbleDiagramModel = new KDChartModel;
    kdBubbleDiagramModel->setDataDimensions( 2 );

    kdBubbleDiagram = new KDChart::Plotter( plotArea->kdChart(), kdPlane );
    kdBubbleDiagram->setModel( kdBubbleDiagramModel );
    
    registerDiagram( kdBubbleDiagram );
    
    kdPlane->addDiagram( kdBubbleDiagram );

    if ( !plotArea->kdChart()->coordinatePlanes().contains( kdPlane ) )
        plotArea->kdChart()->addCoordinatePlane( kdPlane );
    
    //foreach ( Axis *axis, plotArea->axes() ) {
    //    if ( axis->isVisible() )
    //        kdBubbleDiagram->addAxis( axis->kdAxis() );
    //}
    
     // disable the connecting line
    //kdBubbleDiagram->setPen( QPen( Qt::black, 0.0 ) );
    kdBubbleDiagram->setPen( Qt::NoPen );

    KDChart::DataValueAttributes dva( kdBubbleDiagram->dataValueAttributes() );
    dva.setVisible( true );
    
    KDChart::TextAttributes ta( dva.textAttributes() );
    ta.setVisible( false );
    dva.setTextAttributes( ta );

    KDChart::MarkerAttributes ma( dva.markerAttributes() );
    ma.setVisible( true );
    ma.setMarkerStyle( KDChart::MarkerAttributes::MarkerRing );
    //ma.setMarkerSize( QSizeF(30.0,30.0) );
    dva.setMarkerAttributes( ma );

    //KDChart::RelativePosition pos( dva.positivePosition() );
    //pos.setAlignment( Qt::AlignCenter );
    //pos.setHorizontalPadding(0);
    //dva.setPositivePosition( pos );

    kdBubbleDiagram->setDataValueAttributes( dva );

    plotArea->parent()->legend()->kdLegend()->addDiagram( kdBubbleDiagram );
}

void Axis::Private::createSurfaceDiagram()
{
    // This is a so far a by KDChart unsupported chart type.
#if 0
    // The model.
    if ( kdSurfaceDiagramModel == 0 ) {
        kdSurfaceDiagramModel = new KDChartModel;
        //kdSurfaceDiagramModel->setDataDimensions( 2 );
    }

    // No KDChart::Diagram since KDChart doesn't support this type
    if ( kdSurfaceDiagram == 0 ) {
        ...
    }
#else // fallback to bar diagram for now
    createBarDiagram();
    kdSurfaceDiagramModel = kdBarDiagramModel;
    kdBarDiagramModel = 0;
    kdSurfaceDiagram = kdBarDiagram;
    kdBarDiagram = 0;
#endif
}

void Axis::Private::createGanttDiagram()
{
    // This is a so far a by KDChart unsupported chart type (through KDGantt got merged into KDChart with 2.3)
#if 0
    // The model.
    if ( kdGanttDiagramModel == 0 ) {
        kdGanttDiagramModel = new KDChartModel;
        //kdGanttDiagramModel->setDataDimensions( 2 );
    }

    // No KDChart::Diagram since KDChart doesn't support this type
    if ( kdGanttDiagram == 0 ) {
        ...
    }
#else // fallback to bar diagram for now
    createBarDiagram();
    kdSurfaceDiagramModel = kdBarDiagramModel;
    kdBarDiagramModel = 0;
    kdSurfaceDiagram = kdBarDiagram;
    kdBarDiagram = 0;
#endif
}

/**
 * Automatically adjusts the diagram so that all currently displayed
 * diagram types fit together.
 */
void Axis::Private::adjustAllDiagrams()
{
    // If at least one dataset is attached that belongs to a
    // horizontal bar chart, set centerDataPoints to true.
    centerDataPoints = kdBarDiagram != 0;
    if ( kdLineDiagram )
        kdLineDiagram->setCenterDataPoints( centerDataPoints );
    if ( kdAreaDiagram )
        kdAreaDiagram->setCenterDataPoints( centerDataPoints );
}


// ================================================================
//                             class Axis


Axis::Axis( PlotArea *parent )
    : d( new Private( this ) )
{
    Q_ASSERT( parent );

    d->plotArea = parent;
    d->kdAxis       = new KDChart::CartesianAxis();
    d->kdPlane      = new KDChart::CartesianCoordinatePlane();
    d->kdPolarPlane = new KDChart::PolarCoordinatePlane();

    d->plotAreaChartType    = d->plotArea->chartType();
    d->plotAreaChartSubType = d->plotArea->chartSubType();

    KDChart::GridAttributes gridAttributes = d->kdPlane->gridAttributes( Qt::Horizontal );
    gridAttributes.setGridVisible( false );
    gridAttributes.setGridGranularitySequence( KDChartEnums::GranularitySequence_10_50 );
    d->kdPlane->setGridAttributes( Qt::Horizontal, gridAttributes );

    gridAttributes = d->kdPlane->gridAttributes( Qt::Vertical );
    gridAttributes.setGridVisible( false );
    gridAttributes.setGridGranularitySequence( KDChartEnums::GranularitySequence_10_50 );
    d->kdPlane->setGridAttributes( Qt::Vertical, gridAttributes );

    gridAttributes = d->kdPolarPlane->gridAttributes( Qt::Horizontal );
    gridAttributes.setGridVisible( false );
    d->kdPolarPlane->setGridAttributes( Qt::Horizontal, gridAttributes );
    
    gridAttributes = d->kdPolarPlane->gridAttributes( Qt::Vertical );
    gridAttributes.setGridVisible( false );
    d->kdPolarPlane->setGridAttributes( Qt::Vertical, gridAttributes );

    d->title = KoShapeRegistry::instance()->value( TextShapeId )->createDefaultShape(parent->parent()->resourceManager());
    if ( d->title ) {
        d->titleData = qobject_cast<TextLabelData*>( d->title->userData() );
        if ( d->titleData == 0 ) {
            d->titleData = new TextLabelData;
            d->title->setUserData( d->titleData );
        }

        QFont font = d->titleData->document()->defaultFont();
        font.setPointSizeF( 9 );
        d->titleData->document()->setDefaultFont( font );
    }
    else {
        d->title = new TextLabelDummy;
        d->titleData = new TextLabelData;
        d->title->setUserData( d->titleData );
    }
    d->title->setSize( QSizeF( CM_TO_POINT( 3 ), CM_TO_POINT( 0.75 ) ) );

    d->plotArea->parent()->addShape( d->title );
    d->plotArea->parent()->setClipped( d->title, true );
    d->plotArea->parent()->setInheritsTransform(d->title, true);

    connect( d->plotArea, SIGNAL( gapBetweenBarsChanged( int ) ),
             this,        SLOT( setGapBetweenBars( int ) ) );
    connect( d->plotArea, SIGNAL( gapBetweenSetsChanged( int ) ),
             this,        SLOT( setGapBetweenSets( int ) ) );
    connect( d->plotArea, SIGNAL( pieExplodeFactorChanged( DataSet*, int ) ),
             this,        SLOT( setPieExplodeFactor( DataSet*, int ) ) );
    connect( d->plotArea, SIGNAL( pieAngleOffsetChanged( qreal ) ),
             this,        SLOT( setPieAngleOffset( qreal ) ) );
}

Axis::~Axis()
{
    Q_ASSERT( d->plotArea );
    d->plotArea->parent()->KoShapeContainer::removeShape( d->title );

    Q_ASSERT( d->title );
    if ( d->title )
        delete d->title;

   delete d;
}

PlotArea* Axis::plotArea() const
{
    return d->plotArea;
}

AxisPosition Axis::position() const
{
    return d->position;
}

void Axis::setDimension( AxisDimension dimension )
{
    d->dimension = dimension;

    // We don't support z axes yet, so hide them.
    // They are only kept to not lose them when saving a document
    // that previously had a z axis.
    if ( dimension == ZAxisDimension ) {
        d->kdPolarPlane->setReferenceCoordinatePlane( 0 );
        d->kdPlane->setReferenceCoordinatePlane( 0 );
        d->title->setVisible( false );
    } else {
        d->kdPolarPlane->setReferenceCoordinatePlane( d->plotArea->kdPlane() );
        d->kdPlane->setReferenceCoordinatePlane( d->plotArea->kdPlane() );
    }

    requestRepaint();
}

void Axis::setPosition( AxisPosition position )
{
    d->position = position;

    // FIXME: In KChart 2.1, we will have horizontal bar diagrams.
    // That means that e.g. LeftAxisPosition != YAxisDimension!
    if ( position == LeftAxisPosition || position == RightAxisPosition )
        setDimension( YAxisDimension );
    else if ( position == TopAxisPosition || position == BottomAxisPosition )
        setDimension( XAxisDimension );

    if ( position == LeftAxisPosition )
        d->title->rotate( -90 - d->title->rotation() );
    else if ( position == RightAxisPosition )
        d->title->rotate( 90 - d->title->rotation() );

    // KDChart
    d->kdAxis->setPosition( AxisPositionToKDChartAxisPosition( position ) );

    Position layoutPosition;
    switch ( position ) {
        case TopAxisPosition:
            layoutPosition = TopPosition;
            break;
        case BottomAxisPosition:
            layoutPosition = BottomPosition;
            break;
        case LeftAxisPosition:
            layoutPosition = StartPosition;
            break;
        case RightAxisPosition:
            layoutPosition = EndPosition;
            break;
    }
    Layout *layout = plotArea()->parent()->layout();
    layout->setPosition( title(), layoutPosition );
    layout->layout();

    requestRepaint();
}

KoShape *Axis::title() const
{
    return d->title;
}

QString Axis::titleText() const
{
    return d->titleData->document()->toPlainText();
}

bool Axis::showLabels() const
{
    return d->showLabels;
}

QString Axis::id() const
{
    return d->id;
}

AxisDimension Axis::dimension() const
{
    return d->dimension;
}

QList<DataSet*> Axis::dataSets() const
{
    return d->dataSets;
}

bool Axis::attachDataSet( DataSet *dataSet, bool silent )
{
    Q_ASSERT( !d->dataSets.contains( dataSet ) );
    if ( d->dataSets.contains( dataSet ) )
        return false;

    d->dataSets.append( dataSet );

    if ( dimension() == XAxisDimension ) {
        dataSet->setCategoryDataRegionString( d->categoryDataRegionString );
    }
    else if ( dimension() == YAxisDimension ) {
        dataSet->setAttachedAxis( this );

        ChartType chartType = dataSet->chartType();
        if ( chartType == LastChartType )
            chartType = d->plotAreaChartType;

        KDChart::AbstractDiagram *diagram = d->createDiagramIfNeeded( chartType );
        if( ! diagram )
            return false;
        KDChartModel *model = (KDChartModel*)diagram->model();
        if( !model )
            return false;

        dataSet->setKdDiagram( diagram );
        if ( model )
            model->addDataSet( dataSet, silent );

        if ( !silent ) {
            layoutPlanes();
            requestRepaint();
        }
    }

    return true;
}

bool Axis::detachDataSet( DataSet *dataSet, bool silent )
{
    Q_ASSERT( d->dataSets.contains( dataSet ) );
    if ( !d->dataSets.contains( dataSet ) )
        return false;
    d->dataSets.removeAll( dataSet );

    if ( dimension() == XAxisDimension ) {
        dataSet->setCategoryDataRegionString( "" );
    }
    else if ( dimension() == YAxisDimension ) {
        dataSet->setAttachedAxis( 0 );

        ChartType chartType = dataSet->chartType();
        if ( chartType == LastChartType )
            chartType = d->plotAreaChartType;

        KDChart::AbstractDiagram *oldDiagram = d->getDiagram( chartType );
        Q_ASSERT( oldDiagram );
        KDChartModel *oldModel = (KDChartModel*)oldDiagram->model();
        Q_ASSERT( oldModel );

        if ( oldModel ) {
            const int rowCount = oldModel->dataDirection() == Qt::Vertical
                                     ? oldModel->columnCount() : oldModel->rowCount();
            // If there's only as many rows as needed for *one*
            // dataset, that means that the dataset we're removing is
            // the last one in the model --> delete model
            if ( rowCount == oldModel->dataDimensions() ) {
                Q_ASSERT( oldDiagram );
                KDChart::AbstractCoordinatePlane *plane = oldDiagram->coordinatePlane();
                if ( plane ) {
                    plane->takeDiagram( oldDiagram );
                    if ( plane->diagrams().size() == 0 ) {
                        d->plotArea->kdChart()->takeCoordinatePlane( plane );
                    }
                }

                if ( d->plotArea->parent()->legend()->kdLegend() ) {
                    d->plotArea->parent()->legend()->kdLegend()->removeDiagram( oldDiagram );
                }
                d->deleteDiagram( chartType );
            }
            else
                oldModel->removeDataSet( dataSet, silent );
        }

        dataSet->setKdDiagram( 0 );
        dataSet->setKdChartModel( 0 );
        dataSet->setKdDataSetNumber( -1 );

        if ( !silent ) {
            layoutPlanes();
            requestRepaint();
        }
    }

    return true;
}

qreal Axis::majorInterval() const
{
    return d->majorInterval;
}

void Axis::setMajorInterval( qreal interval )
{
    // Don't overwrite if automatic interval is being requested ( for
    // interval = 0 )
    if ( interval != 0.0 ) {
        d->majorInterval = interval;
        d->useAutomaticMajorInterval = false;
    } else
        d->useAutomaticMajorInterval = true;

    // KDChart
    KDChart::GridAttributes attributes = d->kdPlane->gridAttributes( orientation() );
    attributes.setGridStepWidth( interval );
    d->kdPlane->setGridAttributes( orientation(), attributes );

    attributes = d->kdPolarPlane->gridAttributes( orientation() );
    attributes.setGridStepWidth( interval );
    d->kdPolarPlane->setGridAttributes( orientation(), attributes );

    // FIXME: Hide minor tick marks more appropriately
    if ( !d->showMinorGrid && interval != 0.0 )
        setMinorInterval( interval );

    requestRepaint();
}

qreal Axis::minorInterval() const
{
    return ( d->majorInterval / (qreal)d->minorIntervalDivisor );
}

void Axis::setMinorInterval( qreal interval )
{
    if ( interval == 0.0 )
        setMinorIntervalDivisor( 0 );
    else
        setMinorIntervalDivisor( int( qRound( d->majorInterval / interval ) ) );
}

int Axis::minorIntervalDivisor() const
{
    return d->minorIntervalDivisor;
}

void Axis::setMinorIntervalDivisor( int divisor )
{
    // A divisor of 0.0 means automatic minor interval calculation
    if ( divisor != 0 ) {
        d->minorIntervalDivisor = divisor;
        d->useAutomaticMinorInterval = false;
    } else
        d->useAutomaticMinorInterval = true;

    // KDChart
    KDChart::GridAttributes attributes = d->kdPlane->gridAttributes( orientation() );
    attributes.setGridSubStepWidth( (divisor != 0) ? (d->majorInterval / divisor) : 0.0 );
    d->kdPlane->setGridAttributes( orientation(), attributes );
    
    attributes = d->kdPolarPlane->gridAttributes( orientation() );
    attributes.setGridSubStepWidth( (divisor != 0) ? (d->majorInterval / divisor) : 0.0 );
    d->kdPolarPlane->setGridAttributes( orientation(), attributes );

    requestRepaint();
}

bool Axis::useAutomaticMajorInterval() const
{
    return d->useAutomaticMajorInterval;
}

bool Axis::useAutomaticMinorInterval() const
{
    return d->useAutomaticMinorInterval;
}

void Axis::setUseAutomaticMajorInterval( bool automatic )
{
    d->useAutomaticMajorInterval = automatic;
    // A value of 0.0 will activate automatic intervals,
    // but not change d->majorInterval
    setMajorInterval( automatic ? 0.0 : majorInterval() );
}

void Axis::setUseAutomaticMinorInterval( bool automatic )
{
    d->useAutomaticMinorInterval = automatic;
    // A value of 0.0 will activate automatic intervals,
    // but not change d->minorIntervalDivisor
    setMinorInterval( automatic ? 0.0 : minorInterval() );
}

bool Axis::showInnerMinorTicks() const
{
    return d->showInnerMinorTicks;
}

bool Axis::showOuterMinorTicks() const
{
    return d->showOuterMinorTicks;
}

bool Axis::showInnerMajorTicks() const
{
    return d->showInnerMinorTicks;
}

bool Axis::showOuterMajorTicks() const
{
    return d->showOuterMajorTicks;
}

void Axis::setScalingLogarithmic( bool logarithmicScaling )
{
    d->logarithmicScaling = logarithmicScaling;

    if ( dimension() != YAxisDimension )
        return;

    d->kdPlane->setAxesCalcModeY( d->logarithmicScaling
                                  ? KDChart::AbstractCoordinatePlane::Logarithmic
                                  : KDChart::AbstractCoordinatePlane::Linear );
    d->kdPlane->layoutPlanes();
    
    requestRepaint();
}

bool Axis::scalingIsLogarithmic() const
{
    return d->logarithmicScaling;
}

bool Axis::showMajorGrid() const
{
    return d->showMajorGrid;
}

void Axis::setShowMajorGrid( bool showGrid )
{
    d->showMajorGrid = showGrid;

    // KDChart
    KDChart::GridAttributes  attributes = d->kdPlane->gridAttributes( orientation() );
    attributes.setGridVisible( d->showMajorGrid );
    d->kdPlane->setGridAttributes( orientation(), attributes );
    
    attributes = d->kdPolarPlane->gridAttributes( orientation() );
    attributes.setGridVisible( d->showMajorGrid );
    d->kdPolarPlane->setGridAttributes( orientation(), attributes );

    requestRepaint();
}

bool Axis::showMinorGrid() const
{
    return d->showMinorGrid;
}

void Axis::setShowMinorGrid( bool showGrid )
{
    d->showMinorGrid = showGrid;

    // KDChart
    KDChart::GridAttributes  attributes = d->kdPlane->gridAttributes( orientation() );
    attributes.setSubGridVisible( d->showMinorGrid );
    d->kdPlane->setGridAttributes( orientation(), attributes );
    
    attributes = d->kdPolarPlane->gridAttributes( orientation() );
    attributes.setSubGridVisible( d->showMinorGrid );
    d->kdPolarPlane->setGridAttributes( orientation(), attributes );

    requestRepaint();
}

void Axis::setTitleText( const QString &text )
{
    d->titleData->document()->setPlainText( text );
}

void Axis::setShowLabels( bool show )
{
    d->showLabels = show;

    KDChart::TextAttributes textAttr = d->kdAxis->textAttributes();
    textAttr.setVisible( show );
    d->kdAxis->setTextAttributes( textAttr );
}

Qt::Orientation Axis::orientation()
{
    if ( d->position == BottomAxisPosition || d->position == TopAxisPosition )
        return Qt::Horizontal;
    return Qt::Vertical;
}

QString Axis::categoryDataRegionString() const
{
    return d->categoryDataRegionString;
}

void Axis::setCategoryDataRegionString( const QString &region )
{
    d->categoryDataRegionString = region;

    foreach( DataSet *dataSet, d->dataSets )
        dataSet->setCategoryDataRegionString( region );
}

bool Axis::loadOdf( const KoXmlElement &axisElement, KoShapeLoadingContext &context )
{
    KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
    styleStack.save();

    d->title->setVisible( false );
    
    QPen gridPen(Qt::NoPen);
    QPen subGridPen(Qt::NoPen);

    d->showMajorGrid = false;
    d->showMinorGrid = false;

    d->showInnerMinorTicks = false;
    d->showOuterMinorTicks = false;
    d->showInnerMajorTicks = false;
    d->showOuterMajorTicks = true;

    // Use automatic interval calculation by default
    setMajorInterval( 0.0 );
    setMinorInterval( 0.0 );

    if ( !axisElement.isNull() ) {
        KoXmlElement n;
        forEachElement ( n, axisElement ) {
            if ( n.namespaceURI() != KoXmlNS::chart )
                continue;
            if ( n.localName() == "title" ) {
                if ( n.hasAttributeNS( KoXmlNS::svg, "x" )
                     && n.hasAttributeNS( KoXmlNS::svg, "y" ) )
                {
                    const qreal x = KoUnit::parseValue( n.attributeNS( KoXmlNS::svg, "x" ) );
                    const qreal y = KoUnit::parseValue( n.attributeNS( KoXmlNS::svg, "y" ) );
                    d->title->setPosition( QPointF( x, y ) );
                }

                if ( n.hasAttributeNS( KoXmlNS::svg, "width" )
                     && n.hasAttributeNS( KoXmlNS::svg, "height" ) )
                {
                    const qreal width = KoUnit::parseValue( n.attributeNS( KoXmlNS::svg, "width" ) );
                    const qreal height = KoUnit::parseValue( n.attributeNS( KoXmlNS::svg, "height" ) );
                    d->title->setSize( QSizeF( width, height ) );
                }

                if ( n.hasAttributeNS( KoXmlNS::chart, "style-name" ) ) {
                    context.odfLoadingContext().fillStyleStack( n, KoXmlNS::chart, "style-name", "chart" );
                    styleStack.setTypeProperties( "text" );

                    if ( styleStack.hasProperty( KoXmlNS::fo, "font-size" ) ) {
                        const qreal fontSize = KoUnit::parseValue( styleStack.property( KoXmlNS::fo, "font-size" ) );
                        QFont font = d->titleData->document()->defaultFont();
                        font.setPointSizeF( fontSize );
                        d->titleData->document()->setDefaultFont( font );
                    }
                }

                const KoXmlElement textElement = KoXml::namedItemNS( n, KoXmlNS::text, "p" );
                if ( !textElement.isNull() ) {
                    d->title->setVisible( true );
                    setTitleText( textElement.text() );
                }
                else {
                    qWarning() << "Error: Axis' <chart:title> element contains no <text:p>";
                }
            }
            else if ( n.localName() == "grid" ) {
                bool major = false;
                if ( n.hasAttributeNS( KoXmlNS::chart, "class" ) ) {
                    const QString className = n.attributeNS( KoXmlNS::chart, "class" );
                    if ( className == "major" )
                        major = true;
                } else {
                    qWarning() << "Error: Axis' <chart:grid> element contains no valid class. It must be either \"major\" or \"minor\".";
                    continue;
                }

                if ( major ) {
                    d->showMajorGrid = true;
                } else {
                    d->showMinorGrid = true;
                }

                if ( n.hasAttributeNS( KoXmlNS::chart, "style-name" ) ) {
                    context.odfLoadingContext().fillStyleStack( n, KoXmlNS::style, "style-name", "chart" );
                    styleStack.setTypeProperties( "graphic" );
                    if ( styleStack.hasProperty( KoXmlNS::svg, "stroke-color" ) ) {
                        const QString strokeColor = styleStack.property( KoXmlNS::svg, "stroke-color" );
                        d->showMajorGrid = true;
                        if ( major )
                            gridPen = QPen( QColor( strokeColor ) );
                        else
                            subGridPen = QPen( QColor( strokeColor ) );
                    }
                }
            }
            else if ( n.localName() == "categories" ) {
                if ( n.hasAttributeNS( KoXmlNS::table, "cell-range-address" ) )
                    setCategoryDataRegionString( n.attributeNS( KoXmlNS::table, "cell-range-address" ) );
            }
        }

        if ( axisElement.hasAttributeNS( KoXmlNS::chart, "axis-name" ) ) {
            const QString name = axisElement.attributeNS( KoXmlNS::chart, "axis-name", QString() );
            //setTitleText( name );
        }

        if ( axisElement.hasAttributeNS( KoXmlNS::chart, "dimension" ) ) {
            const QString dimension = axisElement.attributeNS( KoXmlNS::chart, "dimension", QString() );
            if ( dimension == "x" )
                setPosition( BottomAxisPosition );
            else if ( dimension == "y" )
                setPosition( LeftAxisPosition );
            else if ( dimension == "z" )
                setDimension( ZAxisDimension );
        }
    }

    if ( axisElement.hasAttributeNS( KoXmlNS::chart, "style-name" ) ) {
        context.odfLoadingContext().fillStyleStack( axisElement, KoXmlNS::chart, "style-name", "chart" );
        styleStack.setTypeProperties( "text" );

        KoCharacterStyle charStyle;
        charStyle.loadOdf( context.odfLoadingContext() );
        setFont( charStyle.font() );

        styleStack.setTypeProperties( "chart" );

        if ( styleStack.hasProperty( KoXmlNS::chart, "logarithmic" )
             && styleStack.property( KoXmlNS::chart, "logarithmic" ) == "true" )
        {
            setScalingLogarithmic( true );
        }

        if ( styleStack.hasProperty( KoXmlNS::chart, "interval-major" ) )
            setMajorInterval( KoUnit::parseValue( styleStack.property( KoXmlNS::chart, "interval-major" ) ) );
        if ( styleStack.hasProperty( KoXmlNS::chart, "interval-minor-divisor" ) )
            setMinorIntervalDivisor( KoUnit::parseValue( styleStack.property( KoXmlNS::chart, "interval-minor-divisor" ) ) );
        if ( styleStack.hasProperty( KoXmlNS::chart, "display-label" ) )
            setShowLabels( styleStack.property( KoXmlNS::chart, "display-label" ) != "false" );
        if ( styleStack.hasProperty( KoXmlNS::chart, "visible" ) )
            setVisible( styleStack.property( KoXmlNS::chart, "visible" )  != "false" );
    } else {
        setShowLabels( KoOdfWorkaround::fixMissingStyle_DisplayLabel( axisElement, context ) );
    }

    KDChart::GridAttributes gridAttr = d->kdPlane->gridAttributes( orientation() );
    gridAttr.setGridVisible( d->showMajorGrid );
    gridAttr.setSubGridVisible( d->showMinorGrid );
    if ( gridPen.style() != Qt::NoPen )
        gridAttr.setGridPen( gridPen );
    if ( subGridPen.style() != Qt::NoPen )
        gridAttr.setSubGridPen( subGridPen );
    d->kdPlane->setGridAttributes( orientation(), gridAttr );

    gridAttr = d->kdPolarPlane->gridAttributes( orientation() );
    gridAttr.setGridVisible( d->showMajorGrid );
    gridAttr.setSubGridVisible( d->showMinorGrid );
    if ( gridPen.style() != Qt::NoPen )
        gridAttr.setGridPen( gridPen );
    if ( subGridPen.style() != Qt::NoPen )
        gridAttr.setSubGridPen( subGridPen );
    d->kdPolarPlane->setGridAttributes( orientation(), gridAttr );

    if ( !loadOdfChartSubtypeProperties( axisElement, context ) )
        return false;

    requestRepaint();

    styleStack.restore();

    return true;
}

bool Axis::loadOdfChartSubtypeProperties( const KoXmlElement &axisElement,
                                          KoShapeLoadingContext &context )
{
    Q_UNUSED(axisElement);
    KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
    styleStack.setTypeProperties( "chart" );

    // Load these attributes regardless of the actual chart type. They'll have
    // no effect if their respective chart type is not in use.
    // However, they'll be saved back to ODF that way.
    if ( styleStack.hasProperty( KoXmlNS::chart, "gap-width" ) )
        setGapBetweenSets( KoUnit::parseValue( styleStack.property( KoXmlNS::chart, "gap-width" ) ) );
    if ( styleStack.hasProperty( KoXmlNS::chart, "overlap" ) )
        // The minus is intended!
        setGapBetweenBars( -KoUnit::parseValue( styleStack.property( KoXmlNS::chart, "overlap" ) ) );

    return true;
}

void Axis::saveOdf( KoShapeSavingContext &context )
{
    KoXmlWriter &bodyWriter = context.xmlWriter();
    KoGenStyles &mainStyles = context.mainStyles();
    bodyWriter.startElement( "chart:axis" );

    KoGenStyle axisStyle( KoGenStyle::ParagraphAutoStyle, "chart" );
    axisStyle.addProperty( "chart:display-label", "true" );

    const QString styleName = mainStyles.insert( axisStyle, "ch" );
    bodyWriter.addAttribute( "chart:style-name", styleName );

    // TODO scale: logarithmic/linear
    // TODO visibility

    if ( dimension() == XAxisDimension )
        bodyWriter.addAttribute( "chart:dimension", "x" );
    else if ( dimension() == YAxisDimension )
        bodyWriter.addAttribute( "chart:dimension", "y" );

    QString name;
    switch( dimension() ) {
    case XAxisDimension:
        name = "x";
        break;
    case YAxisDimension:
        name = "y";
        break;
    case ZAxisDimension:
        name = "z";
        break;
    }
    int i = 1;
    foreach ( Axis *axis, d->plotArea->axes() ) {
        if ( axis == this )
            break;
        if ( axis->dimension() == dimension() )
            i++;
    }
    if ( i == 1 )
        name = "primary-" + name;
    else if ( i == 2 )
        name = "secondary-" + name;
    // Usually, there's not more than two axes of the same dimension.
    // But use a fallback name here nevertheless.
    else
        name = QString::number( i ) + '-' + name;
    bodyWriter.addAttribute( "chart:name", name );

    bodyWriter.startElement( "chart:title" );

    bodyWriter.addAttributePt( "svg:x", d->title->position().x() );
    bodyWriter.addAttributePt( "svg:y", d->title->position().y() );
    bodyWriter.addAttributePt( "svg:width", d->title->size().width() );
    bodyWriter.addAttributePt( "svg:height", d->title->size().height() );

    bodyWriter.startElement( "text:p" );
    bodyWriter.addTextNode( d->titleData->document()->toPlainText() );
    bodyWriter.endElement(); // text:p
    bodyWriter.endElement(); // chart:title

    if ( showMajorGrid() )
        saveOdfGrid( context, OdfMajorGrid );
    if ( showMinorGrid() )
        saveOdfGrid( context, OdfMinorGrid );

    bodyWriter.endElement(); // chart:axis
}

void Axis::saveOdfGrid( KoShapeSavingContext &context, OdfGridClass gridClass )
{
    KoXmlWriter &bodyWriter = context.xmlWriter();
    KoGenStyles &mainStyles = context.mainStyles();

    KoGenStyle gridStyle( KoGenStyle::GraphicAutoStyle, "chart" );

    KDChart::GridAttributes attributes = d->kdPlane->gridAttributes( orientation() );
    QPen gridPen = (gridClass == OdfMinorGrid ? attributes.subGridPen() : attributes.gridPen());
    KoOdfGraphicStyles::saveOdfStrokeStyle( gridStyle, mainStyles, gridPen );

    bodyWriter.startElement( "chart:grid" );
    bodyWriter.addAttribute( "chart:class", gridClass == OdfMinorGrid ? "minor" : "major" );

    bodyWriter.addAttribute( "chart:style-name", mainStyles.insert( gridStyle, "ch" ) );
    bodyWriter.endElement(); // chart:grid
}

void Axis::update() const
{
    if ( d->kdBarDiagram ) {
        d->kdBarDiagram->doItemsLayout();
        d->kdBarDiagram->update();
    }

    if ( d->kdLineDiagram ) {
        d->kdLineDiagram->doItemsLayout();
        d->kdLineDiagram->update();
    }

    if ( d->kdStockDiagram ) {
        d->kdStockDiagram->doItemsLayout();
        d->kdStockDiagram->update();
    }

    d->plotArea->parent()->requestRepaint();
}

KDChart::CartesianAxis *Axis::kdAxis() const
{
    return d->kdAxis;
}

KDChart::AbstractCoordinatePlane *Axis::kdPlane() const
{
    return d->kdPlane;
}

void Axis::plotAreaChartTypeChanged( ChartType newChartType )
{
    // Return if there's nothing to do
    if ( newChartType == d->plotAreaChartType )
        return;

    if ( d->dataSets.isEmpty() ) {
        d->plotAreaChartType = newChartType;
        return;
    }

    KDChartModel **oldModel = 0;
    KDChartModel *newModel = 0;
    KDChart::AbstractDiagram **oldDiagram = 0;
    KDChart::AbstractDiagram *newDiagram = 0;

    // Get pointers to the old model and diagram.
    switch ( d->plotAreaChartType ) {
    case BarChartType:
        oldModel = &d->kdBarDiagramModel;
        oldDiagram = (KDChart::AbstractDiagram**)&d->kdBarDiagram;
        break;
    case LineChartType:
        oldModel = &d->kdLineDiagramModel;
        oldDiagram = (KDChart::AbstractDiagram**)&d->kdLineDiagram;
        break;
    case AreaChartType:
        oldModel = &d->kdAreaDiagramModel;
        oldDiagram = (KDChart::AbstractDiagram**)&d->kdAreaDiagram;
        break;

    case CircleChartType:
        oldModel = &d->kdCircleDiagramModel;
        oldDiagram = (KDChart::AbstractDiagram**)&d->kdCircleDiagram;
        break;
    case RingChartType:
        oldModel = &d->kdRingDiagramModel;
        oldDiagram = (KDChart::AbstractDiagram**)&d->kdRingDiagram;
        break;

    case ScatterChartType:
        oldModel = &d->kdScatterDiagramModel;
        oldDiagram = (KDChart::AbstractDiagram**)&d->kdScatterDiagram;
        break;
    case RadarChartType:
        oldModel = &d->kdRadarDiagramModel;
        oldDiagram = (KDChart::AbstractDiagram**)&d->kdRadarDiagram;
        break;

    case StockChartType:
        oldModel = &d->kdStockDiagramModel;
        oldDiagram = (KDChart::AbstractDiagram**)&d->kdStockDiagram;
        break;
    case BubbleChartType:
        oldModel = &d->kdBubbleDiagramModel;
        oldDiagram = (KDChart::AbstractDiagram**)&d->kdBubbleDiagram;
        break;
    case SurfaceChartType:
        oldModel = &d->kdSurfaceDiagramModel;
        oldDiagram = (KDChart::AbstractDiagram**)&d->kdSurfaceDiagram;
        break;
    case GanttChartType:
        oldModel = &d->kdGanttDiagramModel;
        oldDiagram = (KDChart::AbstractDiagram**)&d->kdGanttDiagram;
        break;

    default:;
        // FIXME: What should happen here?
    }

    // We need to have a coordinate plane that matches the chart type.
    // Choices are cartesian or polar.
    if ( isCartesian( d->plotAreaChartType ) && isPolar( newChartType ) ) {
        if ( d->plotArea->kdChart()->coordinatePlanes().contains( d->kdPlane ) )
            d->plotArea->kdChart()->takeCoordinatePlane( d->kdPlane );
    }
    else if ( isPolar( d->plotAreaChartType ) && isCartesian( newChartType ) ) {
        if ( d->plotArea->kdChart()->coordinatePlanes().contains( d->kdPolarPlane ) )
            d->plotArea->kdChart()->takeCoordinatePlane( d->kdPolarPlane );
    }

    // Get pointers to the new model and diagrams.  Create the new
    // diagram if necessary.
    switch ( newChartType ) {
    case BarChartType:
        if ( !d->kdBarDiagram )
           d->createBarDiagram();
        newModel = d->kdBarDiagramModel;
        newDiagram = d->kdBarDiagram;
        break;
    case LineChartType:
        if ( !d->kdLineDiagram )
           d->createLineDiagram();
        newModel = d->kdLineDiagramModel;
        newDiagram = d->kdLineDiagram;
        break;
    case AreaChartType:
        if ( !d->kdAreaDiagram )
           d->createAreaDiagram();
        newModel = d->kdAreaDiagramModel;
        newDiagram = d->kdAreaDiagram;
        break;

    case CircleChartType:
        if ( !d->kdCircleDiagram )
           d->createCircleDiagram();
        newModel = d->kdCircleDiagramModel;
        newDiagram = d->kdCircleDiagram;
        break;
    case RingChartType:
        if ( !d->kdRingDiagram )
           d->createRingDiagram();
        newModel = d->kdRingDiagramModel;
        newDiagram = d->kdRingDiagram;
        break;

    case ScatterChartType:
        if ( !d->kdScatterDiagram )
           d->createScatterDiagram();
        newModel = d->kdScatterDiagramModel;
        newDiagram = d->kdScatterDiagram;
        break;
    case RadarChartType:
        if ( !d->kdRadarDiagram )
           d->createRadarDiagram();
        newModel = d->kdRadarDiagramModel;
        newDiagram = d->kdRadarDiagram;
        break;

    case StockChartType:
        if ( !d->kdStockDiagram )
           d->createStockDiagram();
        newModel = d->kdStockDiagramModel;
        newDiagram = d->kdStockDiagram;
        break;
    case BubbleChartType:
        if ( !d->kdBubbleDiagram )
           d->createBubbleDiagram();
        newModel = d->kdBubbleDiagramModel;
        newDiagram = d->kdBubbleDiagram;
        break;
    case SurfaceChartType:
        if ( !d->kdSurfaceDiagram )
           d->createSurfaceDiagram();
        newModel = d->kdSurfaceDiagramModel;
        //newDiagram = d->kdSurfaceDiagram;
        newDiagram = 0;
        break;
    case GanttChartType:
        if ( !d->kdGanttDiagram )
           d->createGanttDiagram();
        newModel = d->kdGanttDiagramModel;
        //newDiagram = d->kdGanttDiagram;
        newDiagram = 0;
        break;
    default:;
        // FIXME: Implement more chart types
    }

    // FIXME: This causes a crash on unimplemented types. We should
    //        handle that in some other way.
    Q_ASSERT( newModel );


    if (    ( isPolar( newChartType ) && !isPolar( d->plotAreaChartType ) )
         || ( !isPolar( newChartType ) && isPolar( d->plotAreaChartType ) ) )
    {
        foreach ( DataSet *dataSet, d->dataSets ) {
            if ( dataSet->chartType() != LastChartType ) {
                dataSet->setChartType( LastChartType );
                dataSet->setChartSubType( NoChartSubtype );
            }
        }
    }

    foreach ( DataSet *dataSet, d->dataSets ) {
        if ( dataSet->chartType() != LastChartType )
            continue;

        dataSet->setKdDiagram( newDiagram );
        newModel->addDataSet( dataSet );
        if ( oldModel && *oldModel ) {
            const int dataSetCount = (*oldModel)->dataDirection() == Qt::Vertical
                                     ? (*oldModel)->columnCount() : (*oldModel)->rowCount();
            if ( dataSetCount == (*oldModel)->dataDimensions() ) {
                // This if() is only necessary because we have some
                // unsupported chart types that create a NULL
                // oldDiagram;
                if ( oldDiagram ) {
                    KDChart::AbstractCoordinatePlane *plane = (*oldDiagram)->coordinatePlane();
                    if ( plane ) {
                        plane->takeDiagram( (*oldDiagram) );
                        if ( plane->diagrams().size() == 0 ) {
                            d->plotArea->kdChart()->takeCoordinatePlane( plane );
                        }
                    }

                    if ( d->plotArea->parent()->legend()->kdLegend() )
                        d->plotArea->parent()->legend()->kdLegend()->removeDiagram( (*oldDiagram) );
                    if ( *oldDiagram )
                        delete *oldDiagram;
                }
                delete *oldModel;
                *oldModel = 0;
                *oldDiagram = 0;
            }
            else
                (*oldModel)->removeDataSet( dataSet );
        }
    }

    d->plotAreaChartType = newChartType;

    layoutPlanes();

    requestRepaint();
}

void Axis::plotAreaChartSubTypeChanged( ChartSubtype subType )
{
    d->plotAreaChartSubType = subType;

    switch ( d->plotAreaChartType ) {
    case BarChartType:
        if ( d->kdBarDiagram ) {
            KDChart::BarDiagram::BarType type;
            switch ( subType ) {
            case StackedChartSubtype:
                type = KDChart::BarDiagram::Stacked; break;
            case PercentChartSubtype:
                type = KDChart::BarDiagram::Percent; break;
            default:
                type = KDChart::BarDiagram::Normal;
            }
            d->kdBarDiagram->setType( type );
        }
        break;
    case LineChartType:
        if ( d->kdLineDiagram ) {
            KDChart::LineDiagram::LineType type;
            switch ( subType ) {
            case StackedChartSubtype:
                type = KDChart::LineDiagram::Stacked; break;
            case PercentChartSubtype:
                type = KDChart::LineDiagram::Percent; break;
            default:
                type = KDChart::LineDiagram::Normal;
            }
            d->kdLineDiagram->setType( type );
        }
        break;
    case AreaChartType:
        if ( d->kdAreaDiagram ) {
            KDChart::LineDiagram::LineType type;
            switch ( subType ) {
            case StackedChartSubtype:
                type = KDChart::LineDiagram::Stacked; break;
            case PercentChartSubtype:
                type = KDChart::LineDiagram::Percent; break;
            default:
                type = KDChart::LineDiagram::Normal;
            }
            d->kdAreaDiagram->setType( type );
        }
        break;
    case RadarChartType:
#if 0 // FIXME: Stacked and Percent not supported by KDChart
        if ( d->kdRadarDiagram ) {
            KDChart::PolarDiagram::PolarType type;
            switch ( subType ) {
            case StackedChartSubtype:
                type = KDChart::PolarDiagram::Stacked; break;
            case PercentChartSubtype:
                type = KDChart::PolarDiagram::Percent; break;
            default:
                type = KDChart::PolarDiagram::Normal;
            }
            d->kdRadarDiagram->setType( type );
        }
#endif
        break;
    case StockChartType:
        if ( d->kdStockDiagram ) {
            KDChart::StockDiagram::Type type;
            switch ( subType ) {
#if 0
            case StackedChartSubtype:
                type = KDChart::StockDiagram::Candlestick; break;
            case PercentChartSubtype:
                type = KDChart::StockDiagram::OpenHighLowClose; break;
#endif
            default:
                type = KDChart::StockDiagram::HighLowClose;
            }
            d->kdStockDiagram->setType( type );
        }
        break;
    default:;
        // FIXME: Implement more chart types
    }
}

void Axis::registerKdAxis( KDChart::CartesianAxis *axis )
{
    if ( d->kdBarDiagram )
        d->kdBarDiagram->addAxis( axis );
    if ( d->kdLineDiagram )
        d->kdLineDiagram->addAxis( axis );
    if ( d->kdAreaDiagram )
        d->kdAreaDiagram->addAxis( axis );
    if ( d->kdScatterDiagram )
        d->kdScatterDiagram->addAxis( axis );
    if ( d->kdStockDiagram )
        d->kdStockDiagram->addAxis( axis );
    if ( d->kdBubbleDiagram )
        d->kdBubbleDiagram->addAxis( axis );
    // FIXME: Add all diagrams here
}

void Axis::deregisterKdAxis( KDChart::CartesianAxis *axis )
{
    if ( d->kdBarDiagram )
        d->kdBarDiagram->takeAxis( axis );
    if ( d->kdLineDiagram )
        d->kdLineDiagram->takeAxis( axis );
    if ( d->kdAreaDiagram )
        d->kdAreaDiagram->takeAxis( axis );
    if ( d->kdScatterDiagram )
        d->kdScatterDiagram->takeAxis( axis );
    if ( d->kdStockDiagram )
        d->kdStockDiagram->takeAxis( axis );
    if ( d->kdBubbleDiagram )
        d->kdBubbleDiagram->takeAxis( axis );
    // FIXME: Add all diagrams here
}

void Axis::setThreeD( bool threeD )
{
    // KDChart
    if ( d->kdBarDiagram ) {
        KDChart::ThreeDBarAttributes attributes( d->kdBarDiagram->threeDBarAttributes() );
        attributes.setEnabled( threeD );
        attributes.setDepth( 15.0 );
        d->kdBarDiagram->setThreeDBarAttributes( attributes );
    }

    if ( d->kdLineDiagram ) {
        KDChart::ThreeDLineAttributes attributes( d->kdLineDiagram->threeDLineAttributes() );
        attributes.setEnabled( threeD );
        attributes.setDepth( 15.0 );
        d->kdLineDiagram->setThreeDLineAttributes( attributes );
    }

    if ( d->kdAreaDiagram ) {
        KDChart::ThreeDLineAttributes attributes( d->kdAreaDiagram->threeDLineAttributes() );
        attributes.setEnabled( threeD );
        attributes.setDepth( 15.0 );
        d->kdAreaDiagram->setThreeDLineAttributes( attributes );
    }

    if ( d->kdCircleDiagram ) {
        KDChart::ThreeDPieAttributes attributes( d->kdCircleDiagram->threeDPieAttributes() );
        attributes.setEnabled( threeD );
        attributes.setDepth( 15.0 );
        d->kdCircleDiagram->setThreeDPieAttributes( attributes );
    }

    if ( d->kdRingDiagram ) {
        KDChart::ThreeDPieAttributes attributes( d->kdRingDiagram->threeDPieAttributes() );
        attributes.setEnabled( threeD );
        attributes.setDepth( 15.0 );
        d->kdRingDiagram->setThreeDPieAttributes( attributes );
    }

    // The following types don't support 3D, at least not in KDChart:
    // scatter, radar, stock, bubble, surface, gantt

    requestRepaint();
}

void Axis::requestRepaint() const
{
    d->plotArea->requestRepaint();
}

void Axis::layoutPlanes()
{
    d->kdPlane->layoutPlanes();
    d->kdPolarPlane->layoutPlanes();
}

void Axis::setGapBetweenBars( int percent )
{
    // This method is also used to override KDChart's default attributes.
    // Do not just return and do nothing if value doesn't differ from stored one.
    d->gapBetweenBars = percent;

    if ( d->kdBarDiagram ) {
        KDChart::BarAttributes attributes = d->kdBarDiagram->barAttributes();
        attributes.setBarGapFactor( (float)percent / 100.0 );
        d->kdBarDiagram->setBarAttributes( attributes );
    }

    requestRepaint();
}

void Axis::setGapBetweenSets( int percent )
{
    // This method is also used to override KDChart's default attributes.
    // Do not just return and do nothing if value doesn't differ from stored one.
    d->gapBetweenSets = percent;

    if ( d->kdBarDiagram ) {
        KDChart::BarAttributes attributes = d->kdBarDiagram->barAttributes();
        attributes.setGroupGapFactor( (float)percent / 100.0 );
        d->kdBarDiagram->setBarAttributes( attributes );
    }

    requestRepaint();
}

void Axis::setPieExplodeFactor( DataSet *dataSet, int percent )
{
    if ( d->kdCircleDiagram ) {
        KDChart::PieAttributes attributes = d->kdCircleDiagram->pieAttributes();
        attributes.setExplodeFactor( (float)percent / 100.0 );
        d->kdCircleDiagram->setPieAttributes( dataSet->kdDataSetNumber(),
                                              attributes );
    }

    requestRepaint();
}

void Axis::setPieAngleOffset( qreal angle )
{
    // only set if we already have a diagram else the value will be picked up on creating the diagram
    if ( d->kdPolarPlane->diagram() ) {
        // KDChart takes an int here, though ODF defines it to be a double.
        d->kdPolarPlane->setStartPosition( (int)angle );

        requestRepaint();
    }
}

QFont Axis::font() const
{
    return d->font;
}

void Axis::setFont( const QFont &font )
{
    // Save the font for later retrieval
    d->font = font;

    // Set the KDChart axis to use this font as well.
    KDChart::TextAttributes attr = d->kdAxis->textAttributes();
    attr.setFont( font );
    d->kdAxis->setTextAttributes( attr );
}

bool Axis::isVisible() const
{
    return d->isVisible;
}

void Axis::setVisible( bool visible )
{
    d->isVisible = visible;

    if ( visible )
        registerKdAxis( d->kdAxis );
    else
        deregisterKdAxis( d->kdAxis );
}

#include "Axis.moc"
