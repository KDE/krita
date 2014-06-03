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
#include "kdganttproxymodel.h"
#include "kdganttproxymodel_p.h"


using namespace KDGantt;

typedef ForwardingProxyModel BASE;

ProxyModel::Private::Private( ProxyModel* _q )
#if 0
    : calendarMode( false )
#endif
{
    Q_UNUSED( _q ); // for now

    //columnMap[Qt::DisplayRole]    = 0; Separate ItemNameRole ??
    columnMap[ItemTypeRole]       = 1;
    columnMap[StartTimeRole]      = 2;
    columnMap[EndTimeRole]        = 3;
    columnMap[TaskCompletionRole] = 4;
    columnMap[LegendRole]         = 5;

    roleMap[Qt::DisplayRole]    = Qt::DisplayRole;
    roleMap[ItemTypeRole]       = Qt::DisplayRole;
    roleMap[StartTimeRole]      = Qt::DisplayRole;
    roleMap[EndTimeRole]        = Qt::DisplayRole;
    roleMap[TaskCompletionRole] = Qt::DisplayRole;
    roleMap[LegendRole]         = Qt::DisplayRole;
}

ProxyModel::ProxyModel( QObject* parent )
    : BASE( parent ), _d( new Private( this ) )
{
    init();
}

ProxyModel::~ProxyModel()
{
    delete _d; _d = 0;
}

#define d d_func()

void ProxyModel::init()
{
}

QModelIndex ProxyModel::mapFromSource( const QModelIndex& sourceIdx ) const
{
    if( sourceIdx.isValid() ) {
#if 0
        if ( calendarMode() ) {
            const QAbstractItemModel* model = sourceIdx.model();
            if ( model->hasChildren( sourceIdx ) ) {
                return BASE::mapFromSource( model->index( sourceIdx.row(),0,sourceIdx.parent()));
            } else {
                // Map children to columns
                return BASE::mapFromSource( model->index( sourceIdx.row(),0,sourceIdx.parent()))
                    .child( 0, sourceIdx.column() );
            }
        }
#endif
        return BASE::mapFromSource( sourceIdx );
    }
    else return QModelIndex();
}

QModelIndex ProxyModel::mapToSource( const QModelIndex& proxyIdx ) const
{
    if( proxyIdx.isValid() ) {
#if 0
        if ( calendarMode() && proxyIdx.column() > 0 ) {
            return BASE::mapToSource( proxyIdx.model()->index( proxyIdx.column(), 0, proxyIdx ) );
        }
#endif
        return BASE::mapToSource( proxyIdx );
    }
    else return QModelIndex();
}

void ProxyModel::setColumn( int ganttrole, int col )
{
    d->columnMap[ganttrole] = col;
}

int ProxyModel::column( int ganttrole ) const
{
    return d->columnMap[ganttrole];
}

void ProxyModel::setRole( int ganttrole, int role )
{
    d->roleMap[ganttrole] = role;
}

int ProxyModel::role( int ganttrole ) const
{
    return d->roleMap[ganttrole];
}

#if 0
void ProxyModel::setCalendarMode( bool enable )
{
    if ( d->calendarMode != enable ) {
        d->calendarMode = enable;
        reset();
    }
}

bool ProxyModel::calendarMode() const
{
    return d->calendarMode;
}
#endif

int ProxyModel::rowCount( const QModelIndex& proxyIndex ) const
{
    // TODO
    return BASE::rowCount( proxyIndex );
}

int ProxyModel::columnCount( const QModelIndex& proxyIndex ) const
{
    return qMin( sourceModel()->columnCount( mapToSource( proxyIndex ) ), 1 );
}

QVariant ProxyModel::data( const QModelIndex& proxyIdx, int role ) const
{
    int srole = role;
    int scol  = proxyIdx.column();
    QHash<int, int>::const_iterator it = d->roleMap.find( role );
    if ( it != d->roleMap.end() ) srole = *it;
    it = d->columnMap.find( role );
    if ( it != d->columnMap.end() ) scol = *it;

    //qDebug() << "mapping role"<<static_cast<ItemDataRole>(role)<<"to"<<static_cast<ItemDataRole>(srole);
    //qDebug() << "mapping column"<<proxyIdx.column()<<"to"<<scol;

    const QAbstractItemModel* model = sourceModel();
    return model->data( model->index( proxyIdx.row(), scol, mapToSource( proxyIdx.parent() ) ), srole );
}

bool ProxyModel::setData( const QModelIndex& proxyIdx, const QVariant& value, int role )
{
    int srole = role;
    int scol  = proxyIdx.column();
    QHash<int, int>::const_iterator it = d->roleMap.constFind( role );
    if ( it != d->roleMap.constEnd() ) srole = *it;
    it = d->columnMap.constFind( role );
    if ( it != d->columnMap.constEnd() ) scol = *it;

    QAbstractItemModel* model = sourceModel();
    return model->setData( model->index( proxyIdx.row(), scol, mapToSource( proxyIdx.parent() ) ), value, srole );
}

#include "moc_kdganttproxymodel.cpp"
