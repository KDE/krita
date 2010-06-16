/* -*- Mode: C++ -*-
   KDChart - a multi-platform charting engine
   */

/****************************************************************************
 ** Copyright (C) 2005-2007 Klarälvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Chart library.
 **
 ** This file may be used under the terms of the GNU General Public
 ** License versions 2.0 or 3.0 as published by the Free Software
 ** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
 ** included in the packaging of this file.  Alternatively you may (at
 ** your option) use any later version of the GNU General Public
 ** License if such license has been publicly approved by
 ** Klarälvdalens Datakonsult AB (or its successors, if any).
 ** 
 ** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
 ** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE. Klarälvdalens Datakonsult AB reserves all rights
 ** not expressly granted herein.
 ** 
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 **********************************************************************/

#include "KDChartCartesianDiagramDataCompressor_p.h"

#include <QtDebug>
#include <QAbstractItemModel>

#include "KDChartAbstractCartesianDiagram.h"

#include <KDABLibFakes>

using namespace KDChart;
using namespace std;

CartesianDiagramDataCompressor::CartesianDiagramDataCompressor( QObject* parent )
    : QObject( parent )
    , m_mode( Precise )
    , m_xResolution( 0 )
    , m_yResolution( 0 )
    , m_sampleStep( 0 )
    , m_datasetDimension( 1 )
{
    calculateSampleStepWidth();
}

QModelIndexList CartesianDiagramDataCompressor::indexesAt( const CachePosition& position ) const
{
    if ( isValidCachePosition( position ) ) {
        CachePosition posPrev( position );
        if( m_datasetDimension == 2 ){
            if(posPrev.second)
                --posPrev.second;
        }else{
            if(posPrev.first)
                --posPrev.first;
        }
        const QModelIndexList indPrev = mapToModel( posPrev );
        const QModelIndexList indCur  = mapToModel( position );

        QModelIndexList indexes;
        if( m_datasetDimension == 2 )
        {
            const int iStart = (indPrev.empty() || indPrev==indCur) ? indCur.first().column()
                             : indPrev.first().column() + 1;
            const int iEnd   = indCur.last().column();
            for( int i=iStart; i<=iEnd; ++i){
                indexes << m_model->index( position.first, i, m_rootIndex );
            }
        }
        else
        {
            const int iStart = (indPrev.empty() || indPrev==indCur)  ? indCur.first().row()
                             : indPrev.first().row() + 1;
            const int iEnd   = (indCur.isEmpty()) ? iStart : indCur.first().row();
            //qDebug()<<iStart<<iEnd << iEnd-iStart;
            for( int i=iStart; i<=iEnd; ++i){
                indexes << m_model->index( i, position.second, m_rootIndex );
            }
        }
        return indexes;
    } else {
        return QModelIndexList();
    }
}


CartesianDiagramDataCompressor::DataValueAttributesList CartesianDiagramDataCompressor::aggregatedAttrs(
        AbstractDiagram * diagram,
        const QModelIndex & index,
        const CachePosition& position ) const
{
    // return cached attrs, if any
    DataValueAttributesCache::const_iterator i = m_dataValueAttributesCache.constFind(position);
    if( i != m_dataValueAttributesCache.constEnd() )
        return i.value();
    // retrieve attrs from all cells between the prev. cell and the current one
    CartesianDiagramDataCompressor::DataValueAttributesList allAttrs;
    const QModelIndexList indexes( indexesAt( position ) );
    KDAB_FOREACH( QModelIndex idx, indexes ) {
        DataValueAttributes attrs( diagram->dataValueAttributes( idx ) );
        if( attrs.isVisible() ){
            // make sure no duplicate attrs are stored
            bool isDuplicate = false;
            CartesianDiagramDataCompressor::DataValueAttributesList::const_iterator i = allAttrs.constBegin();
            while (i != allAttrs.constEnd()) {
                if( i.value() == attrs ){
                    isDuplicate = true;
                    continue;
                }
                ++i;
            }
            if( !isDuplicate ){
                //qDebug()<<idx.row();
                allAttrs[idx] = attrs;
            }
        }
    }
    // if none of the attrs had the visible flag set
    // we just take the one set for the index to not return an empty list
    if( allAttrs.empty() ){
        allAttrs[index] = diagram->dataValueAttributes( index );
    }
    // cache the attrs
    m_dataValueAttributesCache[position] = allAttrs;
    return allAttrs;
}


