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
#include "ChartProxyModel.h"
#include "PlotArea.h"

// Qt
#include <QRegion>
#include <QPoint>

// KDE
#include <KDebug>

// Calligra
#include <KoXmlReader.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoXmlWriter.h>
#include <KoGenStyles.h>
#include <KoXmlNS.h>
#include <KoOdfLoadingContext.h>
#include <KoOdfStylesReader.h>
#include <KoOdfGraphicStyles.h>
#include <interfaces/KoChartModel.h>

// KChart
#include "Axis.h"
#include "DataSet.h"
#include "TableSource.h"
#include "OdfLoadingHelper.h"

using namespace KChart;


// ================================================================
//                     Class ChartProxyModel::Private


class ChartProxyModel::Private {
public:
    Private(ChartProxyModel *parent, ChartShape *shape, TableSource *source);
    ~Private();

    ChartProxyModel *const q;
    ChartShape *const shape;
    TableSource *const tableSource;

    /// Set to true if we're in the process of loading data from ODF.
    /// Used to avoid repeatedly updating data.
    bool isLoading;

    bool             firstRowIsLabel;
    bool             firstColumnIsLabel;
    Qt::Orientation  dataDirection;
    int              dataDimensions;

    CellRegion categoryDataRegion;

    QVector< CellRegion > dataSetRegions;

    QList<DataSet*>  dataSets;
    QList<DataSet*>  removedDataSets;

    CellRegion       selection;

    /**
     * Discards old and creates new data sets from the current region selection.
     */
    void rebuildDataMap();

    /**
     * Extracts a list of data sets (with x data region, y data region, etc.
     * assigned) from the current d->selection.
     *
     * Unless the list *dataSetsToRecycle is empty, it will reuse as many
     * DataSet instances from there as possible and remove them from the list.
     *
     * As a side effect, this method sets d->categoryDataRegion if
     * overrideCategories is true.
     */
    QList<DataSet*> createDataSetsFromRegion(QList<DataSet*> *dataSetsToRecycle,
                                              bool overrideCategories = true);
};

ChartProxyModel::Private::Private(ChartProxyModel *parent, ChartShape *shape, TableSource *source)
    : q(parent)
    , shape(shape)
    , tableSource(source)
    , isLoading(false)
{
    firstRowIsLabel    = false;
    firstColumnIsLabel = false;
    dataDimensions     = 1;

    // Determines what orientation the data points in a data series
    // have when multiple data sets are created from one source
    // region. For example, vertical means that each column in the source
    // region is assigned to one data series.
    // Default to Qt::Vertical, as that's what OOo does also.
    dataDirection      = Qt::Vertical;
}

ChartProxyModel::Private::~Private()
{
    qDeleteAll(dataSets);
    qDeleteAll(removedDataSets);
}


// ================================================================
//                          Class ChartProxyModel


ChartProxyModel::ChartProxyModel(ChartShape *shape, TableSource *source)
    : QAbstractTableModel(),
      d(new Private(this, shape, source))
{
    connect(source, SIGNAL(tableAdded(Table*)),
            this,   SLOT(addTable(Table*)));
    connect(source, SIGNAL(tableRemoved(Table*)),
            this,   SLOT(removeTable(Table*)));
}

ChartProxyModel::~ChartProxyModel()
{
    delete d;
}

void ChartProxyModel::reset(const CellRegion& region)
{
    d->selection = region;
    d->rebuildDataMap();
}

CellRegion ChartProxyModel::cellRangeAddress() const
{
    return d->selection;
}

void ChartProxyModel::Private::rebuildDataMap()
{
    // This was intended to speed up the loading process, by executing this
    // method only once in endLoading(), however the approach is actually
    // incorrect as it would potentially override a data set's regions
    // set by "somebody" else in the meantime.
    // if (isLoading)
    //     return;
    q->beginResetModel();
    q->invalidateDataSets();
    dataSets = createDataSetsFromRegion(&removedDataSets);
    q->endResetModel();
}

void ChartProxyModel::addTable(Table *table)
{
    QAbstractItemModel *model = table->model();
    connect(model, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
            this,  SLOT(dataChanged(QModelIndex, QModelIndex)));
}

