/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoResourceModel.h"

#include <KoResourceServerAdapter.h>

KoResourceModel::KoResourceModel( KoAbstractResourceServerAdapter * resourceAdapter, QObject * parent )
    : QAbstractTableModel( parent ), m_resourceAdapter(resourceAdapter), m_columnCount(4)
{
    Q_ASSERT( m_resourceAdapter );
    m_resourceAdapter->connectToResourceServer();
    connect(m_resourceAdapter, SIGNAL(resourceAdded(KoResource*)), 
            this, SLOT(resourceAdded(KoResource*)));
    connect(m_resourceAdapter, SIGNAL(removingResource(KoResource*)), 
            this, SLOT(resourceRemoved(KoResource*)));
}

int KoResourceModel::rowCount( const QModelIndex &/*parent*/ ) const
{
    return m_resourceAdapter->resources().count() / m_columnCount + 1;
}

int KoResourceModel::columnCount ( const QModelIndex & ) const
{
    return m_columnCount;
}

QVariant KoResourceModel::data( const QModelIndex &index, int role ) const
{
    if( ! index.isValid() )
         return QVariant();

    switch( role )
    {
        case Qt::DisplayRole:
        {
            KoResource * resource = static_cast<KoResource*>(index.internalPointer());
            if( ! resource )
                return QVariant();

            return QVariant( resource->name() );
        }
        case Qt::DecorationRole:
        {
            KoResource * resource = static_cast<KoResource*>(index.internalPointer());
            if( ! resource )
                return QVariant();

            return QVariant( resource->img() );
        }
        case KoResourceModel::LargeThumbnailRole:
        {
            KoResource * resource = static_cast<KoResource*>(index.internalPointer());
            if( ! resource )
                return QVariant();

            QSize imgSize = resource->img().size();
            QSize thumbSize( 100, 100 );
            if(imgSize.height() > thumbSize.height() || imgSize.width() > thumbSize.width()) {
                qreal scaleW = static_cast<qreal>( thumbSize.width() ) / static_cast<qreal>( imgSize.width() );
                qreal scaleH = static_cast<qreal>( thumbSize.height() ) / static_cast<qreal>( imgSize.height() );

                qreal scale = qMin( scaleW, scaleH );

                int thumbW = static_cast<int>( imgSize.width() * scale );
                int thumbH = static_cast<int>( imgSize.height() * scale );

                return QVariant(resource->img().scaled( thumbW, thumbH, Qt::IgnoreAspectRatio ));
            }
            else
                return QVariant(resource->img());
        }

        default:
            return QVariant();
    }
}

QModelIndex KoResourceModel::index ( int row, int column, const QModelIndex & ) const
{
    int index = row * m_columnCount + column;
    if( index >= m_resourceAdapter->resources().count() || index < 0)
        return QModelIndex();

    KoResource * resource = m_resourceAdapter->resources()[index];
    return createIndex( row, column, resource );
}

void KoResourceModel::setColumnCount( int columnCount )
{
    if (columnCount != m_columnCount) {
        m_columnCount = columnCount;
        reset();
    }
}

KoAbstractResourceServerAdapter * KoResourceModel::resourceServerAdapter()
{
    return m_resourceAdapter;
}

void KoResourceModel::resourceAdded(KoResource *resource)
{
    int newIndex = m_resourceAdapter->resources().indexOf(resource);
    if (newIndex < 0)
        return;
    
    int row = newIndex / m_columnCount;
    int col = newIndex % m_columnCount;
    if (col == 0 || m_columnCount == 1 ) {
        beginInsertRows(QModelIndex(), row, row);
        endInsertRows();
    } else {
        QModelIndex modelIndex = index(row, col, QModelIndex());
        emit dataChanged(modelIndex, modelIndex);
    }
}

void KoResourceModel::resourceRemoved(KoResource *resource)
{
    int oldIndex = m_resourceAdapter->resources().indexOf(resource);
    if (oldIndex < 0)
        return;
    
    reset();
}

#include "KoResourceModel.moc"
