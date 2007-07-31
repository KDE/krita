/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "kdebug.h"

#include "kis_layer_model.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_paint_layer.h"
#include "kis_image.h"
#include "kis_selection.h"
#include "kis_transparency_mask.h"
#include "kis_effect_mask.h"

class KisLayerModel::Private
{
public:
    KisImageWSP image;
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
    kDebug(41007) <<"KisLayerModel::setImage" << image;
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
    kDebug(41007) <<"KisLayerModel::layerFromIndex" << index;

    if( !index.isValid() )
        return KisLayerSP(0);

    Q_ASSERT(index.model() == this);
    Q_ASSERT(index.internalPointer());

    return KisLayerSP(static_cast<KisLayer*>(index.internalPointer()));
}

vKisLayerSP KisLayerModel::layersFromIndexes(const QModelIndexList &indexes)
{
    kDebug(41007) <<"KisLayerModel::layersFromIndexes" << indexes.count();
    vKisLayerSP out;
    for (int i = 0, n = indexes.count(); i < n; ++i)
        if (KisLayerSP layer = layerFromIndex(indexes.at(i)))
            out << layer;
    return out;
}

QModelIndex KisLayerModel::indexFromLayer(const KisLayer *layer) const
{
    kDebug(41007) <<"KisLayer::indexFromLayer" << layer <<", layer index:" << layer->index();
    Q_ASSERT(layer);
    if ( layer->parentLayer() )
        return createIndex(layer->index(), 0, ( void* )layer);
    else {
        return QModelIndex();
    }
}


KisMaskSP KisLayerModel::maskFromIndex(const QModelIndex &index)
{
    kDebug(41007) <<"KisLayerModel::maskFromIndex" << index;

    if( !index.isValid() )
        return KisMaskSP(0);

    Q_ASSERT(index.model() == this);
    Q_ASSERT(index.internalPointer());

    // XXX: Or does static_cast also return 0 when cast fails. Check
    // when home (bsar)
    return KisMaskSP( static_cast<KisMask*>(index.internalPointer()) ) ;


}

vKisMaskSP KisLayerModel::masksFromIndexes(const QModelIndexList &indexes)
{
    kDebug(41007) <<"KisLayerModel::masksFromIndexes" << indexes.count();
    vKisMaskSP out;
    for (int i = 0, n = indexes.count(); i < n; ++i)
        if (KisMaskSP mask = maskFromIndex(indexes.at(i)))
            out << mask;
    return out;
}

QModelIndex KisLayerModel::indexFromMask(const KisMask *mask) const
{
    // XXX: keep track of all the different types of masks here
    Q_ASSERT(mask);

    KisLayerSP parentLayer = mask->parentLayer();
    Q_ASSERT( parentLayer );

//     int maskIndex = -1;

//     if ( mask == parentLayer->selection() ) {
//         maskIndex = 0;
//     }
//     else if ( parentLayer && parentLayer->hasEffectMasks() ) {

//         KisMask * m = const_cast<KisMask*>( mask );
//         KisEffectMask * effectMask = dynamic_cast<KisEffectMask*>( m );

//         if ( effectMask ) {
//             maskIndex = parentLayer->effectMasks().indexOf( effectMask );
//         }

//         if ( parentLayer->selection() )
//             maskIndex++;

//         kDebug(41007) <<"KisMask::indexFromMask" << mask <<", mask index:" << maskIndex;

//         return createIndex(maskIndex, 0, ( void* )mask);
//     } else {
        return QModelIndex();
//     }
}

