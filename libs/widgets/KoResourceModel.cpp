/* This file is part of the KDE project
 * Copyright (C) 2008-2009 Jan Hambrecht <jaham@gmx.net>
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

#include <klocale.h>

#include <KoResourceServerAdapter.h>
#include <math.h>

KoResourceModel::KoResourceModel( KoAbstractResourceServerAdapter * resourceAdapter, QObject * parent )
    : QAbstractTableModel( parent ), m_resourceAdapter(resourceAdapter), m_columnCount(4)
{
    Q_ASSERT( m_resourceAdapter );
    m_resourceAdapter->connectToResourceServer();
    connect(m_resourceAdapter, SIGNAL(resourceAdded(KoResource*)), 
            this, SLOT(resourceAdded(KoResource*)));
    connect(m_resourceAdapter, SIGNAL(removingResource(KoResource*)), 
            this, SLOT(resourceRemoved(KoResource*)));
    connect(m_resourceAdapter, SIGNAL(resourceChanged(KoResource*)), 
            this, SLOT(resourceChanged(KoResource*)));
}

int KoResourceModel::rowCount( const QModelIndex &/*parent*/ ) const
{
    int resourceCount = m_resourceAdapter->resources().count();
    if (!resourceCount)
        return 0;

    return static_cast<int>(ceil(static_cast<qreal>(resourceCount) / m_columnCount));
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

            return QVariant( i18n( resource->name().toUtf8().data() ) );
        }
        case Qt::DecorationRole:
        {
            KoResource * resource = static_cast<KoResource*>(index.internalPointer());
            if( ! resource )
                return QVariant();

            return QVariant( resource->image() );
        }
        case KoResourceModel::LargeThumbnailRole:
        {
            KoResource * resource = static_cast<KoResource*>(index.internalPointer());
            if( ! resource )
                return QVariant();

            QSize imageSize = resource->image().size();
            QSize thumbSize( 100, 100 );
            if(imageSize.height() > thumbSize.height() || imageSize.width() > thumbSize.width()) {
                qreal scaleW = static_cast<qreal>( thumbSize.width() ) / static_cast<qreal>( imageSize.width() );
                qreal scaleH = static_cast<qreal>( thumbSize.height() ) / static_cast<qreal>( imageSize.height() );

                qreal scale = qMin( scaleW, scaleH );

                int thumbW = static_cast<int>( imageSize.width() * scale );
                int thumbH = static_cast<int>( imageSize.height() * scale );

                return QVariant(resource->image().scaled( thumbW, thumbH, Qt::IgnoreAspectRatio ));
            }
            else
                return QVariant(resource->image());
        }

        default:
            return QVariant();
    }
}

QModelIndex KoResourceModel::index ( int row, int column, const QModelIndex & ) const
{
    int index = row * m_columnCount + column;
    const QList<KoResource*> resources = m_resourceAdapter->resources();
    if( index >= resources.count() || index < 0)
        return QModelIndex();

    return createIndex( row, column, resources[index] );
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
    
    reset();
}

void KoResourceModel::resourceRemoved(KoResource *resource)
{
    Q_UNUSED(resource);
    reset();
}

void KoResourceModel::resourceChanged(KoResource* resource)
{
    int resourceIndex = m_resourceAdapter->resources().indexOf(resource);
    int row = resourceIndex / columnCount();
    int column = resourceIndex % columnCount();

    QModelIndex modelIndex = index(row, column);
    if (!modelIndex.isValid()) {
        return;
    }
    
    emit dataChanged(modelIndex, modelIndex);
}

QModelIndex KoResourceModel::indexFromResource(KoResource* resource)
{
    int resourceIndex = m_resourceAdapter->resources().indexOf(resource);
    int row = resourceIndex / columnCount();
    int column = resourceIndex % columnCount();
    return index(row, column);    
}

#include <KoResourceModel.moc>
