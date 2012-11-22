/****************************************************************************
 ** Copyright (C) 2001-2006 Klarälvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Gantt library.
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
#include "kdganttforwardingproxymodel.h"

#include <cassert>

using namespace KDGantt;

typedef QAbstractProxyModel BASE;

/*! Constructor. Creates a new ForwardingProxyModel with
 * parent \a parent
 */
ForwardingProxyModel::ForwardingProxyModel( QObject* parent )
    : BASE( parent )
{
}

ForwardingProxyModel::~ForwardingProxyModel()
{
}

/*! Converts indexes in the source model to indexes in the proxy model */
QModelIndex ForwardingProxyModel::mapFromSource ( const QModelIndex & sourceIndex ) const
{
    if ( !sourceIndex.isValid() )
        return QModelIndex();
    assert( sourceIndex.model() == sourceModel() );

    // Create an index that preserves the internal pointer from the source;
    // this way KDDataConverterProxyModel preserves the structure of the source model
    return createIndex( sourceIndex.row(), sourceIndex.column(), sourceIndex.internalPointer() );
}

namespace {
    // Think this is ugly? Well, it's not from me, it comes from QProxyModel
    struct KDPrivateModelIndex {
        int r, c;
        void *p;
        const QAbstractItemModel *m;
    };
}

/*! Converts indexes in the proxy model to indexes in the source model */
QModelIndex ForwardingProxyModel::mapToSource ( const QModelIndex & proxyIndex ) const
{
    if ( !proxyIndex.isValid() )
        return QModelIndex();
    assert( proxyIndex.model() == this );
    // So here we need to create a source index which holds that internal pointer.
    // No way to pass it to sourceModel()->index... so we have to do the ugly way:
    QModelIndex sourceIndex;
    KDPrivateModelIndex* hack = reinterpret_cast<KDPrivateModelIndex*>(&sourceIndex);
    hack->r = proxyIndex.row();
    hack->c = proxyIndex.column();
    hack->p = proxyIndex.internalPointer();
    hack->m = sourceModel();
    assert( sourceIndex.isValid() );
    return sourceIndex;
}

/*! Sets the model to be used as the source model for this proxy.
 * The proxy does not take ownership of the model.
 * \see QAbstractProxyModel::setSourceModel
 */
void ForwardingProxyModel::setSourceModel( QAbstractItemModel* model )
{
    if ( sourceModel() ) sourceModel()->disconnect( this );
    BASE::setSourceModel( model );

    if(!model) return;

    connect( model, SIGNAL(modelAboutToBeReset()), this, SLOT(sourceModelAboutToBeReset()) );
    connect( model, SIGNAL(modelReset()), this, SLOT(sourceModelReset()) );
    connect( model, SIGNAL(layoutAboutToBeChanged()), this, SLOT(sourceLayoutAboutToBeChanged()) );
    connect( model, SIGNAL(layoutChanged()), this, SLOT(sourceLayoutChanged()) );

    connect( model, SIGNAL(dataChanged(const QModelIndex&,const QModelIndex&)),
             this, SLOT(sourceDataChanged(const QModelIndex&,const QModelIndex&)) );


    connect( model,  SIGNAL(columnsAboutToBeInserted(const QModelIndex&, int,int)),
             this, SLOT(sourceColumnsAboutToBeInserted(const QModelIndex&,int,int)) );
    connect( model,  SIGNAL(columnsInserted(const QModelIndex&, int,int)),
             this, SLOT(sourceColumnsInserted(const QModelIndex&,int,int)) );
    connect( model,  SIGNAL(columnsAboutToBeRemoved(const QModelIndex&, int,int)),
             this, SLOT(sourceColumnsAboutToBeRemoved(const QModelIndex&,int,int)) );
    connect( model,  SIGNAL(columnsRemoved(const QModelIndex&, int,int)),
             this, SLOT(sourceColumnsRemoved(const QModelIndex&,int,int)) );

    connect( model,  SIGNAL(rowsAboutToBeInserted(const QModelIndex&, int,int)),
             this, SLOT(sourceRowsAboutToBeInserted(const QModelIndex&,int,int)) );
    connect( model,  SIGNAL(rowsInserted(const QModelIndex&, int,int)),
             this, SLOT(sourceRowsInserted(const QModelIndex&,int,int)) );
    connect( model,  SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int,int)),
             this, SLOT(sourceRowsAboutToBeRemoved(const QModelIndex&,int,int)) );
    connect( model,  SIGNAL(rowsRemoved(const QModelIndex&, int,int)),
             this, SLOT(sourceRowsRemoved(const QModelIndex&,int,int)) );
}

/*! Called when the source model is about to be reset.
 * \sa QAbstractItemModel::modelAboutToBeReset()
 */
void ForwardingProxyModel::sourceModelAboutToBeReset()
{
    // The matching signal is emitted be reset()
}

/*! Called when the source model is reset
 * \sa QAbstractItemModel::modelReset()
 */
void ForwardingProxyModel::sourceModelReset()
{
  //qDebug() << "ForwardingProxyModel::sourceModelReset()";
    reset();
}

