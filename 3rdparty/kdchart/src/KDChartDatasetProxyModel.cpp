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

#include "KDChartDatasetProxyModel.h"

#include <QtDebug>

#include <KDABLibFakes>


using namespace KDChart;

DatasetProxyModel::DatasetProxyModel (QObject* parent)
    : QSortFilterProxyModel ( parent )
{
}

QModelIndex DatasetProxyModel::buddy( const QModelIndex& index ) const
{
    return index;
}

Qt::ItemFlags DatasetProxyModel::flags( const QModelIndex& index ) const
{
    return sourceModel()->flags( mapToSource( index ) );
}

void DatasetProxyModel::setDatasetRowDescriptionVector (
    const DatasetDescriptionVector& configuration )
{
    Q_ASSERT_X ( sourceModel(), "DatasetProxyModel::setDatasetRowDescriptionVector",
                 "A source model must be set before the selection can be configured." );
    initializeDatasetDecriptors ( configuration, sourceModel()->rowCount(mRootIndex),
                                  mRowSrcToProxyMap,  mRowProxyToSrcMap );
    clear(); // clear emits layoutChanged()
}

void DatasetProxyModel::setDatasetColumnDescriptionVector (
    const DatasetDescriptionVector& configuration )
{
    Q_ASSERT_X ( sourceModel(), "DatasetProxyModel::setDatasetColumnDescriptionVector",
                 "A source model must be set before the selection can be configured." );
    initializeDatasetDecriptors ( configuration, sourceModel()->columnCount(mRootIndex),
                                  mColSrcToProxyMap, mColProxyToSrcMap );
    clear(); // clear emits layoutChanged()
}

void DatasetProxyModel::setDatasetDescriptionVectors (
    const DatasetDescriptionVector& rowConfig,
    const DatasetDescriptionVector& columnConfig )
{
    setDatasetRowDescriptionVector( rowConfig );
    setDatasetColumnDescriptionVector ( columnConfig );
}

QModelIndex DatasetProxyModel::index( int row, int column, 
                                      const QModelIndex &parent ) const
{
    return mapFromSource( sourceModel()->index( mapProxyRowToSource(row),
                                                mapProxyColumnToSource(column),
                                                parent ) );
}

QModelIndex DatasetProxyModel::parent( const QModelIndex& child ) const
{
//    return mapFromSource( sourceModel()->parent( child ) );
    return mapFromSource( sourceModel()->parent( mapToSource( child ) ) );
}

QModelIndex DatasetProxyModel::mapFromSource ( const QModelIndex & sourceIndex ) const
{
    Q_ASSERT_X ( sourceModel(), "DatasetProxyModel::mapFromSource", "A source "
                 "model must be set before the selection can be configured." );

    if ( !sourceIndex.isValid() ) return sourceIndex;

    if ( mRowSrcToProxyMap.isEmpty() && mColSrcToProxyMap.isEmpty() )
    {
        return createIndex ( sourceIndex.row(), sourceIndex.column(),
                             sourceIndex.internalPointer() );
    } else {
        int row = mapSourceRowToProxy ( sourceIndex.row() );
        int column = mapSourceColumnToProxy ( sourceIndex.column() );
        return createIndex ( row, column, sourceIndex.internalPointer() );
    }
}

QModelIndex DatasetProxyModel::mapToSource ( const QModelIndex& proxyIndex ) const
{
    Q_ASSERT_X ( sourceModel(), "DatasetProxyModel::mapToSource", "A source "
                 "model must be set before the selection can be configured." );

    if ( !proxyIndex.isValid() ) return proxyIndex;
    if ( mRowSrcToProxyMap.isEmpty() && mColSrcToProxyMap.isEmpty() )
    {
        return sourceModel()->index( proxyIndex.row(),  proxyIndex.column(), mRootIndex );
    } else {
        int row = mapProxyRowToSource ( proxyIndex.row() );
        int column = mapProxyColumnToSource ( proxyIndex.column() );
        return sourceModel()->index( row, column, mRootIndex );
    }
}

bool DatasetProxyModel::filterAcceptsRow ( int sourceRow,
                                           const QModelIndex & ) const
{
    if ( mRowSrcToProxyMap.isEmpty() )
    {   // no row mapping set, all rows are passed down:
        return true;
    } else {
        Q_ASSERT ( sourceModel() );
        Q_ASSERT ( mRowSrcToProxyMap.size() == sourceModel()->rowCount(mRootIndex) );
        if ( mRowSrcToProxyMap[sourceRow] == -1 )
        {   // this row is explicitly not accepted:
            return false;
        } else {
            Q_ASSERT ( mRowSrcToProxyMap[sourceRow] >= 0
                       && mRowSrcToProxyMap[sourceRow] < mRowSrcToProxyMap.size() );
            return true;
        }
    }
}

