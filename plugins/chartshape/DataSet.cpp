/* This file is part of the KDE project

   Copyright 2007-2008 Johannes Simon <johannes.simon@gmail.com>
   Copyright 2008-2009 Inge Wallin    <inge@lysator.liu.se>
   Copyright (C) 2010 Carlos Licea    <carlos@kdab.com>
   Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
     Contact: Suresh Chande suresh.chande@nokia.com

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
 * Boston, MA 02110-1301, USA.
*/


// Own
#include "DataSet.h"

// Qt
#include <QAbstractItemModel>
#include <QString>
#include <QPen>
#include <QColor>
#include <QPainter>

// KDE
#include <KLocale>

// KDChart
#include <KDChartDataValueAttributes>
#include <KDChartPieAttributes>
#include <KDChartTextAttributes>
#include <KDChartRelativePosition>
#include <KDChartPosition>
#include <KDChartAbstractDiagram>
#include <KDChartMeasure>
#include "KDChartModel.h"

// KChart
#include "Axis.h"
#include "PlotArea.h"
#include "Surface.h"
#include "OdfLoadingHelper.h"

// Calligra
#include <KoXmlNS.h>
#include <KoOdfGraphicStyles.h>
#include <KoGenStyle.h>
#include <KoXmlReader.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoOdfWorkaround.h>
#include <KoGenStyle.h>
#include <KoGenStyles.h>
#include <KoXmlWriter.h>

using namespace KChart;

const int numDefaultMarkerTypes = 15;

// These are the values that are defined for chart:symbol-name
// Ref: ODF 1.2 20.54 chart:symbol-name, page 716
const QByteArray symbolNames[] = {
    "square", 
    "diamond", 
    "arrow-down",
    "arrow-up", 
    "arrow-right", 
    "arrow-left", 
    "bow-tie", 
    "hourglass", 
    "circle", 
    "star", 
    "x", 
    "plus",
    "asterisk", 
    "horizontal-bar", 
    "vertical-bar"
};

const KDChart::MarkerAttributes::MarkerStyle defaultMarkerTypes[]= { 
    KDChart::MarkerAttributes::MarkerSquare,     // 0
    KDChart::MarkerAttributes::MarkerDiamond,    // 1
    KDChart::MarkerAttributes::MarkerArrowDown,  // 2
    KDChart::MarkerAttributes::MarkerArrowUp,    // 3
    KDChart::MarkerAttributes::MarkerArrowRight, // 4
    KDChart::MarkerAttributes::MarkerArrowLeft,  // 5
    KDChart::MarkerAttributes::MarkerBowTie,     // 6
    KDChart::MarkerAttributes::MarkerHourGlass,  // 7
    KDChart::MarkerAttributes::MarkerCircle,     // 8
    KDChart::MarkerAttributes::MarkerStar,       // 9
    KDChart::MarkerAttributes::MarkerX,          // 10
    KDChart::MarkerAttributes::MarkerCross,      // 11
    KDChart::MarkerAttributes::MarkerAsterisk,   // 12
    KDChart::MarkerAttributes::MarkerHorizontalBar,// 13
    KDChart::MarkerAttributes::MarkerVerticalBar, // 14

    // Not used:
    KDChart::MarkerAttributes::MarkerRing,
    KDChart::MarkerAttributes::MarkerFastCross,
    KDChart::MarkerAttributes::Marker1Pixel,
    KDChart::MarkerAttributes::Marker4Pixels,
    KDChart::MarkerAttributes::NoMarker
};

static KDChart::MarkerAttributes::MarkerStyle odf2kdMarker(OdfMarkerStyle style);

class DataSet::Private
{
public:
    Private(DataSet *parent, int dataSetNr);
    ~Private();

    void         updateSize();
    bool         hasOwnChartType() const;
    ChartType    effectiveChartType() const;
    bool         isValidDataPoint(const QPoint &point) const;
    QVariant     data(const CellRegion &region, int index, int role) const;
    QString      formatData(const CellRegion &region, int index, int role) const;

    QBrush defaultBrush() const;
    QBrush defaultBrush(int section) const;

    KDChart::MarkerAttributes defaultMarkerAttributes() const;

    // Returns an instance of DataValueAttributes with sane default values in
    // relation to KChart
    KDChart::DataValueAttributes defaultDataValueAttributes() const;
    /// Copies Private::dataValueAttributes to this section if it doesn't
    /// have its own DataValueAttributes copy yet.
    void insertDataValueAttributeSectionIfNecessary(int section);

    /**
     * FIXME: Refactor (post-2.3)
     *        1) Maximum bubble width should be determined in ChartProxyModel
     *        2) Actual marker size and other KDChart::MarkerAttributes should
     *           be set by some kind of adapter for KD Chart, e.g. KDChartModel.
     *
     * This determines the maximum bubble size of *all* data points in
     * the diagram this data set belongs to so that the actual value used to
     * draw the bubbles is relative to this value.
     *
     * For more info on how bubble sizes are calculated, see
     * http://qa.openoffice.org/issues/show_bug.cgi?id=64689
     */
    qreal maxBubbleSize() const;

    QPen defaultPen() const;

    void dataChanged(KDChartModel::DataRole role, const QRect &rect) const;
    void setAttributesAccordingToType();

    DataSet      *parent;

    ChartType     chartType;
    ChartSubtype  chartSubType;

    Axis *attachedAxis;
    bool showMeanValue;
    QPen meanValuePen;
    bool showLowerErrorIndicator;
    bool showUpperErrorIndicator;
    QPen errorIndicatorPen;
    ErrorCategory errorCategory;
    qreal errorPercentage;
    qreal errorMargin;
    qreal lowerErrorLimit;
    qreal upperErrorLimit;
    // Determines whether pen has been set
    bool penIsSet;
    // Determines whether brush has been set
    bool brushIsSet;
    // Determines whether marker has been set automatically
    bool markerIsAutoSet;
    QPen pen;
    QBrush brush;
    QMap<int, DataSet::ValueLabelType> valueLabelType;

    KDChart::PieAttributes pieAttributes;
    KDChart::DataValueAttributes dataValueAttributes;

    void readValueLabelType(KoStyleStack &styleStack, int section = -1);

    // Note: Set section-specific attributes only if really necessary.
    //       They will override the respective global attributes.
    QMap<int, QPen> pens;
    QMap<int, QBrush> brushes;
    QMap<int, KDChart::PieAttributes> sectionsPieAttributes;
    QMap<int, KDChart::DataValueAttributes> sectionsDataValueAttributes;

    /// The number of this series is passed in the constructor and after
    /// that never changes.
    const int num;

    // The different CellRegions for a dataset
    // Note: These are all 1-dimensional, i.e. vectors.
    CellRegion labelDataRegion; // one cell that holds the label
    CellRegion yDataRegion;     // normal y values
    CellRegion xDataRegion;     // x values -- only for scatter & bubble charts
    CellRegion customDataRegion;// used for bubble width in bubble charts
    // FIXME: Remove category region from DataSet - this is not the place
    // it belongs to.
    CellRegion categoryDataRegion; // x labels -- same for all datasets

    KDChartModel *kdChartModel;

    int size;

    /// Used if no data region for the label is specified
    const QString defaultLabel;
    bool symbolsActivated;
    int symbolID;
    int loadedDimensions;

    KoOdfNumberStyles::NumericStyleFormat *numericStyleFormat;
};

DataSet::Private::Private(DataSet *parent, int dataSetNr) :
    parent(parent),
    chartType(LastChartType),
    chartSubType(NoChartSubtype),
    attachedAxis(0),
    showMeanValue(false),
    showLowerErrorIndicator(false),
    showUpperErrorIndicator(false),
    errorPercentage(0.0),
    errorMargin(0.0),
    lowerErrorLimit(0.0),
    upperErrorLimit(0.0),
    penIsSet(false),
    brushIsSet(false),
    markerIsAutoSet(true),
    pen(QPen(Qt::black)),
    brush(QColor(Qt::white)),
    dataValueAttributes(defaultDataValueAttributes()),
    num(dataSetNr),
    kdChartModel(0),
    size(0),
    defaultLabel(i18n("Series %1", dataSetNr + 1)),
    symbolsActivated(true),
    symbolID(0),
    loadedDimensions(0),
    numericStyleFormat(0)
{
}

