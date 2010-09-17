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
 */
class CHARTSHAPELIB_EXPORT ChartProxyModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    ChartProxyModel( TableSource *source );
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
    void reset( const CellRegion &region );

public slots:
    QList<DataSet*> createDataSetsFromRegion( QList<DataSet*> dataSetsToRecycle );

    /**
    * Load series from ODF
    */
    bool loadOdf( const KoXmlElement &element, KoShapeLoadingContext &context );
    void saveOdf( KoShapeSavingContext &context ) const;

    virtual QVariant data( const QModelIndex &index, int role ) const;
    virtual QVariant headerData( int section, Qt::Orientation orientation, int role ) const;

    virtual void dataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight );

    virtual QModelIndex parent( const QModelIndex &index ) const;

    /**
     * Returns the number of data sets in this model.
     */
    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;

    /**
     * Returns maximum the number of data points the data sets have.
     */
    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;

    // The following methods are specific to the chart
    void setFirstRowIsLabel( bool b );
    void setFirstColumnIsLabel( bool b );
    void setDataDirection( Qt::Orientation orientation );
    void setDataDimensions( int dimensions );

    bool firstRowIsLabel() const;
    bool firstColumnIsLabel() const;
    Qt::Orientation dataDirection();
    
    CellRegion categoryDataRegion() const;
    void setCategoryDataRegion( const CellRegion &region );

    QList<DataSet*> dataSets() const;

    void invalidateDataSets();
    /**
     * Discards old and creates new data sets from the current region selection
     * if and only if automaticDataSetCreation() returns true.
     */
    void rebuildDataMap();

    /**
     * Called by the TableSource whenever a table is added to it.
     *
     * TODO: It might improve performance if tables are only added when
     * they are really in use. That is not necessarily the case if they
     * are in the TableSource.
     */
    void addTable( Table *table );

    /**
     * Called by the TableSource whenever a table is removed from it.
     */
    void removeTable( Table *table );

signals:
    void dataChanged();

private:
#if QT_VERSION < 0x040600
    //! @todo Remove once we drop support for Qt < 4.6
    //! For compatability with Qt < 4.6
    void beginResetModel();
    void endResetModel();
#endif
    class Private;
    Private *const d;
};

} // namespace KChart

#endif // KCHART_PROXYMODEL_H