void ChartProxyModel::removeTable(Table *table)
{
    QAbstractItemModel *model = table->model();
    model->disconnect(this);
}

/**
 * Returns a row of a given region (i.e. a part of it with height 1), cutting
 * off the first @a colOffset cells in that row.
 *
 * Examples: extractRow(A1:C2, 0, 0) --> A1:C1
 *           extractRow(A1:C2, 1, 0) --> A2:C2
 *           extractRow(A1:C2, 0, 1) --> B1:C1
 *
 * See notes in createDataSetsFromRegion() for further details.
 *
 * @param region The region to extract the row from
 * @param row The number of the row, starting with 0
 * @param colOffset How many of the first columns to cut from the resulting row
 */
static QVector<QRect> extractRow(const QVector<QRect> &rects, int colOffset, bool extractLabel)
{
    if (colOffset == 0)
        return extractLabel ? QVector<QRect>() : rects;
    QVector<QRect> result;
    foreach(const QRect &rect, rects) {
        if (extractLabel) {
            QRect r(rect.topLeft(), QSize(colOffset, rect.height()));
            result.append(r);
        }
        else {
//Q_ASSERT(rect.width() > colOffset);
            if (rect.width() > colOffset) {
                QPoint topLeft = rect.topLeft() + QPoint(colOffset, 0);
                QRect r(topLeft, QSize(rect.width() - colOffset, rect.height()));
                result.append(r);
            }
        }
    }
    return result;
}

/**
 * Returns a column of a given region, cutting off the first @a rowOffset
 * rows in that column.
 *
 * Examples: extractColumn(A1:C2, 0, 0)       --> A1:A2
 *           extractColumn(A1:C2;D1;F2, 0, 0) --> A1:A2;D1:D2
 *
 * See notes in createDataSetsFromRegion() for further details.
 *
 * @param region The region to extract the row from
 * @param col The number of the column, starting with 0
 * @param rowOffset How many of the first rows to cut from the resulting column
 */
static QVector<QRect> extractColumn(const QVector<QRect> &rects, int rowOffset, bool extractLabel)
{
    if (rowOffset == 0)
        return extractLabel ? QVector<QRect>() : rects;
    QVector<QRect> result;
    foreach(const QRect &rect, rects) {
        if (extractLabel) {
            QRect r(rect.topLeft(), QSize(rect.width(), rowOffset));
            result.append(r);
        }
        else {
//Q_ASSERT(rect.height() > rowOffset);
            if (rect.height() > rowOffset) {
                QPoint topLeft = rect.topLeft() + QPoint(0, rowOffset);
                QRect r(topLeft, QSize(rect.width(), rect.height() - rowOffset));
                result.append(r);
            }
        }
    }
    return result;
}

/**
 * Returns a according to the dataDirection sorted data-region.
 *
 * First prepare the dataRegions to have them
 * - proper sorted either from left-to-right if the dataDirection is
 *   Qt::Horizontal or from top-to-bottom when Qt::Vertical.
 * - have multiple dimensions proper split-up into single rows/columns
 *   to produce one DataSet per row/column.
 *
 * The resulting map is build up depending on the dataDirection;
 * - Qt::Horizontal: key is the row and values are the columns in the row.
 * - Qt::Vertical: key is the column and values are the rows in the column.
 *
 * @param dataDirection The direction of the data. This could be either
 * dataDirection == Qt::Horizontal or dataDirection == Qt::Vertical.
 * @param dataRegions The unsorted list of data-regions.
 * @return Compared to the QVector that goes in the resulting map makes
 * following sure;
 * - sorted according to the dataDirection either by row or column
 * - duplicate definitions are removed/compressed.
 * - definitions or multiple rows and columns in one QRect are split
 *   in rows/columns what makes sure we earn one dataRegion per
 *   row/column. This is needed so for example the labelRegion and
 *   all other things operate on exactly one row/column.
 */
