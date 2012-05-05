/* This file is part of the KDE project

   Copyright 2007 Johannes Simon <johannes.simon@gmail.com>

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

#ifndef KCHART_PROXYMODEL_H
#define KCHART_PROXYMODEL_H


#define INTERNAL_TABLE_NAME "ChartTable"


// KChart
#include "ChartShape.h"
#include "CellRegion.h"


namespace KoChart
{
    class ChartModel;
}


// Qt
#include <QAbstractTableModel>


namespace KChart {

/**
 * @brief The ChartProxyModel is a factory for the DataSet's and decorates the ChartTableModel.
 *
 * TODO: Rename this class to something more meaningful (and correct) like
 * "DataProvider" and maybe split it up into one class that handles the
 * QAbstractItemModel part, and another one that handles CellRegions for
 * all the data points.
 */
class CHARTSHAPELIB_EXPORT ChartProxyModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    ChartProxyModel(ChartShape *shape, TableSource *source);
    ~ChartProxyModel();

    /**
     * Used for data retrieval of all relevant dimensions: x, y, z, etc.
     *
     * This enum may be extended at a later point to store and retrieve
     * attributes.
     */
    enum DataRole {
        XDataRole = Qt::UserRole,
        YDataRole,
        CustomDataRole,
        LabelDataRole,
        CategoryDataRole,
    };

    /**
     * Re-initializes the model with data from an arbitrary region.
     *
     * All data will be taken from the data source passed in the constructor.
     * The ProxyModel will not react on insertions or removals in one of
     * these models.
     */
    void reset(const CellRegion &region);

    /**
     * The CellRegion that data in this proxy model is taken from.
     *
     * In ODF, this is an attribute of the PlotArea, but here the proxy model
     * manages all data sets, thus it's also responsible for this attribute.
     *
     * See table:cell-range-address, ODF v1.2, §19.595
     */
    CellRegion cellRangeAddress() const;

    /**
    * Load series from ODF
    */
    bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context,
                 int seriesPerDataset, ChartType type);
    void saveOdf(KoShapeSavingContext &context) const;

    /**
     * Returns data or properties of a data point.
     *
     * TODO: Not implemented yet. At the moment, DataSet's data and attribute
     * getter are used instead.
     */
    virtual QVariant data(const QModelIndex &index, int role) const;

    /**
     * Returns properties that are global to either a data set or a category,
     * depending on the orientation.
     *
     * If @a orientation is Qt::Horizontal, this method will return properties
     * global do the data set with number @a section.
     * If @a orientation is Qt::Vertical, it will return properties global to
     * the category with index @a section.
     *
     * TODO: Not implemented yet. At the moment, DataSet's data and attribute
     * getter are used instead.
     */
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    virtual QModelIndex parent(const QModelIndex &index) const;

    /**
     * Returns the number of data sets in this model.
     */
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;

    /**
     * Returns maximum the number of data points the data sets have.
     */
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

    // The following methods are specific to the chart
    void setFirstRowIsLabel(bool b);
    void setFirstColumnIsLabel(bool b);
    void setDataDirection(Qt::Orientation orientation);
    void setDataDimensions(int dimensions);

    bool firstRowIsLabel() const;
    bool firstColumnIsLabel() const;
    Qt::Orientation dataDirection();
    
    /**
     * @see setCategoryDataRegion()
     */
    CellRegion categoryDataRegion() const;

    /**
     * Sets the region to use for categories, i.e. the labels for a certain
     * index in all data sets. This is what will be used to label points
     * on the x axis in a cartesian chart and what will be used as legend
     * items in a polar chart.
     */
    void setCategoryDataRegion(const CellRegion &region);

    /**
     * A list of all data sets that are currently being used in the chart.
     */
    QList<DataSet*> dataSets() const;

    /**
     * Clears the list of data sets, but keeps them in a list of "removed"
     * data sets for the next time that reset() is called. The latter list
     * will be re-used so that properties of data sets don't get lost.
     */
    void invalidateDataSets();

    /**
     * Called by ChartShape when it begins loading from ODF.
     *
     * The proxy model will then avoid any insertion, removal or data-change
     * signals or calls to speed up loading (significantly for large amounts
     * of data).
     *
     * Properties like firstRowIsLabel() can still be modified, the changes
     * will simply not get propagated until endLoading() is called.
     */
    void beginLoading();

    /**
     * Called by ChartShape when it is done loading from ODF.
     *
     * The proxy model will then be reset with its current properties.
     */
    void endLoading();

    /**
     * Returns true if beginLoading() got called and endLoading() not
     * yet what means we are in a longer loading process.
     */
    bool isLoading() const;

public slots:
    /**
     * Connected to dataChanged() signal of source models in TableSource.
     */
    virtual void dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);

    /**
     * Called by the TableSource whenever a table is added to it.
     *
     * TODO: It might improve performance if tables are only added when
     * they are really in use. That is not necessarily the case if they
     * are in the TableSource.
     */
    void addTable(Table *table);

    /**
     * Called by the TableSource whenever a table is removed from it.
     */
    void removeTable(Table *table);

signals:
    void dataChanged();

private:
    class Private;
    Private *const d;
};

} // namespace KChart

#endif // KCHART_PROXYMODEL_H