void CartesianDiagramDataCompressor::slotRowsAboutToBeInserted( const QModelIndex& parent, int start, int end )
{
    if ( parent != m_rootIndex )
        return;
    Q_ASSERT( start <= end );

    CachePosition startPos = mapToCache( start, 0 );
    CachePosition endPos = mapToCache( end, 0 );

    static const CachePosition NullPosition( -1, -1 );
    if( startPos == NullPosition )
    {
        rebuildCache();
        startPos = mapToCache( start, 0 );
        endPos = mapToCache( end, 0 );
        // The start position still isn't valid,
        // means that no resolution was set yet or we're about to add the first rows
        if( startPos == NullPosition ) {
            return;
        }
    }

    start = startPos.first;
    end = endPos.first;

    for( int i = 0; i < m_data.size(); ++i )
    {
        Q_ASSERT( start >= 0 && start <= m_data[ i ].size() );
        m_data[ i ].insert( start, end - start + 1, DataPoint() );
    }
}

void CartesianDiagramDataCompressor::slotRowsInserted( const QModelIndex& parent, int start, int end )
{
    if ( parent != m_rootIndex )
        return;
    Q_ASSERT( start <= end );

    CachePosition startPos = mapToCache( start, 0 );
    CachePosition endPos = mapToCache( end, 0 );

    static const CachePosition NullPosition( -1, -1 );
    if( startPos == NullPosition )
    {
        // Rebuild the cache at this point if we have added the first rows
        rebuildCache();
        startPos = mapToCache( start, 0 );
        endPos = mapToCache( end, 0 );
        // The start position still isn't valid,
        // means that no resolution was set yet
        if( startPos == NullPosition ) {
            return;
        }
    }

    start = startPos.first;
    end = endPos.first;

    for( int i = 0; i < m_data.size(); ++i )
    {
        for( int j = start; j < m_data[i].size(); ++j ) {
            retrieveModelData( CachePosition( j, i ) );
        }
    }
}

void CartesianDiagramDataCompressor::slotColumnsAboutToBeInserted( const QModelIndex& parent, int start, int end )
{
    if ( parent != m_rootIndex )
        return;
    Q_ASSERT( start <= end );

    CachePosition startPos = mapToCache( 0, start );
    CachePosition endPos = mapToCache( 0, end );

    static const CachePosition NullPosition( -1, -1 );
    if( startPos == NullPosition )
    {
        rebuildCache();
        startPos = mapToCache( 0, start );
        endPos = mapToCache( 0, end );
        // The start position still isn't valid,
        // means that no resolution was set yet or we're about to add the first columns
        if( startPos == NullPosition ) {
            return;
        }
    }

    start = startPos.second;
    end = endPos.second;

    const int rowCount = qMin( m_model ? m_model->rowCount( m_rootIndex ) : 0, m_xResolution );
    Q_ASSERT( start >= 0 && start <= m_data.size() );
    m_data.insert( start, end - start + 1, QVector< DataPoint >( rowCount ) );
}

void CartesianDiagramDataCompressor::slotColumnsInserted( const QModelIndex& parent, int start, int end )
{
    if ( parent != m_rootIndex )
        return;
    Q_ASSERT( start <= end );

    CachePosition startPos = mapToCache( 0, start );
    CachePosition endPos = mapToCache( 0, end );

    static const CachePosition NullPosition( -1, -1 );
    if( startPos == NullPosition )
    {
        // Rebuild the cache at this point if we have added the first columns
        rebuildCache();
        startPos = mapToCache( 0, start );
        endPos = mapToCache( 0, end );
        // The start position still isn't valid,
        // means that no resolution was set yet
        if( startPos == NullPosition ) {
            return;
        }
    }

    start = startPos.second;
    end = endPos.second;

    for( int i = start; i < m_data.size(); ++i )
    {
        for(int j = 0; j < m_data[i].size(); ++j ) {
            retrieveModelData( CachePosition( j, i ) );
        }
    }
}