QMap<int, QVector<QRect> > sortDataRegions(Qt::Orientation dataDirection, QVector<QRect> dataRegions)
{
    QMap<int, QVector<QRect> >  sortedDataRegions;
    if (dataDirection == Qt::Horizontal) {
        // Split up region in horizontal rectangles that are sorted from top to bottom
        QMap<int, QVector<QRect> >  rows;
        foreach (const QRect &rect, dataRegions) {
            int x = rect.topLeft().x();
            for (int y = rect.topLeft().y(); y <= rect.bottomLeft().y(); y++) {
                QRect dataRect = QRect(QPoint(x, y), QSize(rect.width(), 1));
                if (!rows.contains(y))
                    rows.insert(y, QVector<QRect>());
                rows[y].append(dataRect);
            }
        }

        // Sort rectangles in each row from left to right.
        QMapIterator<int, QVector<QRect> >  i(rows);
        while (i.hasNext()) {
            i.next();
            int             row = i.key();
            QVector<QRect>  unsortedRects = i.value();
            QVector<QRect>  sortedRects;
            foreach (const QRect &rect, unsortedRects) {
                int index;
                for (index = 0; index < sortedRects.size(); index++)
                    if (rect.topLeft().x() <= sortedRects[index].topLeft().x())
                        break;
                sortedRects.insert(index, rect);
            }
            sortedDataRegions.insert(row, sortedRects);
        }
    } else {
        // Split up region in horizontal rectangles that are sorted from top to bottom
        QMap<int, QVector<QRect> >  columns;
        foreach (const QRect &rect, dataRegions) {
            int y = rect.topLeft().y();
            for (int x = rect.topLeft().x(); x <= rect.topRight().x(); ++x) {
                QRect dataRect = QRect(QPoint(x, y), QSize(1, rect.height()));
                if (!columns.contains(x))
                    columns.insert(x, QVector<QRect>());
                columns[x].append(dataRect);
            }
        }

        // Sort rectangles in each column from top to bottom
        QMapIterator<int, QVector<QRect> >  i(columns);
        while (i.hasNext()) {
            i.next();
            int             col = i.key();
            QVector<QRect>  unsortedRects = i.value();
            QVector<QRect>  sortedRects;
            foreach (const QRect &rect, unsortedRects) {
                int index;
                for (index = 0; index < sortedRects.size(); ++index)
                    if (rect.topLeft().y() <= sortedRects[index].topLeft().y())
                        break;
                sortedRects.insert(index, rect);
            }
            sortedDataRegions.insert(col, sortedRects);
        }
    }
    return sortedDataRegions;
}