DataSet::Private::~Private()
{
    delete numericStyleFormat;
}

KDChart::MarkerAttributes DataSet::Private::defaultMarkerAttributes() const
{
    KDChart::MarkerAttributes ma;
    // Don't show markers unless we turn them on
    ma.setVisible(false);
    // The marker size is specified in pixels, but scaled by the painter's zoom level
    ma.setMarkerSizeMode(KDChart::MarkerAttributes::AbsoluteSizeScaled);
    return ma;
}

KDChart::DataValueAttributes DataSet::Private::defaultDataValueAttributes() const
{
    KDChart::DataValueAttributes attr;
    KDChart::TextAttributes textAttr = attr.textAttributes();
    // Don't show value labels by default
    textAttr.setVisible(false);
    KDChart::Measure fontSize = textAttr.fontSize();
    attr.setMarkerAttributes(defaultMarkerAttributes());
    fontSize.setValue(10);
    // Don't change font size with chart size
    fontSize.setCalculationMode(KDChartEnums::MeasureCalculationModeAbsolute);
    textAttr.setFontSize(fontSize);
    // Draw text horizontally
    textAttr.setRotation(0);
    attr.setTextAttributes(textAttr);
    // Set positive value position
    KDChart::RelativePosition positivePosition = attr.positivePosition();
    if (chartType ==  KChart::BarChartType && chartSubType != KChart::NormalChartSubtype) {
        positivePosition.setAlignment(Qt::AlignCenter);
        positivePosition.setReferencePosition(KDChartEnums::PositionCenter);
    }
    else if (chartType ==  KChart::BarChartType && chartSubType == KChart::NormalChartSubtype) {
        positivePosition.setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        positivePosition.setReferencePosition(KDChartEnums::PositionNorth);
    }
    else {
        positivePosition.setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        positivePosition.setReferencePosition(KDChartEnums::PositionNorthWest);
    }
    positivePosition.setHorizontalPadding(0.0);
    positivePosition.setVerticalPadding(-100.0);
    attr.setPositivePosition(positivePosition);

    // Set negative value position
    KDChart::RelativePosition negativePosition = attr.negativePosition();
    if (chartType ==  KChart::BarChartType && chartSubType != KChart::NormalChartSubtype) {
        negativePosition.setAlignment(Qt::AlignCenter);
        negativePosition.setReferencePosition(KDChartEnums::PositionCenter);
    }
    else if (chartType ==  KChart::BarChartType && chartSubType == KChart::NormalChartSubtype) {
        negativePosition.setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
        negativePosition.setReferencePosition(KDChartEnums::PositionSouth);
    }
    else {
        negativePosition.setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
        negativePosition.setReferencePosition(KDChartEnums::PositionSouthWest);
    }
    negativePosition.setHorizontalPadding(0.0);
    negativePosition.setVerticalPadding(100.0);
    attr.setNegativePosition(negativePosition);

    // No decimal digits by default
    attr.setDecimalDigits(0);
    // Show all values, even if they overlap
    attr.setShowOverlappingDataLabels(true);
    // Yes, data point labels can repeatedly have the same text. (e.g. the same value)
    attr.setShowRepetitiveDataLabels(true);

    attr.setVisible(true);

    return attr;
}

void DataSet::Private::insertDataValueAttributeSectionIfNecessary(int section)
{
    if (!sectionsDataValueAttributes.contains(section))
        sectionsDataValueAttributes[section] = dataValueAttributes;
}

void DataSet::Private::updateSize()
{
    int newSize = 0;
    newSize = qMax(newSize, xDataRegion.cellCount());
    newSize = qMax(newSize, yDataRegion.cellCount());
    newSize = qMax(newSize, customDataRegion.cellCount());
    newSize = qMax(newSize, categoryDataRegion.cellCount());

    if (size != newSize) {
        size = newSize;
        if (kdChartModel)
            kdChartModel->dataSetSizeChanged(parent, size);
    }
}

bool DataSet::Private::hasOwnChartType() const
{
    return chartType != LastChartType;
}


/**
 * Returns the effective chart type of this data set, i.e.
 * returns the chart type of the diagram this data set is
 * attached to if no chart type is set, or otherwise this data
 * set's chart type.
 */
ChartType DataSet::Private::effectiveChartType() const
{
    if (hasOwnChartType())
        return chartType;

    Q_ASSERT(attachedAxis);
    return attachedAxis->plotArea()->chartType();
}

bool DataSet::Private::isValidDataPoint(const QPoint &point) const
{
    if (point.y() < 0 || point.x() < 0)
        return false;

    // We can't point to horizontal and vertical header data at the same time
    if (point.x() == 0 && point.y() == 0)
        return false;

    return true;
}

QVariant DataSet::Private::data(const CellRegion &region, int index, int role) const
{
    if (!region.isValid())
        return QVariant();
    if (!region.hasPointAtIndex(index))
        return QVariant();

    // Convert the given index in this dataset to a data point in the
    // source model.
    QPoint dataPoint = region.pointAtIndex(index);
    Table *table = region.table();
    Q_ASSERT(table);
    QAbstractItemModel *model = table->model();
    // This means the table the region lies in has been removed, but nobody
    // has changed the region in the meantime. That is a perfectly valid
    // scenario, so just return invalid data.
    if (!model)
        return QVariant();

    // Check if the data point is valid
    const bool validDataPoint = isValidDataPoint(dataPoint);

    // Remove, since it makes kspread crash when inserting a chart for
    // a 1x1 cell region.
    //Q_ASSERT(validDataPoint);
    if (!validDataPoint)
        return QVariant();

    // The top-left point is (1,1). (0,y) or (x,0) refers to header data.
    const bool verticalHeaderData   = dataPoint.x() == 0;
    const bool horizontalHeaderData = dataPoint.y() == 0;
    const int row = dataPoint.y() - 1;
    const int col = dataPoint.x() - 1;

    QVariant data;
    if (verticalHeaderData)
        data = model->headerData(row, Qt::Vertical, role);
    else if (horizontalHeaderData)
        data = model->headerData(col, Qt::Horizontal, role);
    else {
        const QModelIndex &index = model->index(row, col);
        //Q_ASSERT(index.isValid());
        if (index.isValid())
            data = model->data(index, role);
    }
    return data;
}

QString DataSet::Private::formatData(const CellRegion &region, int index, int role) const
{
    QVariant v = data(region, index, role);
    QString s;
    if (v.type() == QVariant::Double) {
        // Don't use v.toString() else a double/float would lose precision
        // and something like "36.5207" would become "36.520660888888912".
        QTextStream ts(&s);
        //ts.setRealNumberNotation(QTextStream::FixedNotation);
        //ts.setRealNumberPrecision();
        ts << v.toDouble();
    } else {
        s = v.toString();
    }
    return numericStyleFormat ? KoOdfNumberStyles::format(s, *numericStyleFormat) : s;
}

QBrush DataSet::Private::defaultBrush() const
{
    Qt::Orientation modelDataDirection = kdChartModel->dataDirection();
    // A data set-wide default brush only makes sense if the legend shows
    // data set labels, not the category data. See notes on data directions
    // in KDChartModel.h for details.
    if (modelDataDirection == Qt::Vertical)
        return defaultDataSetColor(num);
    // FIXME: What to return in the other case?
    return QBrush();
}

QBrush DataSet::Private::defaultBrush(int section) const
{
    Qt::Orientation modelDataDirection = kdChartModel->dataDirection();
    // Horizontally aligned diagrams have a specific color per category
    // See for example pie or ring charts. A pie chart contains a single
    // data set, but the slices default to different brushes.
    if (modelDataDirection == Qt::Horizontal)
        return defaultDataSetColor(section);
    // Vertically aligned diagrams default to one brush per data set
    return defaultBrush();
}

QPen DataSet::Private::defaultPen() const
{
    QPen pen(Qt::black);
    ChartType chartType = effectiveChartType();
    if (chartType == LineChartType ||
         chartType == ScatterChartType) {
        if (penIsSet) {
            pen = pen;
        } else {
            pen = QPen(defaultDataSetColor(num));
        }
    }
    return pen;
}


