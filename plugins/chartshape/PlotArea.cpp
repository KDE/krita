/* This file is part of the KDE project

   Copyright 2007-2008 Johannes Simon <johannes.simon@gmail.com>
   Copyright 2009-2010 Inge Wallin <inge@lysator.liu.se>

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
#include "PlotArea.h"

// Qt
#include <QPointF>
#include <QSizeF>
#include <QList>
#include <QImage>
#include <QPainter>
#include <kdebug.h>

// Calligra
#include <KoUnit.h>
#include <KoXmlNS.h>
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoOdfStylesReader.h>
#include <KoGenStyles.h>
#include <KoOdfLoadingContext.h>
#include <Ko3dScene.h>
#include <KoOdfGraphicStyles.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoTextShapeData.h>
#include <KoViewConverter.h>
#include <KoShapeBackground.h>

// KDChart
#include <KDChartChart>
#include <KDChartCartesianAxis>
#include <KDChartAbstractDiagram>

#include <KDChartAbstractCartesianDiagram>
#include <KDChartBarAttributes>
#include <KDChartCartesianCoordinatePlane>
#include <KDChartPolarCoordinatePlane>
#include <KDChartRadarCoordinatePlane>
// Attribute Classes
#include <KDChartFrameAttributes>
#include <KDChartDataValueAttributes>
#include <KDChartGridAttributes>
#include <KDChartTextAttributes>
#include <KDChartMarkerAttributes>
// Diagram Classes
#include <KDChartBarDiagram>
#include <KDChartPieDiagram>
#include <KDChartLineDiagram>
#include <KDChartRingDiagram>
#include <KDChartPolarDiagram>

// KChart
#include "Legend.h"
#include "Surface.h"
#include "Axis.h"
#include "DataSet.h"
#include "ChartProxyModel.h"
#include "ScreenConversions.h"
#include "ChartLayout.h"

using namespace KChart;

const int MAX_PIXMAP_SIZE = 1000;

Q_DECLARE_METATYPE(QPointer<QAbstractItemModel>);
typedef QList<KDChart::AbstractCoordinatePlane*> CoordinatePlaneList;

class PlotArea::Private
{
public:
    Private(PlotArea *q, ChartShape *parent);
    ~Private();

    void initAxes();
    CoordinatePlaneList coordinatePlanesForChartType(ChartType type);

    PlotArea *q;
    // The parent chart shape
    ChartShape *shape;

    // ----------------------------------------------------------------
    // Parts and properties of the chart

    ChartType     chartType;
    ChartSubtype  chartSubtype;

    Surface       *wall;
    Surface       *floor;       // Only used in 3D charts

    // The axes
    QList<Axis*>     axes;
    QList<KoShape*>  automaticallyHiddenAxisTitles;

    // 3D properties
    bool       threeD;
    Ko3dScene *threeDScene;

    // ----------------------------------------------------------------
    // Data specific to each chart type

    // 1. Bar charts
    // FIXME: OpenOffice stores these attributes in the axes' elements.
    // The specs don't say anything at all about what elements can have
    // these style attributes.
    // chart:vertical attribute: see ODF v1.2, $19.63
    bool  vertical;
    int   gapBetweenBars;
    int   gapBetweenSets;

    // 2. Pie charts
    // TODO: Load+Save
    qreal pieAngleOffset;       // in degrees

    // ----------------------------------------------------------------
    // The embedded KD Chart

    // The KD Chart parts
    KDChart::Chart                    *const kdChart;
    KDChart::CartesianCoordinatePlane *const kdCartesianPlanePrimary;
    KDChart::CartesianCoordinatePlane *const kdCartesianPlaneSecondary;
    KDChart::PolarCoordinatePlane     *const kdPolarPlane;
    KDChart::RadarCoordinatePlane     *const kdRadarPlane;
    QList<KDChart::AbstractDiagram*>   kdDiagrams;

    // Caching: We can rerender faster if we cache KDChart's output
    QImage   image;
    bool     paintPixmap;
    QPointF  lastZoomLevel;
    QSizeF   lastSize;
    mutable bool pixmapRepaintRequested;
};

PlotArea::Private::Private(PlotArea *q, ChartShape *parent)
    : q(q)
    , shape(parent)
    // Default type: normal bar chart
    , chartType(BarChartType)
    , chartSubtype(NormalChartSubtype)
    , wall(0)
    , floor(0)
    , threeD(false)
    , threeDScene(0)
    // By default, x and y axes are not swapped.
    , vertical(false)
    // Data specific for bar charts
    , gapBetweenBars(0)
    , gapBetweenSets(100)
    // OpenOffice.org's default. It means the first pie slice starts at the
    // very top (and then going counter-clockwise).
    , pieAngleOffset(90.0)
    // KD Chart stuff
    , kdChart(new KDChart::Chart())
    , kdCartesianPlanePrimary(new KDChart::CartesianCoordinatePlane(kdChart))
    , kdCartesianPlaneSecondary(new KDChart::CartesianCoordinatePlane(kdChart))
    , kdPolarPlane(new KDChart::PolarCoordinatePlane(kdChart))
    , kdRadarPlane(new KDChart::RadarCoordinatePlane(kdChart))
    // Cache
    , paintPixmap(true)
    , pixmapRepaintRequested(true)
{
    // --- Prepare Primary Cartesian Coordinate Plane ---
    KDChart::GridAttributes gridAttributes;
    gridAttributes.setGridVisible(false);
    gridAttributes.setGridGranularitySequence(KDChartEnums::GranularitySequence_10_50);
    kdCartesianPlanePrimary->setGlobalGridAttributes(gridAttributes);
    // Disable odd default of (1, 1, -3, -3) which only produces weird offsets
    // between axes and plot area frame.
    kdCartesianPlanePrimary->setDrawingAreaMargins(0, 0, 0, 0);

    // --- Prepare Secondary Cartesian Coordinate Plane ---
    kdCartesianPlaneSecondary->setGlobalGridAttributes(gridAttributes);
    kdCartesianPlaneSecondary->setDrawingAreaMargins(0, 0, 0, 0);

    // --- Prepare Polar Coordinate Plane ---
    KDChart::GridAttributes polarGridAttributes;
    polarGridAttributes.setGridVisible(false);
    kdPolarPlane->setGlobalGridAttributes(polarGridAttributes);

    // --- Prepare Radar Coordinate Plane ---
    KDChart::GridAttributes radarGridAttributes;
    polarGridAttributes.setGridVisible(true);
    kdRadarPlane->setGlobalGridAttributes(radarGridAttributes);

    // By default we use a cartesian chart (bar chart), so the polar planes
    // are not needed yet. They will be added on demand in setChartType().
    kdChart->takeCoordinatePlane(kdPolarPlane);
    kdChart->takeCoordinatePlane(kdRadarPlane);

    shape->proxyModel()->setDataDimensions(1);
}

PlotArea::Private::~Private()
{
    qDeleteAll(axes);
    delete kdCartesianPlanePrimary;
    delete kdCartesianPlaneSecondary;
    delete kdPolarPlane;
    delete kdRadarPlane;
    delete kdChart;
    delete wall;
    delete floor;
    delete threeDScene;
}

void PlotArea::Private::initAxes()
{
    // The category data region is anchored to an axis and will be set on addAxis if the
    // axis defines the Axis::categoryDataRegion(). So, clear it now.
    q->proxyModel()->setCategoryDataRegion(CellRegion());
    // Remove all old axes
    while(!axes.isEmpty()) {
        Axis *axis = axes.takeLast();
        Q_ASSERT(axis);
        if (axis->title())
            automaticallyHiddenAxisTitles.removeAll(axis->title());
        delete axis;
    }
    // There need to be at least these two axes. Their constructor will
    // automatically add them to the plot area as child shape.
    new Axis(q, XAxisDimension);
    Axis *yAxis = new Axis(q, YAxisDimension);
    yAxis->setShowMajorGrid(true);
}

PlotArea::PlotArea(ChartShape *parent)
    : QObject()
    , KoShape()
    , d(new Private(this, parent))
{
    setShapeId(ChartShapeId);

    Q_ASSERT(d->shape);
    Q_ASSERT(d->shape->proxyModel());

    connect(d->shape->proxyModel(), SIGNAL(modelReset()),
            this,                   SLOT(proxyModelStructureChanged()));
    connect(d->shape->proxyModel(), SIGNAL(rowsInserted(const QModelIndex, int, int)),
            this,                   SLOT(proxyModelStructureChanged()));
    connect(d->shape->proxyModel(), SIGNAL(rowsRemoved(const QModelIndex, int, int)),
            this,                   SLOT(proxyModelStructureChanged()));
    connect(d->shape->proxyModel(), SIGNAL(columnsInserted(const QModelIndex, int, int)),
            this,                   SLOT(proxyModelStructureChanged()));
    connect(d->shape->proxyModel(), SIGNAL(columnsRemoved(const QModelIndex, int, int)),
            this,                   SLOT(proxyModelStructureChanged()));
    connect(d->shape->proxyModel(), SIGNAL(columnsInserted(const QModelIndex, int, int)),
            this,                   SLOT(plotAreaUpdate()));
    connect(d->shape->proxyModel(), SIGNAL(columnsRemoved(const QModelIndex, int, int)),
            this,                   SLOT(plotAreaUpdate()));
    connect(d->shape->proxyModel(), SIGNAL(dataChanged()),
            this,                   SLOT(plotAreaUpdate()));
}

PlotArea::~PlotArea()
{
    delete d;
}


void PlotArea::plotAreaInit()
{
    d->kdChart->resize(size().toSize());
    d->kdChart->replaceCoordinatePlane(d->kdCartesianPlanePrimary);
    d->kdCartesianPlaneSecondary->setReferenceCoordinatePlane(d->kdCartesianPlanePrimary);

    KDChart::FrameAttributes attr = d->kdChart->frameAttributes();
    attr.setVisible(false);
    d->kdChart->setFrameAttributes(attr);

    d->wall = new Surface(this);
    //d->floor = new Surface(this);

    d->initAxes();
}

void PlotArea::proxyModelStructureChanged()
{
    if (proxyModel()->isLoading())
        return;

    Q_ASSERT(xAxis());
    Q_ASSERT(yAxis());
    QMap<DataSet*, Axis*> attachedAxes;
    QList<DataSet*> dataSets = proxyModel()->dataSets();

    // Remember to what y axis each data set belongs
    foreach(DataSet *dataSet, dataSets)
        attachedAxes.insert(dataSet, dataSet->attachedAxis());

    // Proxy structure and thus data sets changed, drop old state and
    // clear all axes of data sets
    foreach(Axis *axis, axes())
        axis->clearDataSets();

    // Now add the new list of data sets to the axis they belong to
    foreach(DataSet *dataSet, dataSets) {
        xAxis()->attachDataSet(dataSet);
        // If they weren't assigned to a y axis before, use default y axis
        if (attachedAxes[dataSet])
            attachedAxes[dataSet]->attachDataSet(dataSet);
        else
            yAxis()->attachDataSet(dataSet);
    }
}

ChartProxyModel *PlotArea::proxyModel() const
{
    return d->shape->proxyModel();
}


QList<Axis*> PlotArea::axes() const
{
    return d->axes;
}

QList<DataSet*> PlotArea::dataSets() const
{
    return proxyModel()->dataSets();
}

Axis *PlotArea::xAxis() const
{
    foreach(Axis *axis, d->axes) {
        if (axis->dimension() == XAxisDimension)
            return axis;
    }

    return 0;
}

Axis *PlotArea::yAxis() const
{
    foreach(Axis *axis, d->axes) {
        if (axis->dimension() == YAxisDimension)
            return axis;
    }

    return 0;
}

Axis *PlotArea::secondaryXAxis() const
{
    bool firstXAxisFound = false;

    foreach(Axis *axis, d->axes) {
        if (axis->orientation() == Qt::Horizontal) {
            if (firstXAxisFound)
                return axis;
            else
                firstXAxisFound = true;
        }
    }

    return 0;
}

Axis *PlotArea::secondaryYAxis() const
{
    bool firstYAxisFound = false;

    foreach(Axis *axis, d->axes) {
        if (axis->orientation() == Qt::Vertical) {
            if (firstYAxisFound)
                return axis;
            else
                firstYAxisFound = true;
        }
    }

    return 0;
}

ChartType PlotArea::chartType() const
{
    return d->chartType;
}

ChartSubtype PlotArea::chartSubType() const
{
    return d->chartSubtype;
}

bool PlotArea::isThreeD() const
{
    return d->threeD;
}

bool PlotArea::isVertical() const
{
    return d->vertical;
}

Ko3dScene *PlotArea::threeDScene() const
{
    return d->threeDScene;
}

int PlotArea::gapBetweenBars() const
{
    return d->gapBetweenBars;
}

int PlotArea::gapBetweenSets() const
{
    return d->gapBetweenSets;
}

qreal PlotArea::pieAngleOffset() const
{
    return d->pieAngleOffset;
}

bool PlotArea::addAxis(Axis *axis)
{
    if (d->axes.contains(axis)) {
        qWarning() << "PlotArea::addAxis(): Trying to add already added axis.";
        return false;
    }

    if (!axis) {
        qWarning() << "PlotArea::addAxis(): Pointer to axis is NULL!";
        return false;
    }
    d->axes.append(axis);

    if (axis->dimension() == XAxisDimension) {
        // let each axis know about the other axis
        foreach (Axis *_axis, d->axes) {
            if (_axis->isVisible())
                _axis->registerKdAxis(axis->kdAxis());
        }
    }

    requestRepaint();

    return true;
}

bool PlotArea::removeAxis(Axis *axis)
{
    if (!d->axes.contains(axis)) {
        qWarning() << "PlotArea::removeAxis(): Trying to remove non-added axis.";
        return false;
    }

    if (!axis) {
        qWarning() << "PlotArea::removeAxis(): Pointer to axis is NULL!";
        return false;
    }

    if (axis->title())
        d->automaticallyHiddenAxisTitles.removeAll(axis->title());

    d->axes.removeAll(axis);

    if (axis->dimension() == XAxisDimension) {
        foreach (Axis *_axis, d->axes)
            _axis->deregisterKdAxis(axis->kdAxis());
    }

    // This also removes the axis' title, which is a shape as well
    delete axis;

    requestRepaint();

    return true;
}

CoordinatePlaneList PlotArea::Private::coordinatePlanesForChartType(ChartType type)
{
    CoordinatePlaneList result;
    switch (type) {
    case BarChartType:
    case LineChartType:
    case AreaChartType:
    case ScatterChartType:
    case GanttChartType:
    case SurfaceChartType:
    case StockChartType:
    case BubbleChartType:
        result.append(kdCartesianPlanePrimary);
        result.append(kdCartesianPlaneSecondary);
        break;
    case CircleChartType:
    case RingChartType:
        result.append(kdPolarPlane);
        break;
    case RadarChartType:
    case FilledRadarChartType:
        result.append(kdRadarPlane);
        break;
    case LastChartType:
        Q_ASSERT("There's no coordinate plane for LastChartType");
        break;
    }

    Q_ASSERT(!result.isEmpty());
    return result;
}

void PlotArea::setChartType(ChartType type)
{
    if (d->chartType == type)
        return;

    // Lots of things to do if the old and new types of coordinate
    // systems don't match.
    if (!isPolar(d->chartType) && isPolar(type)) {
        foreach (Axis *axis, d->axes) {
            if (!axis->title()->isVisible())
                continue;

            axis->title()->setVisible(false);
            d->automaticallyHiddenAxisTitles.append(axis->title());
        }
    }
    else if (isPolar(d->chartType) && !isPolar(type)) {
        foreach (KoShape *title, d->automaticallyHiddenAxisTitles) {
            title->setVisible(true);
        }
        d->automaticallyHiddenAxisTitles.clear();
    }

    CoordinatePlaneList planesToRemove;
    // First remove secondary cartesian plane as it references the primary
    // plane, otherwise KD Chart will come down crashing on us. Note that
    // removing a plane that's not in the chart is not a problem.
    planesToRemove << d->kdCartesianPlaneSecondary << d->kdCartesianPlanePrimary
                   << d->kdPolarPlane << d->kdRadarPlane;
    foreach(KDChart::AbstractCoordinatePlane *plane, planesToRemove)
        d->kdChart->takeCoordinatePlane(plane);
    CoordinatePlaneList newPlanes = d->coordinatePlanesForChartType(type);
    foreach(KDChart::AbstractCoordinatePlane *plane, newPlanes)
        d->kdChart->addCoordinatePlane(plane);
    Q_ASSERT(d->kdChart->coordinatePlanes() == newPlanes);

    d->chartType = type;

    foreach (Axis *axis, d->axes) {
        axis->plotAreaChartTypeChanged(type);
    }

    requestRepaint();
}

void PlotArea::setChartSubType(ChartSubtype subType)
{
    d->chartSubtype = subType;

    foreach (Axis *axis, d->axes) {
        axis->plotAreaChartSubTypeChanged(subType);
    }

    requestRepaint();
}

void PlotArea::setThreeD(bool threeD)
{
    d->threeD = threeD;

    foreach(Axis *axis, d->axes)
        axis->setThreeD(threeD);

    requestRepaint();
}

void PlotArea::setVertical(bool vertical)
{
    d->vertical = vertical;
    foreach(Axis *axis, d->axes)
        axis->plotAreaIsVerticalChanged();
}

// ----------------------------------------------------------------
//                         loading and saving


bool PlotArea::loadOdf(const KoXmlElement &plotAreaElement,
                       KoShapeLoadingContext &context)
{
    KoStyleStack &styleStack = context.odfLoadingContext().styleStack();

    // The exact position defined in ODF overwrites the default layout position
    if (plotAreaElement.hasAttributeNS(KoXmlNS::svg, "x") ||
        plotAreaElement.hasAttributeNS(KoXmlNS::svg, "y") ||
        plotAreaElement.hasAttributeNS(KoXmlNS::svg, "width") ||
        plotAreaElement.hasAttributeNS(KoXmlNS::svg, "height"))
    {
        parent()->layout()->setPosition(this, FloatingPosition);
    }

    context.odfLoadingContext().fillStyleStack(plotAreaElement, KoXmlNS::chart, "style-name", "chart");
    loadOdfAttributes(plotAreaElement, context, OdfAllAttributes);

    // First step is to clear all old axis instances.
    while (!d->axes.isEmpty()) {
        Axis *axis = d->axes.takeLast();
        Q_ASSERT(axis);
        // Clear this axis of all data sets, deleting any diagram associated with it.
        axis->clearDataSets();
        if (axis->title())
            d->automaticallyHiddenAxisTitles.removeAll(axis->title());
        delete axis;
    }

    // Now find out about things that are in the plotarea style.
    //
    // These things include chart subtype, special things for some
    // chart types like line charts, stock charts, etc.
    //
    // Note that this has to happen BEFORE we create a axis and call
    // there loadOdf method cause the axis will evaluate settings
    // like the PlotArea::isVertical boolean.
    if (plotAreaElement.hasAttributeNS(KoXmlNS::chart, "style-name")) {
        styleStack.clear();
        context.odfLoadingContext().fillStyleStack(plotAreaElement, KoXmlNS::chart, "style-name", "chart");

        styleStack.setTypeProperties("graphic");
        styleStack.setTypeProperties("chart");

        if (styleStack.hasProperty(KoXmlNS::chart, "angle-offset")) {
            bool ok;
            const int angleOffset = styleStack.property(KoXmlNS::chart, "angle-offset").toInt(&ok);
            if (ok)
                setPieAngleOffset(angleOffset);
        }

        // Check for 3D.
        if (styleStack.hasProperty(KoXmlNS::chart, "three-dimensional"))
            setThreeD(styleStack.property(KoXmlNS::chart, "three-dimensional") == "true");
        d->threeDScene = load3dScene(plotAreaElement);

        // Set subtypes stacked or percent.
        // These are valid for Bar, Line, Area and Radar types.
        if (styleStack.hasProperty(KoXmlNS::chart, "percentage")
             && styleStack.property(KoXmlNS::chart, "percentage") == "true")
        {
            setChartSubType(PercentChartSubtype);
        }
        else if (styleStack.hasProperty(KoXmlNS::chart, "stacked")
                  && styleStack.property(KoXmlNS::chart, "stacked") == "true")
        {
            setChartSubType(StackedChartSubtype);
        }

        // Data specific to bar charts
        if (styleStack.hasProperty(KoXmlNS::chart, "vertical"))
            setVertical(styleStack.property(KoXmlNS::chart, "vertical") == "true");

        // Special properties for various chart types
#if 0
        switch () {
        case BarChartType:
            if (styleStack)
                ;
        }
#endif
        styleStack.clear();
        context.odfLoadingContext().fillStyleStack(plotAreaElement, KoXmlNS::chart, "style-name", "chart");
    }

    // Now create and load the axis from the ODF. This needs to happen
    // AFTER we did set some of the basic settings above so the axis
    // can use those basic settings to evaluate it's own settings
    // depending on them. This is especially required for the
    // PlotArea::isVertical() boolean flag else things will go wrong.
    KoXmlElement n;
    forEachElement (n, plotAreaElement) {
        if (n.namespaceURI() != KoXmlNS::chart)
            continue;

        if (n.localName() == "axis") {
            if (!n.hasAttributeNS(KoXmlNS::chart, "dimension"))
                // We have to know what dimension the axis is supposed to be..
                continue;
            const QString dimension = n.attributeNS(KoXmlNS::chart, "dimension", QString());
            AxisDimension dim;
            if      (dimension == "x") dim = XAxisDimension;
            else if (dimension == "y") dim = YAxisDimension;
            else if (dimension == "z") dim = ZAxisDimension;
            else continue;
            Axis *axis = new Axis(this, dim);
            axis->loadOdf(n, context);
        }
    }

    // Two axes are mandatory, check that we have them.
    if (!xAxis()) {
        Axis *xAxis = new Axis(this, XAxisDimension);
        xAxis->setVisible(false);
    }
    if (!yAxis()) {
        Axis *yAxis = new Axis(this, YAxisDimension);
        yAxis->setVisible(false);
    }

    // Now, after the axes, load the datasets.
    // Note that this only contains properties of the datasets, the
    // actual data is not stored here.
    //
    // FIXME: Isn't the proxy model a strange place to store this data?
    proxyModel()->loadOdf(plotAreaElement, context, d->chartType == StockChartType ? 3 : 1, d->chartType);

    // Now load the surfaces (wall and possibly floor)
    // FIXME: Use named tags instead of looping?
    forEachElement (n, plotAreaElement) {
        if (n.namespaceURI() != KoXmlNS::chart)
            continue;

        if (n.localName() == "wall") {
            d->wall->loadOdf(n, context);
        }
        else if (n.localName() == "floor") {
            // The floor is not always present, so allocate it if needed.
            // FIXME: Load floor, even if we don't really support it yet
            // and save it back to ODF.
            //if (!d->floor)
            //    d->floor = new Surface(this);
            //d->floor->loadOdf(n, context);
        }
        else if (d->chartType == StockChartType && n.localName() == "stock-gain-marker") {
            // FIXME
        }
        else if (d->chartType == StockChartType && n.localName() == "stock-loss-marker") {
            // FIXME
        }
        else if (d->chartType == StockChartType && n.localName() == "stock-range-line") {
            if (n.hasAttributeNS(KoXmlNS::chart, "style-name")) {
                styleStack.clear();
                context.odfLoadingContext().fillStyleStack(n, KoXmlNS::chart, "style-name", "chart");

                // stroke-color
                const QString strokeColor = styleStack.property(KoXmlNS::svg, "stroke-color");
                // FIXME: There seem to be no way to set this for the StockChart in KDChart. :-/
                //QPen(QColor(strokeColor));

                // FIXME: svg:stroke-width
            }
        }
        else if (n.localName() != "axis" && n.localName() != "series") {
            qWarning() << "PlotArea::loadOdf(): Unknown tag name " << n.localName();
        }
    }

    requestRepaint();

    return true;
}

void PlotArea::saveOdf(KoShapeSavingContext &context) const
{
    KoXmlWriter &bodyWriter = context.xmlWriter();
    //KoGenStyles &mainStyles = context.mainStyles();
    bodyWriter.startElement("chart:plot-area");

    // FIXME: Somehow this style gets the name gr2 instead of ch2.
    //        Fix that as well.
    KoGenStyle plotAreaStyle(KoGenStyle::ChartAutoStyle, "chart");

    // Data direction
    const Qt::Orientation direction = proxyModel()->dataDirection();
    plotAreaStyle.addProperty("chart:series-source",
                               (direction == Qt::Horizontal)
                               ? "rows" : "columns");
    // Save chart subtype
    saveOdfSubType(bodyWriter, plotAreaStyle);

    bodyWriter.addAttribute("chart:style-name",
                             saveStyle(plotAreaStyle, context));

    const QSizeF s(size());
    const QPointF p(position());
    bodyWriter.addAttributePt("svg:width",  s.width());
    bodyWriter.addAttributePt("svg:height", s.height());
    bodyWriter.addAttributePt("svg:x", p.x());
    bodyWriter.addAttributePt("svg:y", p.y());

    CellRegion cellRangeAddress = d->shape->proxyModel()->cellRangeAddress();
    bodyWriter.addAttribute("table:cell-range-address", cellRangeAddress.toString());

    // About the data:
    //   Save if the first row / column contain headers.
    QString  dataSourceHasLabels;
    if (proxyModel()->firstRowIsLabel()) {
        if (proxyModel()->firstColumnIsLabel())
            dataSourceHasLabels = "both";
        else
            dataSourceHasLabels = "row";
    } else {
        if (proxyModel()->firstColumnIsLabel())
            dataSourceHasLabels = "column";
        else
            dataSourceHasLabels = "none";
    }
    // Note: this is saved in the plotarea attributes and not the style.
    bodyWriter.addAttribute("chart:data-source-has-labels", dataSourceHasLabels);

    if (d->threeDScene) {
        d->threeDScene->saveOdfAttributes(bodyWriter);
    }

    // Done with the attributes, start writing the children.

    // Save the axes.
    foreach(Axis *axis, d->axes) {
        axis->saveOdf(context);
    }

    if (d->threeDScene) {
        d->threeDScene->saveOdfChildren(bodyWriter);
    }

    // Save data series
    d->shape->proxyModel()->saveOdf(context);

    // Save the floor and wall of the plotarea.
    d->wall->saveOdf(context, "chart:wall");
    //if (d->floor)
    //    d->floor->saveOdf(context, "chart:floor");

    bodyWriter.endElement(); // chart:plot-area
}

void PlotArea::saveOdfSubType(KoXmlWriter& xmlWriter,
                               KoGenStyle& plotAreaStyle) const
{
    Q_UNUSED(xmlWriter);

    switch (d->chartType) {
    case BarChartType:
        switch(d->chartSubtype) {
        case NoChartSubtype:
        case NormalChartSubtype:
            break;
        case StackedChartSubtype:
            plotAreaStyle.addProperty("chart:stacked", "true");
            break;
        case PercentChartSubtype:
            plotAreaStyle.addProperty("chart:percentage", "true");
            break;
        }

        if (d->threeD) {
            plotAreaStyle.addProperty("chart:three-dimensional", "true");
        }

        // Data specific to bar charts
        if (d->vertical)
            plotAreaStyle.addProperty("chart:vertical", "true");
        // Don't save this if zero, because that's the default.
        //plotAreaStyle.addProperty("chart:lines-used", 0); // FIXME: for now
        break;

    case LineChartType:
        switch(d->chartSubtype) {
        case NoChartSubtype:
        case NormalChartSubtype:
            break;
        case StackedChartSubtype:
            plotAreaStyle.addProperty("chart:stacked", "true");
            break;
        case PercentChartSubtype:
            plotAreaStyle.addProperty("chart:percentage", "true");
            break;
        }
        if (d->threeD) {
            plotAreaStyle.addProperty("chart:three-dimensional", "true");
            // FIXME: Save all 3D attributes too.
        }
        // FIXME: What does this mean?
        plotAreaStyle.addProperty("chart:symbol-type", "automatic");
        break;

    case AreaChartType:
        switch(d->chartSubtype) {
        case NoChartSubtype:
        case NormalChartSubtype:
            break;
        case StackedChartSubtype:
            plotAreaStyle.addProperty("chart:stacked", "true");
            break;
        case PercentChartSubtype:
            plotAreaStyle.addProperty("chart:percentage", "true");
            break;
        }

        if (d->threeD) {
            plotAreaStyle.addProperty("chart:three-dimensional", "true");
            // FIXME: Save all 3D attributes too.
        }
        break;

    case CircleChartType:
        // FIXME
        break;

    case RingChartType:
        // FIXME
        break;

    case ScatterChartType:
        // FIXME
        break;
    case RadarChartType:
    case FilledRadarChartType:
        // Save subtype of the Radar chart.
        switch(d->chartSubtype) {
        case NoChartSubtype:
        case NormalChartSubtype:
            break;
        case StackedChartSubtype:
            plotAreaStyle.addProperty("chart:stacked", "true");
            break;
        case PercentChartSubtype:
            plotAreaStyle.addProperty("chart:percentage", "true");
            break;
        }
        break;

    case StockChartType:
        switch(d->chartSubtype) {
        case NoChartSubtype:
        case HighLowCloseChartSubtype:
        case OpenHighLowCloseChartSubtype:
            break;
        case CandlestickChartSubtype:
            plotAreaStyle.addProperty("chart:japanese-candle-stick", "true");
            break;
        }
    case BubbleChartType:
    case SurfaceChartType:
    case GanttChartType:
        // FIXME
        break;

        // This is not a valid type, but needs to be handled to avoid
        // a warning from gcc.
    case LastChartType:
    default:
        // FIXME
        break;
    }
}

void PlotArea::setGapBetweenBars(int percent)
{
    d->gapBetweenBars = percent;

    emit gapBetweenBarsChanged(percent);
}

void PlotArea::setGapBetweenSets(int percent)
{
    d->gapBetweenSets = percent;

    emit gapBetweenSetsChanged(percent);
}

void PlotArea::setPieAngleOffset(qreal angle)
{
    d->pieAngleOffset = angle;

    emit pieAngleOffsetChanged(angle);
}

ChartShape *PlotArea::parent() const
{
    // There has to be a valid parent
    Q_ASSERT(d->shape);
    return d->shape;
}

KDChart::CartesianCoordinatePlane *PlotArea::kdCartesianPlane(Axis *axis) const
{
    if (axis) {
        Q_ASSERT(d->axes.contains(axis));
        // Only a secondary y axis gets the secondary plane
        if (axis->dimension() == YAxisDimension && axis != yAxis())
            return d->kdCartesianPlaneSecondary;
    }

    return d->kdCartesianPlanePrimary;
}

KDChart::PolarCoordinatePlane *PlotArea::kdPolarPlane() const
{
    return d->kdPolarPlane;
}

KDChart::RadarCoordinatePlane *PlotArea::kdRadarPlane() const
{
    return d->kdRadarPlane;
}

KDChart::Chart *PlotArea::kdChart() const
{
    return d->kdChart;
}

bool PlotArea::registerKdDiagram(KDChart::AbstractDiagram *diagram)
{
    if (d->kdDiagrams.contains(diagram))
        return false;

    d->kdDiagrams.append(diagram);
    return true;
}

bool PlotArea::deregisterKdDiagram(KDChart::AbstractDiagram *diagram)
{
    if (!d->kdDiagrams.contains(diagram))
        return false;

    d->kdDiagrams.removeAll(diagram);
    return true;
}

void PlotArea::plotAreaUpdate() const
{
    parent()->legend()->update();
    requestRepaint();
    foreach(Axis* axis, d->axes)
        axis->update();

    KoShape::update();
}

void PlotArea::requestRepaint() const
{
    d->pixmapRepaintRequested = true;
}

void PlotArea::paintPixmap(QPainter &painter, const KoViewConverter &converter)
{
    // Adjust the size of the painting area to the current zoom level
    const QSize paintRectSize = converter.documentToView(size()).toSize();
    const QSize plotAreaSize = size().toSize();
    const int borderX = 4;
    const int borderY = 4;

    // Only use a pixmap with sane sizes
    d->paintPixmap = false;//paintRectSize.width() < MAX_PIXMAP_SIZE || paintRectSize.height() < MAX_PIXMAP_SIZE;

    if (d->paintPixmap) {
        d->image = QImage(paintRectSize, QImage::Format_RGB32);

        // Copy the painter's render hints, such as antialiasing
        QPainter pixmapPainter(&d->image);
        pixmapPainter.setRenderHints(painter.renderHints());
        pixmapPainter.setRenderHint(QPainter::Antialiasing, false);

        // scale the painter's coordinate system to fit the current zoom level
        applyConversion(pixmapPainter, converter);

        d->kdChart->paint(&pixmapPainter, QRect(QPoint(borderX, borderY),
                                                QSize(plotAreaSize.width() - 2 * borderX,
                                                      plotAreaSize.height() - 2 * borderY)));
    } else {
        d->kdChart->paint(&painter, QRect(QPoint(borderX, borderY),
                                          QSize(plotAreaSize.width() - 2 * borderX,
                                                plotAreaSize.height() - 2 * borderY)));
    }
}

void PlotArea::paint(QPainter& painter, const KoViewConverter& converter, KoShapePaintingContext &paintContext)
{
    //painter.save();

    // First of all, scale the painter's coordinate system to fit the current zoom level
    applyConversion(painter, converter);

    // Calculate the clipping rect
    QRectF paintRect = QRectF(QPointF(0, 0), size());
    painter.setClipRect(paintRect, Qt::IntersectClip);

    // Paint the background
    if (background()) {
        QPainterPath p;
        p.addRect(paintRect);
        background()->paint(painter, converter, paintContext, p);
    }

    // Get the current zoom level
    QPointF zoomLevel;
    converter.zoom(&zoomLevel.rx(), &zoomLevel.ry());

    // Only repaint the pixmap if it is scheduled, the zoom level
    // changed or the shape was resized.
    /*if (   d->pixmapRepaintRequested
         || d->lastZoomLevel != zoomLevel
         || d->lastSize      != size()
         || !d->paintPixmap) {
        // TODO (js): What if two zoom levels are constantly being
        //            requested?  At the moment, this *is* the case,
        //            due to the fact that the shape is also rendered
        //            in the page overview in KPresenter Every time
        //            the window is hidden and shown again, a repaint
        //            is requested --> laggy performance, especially
        //            when quickly switching through windows.
        //
        // ANSWER (iw): what about having a small mapping between size
        //              in pixels and pixmaps?  The size could be 2 or
        //              at most 3.  We could manage the replacing
        //              using LRU.
        paintPixmap(painter, converter);
        d->pixmapRepaintRequested = false;
        d->lastZoomLevel = zoomLevel;
        d->lastSize      = size();
    }*/
    painter.setRenderHint(QPainter::Antialiasing, false);

    // KDChart thinks in pixels, Calligra in pt
    ScreenConversions::scaleFromPtToPx(painter);

    // Only paint the actual chart if there is a certain minimal size,
    // because otherwise kdchart will crash.
    QRect kdchartRect = ScreenConversions::scaleFromPtToPx(paintRect);
    // Turn off clipping so that border (or "frame") drawn by KDChart::Chart
    // is not not cut off.
    painter.setClipping(false);
    if (kdchartRect.width() > 10 && kdchartRect.height() > 10) {
        d->kdChart->paint(&painter, kdchartRect);
    }
    //painter.restore();

    // Paint the cached pixmap if we got a GO from paintPixmap()
    //if (d->paintPixmap)
    //    painter.drawImage(0, 0, d->image);
}

void PlotArea::relayout() const
{
    d->kdCartesianPlanePrimary->relayout();
    d->kdCartesianPlaneSecondary->relayout();
    d->kdPolarPlane->relayout();
    d->kdRadarPlane->relayout();
    update();
}

#include "PlotArea.moc"