bool DatasetProxyModel::filterAcceptsColumn ( int sourceColumn,
                                              const QModelIndex & ) const
{
    if ( mColSrcToProxyMap.isEmpty() )
    {   // no column mapping set up yet, all columns are passed down:
        return true;
    } else {
        Q_ASSERT ( sourceModel() );
        Q_ASSERT ( mColSrcToProxyMap.size() == sourceModel()->columnCount(mRootIndex) );
        if ( mColSrcToProxyMap[sourceColumn] == -1 )
        {   // this column is explicitly not accepted:
            return false;
        } else {
            Q_ASSERT ( mColSrcToProxyMap[sourceColumn] >= 0
                       && mColSrcToProxyMap[sourceColumn] < mColSrcToProxyMap.size() );
            return true;
        }
    }
}

int DatasetProxyModel::mapProxyRowToSource ( const int& proxyRow ) const
{
    if ( mRowProxyToSrcMap.isEmpty() )
    {   // if no row mapping is set, we pass down the row:
        return proxyRow;
    } else {
        Q_ASSERT ( proxyRow >= 0 && proxyRow < mRowProxyToSrcMap.size() );
        return mRowProxyToSrcMap[ proxyRow ];
    }
}

int DatasetProxyModel::mapProxyColumnToSource ( const int& proxyColumn ) const
{
    if ( mColProxyToSrcMap.isEmpty()  )
    {   // if no column mapping is set, we pass down the column:
        return proxyColumn;
    } else {
        Q_ASSERT ( proxyColumn >= 0 && proxyColumn < mColProxyToSrcMap.size() );
        return mColProxyToSrcMap[ proxyColumn ];
    }
}

int DatasetProxyModel::mapSourceRowToProxy ( const int& sourceRow ) const
{
    if ( mRowSrcToProxyMap.isEmpty() )
    {
        return sourceRow;
    } else {
        Q_ASSERT ( sourceRow >= 0 && sourceRow < mRowSrcToProxyMap.size() );
        return mRowSrcToProxyMap[sourceRow];
    }
}

int DatasetProxyModel::mapSourceColumnToProxy ( const int& sourceColumn ) const
{
    if ( mColSrcToProxyMap.isEmpty() )
    {
        return sourceColumn;
    } else {
        Q_ASSERT ( sourceColumn >= 0 && sourceColumn < mColSrcToProxyMap.size() );
        return mColSrcToProxyMap.at( sourceColumn ) ;
    }
}

void DatasetProxyModel::resetDatasetDescriptions()
{
    mRowSrcToProxyMap.clear();
    mRowProxyToSrcMap.clear();
    mColSrcToProxyMap.clear();
    mColProxyToSrcMap.clear();
    clear();
}

QVariant DatasetProxyModel::data(const QModelIndex &index, int role) const
{
   return sourceModel()->data( mapToSource ( index ), role );
}

bool DatasetProxyModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
    return sourceModel()->setData( mapToSource( index ), value, role );
}

QVariant DatasetProxyModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
    if ( orientation == Qt::Horizontal )
    {
        if ( mapProxyColumnToSource ( section ) == -1 )
        {
            return QVariant();
        } else {
            return sourceModel()->headerData ( mapProxyColumnToSource ( section ),
                                                       orientation,  role );
        }
    } else {
        if ( mapProxyRowToSource ( section ) == -1 )
        {
            return QVariant();
        } else {
            return sourceModel()->headerData ( mapProxyRowToSource ( section ),
                                                       orientation, role );
        }
    }
}

void DatasetProxyModel::initializeDatasetDecriptors (
    const DatasetDescriptionVector& inConfiguration,
    const int sourceCount,
    DatasetDescriptionVector& outSourceToProxyMap,
    DatasetDescriptionVector& outProxyToSourceMap )
{
	// in the current mapping implementation, the proxy-to-source map is
    // identical to the configuration vector:
    outProxyToSourceMap = inConfiguration;
    outSourceToProxyMap.fill ( -1,  sourceCount );
	
    for ( int index = 0; index < inConfiguration.size(); ++index )
    {   
		// make sure the values in inConfiguration point to columns in the
        // source model:
		
		if (inConfiguration[index] == -1)
			continue;

        Q_ASSERT_X ( inConfiguration[index] >= 0
                   && inConfiguration[index] < sourceCount,
                     "DatasetProxyModel::initializeDatasetDecriptors",
                     "column index outside of source model" );
        Q_ASSERT_X ( outSourceToProxyMap[inConfiguration[index]] == -1 ,
                     "DatasetProxyModel::initializeDatasetDecriptors",
                     "no duplicates allowed in mapping configuration, mapping has to be revertible" );
		
        outSourceToProxyMap[inConfiguration[index]] = index;
		
    }
}

void DatasetProxyModel::setSourceModel (QAbstractItemModel *m)
{
    if ( sourceModel() ) {
        disconnect ( sourceModel(),  SIGNAL ( layoutChanged() ),
                     this, SLOT( resetDatasetDescriptions() ) );
    }
    QSortFilterProxyModel::setSourceModel ( m );
    mRootIndex = QModelIndex();
    if ( m ) {
        connect ( m,  SIGNAL ( layoutChanged() ),
                  this, SLOT( resetDatasetDescriptions() ) );
    }
    resetDatasetDescriptions();
}

void DatasetProxyModel::setSourceRootIndex(const QModelIndex& rootIdx)
{
    mRootIndex = rootIdx;
    resetDatasetDescriptions();
}