DataSet::DataSet(int dataSetNr)
    : d(new Private(this, dataSetNr))
{
    Q_ASSERT(dataSetNr >= 0);
}

DataSet::~DataSet()
{
    if (d->attachedAxis)
        d->attachedAxis->detachDataSet(this, true);

    delete d;
}


ChartType DataSet::chartType() const
{
    return d->chartType;
}

ChartSubtype DataSet::chartSubType() const
{
    return d->chartSubType;
}

Axis *DataSet::attachedAxis() const
{
    return d->attachedAxis;
}

bool DataSet::showMeanValue() const
{
    return d->showMeanValue;
}

QPen DataSet::meanValuePen() const
{
    return d->meanValuePen;
}

bool DataSet::showLowerErrorIndicator() const
{
    return d->showLowerErrorIndicator;
}

bool DataSet::showUpperErrorIndicator() const
{
    return d->showUpperErrorIndicator;
}

QPen DataSet::errorIndicatorPen() const
{
    return d->errorIndicatorPen;
}

ErrorCategory DataSet::errorCategory() const
{
    return d->errorCategory;
}

qreal DataSet::errorPercentage() const
{
    return d->errorPercentage;
}

qreal DataSet::errorMargin() const
{
    return d->errorMargin;
}

qreal DataSet::lowerErrorLimit() const
{
    return d->lowerErrorLimit;
}

qreal DataSet::upperErrorLimit() const
{
    return d->upperErrorLimit;
}

#include <QDebug>
void DataSet::Private::setAttributesAccordingToType()
{
    KDChart::DataValueAttributes attr = dataValueAttributes;
    KDChart::RelativePosition positivePosition = attr.positivePosition();
    if (chartType ==  KChart::BarChartType && chartSubType != KChart::NormalChartSubtype) {
        positivePosition.setAlignment(Qt::AlignCenter);
        positivePosition.setReferencePosition(KDChartEnums::PositionCenter);
    }
    else if (chartType ==  KChart::BarChartType && chartSubType == KChart::NormalChartSubtype) {
        positivePosition.setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        positivePosition.setReferencePosition(KDChartEnums::PositionNorth);
    }
    else {
        positivePosition.setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        positivePosition.setReferencePosition(KDChartEnums::PositionNorthWest);
    }
    positivePosition.setHorizontalPadding(0.0);
    positivePosition.setVerticalPadding(-100.0);
    attr.setPositivePosition(positivePosition);

    // Set negative value position
    KDChart::RelativePosition negativePosition = attr.negativePosition();
    if (chartType ==  KChart::BarChartType && chartSubType != KChart::NormalChartSubtype) {
        negativePosition.setAlignment(Qt::AlignCenter);
        negativePosition.setReferencePosition(KDChartEnums::PositionCenter);
    }
    else if (chartType ==  KChart::BarChartType && chartSubType == KChart::NormalChartSubtype) {
        negativePosition.setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
        negativePosition.setReferencePosition(KDChartEnums::PositionSouth);
    }
    else {
        negativePosition.setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
        negativePosition.setReferencePosition(KDChartEnums::PositionSouthWest);
    }
    negativePosition.setHorizontalPadding(0.0);
    negativePosition.setVerticalPadding(100.0);
    attr.setNegativePosition(negativePosition);
    dataValueAttributes = attr;

    for (int i = 0; i < sectionsDataValueAttributes.count(); ++i) {
        KDChart::DataValueAttributes attr = sectionsDataValueAttributes[i];
        KDChart::RelativePosition positivePosition = attr.positivePosition();
        if (chartType ==  KChart::BarChartType && chartSubType != KChart::NormalChartSubtype) {
            positivePosition.setAlignment(Qt::AlignCenter);
            positivePosition.setReferencePosition(KDChartEnums::PositionCenter);
        }
        else if (chartType ==  KChart::BarChartType && chartSubType == KChart::NormalChartSubtype) {
            positivePosition.setAlignment(Qt::AlignHCenter | Qt::AlignTop);
            positivePosition.setReferencePosition(KDChartEnums::PositionNorth);
        }
        else {
            positivePosition.setAlignment(Qt::AlignHCenter | Qt::AlignTop);
            positivePosition.setReferencePosition(KDChartEnums::PositionNorthWest);
        }
        positivePosition.setHorizontalPadding(0.0);
        positivePosition.setVerticalPadding(-100.0);
        attr.setPositivePosition(positivePosition);

        // Set negative value position
        KDChart::RelativePosition negativePosition = attr.negativePosition();
        if (chartType ==  KChart::BarChartType && chartSubType != KChart::NormalChartSubtype) {
            negativePosition.setAlignment(Qt::AlignCenter);
            negativePosition.setReferencePosition(KDChartEnums::PositionCenter);
        }
        else if (chartType == KChart::BarChartType && chartSubType == KChart::NormalChartSubtype) {
            negativePosition.setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
            negativePosition.setReferencePosition(KDChartEnums::PositionSouth);
        }
        else {
            negativePosition.setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
            negativePosition.setReferencePosition(KDChartEnums::PositionSouthWest);
        }
        negativePosition.setHorizontalPadding(0.0);
        negativePosition.setVerticalPadding(100.0);
        attr.setNegativePosition(negativePosition);
        sectionsDataValueAttributes[i] = attr;
    }
}


void DataSet::setChartType(ChartType type)
{
    if (type == d->chartType)
        return;

    Axis  *axis = d->attachedAxis;
    if (axis)
        axis->detachDataSet(this);

    d->chartType = type;
    d->setAttributesAccordingToType();

    if (axis)
        axis->attachDataSet(this);    
}

void DataSet::setChartSubType(ChartSubtype subType)
{
    if (subType == d->chartSubType)
        return;

    Axis *axis = d->attachedAxis;
    axis->detachDataSet(this);

    d->chartSubType = subType;
    d->setAttributesAccordingToType();

    axis->attachDataSet(this);
}


void DataSet::setAttachedAxis(Axis *axis)
{
    d->attachedAxis = axis;
}

QPen DataSet::pen() const
{
    return d->penIsSet ? d->pen : d->defaultPen();
}

QBrush DataSet::brush() const
{
    return d->brushIsSet ? d->brush : d->defaultBrush();
}

OdfMarkerStyle DataSet::markerStyle() const
{
    OdfMarkerStyle style = (OdfMarkerStyle)(d->symbolID);
    return style;
}

QIcon DataSet::markerIcon(OdfMarkerStyle markerStyle)
{
    if (markerStyle != NoMarker) {
        QPixmap *markerPixmap = new QPixmap(16,16);
        markerPixmap->fill(QColor(255,255,255,0));
        QPainter *painter = new QPainter(markerPixmap);
        KDChart::MarkerAttributes matt;
        matt.setMarkerStyle(odf2kdMarker(markerStyle));
        KDChart::AbstractDiagram::paintMarker(painter, matt, brush(), pen(), QPointF(7,7), QSizeF(12,12));
        QIcon markerIcon = QIcon(*markerPixmap);
        return markerIcon;
    }
    return QIcon();
}

QPen DataSet::pen(int section) const
{
    if (d->pens.contains(section))
        return d->pens[section];
    return pen();
}

KDChart::PieAttributes DataSet::pieAttributes() const
{
    return d->pieAttributes;
}

QBrush DataSet::brush(int section) const
{
    Qt::Orientation modelDataDirection = d->kdChartModel->dataDirection();
    // Horizontally aligned diagrams have a specific color per category
    // See for example pie or ring charts. A pie chart contains a single
    // data set, but the slices must have different brushes.
    if (modelDataDirection == Qt::Horizontal) {
        if (d->brushes.contains(section)) {
            return d->brushes[section];
        }
        return d->defaultBrush(section);
    }
    // Vertically aligned diagrams only have one brush per data set
    return brush();
}

KDChart::PieAttributes DataSet::pieAttributes(int section) const
{
    if(d->sectionsPieAttributes.contains(section))
        return d->sectionsPieAttributes[section];
    return pieAttributes();
}