QList<DataSet*> ChartProxyModel::Private::createDataSetsFromRegion(QList<DataSet*> *dataSetsToRecycle,
                                                                    bool overrideCategories)
{
    if (!selection.isValid())
        return QList<DataSet*>();

    // First prepare the dataRegions to have them
    // 1) proper sorted either from left-to-right if the dataDirection is
    //    Qt::Horizontal or from top-to-bottom when Qt::Vertical.
    // 2) have multiple dimensions proper split-up into single rows/columns
    //    to produce one DataSet per row/column.
    //
    // The resulting map is build up depending on the dataDirection;
    // - Qt::Horizontal: key is the row and values are the columns in the row.
    // - Qt::Vertical: key is the column and values are the rows in the column.
    QMap<int, QVector<QRect> > sortedDataRegions = sortDataRegions(dataDirection, selection.rects());

    // In the end, the contents of this list will look something like this:
    // (Category-Data, X-Data, Y-Data, Y-Data, Y-Data)
    // Semantic separation of the regions will follow later.
    QList<CellRegion> dataRegions;
    // This region exclusively contains (global) data set labels, i.e.
    // one label per data set (thus in opposite data direction)
    QList<CellRegion> labelRegions;

    // Fill dataRegions and set categoryRegion.
    // Note that here, we don't exactly know yet what region will be used for
    // what data set, we also don't know yet what data these regions contain.
    int rowOffset = firstRowIsLabel ? 1 : 0;
    int colOffset = firstColumnIsLabel ? 1 : 0;

    // Determines how many individual rows/columns will be assigned per data set.
    // It is at least one, but if there's more than one data dimension, the x
    // data is shared among all data sets, thus - 1.
    int regionsPerDataSet = qMax(1, dataDimensions - 1);

    // Determinate the number of rows and maximum number of columns used.
    // FIXME this logic is rather buggy. We better should use our sortedDataRegions
    //       here but that would need some testing for regression in e.g.
    //       bubble-charts which I don't had the time to do. So, leaving the logic as
    //       it was before for now. If you run into problems with bubble charts and
    //       there x-values then this foreach-logic here is very likely the reason.
    int rows = 0;
    int cols = 0;
    foreach(const QRect &rect, selection.rects()) {
        rows += rect.height();
        cols = qMax(cols, rect.width());
    }

    bool extractXData = dataDimensions > 1 &&
                        // Checks if the remaining data regions would fit exactly to the
                        // remaining data sets. If not, skip x data. This is only the case
                        // for bubble charts, (the only case of regionsPerDataSet == 2), so
                        // skipping x data will allow the last data set to also be assigned
                        // a bubble width region. This is exactly what OOo does.
                        (dataDirection == Qt::Horizontal ? rows - 1 - rowOffset
                                                         : cols - 1 -colOffset) % regionsPerDataSet == 0;

    // When x data is present, it occupies the first non-header row/column
    if (extractXData && dataDirection == Qt::Horizontal)
        ++rowOffset;
    if (extractXData && dataDirection == Qt::Vertical)
        ++colOffset;

    // This is the logic that extracts all the subregions from selection
    // that are later used for the data sets
    Table *internalTable = shape ? shape->tableSource()->get(shape->internalModel()) : 0;
    QList<int> sortedDataKeys = sortedDataRegions.keys();
    qSort(sortedDataKeys);
    foreach(int key, sortedDataKeys) {
        QVector<QRect> rects = sortedDataRegions[key];
        QVector<QRect> dataRects;
        CellRegion labelRegion;
        if (dataDirection == Qt::Horizontal) {
            if (firstColumnIsLabel) {
                QVector<QRect> labelRects = extractRow(rects, colOffset, true);
                labelRegion = labelRects.isEmpty() ? CellRegion() : CellRegion(selection.table(), labelRects);
            }
            else {
                labelRegion = internalTable ? CellRegion(internalTable, QPoint(1, key)) : CellRegion();
            }
            dataRects = extractRow(rects, colOffset, false);
        }
        else {
            if (firstRowIsLabel) {
                QVector<QRect> labelRects = extractColumn(rects, rowOffset, true);
                labelRegion = labelRects.isEmpty() ? CellRegion() : CellRegion(selection.table(), labelRects);
            }
            else {
                labelRegion = internalTable ? CellRegion(internalTable, QPoint(key, 1)) : CellRegion();
            }
            dataRects = extractColumn(rects, rowOffset, false);
        }

        labelRegions.append(labelRegion);
        dataRegions.append(dataRects.isEmpty() ? CellRegion() : CellRegion(selection.table(), dataRects));
    }

    bool useCategories =
            (dataDirection == Qt::Horizontal && firstRowIsLabel) ||
            (dataDirection == Qt::Vertical && firstColumnIsLabel);

    /*
    kDebug(35001) << "selection=" << selection.toString();
    kDebug(35001) << "dataDirection=" << (dataDirection == Qt::Horizontal ? "Horizontal" : "Vertical");
    kDebug(35001) << "firstRowIsLabel=" << firstRowIsLabel;
    kDebug(35001) << "firstColumnIsLabel=" << firstColumnIsLabel;
    kDebug(35001) << "overrideCategories=" << overrideCategories;
    kDebug(35001) << "useCategories=" << useCategories;
    kDebug(35001) << "dataRegions.count()="<<dataRegions.count();
    */

    // Regions shared by all data sets: categories and x-data

    //FIXME don't use overrideCategories for the categoryDataRegion's but for
    //the categoryLabelRegion (which we need to introduce and use).
if(overrideCategories) categoryDataRegion = CellRegion();

    if (!dataRegions.isEmpty()) {
// Q_ASSERT(label.isValid());
// Q_ASSERT(data.isValid());
        if (useCategories) {
            labelRegions.takeFirst();
            categoryDataRegion = dataRegions.takeFirst();
        }
    }

    CellRegion xData;
    if (!dataRegions.isEmpty() && extractXData) {
        labelRegions.removeFirst();
        xData = dataRegions.takeFirst();
    }

    QList<DataSet*> createdDataSets;
    int dataSetNumber = 0;
    // Now assign all dataRegions to a number of data sets.
    // Here they're semantically separated into x data, y data, etc.
    Q_ASSERT(dataRegions.count() == labelRegions.count());
    while (!dataRegions.isEmpty()) {
        // Get a data set instance we can use
        DataSet *dataSet;
        if (!dataSetsToRecycle->isEmpty())
            dataSet = dataSetsToRecycle->takeFirst();
        else
            dataSet = new DataSet(dataSetNumber);

        // category and x data are "global" regions shared among all data sets
        dataSet->setCategoryDataRegion(categoryDataRegion);
        dataSet->setXDataRegion(xData);

        // the name-value used for e.g. the legend label
        dataSet->setLabelDataRegion(labelRegions.takeFirst());

        // the range for y-values
        dataSet->setYDataRegion(dataRegions.takeFirst());

        // the custom data (e.g. bubble width)
        if (!dataRegions.isEmpty() && dataDimensions > 2)
            dataSet->setCustomDataRegion(dataRegions.takeFirst());
        else
            dataSet->setCustomDataRegion(CellRegion());

        /*
        kDebug(35001) << "xDataRegion=" << dataSet->xDataRegion().toString();
        kDebug(35001) << "yDataRegion=" << dataSet->yDataRegion().toString();
        kDebug(35001) << "categoryDataRegion=" << dataSet->categoryDataRegion().toString();
        kDebug(35001) << "labelDataRegion=" << dataSet->labelDataRegion().toString();
        kDebug(35001) << "customDataRegion=" << dataSet->customDataRegion().toString();
        */

        createdDataSets.append(dataSet);

        // Increment number at the very end!
        dataSetNumber++;
    }

    return createdDataSets;
}