void CartesianDiagramDataCompressor::slotRowsAboutToBeRemoved( const QModelIndex& parent, int start, int end )
{
    if ( parent != m_rootIndex )
        return;
    Q_ASSERT( start <= end );

    CachePosition startPos = mapToCache( start, 0 );
    CachePosition endPos = mapToCache( end, 0 );

    static const CachePosition NullPosition( -1, -1 );
    if( startPos == NullPosition )
    {
        rebuildCache();
        startPos = mapToCache( start, 0 );
        endPos = mapToCache( end, 0 );
        // The start position still isn't valid,
        // probably means that no resolution was set yet
        if( startPos == NullPosition ) {
            return;
        }
    }

    start = startPos.first;
    end = endPos.first;

    for( int i = 0; i < m_data.size(); ++i )
    {
        m_data[ i ].remove( start, end - start + 1 );
    }
}

void CartesianDiagramDataCompressor::slotRowsRemoved( const QModelIndex& parent, int start, int end )
{
    if ( parent != m_rootIndex )
        return;
    Q_ASSERT( start <= end );

    CachePosition startPos = mapToCache( start, 0 );
    CachePosition endPos = mapToCache( end, 0 );

    start = startPos.first;
    end = endPos.first;

    static const CachePosition NullPosition( -1, -1 );
    if( startPos == NullPosition )
    {
        // Since we should already have rebuilt the cache, it won't help to rebuild it again.
        // Do not Q_ASSERT() though, since the resolution might simply not be set or we might now have 0 rows
        return;
    }

    for( int i = 0; i < m_data.size(); ++i ) {
        for(int j = start; j < m_data[i].size(); ++j ) {
            retrieveModelData( CachePosition( j, i ) );
        }
    }
}

void CartesianDiagramDataCompressor::slotColumnsAboutToBeRemoved( const QModelIndex& parent, int start, int end )
{
    if ( parent != m_rootIndex )
        return;
    Q_ASSERT( start <= end );

    CachePosition startPos = mapToCache( 0, start );
    CachePosition endPos = mapToCache( 0, end );

    static const CachePosition NullPosition( -1, -1 );
    if( startPos == NullPosition )
    {
        rebuildCache();
        startPos = mapToCache( 0, start );
        endPos = mapToCache( 0, end );
        // The start position still isn't valid,
        // probably means that no resolution was set yet
        if( startPos == NullPosition ) {
            return;
        }
    }

    start = startPos.second;
    end = endPos.second;

    m_data.remove( start, end - start + 1 );
}

void CartesianDiagramDataCompressor::slotColumnsRemoved( const QModelIndex& parent, int start, int end )
{
    if ( parent != m_rootIndex )
        return;
    Q_ASSERT( start <= end );

    const CachePosition startPos = mapToCache( 0, start );
    const CachePosition endPos = mapToCache( 0, end );

    start = startPos.second;
    end = endPos.second;

    static const CachePosition NullPosition( -1, -1 );
    if( startPos == NullPosition )
    {
        // Since we should already have rebuilt the cache, it won't help to rebuild it again.
        // Do not Q_ASSERT() though, since the resolution might simply not be set or we might now have 0 columns
        return;
    }

    for( int i = start; i < m_data.size(); ++i ) {
        for( int j = 0; j < m_data[i].size(); ++j ) {
            retrieveModelData( CachePosition( j, i ) );
        }
    }
}