qreal DataSet::Private::maxBubbleSize() const
{
    // TODO: Improve performance by caching. This is currently O(n^2).
    // this is not in O(n^2), its quite linear on the number of datapoints
    // hoever it could be constant for any then the first case by implementing
    // cashing
    qreal max = 0.0;
    Q_ASSERT(kdChartModel);
    QList<DataSet*> dataSets = kdChartModel->dataSets();
    foreach(DataSet *dataSet, dataSets)
        for (int i = 0; i < dataSet->size(); i++)
            max = qMax(max, dataSet->customData(i).toReal());
    return max;
}

KDChart::DataValueAttributes DataSet::dataValueAttributes(int section /* = -1 */) const
{
    KDChart::DataValueAttributes attr(d->dataValueAttributes);
    Q_ASSERT(attr.isVisible() == d->dataValueAttributes.isVisible());
    if (d->sectionsDataValueAttributes.contains(section))
        attr = d->sectionsDataValueAttributes[section];

    /*
     * Update attributes that are related to properties out of the data
     * sets's reach and thus might have changed in the meanwhile.
     */
    KDChart::MarkerAttributes ma(attr.markerAttributes());

    // The chart type is a property of the plot area, check that.
    switch (d->effectiveChartType()) {
//     case ScatterChartType:
//         // TODO: Marker type should be customizable
//         // TODO: Marker size should be customizable
//         ma.setMarkerStyle(defaultMarkerTypes[number() % numDefaultMarkerTypes]);
//         ma.setVisible(true);
//         break;
    case BarChartType:
    case CircleChartType:
    case RingChartType:
    case StockChartType:
    {
        Q_ASSERT(attr.isVisible());
        ma.setMarkerStyle(KDChart::MarkerAttributes::MarkerSquare);
        ma.setMarkerSize(QSize(10, 10));
        ma.setVisible(true);
        d->symbolsActivated = false;
        break;
    }
    case BubbleChartType:
    {
        Q_ASSERT(attachedAxis());
        Q_ASSERT(attachedAxis()->plotArea());
        ma.setMarkerStyle(KDChart::MarkerAttributes::MarkerCircle);        
        ma.setThreeD(attachedAxis()->plotArea()->isThreeD());
        qreal maxSize = d->maxBubbleSize();
        if (section >= 0) {
            qreal bubbleWidth = customData(section).toReal();
            // All bubble sizes are relative to the maximum bubble size
            if (maxSize != 0.0)
                bubbleWidth /= maxSize;
            // Whereas the maximum size is relative to 1/4 * min(dw, dh),
            // with dw, dh being the width and height of the diagram
            bubbleWidth *= 0.25;
            ma.setMarkerSizeMode(KDChart::MarkerAttributes::RelativeToDiagramWidthHeightMin);
            ma.setMarkerSize(QSizeF(bubbleWidth, bubbleWidth));
        }
        ma.setVisible(true);
        d->symbolsActivated = false;
        break;
    }
    default:
        // TODO: Make markers customizable even for other types
        Q_ASSERT(attr.isVisible());
        ma.setMarkerStyle(odf2kdMarker((OdfMarkerStyle)d->symbolID));
        ma.setMarkerSize(QSize(10, 10));
        ma.setVisible(true);
        d->symbolsActivated = true;
//             attr.setVisible(true);
        // Do not overwrite visibility in this case. It could very well have
        // been set to 'visible' on purpose by e.g. loadOdf().
        //else
        //    ma.setVisible(false);
        break;
    }

    ma.setMarkerColor(brush(section).color());
    ma.setPen(pen(section));

    QString dataLabel = ""; // initialize with empty and NOT null!
    ValueLabelType type = valueLabelType(section);
    if (type.symbol) {
        //TODO is that correct?
        ma.setVisible(true);
    }
    if (type.category) {
        QString s = categoryData(section, Qt::DisplayRole).toString().trimmed();
        if (!s.isEmpty()) dataLabel += s + " ";
    }
    if (type.number) {
        QString s = d->formatData(d->yDataRegion, section, Qt::DisplayRole);
        if (!s.isEmpty()) dataLabel += s + " ";
    }
    if (type.percentage) {
        bool ok;
        qreal value = yData(section, Qt::EditRole).toDouble(&ok);
        if (ok) {
            qreal sum = 0.0;
            for(int i = 0; i < d->yDataRegion.cellCount(); ++i) {
                sum += yData(i, Qt::EditRole).toDouble();
            }
            if (sum == 0.0)
                ok = false;
            else
                value = value / sum * 100.0;
        }
        if (ok)
            dataLabel += QString::number(value, 'f', 0) + "% ";
    }
    attr.setDataLabel(dataLabel.trimmed());

    attr.setMarkerAttributes(ma);

    return attr;
}

KDChart::MarkerAttributes DataSet::getMarkerAttributes(int section, bool *success) const
{
    KDChart::DataValueAttributes attr(d->dataValueAttributes);
    Q_ASSERT(attr.isVisible() == d->dataValueAttributes.isVisible());
    if (d->sectionsDataValueAttributes.contains(section))
        attr = d->sectionsDataValueAttributes[section];

    KDChart::MarkerAttributes ma(attr.markerAttributes());
    ma.setMarkerStyle(odf2kdMarker((OdfMarkerStyle)d->symbolID));
    ma.setMarkerSize(QSize(10, 10));
    ma.setVisible(true);

    return ma;
}

bool DataSet::markerAutoSet() const
{
    return d->markerIsAutoSet;
}

void DataSet::setMarkerAttributes(const KDChart::MarkerAttributes &attribs, int section)
{
    KDChart::DataValueAttributes attr(d->dataValueAttributes);
    Q_ASSERT(attr.isVisible() == d->dataValueAttributes.isVisible());
    if (d->sectionsDataValueAttributes.contains(section))
        attr = d->sectionsDataValueAttributes[section];

    attr.setMarkerAttributes(attribs);
    d->dataValueAttributes = attr;
}

void DataSet::setAutoMarker(bool isAuto)
{
    d->markerIsAutoSet = isAuto;
}

void DataSet::setPen(const QPen &pen)
{
    d->pen = pen;
    d->penIsSet = true;
    if (d->kdChartModel)
        d->kdChartModel->dataSetChanged(this);
//     KDChart::MarkerAttributes ma(d->dataValueAttributes.markerAttributes());
//     ma.setPen(pen);
//     d->dataValueAttributes.setMarkerAttributes(ma);
//     for (QMap< int, KDChart::DataValueAttributes >::iterator it = d->sectionsDataValueAttributes.begin();
//           it != d->sectionsDataValueAttributes.end(); ++it){
//         KDChart::MarkerAttributes mattr(it->markerAttributes());
//         mattr.setMarkerColor(pen.color());
//         it->setMarkerAttributes(mattr);
//     }
    
}

void DataSet::setBrush(const QBrush &brush)
{
    d->brush = brush;
    d->brushIsSet = true;
    if (d->kdChartModel)
        d->kdChartModel->dataSetChanged(this);
//     KDChart::MarkerAttributes ma(d->dataValueAttributes.markerAttributes());
//     ma.setMarkerColor(brush.color());
//     d->dataValueAttributes.setMarkerAttributes(ma);
//     for (QMap< int, KDChart::DataValueAttributes >::iterator it = d->sectionsDataValueAttributes.begin();
//           it != d->sectionsDataValueAttributes.end(); ++it){
//         KDChart::MarkerAttributes mattr(it->markerAttributes());
//         mattr.setMarkerColor(brush.color());
//         it->setMarkerAttributes(mattr);
//     }
}

void DataSet::setPieExplodeFactor(int factor)
{
    d->pieAttributes.setExplodeFactor((qreal)factor / (qreal)100);
    if(d->kdChartModel)
        d->kdChartModel->dataSetChanged(this);
}

void DataSet::setPen(int section, const QPen &pen)
{
    d->pens[section] = pen;
    if (d->kdChartModel)
        d->kdChartModel->dataSetChanged(this, KDChartModel::PenDataRole, section);
    d->insertDataValueAttributeSectionIfNecessary(section);
//     KDChart::MarkerAttributes mas(d->sectionsDataValueAttributes[section].markerAttributes());
//     mas.setPen(pen);
//     d->sectionsDataValueAttributes[section].setMarkerAttributes(mas);
}

