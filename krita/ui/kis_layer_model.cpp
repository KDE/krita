/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  this program is free software; you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation; either version 2 of the license, or
 *  (at your option) any later version.
 *
 *  this program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with this program; if not, write to the free software
 *  foundation, inc., 675 mass ave, cambridge, ma 02139, usa.
 */
#include "kdebug.h"

#include "kis_layer_model.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_paint_layer.h"
#include "kis_image.h"

class KisLayerModel::Private
{
public:
    KisImageSP image;
};

KisLayerModel::KisLayerModel( QObject * parent )
    : KoDocumentSectionModel( parent )
    , m_d( new Private )
{

}

KisLayerModel::~KisLayerModel()
{
    delete m_d;
}

void KisLayerModel::setImage( KisImageSP image )
{
    kDebug(41007) << "KisLayerModel::setImage " << image << endl;
    if ( m_d->image ) {
        m_d->image->disconnect( this );
    }
    m_d->image = image;

    connect( m_d->image, SIGNAL( sigAboutToAddALayer( KisGroupLayer*, int) ),
             SLOT(beginInsertLayers( KisGroupLayer*, int ) ) );
    connect( m_d->image, SIGNAL( sigLayerHasBeenAdded( KisGroupLayer*, int) ),
             SLOT(endInsertLayers(KisGroupLayer*, int ) ) );
    connect( m_d->image, SIGNAL( sigAboutToRemoveALayer( KisGroupLayer*, int) ),
             SLOT(beginRemoveLayers(KisGroupLayer*, int ) ) );
    connect( m_d->image, SIGNAL( sigALayerHasBeenRemoved( KisGroupLayer*, int) ),
             SLOT(endRemoveLayers( KisGroupLayer*, int ) ) );
}

KisLayerSP KisLayerModel::layerFromIndex(const QModelIndex &index)
{
    kDebug(41007) << "KisLayerModel::layerFromIndex " << index << endl;

    if( !index.isValid() )
        return KisLayerSP(0);

    Q_ASSERT(index.model() == this);
    Q_ASSERT(index.internalPointer());

    return KisLayerSP(static_cast<KisLayer*>(index.internalPointer()));
}

vKisLayerSP KisLayerModel::layersFromIndexes(const QModelIndexList &indexes)
{
    kDebug(41007) << "KisLayerModel::layersFromIndexes " << indexes.count() << endl;
    vKisLayerSP out;
    for (int i = 0, n = indexes.count(); i < n; ++i)
        if (KisLayerSP layer = layerFromIndex(indexes.at(i)))
            out << layer;
    return out;
}

QModelIndex KisLayerModel::indexFromLayer(const KisLayer *layer) const
{
    kDebug(41007) << "KisLayer::indexFromLayer " << layer << ", layer index: " << layer->index() << endl;
    Q_ASSERT(layer);
    if ( layer->parent() )
        return createIndex(layer->index(), 0, ( void* )layer);
    else {
        return createIndex(0, 0, ( void* )layer);
    }
}

int KisLayerModel::rowCount(const QModelIndex &parent) const
{
    kDebug(41007) << "KisLayerModel::rowCount " << parent << endl;

    if (!parent.isValid()) {
        // Root layer
        kDebug(41007) << "Root layer: " << m_d->image->rootLayer() << ", childcount: " << m_d->image->rootLayer()->childCount() << endl;;
        return m_d->image->rootLayer()->childCount();
    }
    else  {
        return static_cast<KisLayer*>(parent.internalPointer())->childCount();
    }
}

int KisLayerModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QModelIndex KisLayerModel::index(int row, int column, const QModelIndex &parent) const
{
    kDebug(41007) << "KisLayerModel::index(row = " << row << ", column=" << column << ", parent=" << parent << " parent is valid: " << parent.isValid() << endl;

    if (!parent.isValid())
    {
        return indexFromLayer( m_d->image->rootLayer()->at( row ) );
    }

    Q_ASSERT(parent.model() == this);
    Q_ASSERT(parent.internalPointer());

    return createIndex(row, column, static_cast<KisLayer*>(parent.internalPointer())->at(row).data());

}

