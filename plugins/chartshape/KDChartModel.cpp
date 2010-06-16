/* This file is part of the KDE project

   Copyright 2008 Johannes Simon <johannes.simon@gmail.com>
   Copyright 2009 Inge Wallin    <inge@lysator.liu.se>
   Copyright (C) 2010 Carlos Licea <carlos@kdab.com>
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
   Boston, MA 02110-1301, USA.
*/


// Own
#include "KDChartModel.h"

// KDE
#include <KDebug>

// KChart
#include "DataSet.h"

// KDChart
#include <KDChartGlobal>
#include <KDChartPieAttributes>
#include <KDChartDataValueAttributes>

using namespace KChart;

class KDChartModel::Private {
public:
    Private();
    ~Private();

    /**
     * Returns the index of a given dataSet. If it is not present in
     * this model (i.e. not attached, thus not in dataSets), an index
     * is returned before which this dataSet should be inserted.
     */
    int  dataSetIndex( DataSet *dataSet ) const;

    /**
     * Returns the cached (!) max size of all data sets in this model.
     */
    int  maxDataSetSize() const;

    /**
     * Calculates the maximum size of all data sets in the passed list.
     */
    int  calcMaxDataSetSize( QList<DataSet*> list ) const;

    /**
     * Only calculates the new size for the current data set list,
     * but does not update it.
     */
    int  calcMaxDataSetSize() const;

    bool isKnownDataRole( int role ) const;

    int             dataDimensions;
    int             biggestDataSetSize;
    QList<DataSet*> dataSets;

    Qt::Orientation dataDirection;
};


KDChartModel::Private::Private()
{
    dataDimensions      = 1;
    dataDirection       = Qt::Vertical;
    biggestDataSetSize  = 0;
}

KDChartModel::Private::~Private()
{
}

int KDChartModel::Private::maxDataSetSize() const
{
    return biggestDataSetSize;
}

int KDChartModel::Private::calcMaxDataSetSize( QList<DataSet*> list ) const
{
    int maxSize = 0;
    foreach ( DataSet *dataSet, list )
        maxSize = qMax( maxSize, dataSet->size() );
    return maxSize;
}

int KDChartModel::Private::calcMaxDataSetSize() const
{
    return calcMaxDataSetSize( dataSets );
}

int KDChartModel::Private::dataSetIndex( DataSet *dataSet ) const
{
    // If data set is not in our list yet, find out where to insert it
    if ( !dataSets.contains( dataSet ) ) {
        for ( int i = 0; i < dataSets.size(); i++ ) {
            if ( dataSet->number() < dataSets[ i ]->number() )
                return i;
        }

        return dataSets.size();
    }

    // Otherwise simply return its index
    return dataSets.indexOf( dataSet );
}


// ================================================================
//                     class KDChartModel


KDChartModel::KDChartModel( QObject *parent /* = 0 */ )
    : QAbstractItemModel( parent ),
      d( new Private )
{
}

KDChartModel::~KDChartModel()
{
    delete d;
}


void KDChartModel::setDataDirection( Qt::Orientation direction )
{
    d->dataDirection = direction;
}

Qt::Orientation KDChartModel::dataDirection() const
{
    return d->dataDirection;
}

Qt::Orientation KDChartModel::categoryDirection() const
{
    return d->dataDirection == Qt::Horizontal ? Qt::Vertical : Qt::Horizontal;
}

QVariant KDChartModel::data( const QModelIndex &index,
                             int role /* = Qt::DisplayRole */ ) const
{
    if ( !index.isValid() ||
         !d->isKnownDataRole( role ) ) {
        return QVariant();
    }

    int dataSetNumber, section;
    // Offset from the data set's row or column (depending on the data direction).
    // With one data dimension, it's always 0. Otherwise it's 1 for y data, and
    // in all other cases 0 as well.
    // In other words, with one data dimension, there's only y data. With two data
    // dimensions, there's x and y data, and the x data comes before the y data,
    // each data set thus occupying two rows/columns.
    // With three data dimensions (though we don't support that yet) there will be
    // x, y and some kind of custom data. Like the size of a bubble in a bubble chart.
    int dataSection;

    if ( dataDirection() == Qt::Horizontal ) {
        dataSetNumber = index.row() / d->dataDimensions;
        dataSection = index.row() % d->dataDimensions;
        section = index.column();
    } else {
        dataSetNumber = index.column() / d->dataDimensions;
        dataSection = index.column() % d->dataDimensions;
        section = index.row();
    }

    if( dataSetNumber >= d->dataSets.size() )
        return QVariant();

    DataSet  *dataSet       = d->dataSets[ dataSetNumber ];

    switch ( role ) {
    case Qt::DisplayRole:
        if ( d->dataDimensions > 1 && dataSection == 0 )
            return dataSet->xData( section );
        else
            return dataSet->yData( section ).isValid() ? dataSet->yData( section ) : QVariant( 1 );
    case KDChart::DatasetBrushRole:
        return dataSet->brush( section );
    case KDChart::DatasetPenRole:
        return dataSet->pen( section );
    case KDChart::PieAttributesRole:
        return QVariant::fromValue( dataSet->pieAttributes( section ) );
    case KDChart::DataValueLabelAttributesRole:
        return QVariant::fromValue( dataSet->dataValueAttributes( section ) );
    }

    // TODO (Johannes): Support for third data dimension
    // We need to implement zData in Dataset first.

    // Should never happen
    return QVariant();
}


