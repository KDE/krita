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


// KChart
#include "ChartShape.h"
#include "CellRegion.h"


namespace KoChart
{
    class ChartModel;
}


// Qt
#include <QAbstractProxyModel>


namespace KChart {

/**
 * @brief The ChartProxyModel is a factory for the DataSet's and decorates the ChartTableModel.
 */
class CHARTSHAPELIB_EXPORT ChartProxyModel : public QAbstractProxyModel
{
    Q_OBJECT

public:
    ChartProxyModel();
    ~ChartProxyModel();

public slots:
    virtual void setSourceModel( QAbstractItemModel *sourceModel );
    virtual void setSourceModel( QAbstractItemModel *sourceModel,
                                 const QVector<QRect> &selection );

    void setSelection( const QVector<QRect> &selection );
    
    void setAutomaticDataSetCreation( bool enable );
    bool automaticDataSetCreation() const;

    QList<DataSet*> createDataSetsFromRegion( QList<DataSet*> dataSetsToRecycle );

    /**
    * Load series from ODF
    */
    bool loadOdf( const KoXmlElement &element, KoShapeLoadingContext &context );
    void saveOdf( KoShapeSavingContext &context ) const;

    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    virtual void dataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight );

    virtual QMap<int, QVariant> itemData( const QModelIndex &index ) const;

    virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
    virtual QModelIndex parent( const QModelIndex &index ) const;

    virtual QModelIndex mapFromSource( const QModelIndex &sourceIndex ) const;
    virtual QModelIndex mapToSource( const QModelIndex &proxyIndex ) const;

    Qt::Orientation mapFromSource( Qt::Orientation orientation ) const;
    Qt::Orientation mapToSource( Qt::Orientation orientation ) const;

    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;

    // The following methods are specific to the chart
    void setFirstRowIsLabel( bool b );
    void setFirstColumnIsLabel( bool b );
    void setDataDirection( Qt::Orientation orientation );
    void setDataDimensions( int dimensions );
    
    void slotRowsInserted( const QModelIndex &parent, int start, int end );
    void slotRowsRemoved( const QModelIndex &parent, int start, int end );
    void slotColumnsInserted( const QModelIndex &parent, int start, int end );
    void slotColumnsRemoved( const QModelIndex &parent, int start, int end );

    bool firstRowIsLabel() const;
    bool firstColumnIsLabel() const;
    Qt::Orientation dataDirection();
    
    QString categoryDataRegion() const;
    void setCategoryDataRegion(const QString& region);

    QList<DataSet*> dataSets() const;

    void invalidateDataSets();
    /**
     * Discards old and creates new data sets from the current region selection
     * if and only if automaticDataSetCreation() returns true.
     */
    void rebuildDataMap();

signals:
    void dataChanged();

protected slots:
    void slotModelReset();

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