QModelIndex KisLayerModel::parent(const QModelIndex &index) const
{
    kDebug(41007) << "KisLayerModel::parent " << index << endl;
    if (!index.isValid())
        return QModelIndex();

    Q_ASSERT(index.model() == this);
    Q_ASSERT(index.internalPointer());

    KisLayer * l = static_cast<KisLayer*>( index.internalPointer() );
    kDebug(41007) << " layer: " << l << ", name: " << l->name() << ", parent: " << l->parent() << endl;

    KisGroupLayer *p = l->parent().data();

    // If the parent is the root layer, we want to return an invalid
    // parent, because the qt model shouldn't know about our root layer.
    if ( p && p->parent().data() ) {
        kDebug(41007) << "parent layer: " << p << ", name: " << p->name() << ", parent: " << p->parent() << endl;
        return indexFromLayer( p );
    }
    else
        return QModelIndex();

}

QVariant KisLayerModel::data(const QModelIndex &index, int role) const
{
    kDebug(41007) << "KisLayerModel::data(index=" << index << ", role=" << role << endl;
    if (!index.isValid())
        return QVariant();

    Q_ASSERT(index.model() == this);
    Q_ASSERT(index.internalPointer());

    KisLayer *layer = static_cast<KisLayer*>(index.internalPointer());

    switch (role)
    {
        case Qt::DisplayRole: return layer->name();
        case Qt::DecorationRole: return layer->icon();
        case Qt::EditRole: return layer->name();
        case Qt::SizeHintRole: return layer->image()->size();
        case ActiveRole: return layer->isActive();
        case PropertiesRole: return QVariant::fromValue(layer->properties());
        case AspectRatioRole: return double(layer->image()->width()) / layer->image()->height();
        default:
            if (role >= int(BeginThumbnailRole))
                return layer->createThumbnail(role - int(BeginThumbnailRole), role - int(BeginThumbnailRole));
            else
                return QVariant();
    }

    return QVariant();
}

Qt::ItemFlags KisLayerModel::flags(const QModelIndex &index) const
{
    kDebug(41007) << "KisLayerModel::flags " << index << endl;
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    Q_ASSERT(index.model() == this);
    Q_ASSERT(index.internalPointer());

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEditable;

    // XXX: layers will also be drop enabled for masks and selections.
    if (qobject_cast<KisGroupLayer*>(static_cast<KisLayer*>(index.internalPointer()))) {
        flags |= Qt::ItemIsDropEnabled;
    }

    return flags;
}

bool KisLayerModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    kDebug(41007) << "KisLayerModel::setData( index=" << index << ", value=" << value << ", role=" << role << endl;
    if (!index.isValid())
        return false;

    Q_ASSERT(index.model() == this);
    Q_ASSERT(index.internalPointer());

    KisLayer *layer = static_cast<KisLayer*>(index.internalPointer());

    switch (role)
    {
        case Qt::DisplayRole:
        case Qt::EditRole:
            layer->setName(value.toString());
            emit dataChanged( index, index );
            return true;
        case PropertiesRole:
            layer->setProperties(value.value<PropertyList>());
            emit dataChanged( index, index );
            return true;
        case ActiveRole:
            if (value.toBool())
            {
                layer->setActive();
                emit dataChanged( index, index );
                return true;
            }
    }

    return false;
}


void KisLayerModel::beginInsertLayers( KisGroupLayer * parent, int index )
{
    kDebug(41007) << "KisLayerModel::beginInsertLayers parent=" << parent << ", index=" << index << endl;
    beginInsertRows( indexFromLayer( parent ), index, index );
}

void KisLayerModel::endInsertLayers( KisGroupLayer *, int)
{
    kDebug(41007) << "KisLayerModel::endInsertLayers\n";
    endInsertRows();
}

void KisLayerModel::beginRemoveLayers( KisGroupLayer * parent, int index )
{
    kDebug(41007) << "KisLayerModel::beginRemoveLayers parent=" << parent << ", index=" << index << endl;
    beginRemoveRows( indexFromLayer( parent ), index, index );
}

void KisLayerModel::endRemoveLayers( KisGroupLayer *, int )
{
    kDebug(41007) << "KisLayerModel::endRemoveLayers\n";
    endRemoveRows();
}

#include "kis_layer_model.moc"