void KDChartModel::dataSetChanged( DataSet *dataSet )
{
    Q_ASSERT( d->dataSets.contains( dataSet ) );
    if ( !d->dataSets.contains( dataSet ) )
        return;

    int dataSetColumn = d->dataSetIndex( dataSet ) * dataDimensions();

    emit headerDataChanged( categoryDirection(), dataSetColumn, dataSetColumn );
}

void KDChartModel::dataSetChanged( DataSet *dataSet, DataRole role, int first, int last /* = -1 */ )
{
    Q_ASSERT( d->dataSets.contains( dataSet ) );
    if ( !d->dataSets.contains( dataSet ) )
        return;

    // be sure the 'first' and 'last' referenced rows are within our boundaries
    const int rows = rowCount();
    if ( first >= rows )
        first = rows - 1;
    if ( last >= rows )
        last = rows - 1;
    // 'last' defaults to -1, which means only one column was changed
    if ( last == -1 )
        last = first;
    // 'first' can be negative either cause rowCount()==0 or cause it still contains the default value of -1. In both cases we abort
    // and don't progress the update-request future. Same is true for last which should at this point contain a valid row index too.
    if ( first < 0 || last < 0 )
        return;
    // be sure we are not dealing with inverse order
    if ( last < first )
        qSwap(first , last);

    int dataSetColumn = d->dataSetIndex( dataSet ) * dataDimensions();

    if ( dataDimensions() > 1 ) {
        switch ( role ) {
        case YDataRole:
            dataSetColumn += 1;
            break;
        default:
            ;
        }
    }

    emit dataChanged( index( first, dataSetColumn ),
                      index( last,  dataSetColumn ) );
}

void KDChartModel::dataSetSizeChanged( DataSet *dataSet, int newSize )
{
    Q_UNUSED( newSize );

    int dataSetIndex = d->dataSets.indexOf( dataSet );
    if ( dataSetIndex < 0 ) {
        qWarning() << "KDChartModel::dataSetSizeChanged(): The data set is not assigned to this model.";
        return;
    }

    // old max data set size is cached
    const int oldMaxSize = d->maxDataSetSize();
    // Determine new max data set size (the size of dataSet has been changed already)
    const int newMaxSize = d->calcMaxDataSetSize();

    // Columns/rows have been added
    if ( newMaxSize > oldMaxSize ) {
        if ( d->dataDirection == Qt::Horizontal )
            beginInsertColumns( QModelIndex(), oldMaxSize, newMaxSize - 1 );
        else
            beginInsertRows( QModelIndex(), oldMaxSize, newMaxSize - 1 );

        d->biggestDataSetSize = d->calcMaxDataSetSize();

        if ( d->dataDirection == Qt::Horizontal )
            endInsertColumns();
        else
            endInsertRows();
        // Columns/rows have been removed
    } else if ( newMaxSize < oldMaxSize ) {
        if ( d->dataDirection == Qt::Horizontal )
            beginRemoveColumns( QModelIndex(), newMaxSize, oldMaxSize - 1 );
        else
            beginRemoveRows( QModelIndex(), newMaxSize, oldMaxSize - 1 );

        d->biggestDataSetSize = d->calcMaxDataSetSize();

        if ( d->dataDirection == Qt::Horizontal )
            endRemoveColumns();
        else
            endRemoveRows();
    }
}

void KDChartModel::slotColumnsInserted( const QModelIndex& parent,
                                        int start, int end )
{
    if ( d->dataDirection == Qt::Horizontal ) {
        beginInsertColumns( parent, start, end );
        endInsertColumns();
    } else {
        beginInsertRows( parent, start, end );
        endInsertRows();
    }
}