int KisLayerModel::rowCount(const QModelIndex &parent) const
{
    kDebug(41007) <<"KisLayerModel::rowCount" << parent;

    if (!parent.isValid()) {
        // Root layer
        kDebug(41007) <<"Root layer:" << m_d->image->rootLayer() <<", childcount:" << m_d->image->rootLayer()->childCount();;
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
    kDebug(41007) <<"KisLayerModel::index(row =" << row <<", column=" << column <<", parent=" << parent <<" parent is valid:" << parent.isValid();

    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

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
    kDebug(41007) <<"KisLayerModel::parent" << index;
    if (!index.isValid())
        return QModelIndex();

    Q_ASSERT(index.model() == this);
    Q_ASSERT(index.internalPointer());

    KisLayer * l = static_cast<KisLayer*>( index.internalPointer() );
    kDebug(41007) <<" layer:" << l <<", name:" << l->name() <<", parent:" << l->parent();

    KisGroupLayer *p = l->parentLayer().data();

    // If the parent is the root layer, we want to return an invalid
    // parent, because the qt model shouldn't know about our root layer.
    if ( p && p->parentLayer().data() ) {
        kDebug(41007) <<"parent layer:" << p <<", name:" << p->name() <<", parent:" << p->parentLayer();
        return indexFromLayer( p );
    }
    else
        return QModelIndex();

}

QVariant KisLayerModel::data(const QModelIndex &index, int role) const
{
//     kDebug(41007) <<"KisLayerModel::data(index=" << index <<", role=" << role;
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
    kDebug(41007) <<"KisLayerModel::flags" << index;
    if (!index.isValid())
        return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;

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
    kDebug(41007) <<"KisLayerModel::setData( index=" << index <<", value=" << value <<", role=" << role;
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
                emit layerActivated( layer );
                emit dataChanged( index, index );
                return true;
            }
    }

    return false;
}


void KisLayerModel::beginInsertLayers( KisGroupLayer * parent, int index )
{
    kDebug(41007) <<"KisLayerModel::beginInsertLayers parent=" << parent <<", index=" << index;
    beginInsertRows( indexFromLayer( parent ), index, index );
}

void KisLayerModel::endInsertLayers( KisGroupLayer *, int)
{
    kDebug(41007) <<"KisLayerModel::endInsertLayers";
    endInsertRows();
}

void KisLayerModel::beginRemoveLayers( KisGroupLayer * parent, int index )
{
    kDebug(41007) <<"KisLayerModel::beginRemoveLayers parent=" << parent <<", index=" << index;
    beginRemoveRows( indexFromLayer( parent ), index, index );
}

void KisLayerModel::endRemoveLayers( KisGroupLayer *, int )
{
    kDebug(41007) <<"KisLayerModel::endRemoveLayers";
    endRemoveRows();
}


void KisLayerModel::beginInsertMasks( KisLayer * parent,  int index )
{
    beginInsertRows( indexFromLayer( parent ), index, index );
}

void KisLayerModel::endInsertMasks( KisLayer *, int )
{
    endInsertRows();
}


void KisLayerModel::beginRemoveMasks( KisLayer * parent, int index )
{
    beginRemoveRows( indexFromLayer( parent ), index, index );
}


void KisLayerModel::endRemoveMasks( KisLayer * , int )
{
    endRemoveRows();
}



#if 0
QMimeData * KisLayerModel::mimeData ( const QModelIndexList & indexes ) const
{
    kDebug(41007) <<"KisLayerModel::mimeData";
    QMimeData* data = new QMimeData;
    // TODO: manage the drag

}
#endif

bool KisLayerModel::dropMimeData ( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent )
{ // TODO: manage the drop
    kDebug(41007) <<"KisLayerModel::dropMimeData";
    kDebug(41007) <<"KisLayerModel::dropMimeData" << data->formats();
//     const QString format = "application/x-qabstractitemmodeldatalist";
/*    if(not data->hasFormat( format ))
    {
        return false;
    }*/
/*    QByteArray encoded = data->data(format);
    QDataStream stream(&encoded, QIODevice::ReadOnly);*/
    if(action == Qt::CopyAction)
    {
        kDebug(41007) <<"KisLayerModel::dropMimeData copy action";
/*        while (!stream.atEnd()) {
            int r, c;
            QMap<int, QVariant> v;
            stream >> r >> c >> v;
            kDebug(41007) <<"KisLayerModel::dropMimeData copy action" << r <<"" << c;
        }*/
        return true;
    } else if(action == Qt::MoveAction) {
        kDebug(41007) <<"KisLayerModel::dropMimeData move action";
        return true;
    }
    return false;
}

Qt::DropActions KisLayerModel::supportedDragActions () const
{
  return Qt::CopyAction | Qt::MoveAction;
}

#include "kis_layer_model.moc"