void DataSet::setBrush(int section, const QBrush &brush)
{
    d->brushes[section] = brush;
    if (d->kdChartModel)
        d->kdChartModel->dataSetChanged(this, KDChartModel::BrushDataRole, section);
    d->insertDataValueAttributeSectionIfNecessary(section);
//     KDChart::MarkerAttributes mas(d->sectionsDataValueAttributes[section].markerAttributes());
//     mas.setMarkerColor(brush.color());
//     d->sectionsDataValueAttributes[section].setMarkerAttributes(mas);
}

void DataSet::setMarkerStyle(OdfMarkerStyle style)
{
    KDChart::MarkerAttributes matt = getMarkerAttributes();
    matt.setMarkerStyle(odf2kdMarker(style));
    setMarkerAttributes(matt);

    d->symbolsActivated = true;
    d->symbolID = style;
}

void DataSet::setPieExplodeFactor(int section, int factor)
{
    KDChart::PieAttributes &pieAttributes = d->sectionsPieAttributes[section];
    pieAttributes.setExplodeFactor((qreal)factor / (qreal)100);
    if (d->kdChartModel)
        d->kdChartModel->dataSetChanged(this, KDChartModel::PieAttributesRole, section);
}

int DataSet::number() const
{
    return d->num;
}

void DataSet::setShowMeanValue(bool show)
{
    d->showMeanValue = show;
}

void DataSet::setMeanValuePen(const QPen &pen)
{
    d->meanValuePen = pen;
}

void DataSet::setShowLowerErrorIndicator(bool show)
{
    d->showLowerErrorIndicator = show;
}

void DataSet::setShowUpperErrorIndicator(bool show)
{
    d->showUpperErrorIndicator = show;
}

void DataSet::setShowErrorIndicators(bool lower, bool upper)
{
    setShowLowerErrorIndicator(lower);
    setShowUpperErrorIndicator(upper);
}

void DataSet::setErrorIndicatorPen(const QPen &pen)
{
    d->errorIndicatorPen = pen;
}

void DataSet::setErrorCategory(ErrorCategory category)
{
    d->errorCategory = category;
}

void DataSet::setErrorPercentage(qreal percentage)
{
    d->errorPercentage = percentage;
}

void DataSet::setErrorMargin(qreal margin)
{
    d->errorMargin = margin;
}

void DataSet::setLowerErrorLimit(qreal limit)
{
    d->lowerErrorLimit = limit;
}

void DataSet::setUpperErrorLimit(qreal limit)
{
    d->upperErrorLimit = limit;
}

QVariant DataSet::xData(int index, int role) const
{
    // Sometimes a bubble chart is created with a table with 4 columns.
    // What we do here is assign the 2 columns per data set, so we have
    // 2 data sets in total afterwards. The first column is y data, the second
    // bubble width. Same for the second data set. So there is nothing left
    // for x data. Instead use a fall-back to the data points index.
    QVariant data = d->data(d->xDataRegion, index, role);
    if (data.isValid() && data.canConvert< double >() && data.convert(QVariant::Double) )
        return data;
    return QVariant(index + 1);
}

QVariant DataSet::yData(int index, int role) const
{
    // No fall-back necessary. y data region must be specified if needed.
    // (may also be part of 'domain' in ODF terms, but only in case of
    // scatter and bubble charts)
    return d->data(d->yDataRegion, index, role);
}

QVariant DataSet::customData(int index, int role) const
{
    // No fall-back necessary. ('custom' [1]) data region (part of 'domain' in
    // ODF terms) must be specified if needed. See ODF v1.1 §10.9.1
    return d->data(d->customDataRegion, index, role);
    // [1] In fact, 'custom' data only refers to the bubble width of bubble
    // charts at the moment.
}

QVariant DataSet::categoryData(int index, int role) const
{
     // There's no cell that holds this category's data
     // (i.e., the region is either too short or simply empty)
//     if (!d->categoryDataRegion.hasPointAtIndex(index))
//         return QString::number(index + 1);

    if (d->categoryDataRegion.rects().isEmpty()) {
        // There's no cell that holds this category's data
        // (i.e., the region is either too short or simply empty)
        return QString::number(index + 1);
    }

    foreach (const QRect &rect, d->categoryDataRegion.rects()) {
        if (rect.width() == 1 || rect.height() == 1) {
            // Handle the clear case of either horizontal or vertical
            // ranges with only one row/column.
            const QVariant data = d->data(d->categoryDataRegion, index, role);
            if (data.isValid())
                return data;
        } else {
            // Operate on the last row in the defined in the categoryDataRegion.
            // If multiple rows are given then we would need to build up multiple
            // lines of category labels. Each line of labels would represent
            // one row. If the category labels are displayed at the x-axis below
            // the chart then the last row will be displayed as first label line,
            // the row before the last row would be displayed as second label
            // line below the first line and so on. Since we don't support
            // multiple label lines for categories yet we only display the last
            // row aka the very first label line.
            CellRegion c(d->categoryDataRegion.table(), QRect(rect.x(), rect.bottom(), rect.width(), 1));
            const QVariant data = d->data(c, index, role);
            if (data.isValid() /* && !data.toString().isEmpty() */)
                return data;
        }
    }

    // The cell is empty
    return QString("");
}

QVariant DataSet::labelData() const
{
    QString label;
    if (d->labelDataRegion.isValid()) {
        const int cellCount = d->labelDataRegion.cellCount();
        for (int i = 0; i < cellCount; i++) {
            QString s = d->data(d->labelDataRegion, i, Qt::EditRole).toString();
            if (!s.isEmpty()) {
                if (!label.isEmpty())
                    label += " ";
                label += s;
            }
        }
    }
    if (label.isEmpty()) {
        label = d->defaultLabel;
    }
    return QVariant(label);
}

QString DataSet::defaultLabelData() const
{
    return d->defaultLabel;
}

CellRegion DataSet::xDataRegion() const
{
    return d->xDataRegion;
}

CellRegion DataSet::yDataRegion() const
{
    return d->yDataRegion;
}

CellRegion DataSet::customDataRegion() const
{
    return d->customDataRegion;
}

CellRegion DataSet::categoryDataRegion() const
{
    return d->categoryDataRegion;
}

CellRegion DataSet::labelDataRegion() const
{
    return d->labelDataRegion;
}


void DataSet::setXDataRegion(const CellRegion &region)
{
    d->xDataRegion = region;
    d->updateSize();

    if (d->kdChartModel)
        d->kdChartModel->dataSetChanged(this, KDChartModel::XDataRole);
}

void DataSet::setYDataRegion(const CellRegion &region)
{
    d->yDataRegion = region;
    d->updateSize();

    if (d->kdChartModel)
        d->kdChartModel->dataSetChanged(this, KDChartModel::YDataRole);
}

void DataSet::setCustomDataRegion(const CellRegion &region)
{
    d->customDataRegion = region;
    d->updateSize();
    
    if (d->kdChartModel)
        d->kdChartModel->dataSetChanged(this, KDChartModel::CustomDataRole);
}

void DataSet::setCategoryDataRegion(const CellRegion &region)
{
    d->categoryDataRegion = region;
    d->updateSize();

    if (d->kdChartModel)
        d->kdChartModel->dataSetChanged(this, KDChartModel::CategoryDataRole);
}

void DataSet::setLabelDataRegion(const CellRegion &region)
{
    d->labelDataRegion = region;
    d->updateSize();

    if (d->kdChartModel)
        d->kdChartModel->dataSetChanged(this);
}


int DataSet::size() const
{
    return qMax(1, d->size);
}

void DataSet::Private::dataChanged(KDChartModel::DataRole role, const QRect &rect) const
{
    if (!kdChartModel)
        return;
    Q_UNUSED(rect);

    // Stubbornly pretend like everything changed. This as well should be
    // refactored to be done in ChartProxyModel, then we can also fine-tune
    // it for performance.
    kdChartModel->dataSetChanged(parent, role, 0, size - 1);
}