void ChartProxyModel::saveOdf(KoShapeSavingContext &context) const
{
    foreach (DataSet *dataSet, d->dataSets)
        dataSet->saveOdf(context);
}

// This loads the properties of the datasets (chart:series).
// FIXME: This is a strange place to load them (the proxy model)
bool ChartProxyModel::loadOdf(const KoXmlElement &element,
                              KoShapeLoadingContext &context, int seriesPerDataset, ChartType type)
{
    Q_ASSERT(d->isLoading);

//     PlotArea* plotArea = dynamic_cast< PlotArea* >(QObject::parent());

//     const bool stockChart = element.attributeNS(KoXmlNS::chart, "class", QString()) == "chart:stock";

    OdfLoadingHelper *helper = (OdfLoadingHelper*)context.sharedData(OdfLoadingHelperId);
    bool ignoreCellRanges = helper->chartUsesInternalModelOnly;
// Some OOo documents save incorrect cell ranges. For those this fix was intended.
// Find out which documents exactly and only use fix for as few cases as possible.
#if 0
    // If we exclusively use the chart's internal model then all data
    // is taken from there and each data set is automatically assigned
    // the rows it belongs to.
    bool ignoreCellRanges = helper->chartUsesInternalModelOnly;
#endif

    beginResetModel();

    if (element.hasAttributeNS(KoXmlNS::chart, "style-name")) {
        KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
        styleStack.clear();
        context.odfLoadingContext().fillStyleStack(element, KoXmlNS::chart, "style-name", "chart");

        // Data direction: It's in the plotarea style.
        if (styleStack.hasProperty(KoXmlNS::chart, "series-source")) {
            QString seriesSource = styleStack.property(KoXmlNS::chart, "series-source");
            // Check if the direction for data series is vertical or horizontal.
            if (seriesSource == "rows")
                d->dataDirection = Qt::Horizontal;
            else if (seriesSource == "columns")
                d->dataDirection = Qt::Vertical;
            // Otherwise leave our default value
        }
    }

    // Find out if the data table contains labels as first row and/or column.
    // This is in the plot-area element itself.

    // Do not ignore the data-source-has-labels in any case, even if a
    // category data region is specified for an axis, as the first
    // column still has to be exluded from the actual data region if
    // e.g. data-source-has-labels is set to "column" If an axis
    // contains the chart:categories element, the category data region
    // will automatically be set on every data set attached to that
    // axis. See Axis::attachDataSet().
    const QString dataSourceHasLabels
        = element.attributeNS(KoXmlNS::chart, "data-source-has-labels");
    if (dataSourceHasLabels == "both") {
        d->firstRowIsLabel = true;
        d->firstColumnIsLabel = true;
    } else if (dataSourceHasLabels == "row") {
        d->firstRowIsLabel = true;
        d->firstColumnIsLabel = false;
    } else if (dataSourceHasLabels == "column") {
        d->firstRowIsLabel = false;
        d->firstColumnIsLabel = true;
    } else {
        // dataSourceHasLabels == "none" or wrong value
        d->firstRowIsLabel = false;
        d->firstColumnIsLabel = false;
    }

    // For every data set, there must be an explicit <chart:series> element,
    // which we will load later.
    d->dataSets.clear();
    d->removedDataSets.clear();

    // A cell range for all data is optional.
    // If cell ranges are in addition specified for one or more of these
    // data series, they'll be overwritten by these values.
    // Note: In case ignoreCellRanges is true, ChartShape::loadOdf() has
    // already made sure the proxy is reset with data from the internal model.
    if (!ignoreCellRanges
        && element.hasAttributeNS(KoXmlNS::table, "cell-range-address"))
    {
        QString cellRangeAddress = element.attributeNS(KoXmlNS::table, "cell-range-address");
        d->selection = CellRegion(d->tableSource, cellRangeAddress);
    // Otherwise use all data from internal table
    } else if (helper->chartUsesInternalModelOnly) {
        QList<Table*> tables = helper->tableSource->tableMap().values();
        Q_ASSERT(tables.count() == 1);
        Table *internalTable = tables.first();
        Q_ASSERT(internalTable->model());
        int rowCount = internalTable->model()->rowCount();
        int colCount = internalTable->model()->columnCount();
        d->selection = CellRegion(internalTable, QRect(1, 1, colCount, rowCount));
    }

    // This is what we'll use as basis for the data sets we "produce" from ODF.
    // This might be data sets that were "instantiated" from the internal
    // table or from an arbitrary selection of other tables as specified
    // in the PlotArea's table:cell-range-address attribute (parsed above).
    QList<DataSet*> createdDataSets = d->createDataSetsFromRegion(&d->removedDataSets,
                                                                   !helper->categoryRegionSpecifiedInXAxis);

    bool isBubble = d->shape->plotArea()->chartType() == BubbleChartType;
    bool isScatter = d->shape->plotArea()->chartType() == ScatterChartType;
    CellRegion prevXData, prevYData;

    int loadedDataSetCount = 0;
    KoXmlElement n;
    QPen p;
    QBrush brush;
    bool penLoaded = false;
    bool brushLoaded = false;
    int stockSeriesCounter = 0;
    forEachElement (n, element) {
        if (n.namespaceURI() != KoXmlNS::chart)
            continue;

        if (n.localName() == "series") {
            if (stockSeriesCounter == 0) {
                DataSet *dataSet;
                if (loadedDataSetCount < createdDataSets.size()) {
                    dataSet = createdDataSets[loadedDataSetCount];
                }
                else {
                    // the datasetnumber needs to be known at construction time, to ensure
                    // default colors are set correctly
                    dataSet = new DataSet(d->dataSets.size());
                    // add the newly created dataSet to the createdDataSets list so our
                    // stockSeriesCounter != 0 condition below is able to pick it up.
                    createdDataSets.append(dataSet);
                }
                dataSet->setChartType(type);
                d->dataSets.append(dataSet);
                if (d->categoryDataRegion.isValid()) {
                    dataSet->setCategoryDataRegion(d->categoryDataRegion);
                }
                dataSet->loadOdf(n, context);

                if (isBubble || isScatter) {
                    // bubble- and scatter-charts have chart:domain's that define the
                    // x- and y-data. But if they are not defined in the series then
                    // a previous defined one needs to be used.
                    if (dataSet->xDataRegion().isValid()) {
                        prevXData = dataSet->xDataRegion();
                    }
                    else {
                        dataSet->setXDataRegion(prevXData);
                    }
                    if (isBubble) {
                        if (dataSet->yDataRegion().isValid()) {
                            prevYData = dataSet->yDataRegion();
                        }
                        else {
                            dataSet->setYDataRegion(prevYData);
                        }
                    }
                }

                if (penLoaded)
                    dataSet->setPen(p);
                if (brushLoaded)
                    dataSet->setBrush(brush);
            }
            else {
                DataSet *dataSet;
                if (loadedDataSetCount < createdDataSets.size())
                    dataSet = createdDataSets[loadedDataSetCount];
                else {
                    Q_ASSERT_X(false, __FUNCTION__, "Unexpected series. Is the document broken?");
                    continue; // be sure we don't crash in release-mode if that happens
                }
                dataSet->loadSeriesIntoDataset(n, context);
            }
            stockSeriesCounter = (stockSeriesCounter + 1) %  seriesPerDataset;
            if (stockSeriesCounter == 0)
                ++loadedDataSetCount;
        } else if (n.localName() == "stock-range-line") {
            KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
            styleStack.clear();

            context.odfLoadingContext().fillStyleStack(n, KoXmlNS::chart, "style-name", "chart");
            if (n.hasAttributeNS(KoXmlNS::chart, "style-name")) {
                KoOdfLoadingContext &odfLoadingContext = context.odfLoadingContext();
                brushLoaded = false;
                penLoaded = false;

                styleStack.setTypeProperties("graphic");

                if (styleStack.hasProperty(KoXmlNS::svg, "stroke-color")) {
                    QString stroke = "solid";/*styleStack.property(KoXmlNS::svg, "stroke-color");*/
                    p = KoOdfGraphicStyles::loadOdfStrokeStyle(styleStack, stroke, odfLoadingContext.stylesReader());
                    penLoaded = true;
                    Q_FOREACH(DataSet* set, d->dataSets) {
                        set->setPen(p);
                        set->setBrush(p.color());
                    }
                }

                if (styleStack.hasProperty(KoXmlNS::draw, "fill")) {
                    QString fill = styleStack.property(KoXmlNS::draw, "fill");
                    if (fill == "solid" || fill == "hatch") {
                        brush = KoOdfGraphicStyles::loadOdfFillStyle(styleStack, fill, odfLoadingContext.stylesReader());
                        brushLoaded = true;
                    } else if (fill == "gradient") {
                        brush = KoOdfGraphicStyles::loadOdfGradientStyle(styleStack, odfLoadingContext.stylesReader(), QSizeF(5.0, 60.0));
                        brushLoaded = true;
                    }
                    Q_FOREACH(DataSet* set, d->dataSets) {
                        set->setBrush(brush);
                    }
                }
            }
        }
    }

    //rebuildDataMap();
    endResetModel();

    return true;
}


