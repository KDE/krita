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
#include "kis_layer_model.h"
#include "kis_layer.h"
#include "kis_group_layer.h"

class KisLayerModel::Private
{
public:
    KisGroupLayerSP rootLayer;
};

KisLayerModel::KisLayerModel( KisGroupLayerSP rootLayer, QObject * parent )
    : KoDocumentSectionModel( parent )
    , m_d( new Private )
{
    m_d->rootLayer = rootLayer;

}

KisLayerModel::~KisLayerModel()
{
    delete m_d;
}

QModelIndex KisLayerModel::indexFromLayer(KisLayer *layer) const
{
    Q_ASSERT(layer);
    return createIndex(layer->index(), 0, layer);
}

int KisLayerModel::rowCount(const QModelIndex &parent) const
{
/*
    if (!parent.isValid())
        return childCount();
    Q_ASSERT(parent.model() == this);
    Q_ASSERT(parent.internalPointer());

    return static_cast<KisLayer*>(parent.internalPointer())->childCount();
*/
    return 0;
}

int KisLayerModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QModelIndex KisLayerModel::index(int row, int column, const QModelIndex &parent) const
{
/*
    if (!parent.isValid())
    {
        if( static_cast<uint>( row ) < childCount() )
            return createIndex(row, column, at(row).data());
        else
            return QModelIndex();
    }

    Q_ASSERT(parent.model() == this);
    Q_ASSERT(parent.internalPointer());

    return createIndex(row, column,
    static_cast<KisLayer*>(parent.internalPointer())->at(row).data());
*/
    return QModelIndex();
}

QModelIndex KisLayerModel::parent(const QModelIndex &i) const
{
/*
    if (!i.isValid())
        return QModelIndex();
    Q_ASSERT(i.model() == this);
    Q_ASSERT(i.internalPointer());

    if (static_cast<KisLayer*>(i.internalPointer())->parent().data() == this)
        return QModelIndex();
    else if (KisGroupLayer *p = static_cast<KisLayer*>(i.internalPointer())->parent().data())
        return createIndex(p->KisLayerModel::index(), 0, p); //gcc--
    else
        return QModelIndex();
*/
    return QModelIndex();
}

QVariant KisLayerModel::data(const QModelIndex &index, int role) const
{
/*
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
*/
    return QVariant();
}

Qt::ItemFlags KisLayerModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;
    Q_ASSERT(index.model() == this);
    Q_ASSERT(index.internalPointer());

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEditable;
/*
    if (qobject_cast<KisGroupLayer*>(static_cast<KisLayer*>(index.internalPointer()))) //gcc--
        flags |= Qt::ItemIsDropEnabled;
*/
    return flags;
}

bool KisLayerModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
/*
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
            return true;
        case PropertiesRole:
            layer->setProperties(value.value<PropertyList>());
            return true;
        case ActiveRole:
            if (value.toBool())
            {
                layer->setActive();
                return true;
            }
    }
*/
    return false;
}

#include "kis_layer_model.moc"