void CartesianDiagramDataCompressor::slotModelHeaderDataChanged( Qt::Orientation orientation, int first, int last )
{
    if( orientation != Qt::Vertical )
        return;

    const QModelIndex firstRow = m_model->index( 0, first, m_rootIndex );
    const QModelIndex lastRow = m_model->index( m_model->rowCount( m_rootIndex ) - 1, last, m_rootIndex );

    slotModelDataChanged( firstRow, lastRow );
}

void CartesianDiagramDataCompressor::slotModelDataChanged(
    const QModelIndex& topLeftIndex,
    const QModelIndex& bottomRightIndex )
{
    if ( topLeftIndex.parent() != m_rootIndex )
        return;
    Q_ASSERT( topLeftIndex.parent() == bottomRightIndex.parent() );
    Q_ASSERT( topLeftIndex.row() <= bottomRightIndex.row() );
    Q_ASSERT( topLeftIndex.column() <= bottomRightIndex.column() );
    CachePosition topleft = mapToCache( topLeftIndex );
    CachePosition bottomright = mapToCache( bottomRightIndex );
    for ( int row = topleft.first; row <= bottomright.first; ++row )
        for ( int column = topleft.second; column <= bottomright.second; ++column )
            invalidate( CachePosition( row, column ) );
}

void CartesianDiagramDataCompressor::slotModelLayoutChanged()
{
    rebuildCache();
    calculateSampleStepWidth();
}

void CartesianDiagramDataCompressor::slotDiagramLayoutChanged( AbstractDiagram* diagramBase )
{
    AbstractCartesianDiagram* diagram = qobject_cast< AbstractCartesianDiagram* >( diagramBase );
    Q_ASSERT( diagram );
    if ( diagram->datasetDimension() != m_datasetDimension ) {
        setDatasetDimension( diagram->datasetDimension() );
    }
}

int CartesianDiagramDataCompressor::modelDataColumns() const
{
    Q_ASSERT( m_datasetDimension != 0 );
    // only operational if there is a model and a resolution
    if ( m_model ) {
        const int columns = m_model->columnCount( m_rootIndex ) / m_datasetDimension;

        if( columns != m_data.size() )
        {
            rebuildCache();
        }

        Q_ASSERT( columns == m_data.size() );
        return columns;
    } else {
        return 0;
    }
}

int CartesianDiagramDataCompressor::modelDataRows() const
{
    // only operational if there is a model, columns, and a resolution
    if ( m_model && m_model->columnCount( m_rootIndex ) > 0 && m_xResolution > 0 ) {
        return m_data.isEmpty() ? 0 : m_data.first().size();
    } else {
        return 0;
    }
}