QVariant ChartProxyModel::data(const QModelIndex &index,
                                int role) const
{
    Q_UNUSED(index);
    Q_UNUSED(role);
    Q_ASSERT("To be implemented");
    return QVariant();
}

void ChartProxyModel::dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    QPoint topLeftPoint(topLeft.column() + 1, topLeft.row() + 1);

    // Excerpt from the Qt reference for QRect::bottomRight() which is
    // used for calculating bottomRight.  Note that for historical
    // reasons this function returns
    //   QPoint(left() + width() -1, top() + height() - 1).
    QPoint bottomRightPoint(bottomRight.column() + 1, bottomRight.row() + 1);
    QRect dataChangedRect = QRect(topLeftPoint,
                                   QSize(bottomRightPoint.x() - topLeftPoint.x() + 1,
                                          bottomRightPoint.y() - topLeftPoint.y() + 1));
    // Precisely determine what data in what table changed so that we don't
    // do unnecessary, expensive updates.
    Table *table = d->tableSource->get(topLeft.model());
    CellRegion dataChangedRegion(table, dataChangedRect);

    foreach (DataSet *dataSet, d->dataSets) {
        if (dataSet->xDataRegion().intersects(dataChangedRegion))
            dataSet->xDataChanged(QRect());

        if (dataSet->yDataRegion().intersects(dataChangedRegion))
            dataSet->yDataChanged(QRect());

        if (dataSet->categoryDataRegion().intersects(dataChangedRegion))
            dataSet->categoryDataChanged(QRect());

        if (dataSet->labelDataRegion().intersects(dataChangedRegion))
            dataSet->labelDataChanged(QRect());

        if (dataSet->customDataRegion().intersects(dataChangedRegion))
            dataSet->customDataChanged(QRect());
    }

    emit dataChanged();
}