bool KDChartModel::Private::isKnownDataRole( int role ) const
{
    switch ( role ) {
    case Qt::DisplayRole:
    case KDChart::DatasetPenRole:
    case KDChart::DatasetBrushRole:
    case KDChart::PieAttributesRole:
    case KDChart::DataValueLabelAttributesRole:
        return true;
    }

    return false;
}

/**
 * Method for debugging purposes.
 */
QString roleToString( int role )
{
    switch ( role ) {
    case Qt::DisplayRole:
        return "Qt::DisplayRole";
    case KDChart::DatasetBrushRole:
        return "KDChart::DatasetBrushRole";
    case KDChart::DatasetPenRole:
        return "KDChart::DatasetPenRole";
    case KDChart::PieAttributesRole:
        return "KDChart::PieAttributesRole";
    }
    return "Unknown DataRole";
}

QVariant KDChartModel::headerData( int section,
                                   Qt::Orientation orientation,
                                   int role /* = Qt::DisplayRole */ ) const
{
    if ( !d->isKnownDataRole( role ) ) {
        return QVariant();
    }

    if ( d->dataSets.isEmpty() ) {
        qWarning() << "KDChartModel::headerData(): Attempting to request header, but model has no datasets assigned to it.";
        return QVariant();
    }

    if ( orientation != d->dataDirection ) {
        int dataSetNumber = section / d->dataDimensions;
        if ( d->dataSets.count() <= dataSetNumber ) {
            qWarning() << "KDChartModel::headerData(): trying to get more datasets than we have.";
            return QVariant();
        }

        DataSet *dataSet  = d->dataSets[ dataSetNumber ];

        switch ( role ) {
        case Qt::DisplayRole:
            return dataSet->labelData();
        case KDChart::DatasetBrushRole:
            return dataSet->brush();
        case KDChart::DatasetPenRole:
            return dataSet->pen();
        case KDChart::PieAttributesRole:
            return QVariant::fromValue(dataSet->pieAttributes());
        case KDChart::DataValueLabelAttributesRole:
            return QVariant::fromValue( dataSet->dataValueAttributes() );
        }
    }
    /* else if ( orientation == d->dataDirection ) { */
    // FIXME: Find a way to *not* use the first data set, but some method
    // to set category data (including category pen and brush) properly
    // (Setter methods in this class?)
    DataSet *dataSet = d->dataSets[ 0 ];
    switch ( role ) {
    case Qt::DisplayRole:
        return dataSet->categoryData( section );
    case KDChart::DatasetBrushRole:
        return dataSet->brush( section );
    case KDChart::DatasetPenRole:
        return dataSet->pen( section );
    case KDChart::PieAttributesRole:
        return QVariant::fromValue(dataSet->pieAttributes( section ));
    }
    /* } */

    // Do return something even though we should never get to this point.
    return QVariant();
}

QModelIndex KDChartModel::index( int row, int column,
                                 const QModelIndex &parent ) const
{
    Q_UNUSED( parent );

    return createIndex( row, column, 0 );
}

QModelIndex KDChartModel::parent( const QModelIndex &index ) const
{
    Q_UNUSED( index );

    return QModelIndex();
}

int KDChartModel::rowCount( const QModelIndex &parent /* = QModelIndex() */ ) const
{
    Q_UNUSED( parent );

    int rows;
    if ( d->dataDirection == Qt::Vertical )
        rows = d->maxDataSetSize();
    else
        rows = d->dataSets.size() * d->dataDimensions;

    return rows;
}

int KDChartModel::columnCount( const QModelIndex &parent /* = QModelIndex() */ ) const
{
    Q_UNUSED( parent );

    int columns;
    if ( d->dataDirection == Qt::Vertical )
        columns = d->dataSets.size() * d->dataDimensions;
    else
        columns = d->maxDataSetSize();

    return columns;
}

void KDChartModel::setDataDimensions( int dataDimensions )
{
    d->dataDimensions = dataDimensions;
}

int KDChartModel::dataDimensions() const
{
    return d->dataDimensions;
}