void CartesianDiagramDataCompressor::setModel( QAbstractItemModel* model )
{
    if ( m_model != 0 && m_model != model ) {
        disconnect( m_model, SIGNAL( headerDataChanged( Qt::Orientation, int, int ) ),
                 this, SLOT( slotModelHeaderDataChanged( Qt::Orientation, int, int ) ) );
        disconnect( m_model, SIGNAL( dataChanged( QModelIndex, QModelIndex ) ),
                 this, SLOT( slotModelDataChanged( QModelIndex, QModelIndex ) ) );
        disconnect( m_model, SIGNAL( layoutChanged() ),
                 this, SLOT( slotModelLayoutChanged() ) );
        disconnect( m_model, SIGNAL( rowsAboutToBeInserted( QModelIndex, int, int ) ),
                 this, SLOT( slotRowsAboutToBeInserted( QModelIndex, int, int ) ) );
        disconnect( m_model, SIGNAL( rowsInserted( QModelIndex, int, int ) ),
                 this, SLOT( slotRowsInserted( QModelIndex, int, int ) ) );
        disconnect( m_model, SIGNAL( rowsAboutToBeRemoved( QModelIndex, int, int ) ),
                 this, SLOT( slotRowsAboutToBeRemoved( QModelIndex, int, int ) ) );
        disconnect( m_model, SIGNAL( rowsRemoved( QModelIndex, int, int ) ),
                 this, SLOT( slotRowsRemoved( QModelIndex, int, int ) ) );
        disconnect( m_model, SIGNAL( columnsAboutToBeInserted( QModelIndex, int, int ) ),
                 this, SLOT( slotColumnsAboutToBeInserted( QModelIndex, int, int ) ) );
        disconnect( m_model, SIGNAL( columnsInserted( QModelIndex, int, int ) ),
                 this, SLOT( slotColumnsInserted( QModelIndex, int, int ) ) );
        disconnect( m_model, SIGNAL( columnsRemoved( QModelIndex, int, int ) ),
                 this, SLOT( slotColumnsRemoved( QModelIndex, int, int ) ) );
        disconnect( m_model, SIGNAL( columnsAboutToBeRemoved( QModelIndex, int, int ) ),
                 this, SLOT( slotColumnsAboutToBeRemoved( QModelIndex, int, int ) ) );
        disconnect( m_model, SIGNAL( modelReset() ),
                    this, SLOT( rebuildCache() ) );
        m_model = 0;
    }

    m_modelCache.setModel( model );

    if ( model != 0 ) {
        m_model = model;
        connect( m_model, SIGNAL( headerDataChanged( Qt::Orientation, int, int ) ),
                 SLOT( slotModelHeaderDataChanged( Qt::Orientation, int, int ) ) );
        connect( m_model, SIGNAL( dataChanged( QModelIndex, QModelIndex ) ),
                 SLOT( slotModelDataChanged( QModelIndex, QModelIndex ) ) );
        connect( m_model, SIGNAL( layoutChanged() ),
                 SLOT( slotModelLayoutChanged() ) );
        connect( m_model, SIGNAL( rowsAboutToBeInserted( QModelIndex, int, int ) ),
                 SLOT( slotRowsAboutToBeInserted( QModelIndex, int, int ) ) );
        connect( m_model, SIGNAL( rowsInserted( QModelIndex, int, int ) ),
                 SLOT( slotRowsInserted( QModelIndex, int, int ) ) );
        connect( m_model, SIGNAL( rowsAboutToBeRemoved( QModelIndex, int, int ) ),
                 SLOT( slotRowsAboutToBeRemoved( QModelIndex, int, int ) ) );
        connect( m_model, SIGNAL( rowsRemoved( QModelIndex, int, int ) ),
                 SLOT( slotRowsRemoved( QModelIndex, int, int ) ) );
        connect( m_model, SIGNAL( columnsAboutToBeInserted( QModelIndex, int, int ) ),
                 SLOT( slotColumnsAboutToBeInserted( QModelIndex, int, int ) ) );
        connect( m_model, SIGNAL( columnsInserted( QModelIndex, int, int ) ),
                 SLOT( slotColumnsInserted( QModelIndex, int, int ) ) );
        connect( m_model, SIGNAL( columnsRemoved( QModelIndex, int, int ) ),
                 SLOT( slotColumnsRemoved( QModelIndex, int, int ) ) );
        connect( m_model, SIGNAL( columnsAboutToBeRemoved( QModelIndex, int, int ) ),
                 SLOT( slotColumnsAboutToBeRemoved( QModelIndex, int, int ) ) );
        connect( m_model, SIGNAL( modelReset() ),
                    this, SLOT( rebuildCache() ) );
    }
    rebuildCache();
    calculateSampleStepWidth();
}

void CartesianDiagramDataCompressor::setRootIndex( const QModelIndex& root )
{
    if ( m_rootIndex != root ) {
        Q_ASSERT( root.model() == m_model || !root.isValid() );
        m_rootIndex = root;
        m_modelCache.setRootIndex( root );
        rebuildCache();
        calculateSampleStepWidth();
    }
}
void CartesianDiagramDataCompressor::setResolution( int x, int y )
{
    const int oldX = m_xResolution;
    const int oldY = m_yResolution;

    if( m_datasetDimension != 1 )
    {
        // just ignore the resolution in that case
        m_xResolution = m_model == 0 ? 0 : m_model->rowCount( m_rootIndex );
        m_yResolution = qMax( 0, y );
    }
    else if ( x != m_xResolution || y != m_yResolution ) {
        m_xResolution = qMax( 0, x );
        m_yResolution = qMax( 0, y );
        rebuildCache();
        calculateSampleStepWidth();
    }

    if( oldX != m_xResolution || oldY != m_yResolution )
    {
        rebuildCache();
        calculateSampleStepWidth();
    }
}