QVariant ChartProxyModel::headerData(int section,
                                     Qt::Orientation orientation,
                                     int role /* = Qt::DisplayRole */) const
{
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(role);
    Q_ASSERT("To be implemented");
    return QVariant();
}


QModelIndex ChartProxyModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);

    return QModelIndex();
}

int ChartProxyModel::rowCount(const QModelIndex &/*parent  = QModelIndex() */) const
{
    return d->dataSets.count();
}


int ChartProxyModel::columnCount(const QModelIndex &/*parent = QModelIndex() */) const
{
    // FIXME: Replace this by the actual column count once the proxy is properly being used.
    return INT_MAX;
}

void ChartProxyModel::setFirstRowIsLabel(bool b)
{
    if (b == d->firstRowIsLabel)
        return;

    d->firstRowIsLabel = b;
    d->rebuildDataMap();
}


void ChartProxyModel::setFirstColumnIsLabel(bool b)
{
    if (b == d->firstColumnIsLabel)
        return;

    d->firstColumnIsLabel = b;
    d->rebuildDataMap();
}

Qt::Orientation ChartProxyModel::dataDirection()
{
    return d->dataDirection;
}

void ChartProxyModel::invalidateDataSets()
{
    d->removedDataSets = d->dataSets;
    d->dataSets.clear();
}