void KDChartModel::addDataSet( DataSet *dataSet, bool silent )
{
    if ( d->dataSets.contains( dataSet ) ) {
        qWarning() << "KDChartModel::addDataSet(): Attempting to insert already-contained data set";
        return;
    }

    dataSet->setKdChartModel( this );

    int dataSetIndex = d->dataSetIndex( dataSet );

    if ( silent ) {
        d->dataSets.insert( dataSetIndex, dataSet );
        d->biggestDataSetSize = d->calcMaxDataSetSize();

        // Regenerate the numbers for all the datasets.
        for ( int i = dataSetIndex; i < d->dataSets.size(); i++ )
            d->dataSets[ i ]->setKdDataSetNumber( i );
    }
    else if ( !d->dataSets.isEmpty() ) {
        const int columnAboutToBeInserted = dataSetIndex * d->dataDimensions;
        if ( d->dataDirection == Qt::Vertical ) {
            beginInsertColumns( QModelIndex(), columnAboutToBeInserted,
                                columnAboutToBeInserted + d->dataDimensions - 1 );
        }
        else
            beginInsertRows( QModelIndex(), columnAboutToBeInserted,
                             columnAboutToBeInserted + d->dataDimensions - 1 );
        d->dataSets.insert( dataSetIndex, dataSet );

        // Regenerate the numbers for all the datasets.
        for ( int i = dataSetIndex; i < d->dataSets.size(); i++ )
            d->dataSets[ i ]->setKdDataSetNumber( i );

        if ( d->dataDirection == Qt::Vertical )
            endInsertColumns();
        else
            endInsertRows();

        const int dataSetSize = dataSet->size();
        if ( dataSetSize > d->maxDataSetSize() ) {
            if ( d->dataDirection == Qt::Vertical )
                beginInsertRows( QModelIndex(),
                                 d->maxDataSetSize(), dataSetSize - 1 );
            else
                beginInsertColumns( QModelIndex(),
                                    d->maxDataSetSize(), dataSetSize - 1 );
            d->biggestDataSetSize = d->calcMaxDataSetSize();
            if ( d->dataDirection == Qt::Vertical )
                endInsertRows();
            else
                endInsertColumns();
        }
    }
    else {
        // If we had no datasets before, we haven't had a valid
        // structure yet.  Thus, emit the modelReset() signal.
        d->dataSets.append( dataSet );
        dataSet->setKdDataSetNumber( 0 );
        d->biggestDataSetSize = d->calcMaxDataSetSize();

        reset();
    }
}

void KDChartModel::removeDataSet( DataSet *dataSet, bool silent )
{
    const int dataSetIndex = d->dataSets.indexOf( dataSet );
    if ( dataSetIndex < 0 )
        return;

    if ( silent ) {
        d->dataSets.removeAt( dataSetIndex );
        d->biggestDataSetSize = d->calcMaxDataSetSize();
    }
    else {
        // Simulate removing this dataSet without actually doing so
        // in order to calculate new max data set size
        QList<DataSet*> _dataSets( d->dataSets );
        _dataSets.removeAll( dataSet );
        // Cached size
        int oldMaxDataSetSize = d->maxDataSetSize();
        // Max size for new list
        int newMaxDataSetSize = d->calcMaxDataSetSize( _dataSets );

        if ( newMaxDataSetSize < oldMaxDataSetSize ) {
            if ( d->dataDirection == Qt::Horizontal )
                beginRemoveColumns( QModelIndex(), newMaxDataSetSize, newMaxDataSetSize - 1 );
            else
                beginRemoveRows( QModelIndex(), newMaxDataSetSize, newMaxDataSetSize - 1 );
            d->biggestDataSetSize = d->calcMaxDataSetSize();
            if ( d->dataDirection == Qt::Horizontal )
                endRemoveColumns();
            else
                endRemoveRows();
        }

        int columnAboutToBeRemoved = dataSetIndex * d->dataDimensions;
        if ( d->dataDirection == Qt::Horizontal )
            beginRemoveRows( QModelIndex(), columnAboutToBeRemoved,
                             columnAboutToBeRemoved + d->dataDimensions - 1 );
        else
            beginRemoveColumns( QModelIndex(), columnAboutToBeRemoved,
                                columnAboutToBeRemoved + d->dataDimensions - 1 );
        d->dataSets.removeAt( dataSetIndex );
        if ( d->dataDirection == Qt::Horizontal )
            endRemoveRows();
        else
            endRemoveColumns();
    }

    for ( int i = dataSetIndex; i < d->dataSets.size(); i++ ) {
        d->dataSets[ i ]->setKdDataSetNumber( i );
    }
}

QList<DataSet*> KDChartModel::dataSets() const
{
    return d->dataSets;
}

void KDChartModel::emitReset()
{
    // Update max data set sizes
    d->biggestDataSetSize = d->calcMaxDataSetSize();

    emit reset();
}

#include "KDChartModel.moc"