void DataSet::yDataChanged(const QRect &region) const
{
    d->dataChanged(KDChartModel::YDataRole, region);
}

void DataSet::xDataChanged(const QRect &region) const
{
    d->dataChanged(KDChartModel::XDataRole, region);
}

void DataSet::customDataChanged(const QRect &region) const
{
    d->dataChanged(KDChartModel::CustomDataRole, region);
}

void DataSet::labelDataChanged(const QRect &region) const
{
    d->dataChanged(KDChartModel::LabelDataRole, region);
}

void DataSet::categoryDataChanged(const QRect &region) const
{
    d->dataChanged(KDChartModel::CategoryDataRole, region);
}

int DataSet::dimension() const
{
    return numDimensions(d->effectiveChartType());
}

void DataSet::setKdChartModel(KDChartModel *model)
{
    d->kdChartModel = model;
}

KDChartModel *DataSet::kdChartModel() const
{
    return d->kdChartModel;
}

void DataSet::setValueLabelType(const ValueLabelType &type, int section /* = -1 */)
{
    if (section >= 0)
        d->insertDataValueAttributeSectionIfNecessary(section);

    d->valueLabelType[section] = type;

    // This is a reference, not a copy!
    KDChart::DataValueAttributes &attr = section >= 0 ?
                                         d->sectionsDataValueAttributes[section] :
                                         d->dataValueAttributes;

    KDChart::TextAttributes ta (attr.textAttributes());

    ta.setVisible(!type.noLabel());

    KDChart::Measure m = ta.fontSize();
    m.setValue(8); // same small font the legend is using
    ta.setFontSize(m);

    attr.setTextAttributes(ta);

    if (d->kdChartModel) {
        if (section >= 0)
            d->kdChartModel->dataSetChanged(this, KDChartModel::DataValueAttributesRole, section);
        else
            d->kdChartModel->dataSetChanged(this);
    }
}

DataSet::ValueLabelType DataSet::valueLabelType(int section /* = -1 */) const
{
    if (d->valueLabelType.contains(section))
        return d->valueLabelType[section];
    if (d->valueLabelType.contains(-1))
        return d->valueLabelType[-1];
    return ValueLabelType();
}

bool loadBrushAndPen(KoStyleStack &styleStack, KoShapeLoadingContext &context,
                     const KoXmlElement &n, QBrush& brush, bool& brushLoaded, QPen& pen, bool& penLoaded)
{
    if (n.hasAttributeNS(KoXmlNS::chart, "style-name")) {
        KoOdfLoadingContext &odfLoadingContext = context.odfLoadingContext();
        brushLoaded = false;
        penLoaded = false;

        styleStack.setTypeProperties("graphic");

        if (styleStack.hasProperty(KoXmlNS::draw, "stroke")) {
            QString stroke = styleStack.property(KoXmlNS::draw, "stroke");
            pen = KoOdfGraphicStyles::loadOdfStrokeStyle(styleStack, stroke, odfLoadingContext.stylesReader());
            penLoaded = true;
        }

        if (styleStack.hasProperty(KoXmlNS::draw, "fill")) {
            QString fill = styleStack.property(KoXmlNS::draw, "fill");
            if (fill == "solid" || fill == "hatch") {
                brush = KoOdfGraphicStyles::loadOdfFillStyle(styleStack, fill, odfLoadingContext.stylesReader());
                brushLoaded = true;
            } else if (fill == "gradient") {
                brush = KoOdfGraphicStyles::loadOdfGradientStyle(styleStack, odfLoadingContext.stylesReader(), QSizeF(5.0, 60.0));
                brushLoaded = true;
            } else if (fill == "bitmap") {
                brush = Surface::loadOdfPatternStyle(styleStack, odfLoadingContext, QSizeF(5.0, 60.0));
                brushLoaded = true;
            }
        }
    }

#ifndef NWORKAROUND_ODF_BUGS
    if(! penLoaded) {
        penLoaded = KoOdfWorkaround::fixMissingStroke(pen, n, context);
    }
    if(! brushLoaded) {
        QColor fixedColor = KoOdfWorkaround::fixMissingFillColor(n, context);
        if (fixedColor.isValid()) {
            brush = fixedColor;
            brushLoaded = true;
        }
    }
#endif
    return true;
}

/**
* The valueLabelType can be read from a few different places;
*   - chart:data-label
*   - chart:data-point
*   - chart:series
*   - chart:plot-area
*
* Since we somehow need to merge e.g. a global one defined at the plot-area
* together with a local one defined in a series we need to make sure to
* fetch + change only what is redefined + reapply.
*
* The question is if this is 100% the correct thing to do or if we
* need more logic that e.g. differs between where the data-labels got
* defined? It would make sense but there is no information about that
* available and it seems OO.org/LO just save redundant information
* here at least with pie-charts...
*/
void DataSet::Private::readValueLabelType(KoStyleStack &styleStack, int section /* = -1 */)
{
    DataSet::ValueLabelType type = parent->valueLabelType(section);

    const QString number = styleStack.property(KoXmlNS::chart, "data-label-number");
    if (!number.isNull()) {
        type.number = (number == "value" || number == "value-and-percentage");
        type.percentage = (number == "percentage" || number == "value-and-percentage");
    }

    const QString text = styleStack.property(KoXmlNS::chart, "data-label-text");
    if (!text.isNull()) {
        type.category = (text == "true");
    }

    const QString symbol = styleStack.property(KoXmlNS::chart, "data-label-symbol");
    if (!symbol.isNull()) {
        type.symbol = (symbol == "true");
    }

    parent->setValueLabelType(type, section);
}