/*! Called just before the layout of the source model is changed.
 * \sa QAbstractItemModel::layoutAboutToBeChanged()
 */

void ForwardingProxyModel::sourceLayoutAboutToBeChanged()
{
  //qDebug() << "ForwardingProxyModel::sourceLayoutAboutToBeChanged()";
    emit layoutAboutToBeChanged();
}

/*! Called when the layout of the source model has changed.
 * \sa QAbstractItemModel::layoutChanged()
 */
void ForwardingProxyModel::sourceLayoutChanged()
{
  //qDebug() << "ForwardingProxyModel::sourceLayoutChanged()";
    reset();
}

/*! Called when the data in an existing item in the source model changes.
 * \sa QAbstractItemModel::dataChanged()
 */
void ForwardingProxyModel::sourceDataChanged( const QModelIndex& from, const QModelIndex& to )
{
  //qDebug() << "ForwardingProxyModel::sourceDataChanged("<<from<<to<<")";
    emit dataChanged( mapFromSource( from ), mapFromSource( to ) );
}

/*! Called just before columns are inserted into the source model.
 * \sa QAbstractItemModel::columnsAboutToBeInserted()
 */
void ForwardingProxyModel::sourceColumnsAboutToBeInserted( const QModelIndex& parentIdx,
                                                                    int start,
                                                                    int end )
{
    beginInsertColumns( mapFromSource( parentIdx ), start, end );
}

/*! Called after columns have been inserted into the source model.
 * \sa QAbstractItemModel::columnsInserted()
 */
void ForwardingProxyModel::sourceColumnsInserted( const QModelIndex& parentIdx, int start, int end )
{
    Q_UNUSED( parentIdx );
    Q_UNUSED( start );
    Q_UNUSED( end );
    endInsertColumns();
}

/*! Called just before columns are removed from the source model.
 * \sa QAbstractItemModel::columnsAboutToBeRemoved()
 */
void ForwardingProxyModel::sourceColumnsAboutToBeRemoved( const QModelIndex& parentIdx,
                                                                    int start,
                                                                    int end )
{
    beginRemoveColumns( mapFromSource( parentIdx ), start, end );
}

/*! Called after columns have been removed from the source model.
 * \sa QAbstractItemModel::columnsRemoved()
 */
void ForwardingProxyModel::sourceColumnsRemoved( const QModelIndex& parentIdx, int start, int end )
{
    Q_UNUSED( parentIdx );
    Q_UNUSED( start );
    Q_UNUSED( end );
    endRemoveColumns();
}

/*! Called just before rows are inserted into the source model.
 * \sa QAbstractItemModel::rowsAboutToBeInserted()
 */
void ForwardingProxyModel::sourceRowsAboutToBeInserted( const QModelIndex & parentIdx, int start, int end )
{
    beginInsertRows( mapFromSource( parentIdx ), start, end );
}

/*! Called after rows have been inserted into the source model.
 * \sa QAbstractItemModel::rowsInserted()
 */
void ForwardingProxyModel::sourceRowsInserted( const QModelIndex& parentIdx, int start, int end )
{
    Q_UNUSED( parentIdx );
    Q_UNUSED( start );
    Q_UNUSED( end );
    endInsertRows();
}

/*! Called just before rows are removed from the source model.
 * \sa QAbstractItemModel::rowsAboutToBeRemoved()
 */
void ForwardingProxyModel::sourceRowsAboutToBeRemoved( const QModelIndex & parentIdx, int start, int end )
{
    beginRemoveRows( mapFromSource( parentIdx ), start, end );
}

/*! Called after rows have been removed from the source model.
 * \sa QAbstractItemModel::rowsRemoved()
 */
void ForwardingProxyModel::sourceRowsRemoved( const QModelIndex& parentIdx, int start, int end )
{
    Q_UNUSED( parentIdx );
    Q_UNUSED( start );
    Q_UNUSED( end );
    endRemoveRows();
}

/*! \see QAbstractItemModel::rowCount */
int ForwardingProxyModel::rowCount( const QModelIndex& idx ) const
{
    return sourceModel()->rowCount( mapToSource( idx ) );
}

/*! \see QAbstractItemModel::columnCount */
int ForwardingProxyModel::columnCount( const QModelIndex& idx ) const
{
    return sourceModel()->columnCount( mapToSource( idx ) );
}

/*! \see QAbstractItemModel::index */
QModelIndex ForwardingProxyModel::index( int row, int column, const QModelIndex& parent ) const
{
    return mapFromSource( sourceModel()->index( row, column, mapToSource( parent ) ) );
}

/*! \see QAbstractItemModel::parent */
QModelIndex ForwardingProxyModel::parent( const QModelIndex& idx ) const
{
    return mapFromSource( sourceModel()->parent( mapToSource( idx ) ) );
}

/*! \see QAbstractItemModel::setData */
bool ForwardingProxyModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
  //qDebug() << "ForwardingProxyModel::setData( " << index<<value<< role<<")";
    return sourceModel()->setData( mapToSource( index ), value, role );
}

#include "moc_kdganttforwardingproxymodel.cpp"