void CartesianDiagramDataCompressor::clearCache()
{
    for ( int column = 0; column < m_data.size(); ++column )
        m_data[column].fill( DataPoint() );
}

void CartesianDiagramDataCompressor::rebuildCache() const
{
    Q_ASSERT( m_datasetDimension != 0 );

    m_data.clear();
    const int columnCount = m_model ? m_model->columnCount( m_rootIndex ) / m_datasetDimension : 0;
    const int rowCount = qMin( m_model ? m_model->rowCount( m_rootIndex ) : 0, m_xResolution );
    m_data.resize( columnCount );
    for ( int i = 0; i < columnCount; ++i ) {
        m_data[i].resize( rowCount );
    }
    // also empty the attrs cache
    m_dataValueAttributesCache.clear();
}

const CartesianDiagramDataCompressor::DataPoint& CartesianDiagramDataCompressor::data( const CachePosition& position ) const
{
    static DataPoint NullDataPoint;
    if ( ! isValidCachePosition( position ) ) return NullDataPoint;
    if ( ! isCached( position ) ) retrieveModelData( position );
    return m_data[ position.second ][ position.first ];
}

void CartesianDiagramDataCompressor::retrieveModelData( const CachePosition& position ) const
{
    Q_ASSERT( isValidCachePosition( position ) );
    DataPoint result;

    switch(m_mode ) {
    case Precise:
    {
        bool forceHidden = false;
        result.hidden = true;
        const QModelIndexList indexes = mapToModel( position );
        if( m_datasetDimension != 1 )
        {
            Q_ASSERT( indexes.count() == 2 );
            const QModelIndex xIndex = indexes.first();
            const QModelIndex yIndex = indexes.last();
            const double xData = m_modelCache.data( xIndex );
            const double yData = m_modelCache.data( yIndex );
            result.index = xIndex;
            result.key   = xData;
            result.value = yData;
        }
        else
        {
            if ( ! indexes.isEmpty() ) {
                result.value = std::numeric_limits< double >::quiet_NaN();
                result.key = 0.0;
                Q_FOREACH( const QModelIndex& index, indexes ) {
                    const double value = m_modelCache.data( index );
                    if( !ISNAN( value ) )
                    {
                        result.value = ISNAN( result.value ) ? value : result.value + value;
                    }
                    result.key += index.row();
                }
                result.index = indexes.at( 0 );
                result.key /= indexes.size();
                result.value /= indexes.size();
            }
        }
        if( !forceHidden )
        {
        Q_FOREACH( const QModelIndex& index, indexes )
        {
            // the point is visible if any of the points at this pixel position is visible
            if ( qVariantValue<bool>( m_model->data( index, DataHiddenRole ) ) == false ) {
                result.hidden = false;
            }
        }
        }
    }
    break;
    case SamplingSeven:
    default:
    {
    }
    break;
    };

    m_data[position.second][position.first] = result;
    Q_ASSERT( isCached( position ) );
}

CartesianDiagramDataCompressor::CachePosition CartesianDiagramDataCompressor::mapToCache(
        const QModelIndex& index ) const
{
    Q_ASSERT( m_datasetDimension != 0 );

    static const CachePosition NullPosition( -1, -1 );
    if ( ! index.isValid() ) return NullPosition;
    return mapToCache( index.row(), index.column() );
}