bool DataSet::loadOdf(const KoXmlElement &n,
                      KoShapeLoadingContext &context)
{
    d->symbolsActivated = false;
    KoOdfLoadingContext &odfLoadingContext = context.odfLoadingContext();
    KoOdfStylesReader &stylesReader = odfLoadingContext.stylesReader();
    KoStyleStack &styleStack = odfLoadingContext.styleStack();
    styleStack.clear();
    odfLoadingContext.fillStyleStack(n, KoXmlNS::chart, "style-name", "chart");

    QString styleName = n.attributeNS(KoXmlNS::chart, "style-name", QString());
    const KoXmlElement *stylElement = stylesReader.findStyle(styleName, "chart");
    if (stylElement) {
        const QString dataStyleName = stylElement->attributeNS(KoXmlNS::style, "data-style-name", QString());
        if (!dataStyleName.isEmpty()) {
            if (stylesReader.dataFormats().contains(dataStyleName)) {
                QPair<KoOdfNumberStyles::NumericStyleFormat, KoXmlElement*> dataStylePair = stylesReader.dataFormats()[dataStyleName];
                delete d->numericStyleFormat;
                d->numericStyleFormat = new KoOdfNumberStyles::NumericStyleFormat(dataStylePair.first);
            }
        }
    }

    OdfLoadingHelper *helper = (OdfLoadingHelper*)context.sharedData(OdfLoadingHelperId);
    // OOo assumes that if we use an internal model only, the columns are
    // interpreted as consecutive data series. Thus we can (and must) ignore
    // any chart:cell-range-address attribute associated with a series or
    // data point. Instead the regions are used that are automatically
    // assigned by SingleModelHelper whenever the structure of the internal
    // model changes.
    bool ignoreCellRanges = false;
// Some OOo documents save incorrect cell ranges. For those this fix was intended.
// Find out which documents exactly and only use fix for as few cases as possible.
#if 0
#ifndef NWORKAROUND_ODF_BUGS
    if (context.odfLoadingContext().generatorType() == KoOdfLoadingContext::OpenOffice)
        ignoreCellRanges = helper->chartUsesInternalModelOnly;
#endif
#endif

    {
        QBrush brush(Qt::NoBrush);
        QPen pen(Qt::NoPen);
        bool brushLoaded = false;
        bool penLoaded = false;
        loadBrushAndPen(styleStack, context, n, brush, brushLoaded, pen, penLoaded);
        if (penLoaded)
            setPen(pen);
        if (brushLoaded)
            setBrush(brush);
        styleStack.setTypeProperties("chart");
        if (styleStack.hasProperty(KoXmlNS::chart, "pie-offset"))
            setPieExplodeFactor(styleStack.property(KoXmlNS::chart, "pie-offset").toInt());
    }

    bool bubbleChart = false;
    bool scatterChart = false;
    if (n.hasAttributeNS(KoXmlNS::chart, "class")) {
        QString charttype = n.attributeNS(KoXmlNS::chart, "class", QString());
        bubbleChart = charttype == "chart:bubble";
        scatterChart = charttype == "chart:scatter";
    }

    // The <chart:domain> element specifies coordinate values required by particular chart types.
    // For scatter charts, one <chart:domain> element shall exist. Its table:cell-range-address
    // attribute references the x coordinate values for the scatter chart.
    // For bubble charts, two <chart:domain> elements shall exist. The values for the y-coordinates are
    // given by the first <chart:domain> element. The values for the x-coordinates are given by the
    // second <chart:domain> element.
    // At least one <chart:series> element of a given chart:class shall have the necessary
    // number of <chart:domain> sub-elements. All other <chart:series> elements with the same
    // chart:class may omit the <chart:domain> sub-elements and use the previously defined.
    if ((scatterChart || bubbleChart) && n.hasChildNodes()) {
        int domainCount = 0;
        KoXmlNode cn = n.firstChild();
        while (!cn.isNull()){
            KoXmlElement elem = cn.toElement();
            const QString name = elem.tagName();
            if (name == "domain" && elem.hasAttributeNS(KoXmlNS::table, "cell-range-address") && !ignoreCellRanges) {
                if ((domainCount == 0 && scatterChart) || (domainCount == 1 && bubbleChart)) {
                    const QString region = elem.attributeNS(KoXmlNS::table, "cell-range-address", QString());
                    setXDataRegion(CellRegion(helper->tableSource, region));
                }
                else {
                    const QString region = elem.attributeNS(KoXmlNS::table, "cell-range-address", QString());                    
                    setYDataRegion(CellRegion(helper->tableSource, region));
                }
                ++domainCount;
                if ((bubbleChart && domainCount == 2) || scatterChart)
                    break; // We are finished and don't expect more domain's.
            }
            cn = cn.nextSibling();
        }
    }

    if (n.hasAttributeNS(KoXmlNS::chart, "values-cell-range-address") && !ignoreCellRanges) {
        const QString regionString = n.attributeNS(KoXmlNS::chart, "values-cell-range-address", QString());
        const CellRegion region(helper->tableSource, regionString);
        if (bubbleChart) {
            setCustomDataRegion(region);            
        }
        else {
            setYDataRegion(region);
        }

        if (!bubbleChart && d->loadedDimensions == 0) {
            setYDataRegion(region);
            ++d->loadedDimensions;
        }
    }
    if (n.hasAttributeNS(KoXmlNS::chart, "label-cell-address") && !ignoreCellRanges) {
        const QString region = n.attributeNS(KoXmlNS::chart, "label-cell-address", QString());
        setLabelDataRegion(CellRegion(helper->tableSource, region));
    }

    if (n.hasAttributeNS(KoXmlNS::chart, "class") && !ignoreCellRanges) {
        const QString chartClass = n.attributeNS(KoXmlNS::chart, "class", QString());
        KChart::ChartType chartType = KChart::BarChartType;
        for (int type = 0; type < (int)LastChartType; ++type) {
            if (chartClass == odfCharttype(type)) {
                chartType = (ChartType)type;
                setChartType(chartType);
                break;
            }
        }
    }

    d->readValueLabelType(styleStack);

    if (styleStack.hasProperty(KoXmlNS::chart, "symbol-type")) {

        const QString name = styleStack.property(KoXmlNS::chart, "symbol-type");
        if (name == "automatic") {
            d->symbolsActivated = true;
            d->symbolID = d->num % numDefaultMarkerTypes;
            d->markerIsAutoSet = true;
        }
        else if (name == "named-symbol") {
            d->symbolsActivated = true;
            d->markerIsAutoSet = false;
            if (styleStack.hasProperty(KoXmlNS::chart, "symbol-name")) {

                const QString type = styleStack.property(KoXmlNS::chart, "symbol-name");
                if (type == "square")
                    d->symbolID = 0;
                else if (type == "diamond")
                    d->symbolID = 1;
                else if (type == "arrow-down")
                    d->symbolID = 2;
                else if (type == "arrow-up")
                    d->symbolID = 3;
                else if (type == "arrow-right")
                    d->symbolID = 4;
                else if (type == "arrow-left")
                    d->symbolID = 5;
                else if (type == "bow-tie")
                    d->symbolID = 6;
                else if (type == "hourglass")
                    d->symbolID = 7;
                else if (type == "circle")
                    d->symbolID = 8;
                else if (type == "star")
                    d->symbolID = 9;
                else if (type == "x")
                    d->symbolID = 10;
                else if (type == "plus")
                    d->symbolID = 11;
                else if (type == "asterisk")
                    d->symbolID = 12;
                else if (type == "horizontal-bar")
                    d->symbolID = 13;
                else if (type == "vertical-bar")
                    d->symbolID = 14;
                else
                    d->symbolID = 0;
            }
        } else if (name == "none") {
            d->symbolsActivated = false;
            d->symbolID = 19;
            d->markerIsAutoSet = false;
        }
    }

    // load data points
    KoXmlElement m;
    int loadedDataPointCount = 0;
    forEachElement (m, n) {
        if (m.namespaceURI() != KoXmlNS::chart)
            continue;
        if (m.localName() != "data-point")
            continue;

        styleStack.clear();
        odfLoadingContext.fillStyleStack(m, KoXmlNS::chart, "style-name", "chart");

        QBrush brush(Qt::NoBrush);
        QPen pen(Qt::NoPen);
        bool brushLoaded = false;
        bool penLoaded = false;
        loadBrushAndPen(styleStack, context, m, brush, brushLoaded, pen, penLoaded);
        if(penLoaded)
            setPen(loadedDataPointCount, pen);
        if(brushLoaded)
            setBrush(loadedDataPointCount, brush);

        //load pie explode factor
        styleStack.setTypeProperties("chart");
        if(styleStack.hasProperty(KoXmlNS::chart, "pie-offset"))
            setPieExplodeFactor(loadedDataPointCount, styleStack.property(KoXmlNS::chart, "pie-offset").toInt());

        d->readValueLabelType(styleStack, loadedDataPointCount);

        ++loadedDataPointCount;
    }
    return true;
}

