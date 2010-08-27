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

#include "KDChartAbstractProxyModel.h"

#include <QDebug>

#include <KDABLibFakes>


namespace KDChart {

  /** This is basically KDAbstractProxyModel, but only the
      bits that we really need from it */
AbstractProxyModel::AbstractProxyModel(QObject* parent) 
  : QAbstractProxyModel(parent) {}

// Think this is ugly? Well, it's not from me, it comes from QProxyModel
struct KDPrivateModelIndex
{
  int r, c;
  void *p;
  const QAbstractItemModel *m;
};

QModelIndex AbstractProxyModel::mapFromSource( const QModelIndex & sourceIndex ) const
{
  if ( !sourceIndex.isValid() )
    return QModelIndex();
  //qDebug() << "sourceIndex.model()="<<sourceIndex.model();
  //qDebug() << "model()="<<sourceModel();
  Q_ASSERT( sourceIndex.model() == sourceModel() );

  // Create an index that preserves the internal pointer from the source;
  // this way AbstractProxyModel preserves the structure of the source model
  return createIndex( sourceIndex.row(), sourceIndex.column(), sourceIndex.internalPointer() );
}

QModelIndex AbstractProxyModel::mapToSource( const QModelIndex &proxyIndex ) const
{
  if ( !proxyIndex.isValid() )
    return QModelIndex();
  if( proxyIndex.model() != this )
    qDebug() << proxyIndex.model() << this;
  Q_ASSERT( proxyIndex.model() == this );
  // So here we need to create a source index which holds that internal pointer.
  // No way to pass it to sourceModel()->index... so we have to do the ugly way:
  QModelIndex sourceIndex;
  KDPrivateModelIndex* hack = reinterpret_cast<KDPrivateModelIndex*>(&sourceIndex);
  hack->r = proxyIndex.row();
  hack->c = proxyIndex.column();
  hack->p = proxyIndex.internalPointer();
  hack->m = sourceModel();
  Q_ASSERT( sourceIndex.isValid() );
  return sourceIndex;
}

QModelIndex AbstractProxyModel::index( int row, int col, const QModelIndex& index ) const
{
    Q_ASSERT(sourceModel());
    return mapFromSource(sourceModel()->index( row, col, mapToSource(index) ));
}

QModelIndex AbstractProxyModel::parent( const QModelIndex& index ) const
{
    Q_ASSERT(sourceModel());
    return mapFromSource(sourceModel()->parent( mapToSource(index) ));
}

}