void ChartProxyModel::beginLoading()
{
    Q_ASSERT(!d->isLoading);

    beginResetModel();

    // FIXME: invalidateDataSets() used to be called explicitly at the beginning
    // of ChartShape::loadOdf(). Now beginLoading() is called instead.
    // So, is invalidateDataSets() still necessary here?
    invalidateDataSets();
    d->isLoading = true;
}

void ChartProxyModel::endLoading()
{
    Q_ASSERT(d->isLoading);
    d->isLoading = false;

    // Doing this here is wrong, the data set's cell regions set in
    // DataSet::loadOdf() would get overridden.
    // d->rebuildDataMap();

    endResetModel();
}

bool ChartProxyModel::isLoading() const
{
    return d->isLoading;
}

void ChartProxyModel::setDataDirection(Qt::Orientation orientation)
{
    if (d->dataDirection == orientation)
        return;

    d->dataDirection = orientation;
    d->rebuildDataMap();
}

void ChartProxyModel::setDataDimensions(int dimensions)
{
    if (d->dataDimensions == dimensions)
        return;

    d->dataDimensions = dimensions;
    d->rebuildDataMap();
}

bool ChartProxyModel::firstRowIsLabel() const
{
    return d->firstRowIsLabel;
}

bool ChartProxyModel::firstColumnIsLabel() const
{
    return d->firstColumnIsLabel;
}

CellRegion ChartProxyModel::categoryDataRegion() const
{
    return d->categoryDataRegion;
}

void ChartProxyModel::setCategoryDataRegion(const CellRegion &region)
{
    d->categoryDataRegion = region;
}

QList<DataSet*> ChartProxyModel::dataSets() const
{
    return d->dataSets;
}

#include "ChartProxyModel.moc"