bool DataSet::loadSeriesIntoDataset(const KoXmlElement &n, KoShapeLoadingContext &context)
{
    d->symbolsActivated = false;
    KoOdfLoadingContext &odfLoadingContext = context.odfLoadingContext();
    KoStyleStack &styleStack = odfLoadingContext.styleStack();
    styleStack.clear();
    odfLoadingContext.fillStyleStack(n, KoXmlNS::chart, "style-name", "chart");

    OdfLoadingHelper *helper = (OdfLoadingHelper*)context.sharedData(OdfLoadingHelperId);
    // OOo assumes that if we use an internal model only, the columns are
    // interpreted as consecutive data series. Thus we can (and must) ignore
    // any chart:cell-range-address attribute associated with a series or
    // data point. Instead the regions are used that are automatically
    // assigned by SingleModelHelper whenever the structure of the internal
    // model changes.
    bool ignoreCellRanges = false;
    styleStack.setTypeProperties("chart");

    if (n.hasChildNodes()){
        KoXmlNode cn = n.firstChild();
        while (!cn.isNull()){
            KoXmlElement elem = cn.toElement();
            const QString name = elem.tagName();
            if (name == "domain" && elem.hasAttributeNS(KoXmlNS::table, "cell-range-address") && !ignoreCellRanges) {
                Q_ASSERT(false);
                if (d->loadedDimensions == 0) {
                    const QString region = elem.attributeNS(KoXmlNS::table, "cell-range-address", QString());
                    setXDataRegion(CellRegion(helper->tableSource, region));
                    ++d->loadedDimensions;
                }
                else if (d->loadedDimensions == 1) {
                    const QString region = elem.attributeNS(KoXmlNS::table, "cell-range-address", QString());
                    // as long as there is not default table for missing data series the same region is used twice
                    // to ensure the diagram is displayed, even if not as expected from o office or ms office
                    setYDataRegion(CellRegion(helper->tableSource, region));
                    ++d->loadedDimensions;
                }
                else if (d->loadedDimensions == 2) {
                    const QString region = elem.attributeNS(KoXmlNS::table, "cell-range-address", QString());
                    // as long as there is not default table for missing data series the same region is used twice
                    // to ensure the diagram is displayed, even if not as expected from o office or ms office
                    setCustomDataRegion(CellRegion(helper->tableSource, region));
                    ++d->loadedDimensions;
                }

            }
            cn = cn.nextSibling();
        }
    }

    if (n.hasAttributeNS(KoXmlNS::chart, "values-cell-range-address") && !ignoreCellRanges) {
        const QString regionString = n.attributeNS(KoXmlNS::chart, "values-cell-range-address", QString());
        const CellRegion region(helper->tableSource, regionString);
        if (d->loadedDimensions == 0) {
            setYDataRegion(CellRegion(region));
            ++d->loadedDimensions;
        }
        else if (d->loadedDimensions == 1) {
            // as long as there is not default table for missing data series the same region is used twice
            // to ensure the diagram is displayed, even if not as expected from o office or ms office
            setYDataRegion(CellRegion(region));
            ++d->loadedDimensions;
        }
        else if (d->loadedDimensions == 2) {
            // As long as there is no default table for missing data
            // series the same region is used twice to ensure the
            // diagram is displayed, even if not as expected from open
            // office or ms office.
            setCustomDataRegion(CellRegion(region));
            ++d->loadedDimensions;
        }
    }
    //store the cell address corresponding to the label of the correct data series
    if (d->loadedDimensions == 2 && n.hasAttributeNS(KoXmlNS::chart, "label-cell-address") && !ignoreCellRanges) {
        const QString region = n.attributeNS(KoXmlNS::chart, "label-cell-address", QString());
        setLabelDataRegion(CellRegion(helper->tableSource, region));
    }

    d->readValueLabelType(styleStack);

    return true;
}

void DataSet::saveOdf(KoShapeSavingContext &context) const
{
    KoXmlWriter &bodyWriter = context.xmlWriter();
    KoGenStyles &mainStyles = context.mainStyles();

    bodyWriter.startElement("chart:series");

    KoGenStyle style(KoGenStyle::ChartAutoStyle, "chart");

    if (pieAttributes().explode()) {
        const int pieExplode = (int)(pieAttributes().explodeFactor()*100);
        style.addProperty("chart:pie-offset", pieExplode, KoGenStyle::ChartType);
    }

    DataSet::ValueLabelType type = valueLabelType();
    if (type.number && type.percentage)
        style.addProperty("chart:data-label-number", "value-and-percentage");
    else if (type.number)
        style.addProperty("chart:data-label-number", "value");
    else if (type.percentage)
        style.addProperty("chart:data-label-number", "percentage");
    if (type.category)
        style.addProperty("chart:data-label-text", "true");
    if (type.symbol)
        style.addProperty("chart:data-label-symbol", "true");

    if (d->symbolsActivated) {
        QString symbolName = "";
        QString symbolType = "named-symbol";

        if (!d->markerIsAutoSet) {
            switch (d->symbolID) {
            case 0: symbolName = "square"; break;
            case 1: symbolName = "diamond"; break;
            case 2: symbolName = "arrow-down"; break;
            case 3: symbolName = "arrow-up"; break;
            case 4: symbolName = "arrow-right"; break;
            case 5: symbolName = "arrow-left"; break;
            case 6: symbolName = "bow-tie"; break;
            case 7: symbolName = "hourglass"; break;
            case 8: symbolName = "circle"; break;
            case 9: symbolName = "star"; break;
            case 10: symbolName = "x"; break;
            case 11: symbolName = "plus"; break;
            case 12: symbolName = "asterisk"; break;
            case 13: symbolName = "horizontal-bar"; break;
            case 14: symbolName = "vertical-bar"; break;
            case 19: symbolType = "none"; break;
            default: symbolType = "automatic"; break;
            }
        } else {
            symbolType = "automatic";
        }

        style.addProperty("chart:symbol-type", symbolType, KoGenStyle::ChartType);
        if (!symbolName.isEmpty())
            style.addProperty("chart:symbol-name", symbolName, KoGenStyle::ChartType);
    }

    KoOdfGraphicStyles::saveOdfFillStyle(style, mainStyles, brush());
    KoOdfGraphicStyles::saveOdfStrokeStyle(style, mainStyles, pen());

    const QString styleName = mainStyles.insert(style, "ch");
    bodyWriter.addAttribute("chart:style-name", styleName);

    // Save cell regions for values if defined.
    QString values = yDataRegion().toString();
    if (!values.isEmpty())
        bodyWriter.addAttribute("chart:values-cell-range-address", values);

    // Save cell regions for labels if defined. If not defined then the internal
    // table:table "local-table" (the data is stored in the ChartTableModel) is used.
    QString label = labelDataRegion().toString();
    if (!label.isEmpty())
        bodyWriter.addAttribute("chart:label-cell-address", label);

    int charttype = (chartType() < LastChartType) ? chartType() : 0;
    QString chartClass = odfCharttype(charttype);
    if (!chartClass.isEmpty())
        bodyWriter.addAttribute("chart:class", chartClass);

    if (chartType() == KChart::CircleChartType || chartType() == KChart::RingChartType) {
        for (int j=0; j<yDataRegion().cellCount(); ++j) {
            bodyWriter.startElement("chart:data-point");

            KoGenStyle dps(KoGenStyle::GraphicAutoStyle, "chart");
            dps.addProperty("draw:fill", "solid", KoGenStyle::GraphicType);
            dps.addProperty("draw:fill-color", brush(j).color().name(), KoGenStyle::GraphicType);

            const QString styleName = mainStyles.insert(dps, "ch");
            bodyWriter.addAttribute("chart:style-name", styleName );

            bodyWriter.endElement();
        }
    }

    bodyWriter.endElement(); // chart:series
}

static KDChart::MarkerAttributes::MarkerStyle odf2kdMarker(OdfMarkerStyle style) {
    switch (style) {
    case MarkerSquare:
        return KDChart::MarkerAttributes::MarkerSquare;
    case MarkerDiamond:
        return KDChart::MarkerAttributes::MarkerDiamond;
    case MarkerArrowDown:
        return KDChart::MarkerAttributes::MarkerArrowDown;
    case MarkerArrowUp:
        return KDChart::MarkerAttributes::MarkerArrowUp;
    case MarkerArrowRight:
        return KDChart::MarkerAttributes::MarkerArrowRight;
    case MarkerArrowLeft:
        return KDChart::MarkerAttributes::MarkerArrowLeft;
    case MarkerBowTie:
        return KDChart::MarkerAttributes::MarkerBowTie;
    case MarkerHourGlass:
        return KDChart::MarkerAttributes::MarkerHourGlass;
    case MarkerCircle:
        return KDChart::MarkerAttributes::MarkerCircle;
    case MarkerStar:
        return KDChart::MarkerAttributes::MarkerStar;
    case MarkerX:
        return KDChart::MarkerAttributes::MarkerX;
    case MarkerCross:
        return KDChart::MarkerAttributes::MarkerCross;
    case MarkerAsterisk:
        return KDChart::MarkerAttributes::MarkerAsterisk;
    case MarkerHorizontalBar:
        return KDChart::MarkerAttributes::MarkerHorizontalBar;
    case MarkerVerticalBar:
        return KDChart::MarkerAttributes::MarkerVerticalBar;
    case MarkerRing:
        return KDChart::MarkerAttributes::MarkerRing;
    case MarkerFastCross:
        return KDChart::MarkerAttributes::MarkerFastCross;
    case Marker1Pixel:
        return KDChart::MarkerAttributes::Marker1Pixel;
    case Marker4Pixels:
        return KDChart::MarkerAttributes::Marker4Pixels;
    case NoMarker:
        return KDChart::MarkerAttributes::NoMarker;
    }

    return KDChart::MarkerAttributes::MarkerSquare;
}