CartesianDiagramDataCompressor::CachePosition CartesianDiagramDataCompressor::mapToCache(
        int row, int column ) const
{
    Q_ASSERT( m_datasetDimension != 0 );

    if ( m_data.size() == 0 || m_data[0].size() == 0 ) return mapToCache( QModelIndex() );
    // assumption: indexes per column == 1
    if ( indexesPerPixel() == 0 ) return mapToCache( QModelIndex() );
    return CachePosition( static_cast< int >( ( row ) / indexesPerPixel() ), column / m_datasetDimension );
}

QModelIndexList CartesianDiagramDataCompressor::mapToModel( const CachePosition& position ) const
{
    if ( isValidCachePosition( position ) ) {
        QModelIndexList indexes;
        if( m_datasetDimension == 2 )
        {
            indexes << m_model->index( position.first, position.second * 2, m_rootIndex );
            indexes << m_model->index( position.first, position.second * 2 + 1, m_rootIndex );
        }
        else
        {
        // assumption: indexes per column == 1
            const qreal ipp = indexesPerPixel();
            for ( int i = 0; i < ipp; ++i ) {
                const QModelIndex index = m_model->index( qRound( position.first * ipp ) + i, position.second, m_rootIndex );
                if( index.isValid() )
                    indexes << index;
            }
        }
        return indexes;
    } else {
        return QModelIndexList();
    }
}

qreal CartesianDiagramDataCompressor::indexesPerPixel() const
{
    if ( m_data.size() == 0 ) return 0;
    if ( m_data[0].size() == 0 ) return 0;
    if ( ! m_model ) return 0;
    return static_cast< qreal >( m_model->rowCount( m_rootIndex ) ) / static_cast< qreal >( m_data[0].size() );
}

bool CartesianDiagramDataCompressor::isValidCachePosition( const CachePosition& position ) const
{
    if ( ! m_model ) return false;
    if ( m_data.size() == 0 || m_data[0].size() == 0 ) return false;
    if ( position.second < 0 || position.second >= m_data.size() ) return false;
    if ( position.first < 0 || position.first >= m_data[0].size() ) return false;
    return true;
}

void CartesianDiagramDataCompressor::invalidate( const CachePosition& position )
{
    if ( isValidCachePosition( position ) ) {
        m_data[position.second][position.first] = DataPoint();
        // Also invalidate the data value attributes at "position".
        // Otherwise the user overwrites the attributes without us noticing
        // it because we keep reading what's in the cache.
        m_dataValueAttributesCache.remove( position );
    }
}

bool CartesianDiagramDataCompressor::isCached( const CachePosition& position ) const
{
    Q_ASSERT( isValidCachePosition( position ) );
    const DataPoint& p = m_data[position.second][position.first];
    return p.index.isValid();
}

void CartesianDiagramDataCompressor::calculateSampleStepWidth()
{
    if ( m_mode == Precise ) {
        m_sampleStep = 1;
        return;
    }

    static unsigned int SomePrimes[] = {
        2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47,
        53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101,
        151, 211, 313, 401, 503, 607, 701, 811, 911, 1009,
        10037, 12911, 16001, 20011, 50021,
        100003, 137867, 199999, 500009, 707753, 1000003, 0
    }; // ... after that, having a model at all becomes impractical

    // we want at least 17 samples per data point, using a prime step width
    const double WantedSamples = 17;
    if ( WantedSamples > indexesPerPixel() ) {
        m_sampleStep = 1;
    } else {
        int i;
        for ( i = 0; SomePrimes[i] != 0; ++i ) {
            if ( WantedSamples * SomePrimes[i+1] > indexesPerPixel() ) {
                break;
            }
        }
        m_sampleStep = SomePrimes[i];
        if ( SomePrimes[i] == 0 ) {
            m_sampleStep = SomePrimes[i-1];
        } else {
            m_sampleStep = SomePrimes[i];
        }
    }
}

void CartesianDiagramDataCompressor::setDatasetDimension( int dimension )
{
    if ( dimension != m_datasetDimension ) {
        m_datasetDimension = dimension;
        rebuildCache();
        calculateSampleStepWidth();
    }
}
