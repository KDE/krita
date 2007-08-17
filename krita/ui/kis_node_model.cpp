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

#include <QMimeData>

#include "kis_node_model.h"
#include "kis_node.h"
#include "kis_image.h"
#include "kis_selection.h"


class KisNodeModel::Private
{
public:
    KisImageWSP image;
};

KisNodeModel::KisNodeModel( QObject * parent )
    : KoDocumentSectionModel( parent )
    , m_d( new Private )
{
}

KisNodeModel::~KisNodeModel()
{
    delete m_d;
}

void KisNodeModel::setImage( KisImageSP image )
{
    kDebug(41007) <<"KisNodeModel::setImage" << image;
    if ( m_d->image ) {
        m_d->image->disconnect( this );
    }
    m_d->image = image;

    connect( m_d->image, SIGNAL( sigAboutToAddANode( KisNode*, int) ),
             SLOT(beginInsertNodes( KisNode*, int ) ) );
    connect( m_d->image, SIGNAL( sigNodeHasBeenAdded( KisNode*, int) ),
             SLOT(endInsertNodes(KisNode*, int ) ) );
    connect( m_d->image, SIGNAL( sigAboutToRemoveANode( KisNode*, int) ),
             SLOT(beginRemoveNodes(KisNode*, int ) ) );
    connect( m_d->image, SIGNAL( sigNodeHasBeenRemoved( KisNode*, int) ),
             SLOT(endRemoveNodes( KisNode*, int ) ) );
}

KisNodeSP KisNodeModel::nodeFromIndex(const QModelIndex &index)
{
    kDebug(41007) <<"KisNodeModel::nodeFromIndex" << index;

    if( !index.isValid() )
        return KisNodeSP(0);

    Q_ASSERT(index.model() == this);
    Q_ASSERT(index.internalPointer());

    return KisNodeSP(static_cast<KisNode*>(index.internalPointer()));
}

vKisNodeSP KisNodeModel::nodesFromIndexes(const QModelIndexList &indexes)
{
    kDebug(41007) <<"KisNodeModel::nodesFromIndexes" << indexes.count();
    vKisNodeSP out;
    for (int i = 0, n = indexes.count(); i < n; ++i)
        if (KisNodeSP node = nodeFromIndex(indexes.at(i)))
            out << node;
    return out;
}

QModelIndex KisNodeModel::indexFromNode(const KisNodeSP node) const
{
    Q_ASSERT(node);
    if ( node->parent() )
        return createIndex(node->parent()->index( node ), 0, ( void* )node.data());
    else {
        return QModelIndex();
    }
}


int KisNodeModel::rowCount(const QModelIndex &parent) const
{
    kDebug(41007) <<"KisNodeModel::rowCount" << parent;

    if (!parent.isValid()) {
        // Root node
        kDebug(41007) <<"Root node:" << m_d->image->root() <<", childcount:" << m_d->image->root()->childCount();;
        return m_d->image->root()->childCount();
    }
    else  {
        return static_cast<KisNode*>(parent.internalPointer())->childCount();
    }
}

int KisNodeModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QModelIndex KisNodeModel::index(int row, int column, const QModelIndex &parent) const
{
    kDebug(41007) <<"KisNodeModel::index(row =" << row <<", column=" << column <<", parent=" << parent <<" parent is valid:" << parent.isValid();

    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

    if (!parent.isValid())
    {
        return indexFromNode( m_d->image->root()->at( row ) );
    }

    Q_ASSERT(parent.model() == this);
    Q_ASSERT(parent.internalPointer());

    return createIndex(row, column, static_cast<KisNode*>(parent.internalPointer())->at(row).data());

}

QModelIndex KisNodeModel::parent(const QModelIndex &index) const
{
    kDebug(41007) <<"KisNodeModel::parent" << index;
    if (!index.isValid())
        return QModelIndex();

    Q_ASSERT(index.model() == this);
    Q_ASSERT(index.internalPointer());

    KisNode * l = static_cast<KisNode*>( index.internalPointer() );
    kDebug(41007) <<" node:" << l <<", name:" << l->name() <<", parent:" << l->parent();

    KisNode *p = l->parent().data();

    // If the parent is the root node, we want to return an invalid
    // parent, because the qt model shouldn't know about our root node.
    if ( p && p->parent().data() ) {
        kDebug(41007) <<"parent node:" << p <<", name:" << p->name() <<", parent:" << p->parent();
        return indexFromNode( p );
    }
    else
        return QModelIndex();

}

QVariant KisNodeModel::data(const QModelIndex &index, int role) const
{
//     kDebug(41007) <<"KisNodeModel::data(index=" << index <<", role=" << role;
    if (!index.isValid())
        return QVariant();

    Q_ASSERT(index.model() == this);
    Q_ASSERT(index.internalPointer());

    KisNode *node = static_cast<KisNode*>(index.internalPointer());

    switch (role)
    {
        case Qt::DisplayRole: return node->name();
        case Qt::DecorationRole: return node->icon();
        case Qt::EditRole: return node->name();
        case Qt::SizeHintRole: return m_d->image->size();
        case PropertiesRole: return QVariant::fromValue(node->sectionModelProperties());
        case AspectRatioRole: return double(m_d->image->width()) / m_d->image->height();
        default:
            if (role >= int(BeginThumbnailRole))
                return node->createThumbnail(role - int(BeginThumbnailRole), role - int(BeginThumbnailRole));
            else
                return QVariant();
    }

    return QVariant();
}

Qt::ItemFlags KisNodeModel::flags(const QModelIndex &index) const
{
    kDebug(41007) <<"KisNodeModel::flags" << index;
    if (!index.isValid())
        return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;

    Q_ASSERT(index.model() == this);
    Q_ASSERT(index.internalPointer());

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEditable;

    // XXX: nodes will also be drop enabled for masks and selections.
    if (qobject_cast<KisNode*>(static_cast<KisNode*>(index.internalPointer()))) {
        flags |= Qt::ItemIsDropEnabled;
    }

    return flags;
}

bool KisNodeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    kDebug(41007) <<"KisNodeModel::setData( index=" << index <<", value=" << value <<", role=" << role;
    if (!index.isValid())
        return false;

    Q_ASSERT(index.model() == this);
    Q_ASSERT(index.internalPointer());

    KisNode *node = static_cast<KisNode*>(index.internalPointer());

    switch (role)
    {
        case Qt::DisplayRole:
        case Qt::EditRole:
            node->setName(value.toString());
            emit dataChanged( index, index );
            return true;
        case PropertiesRole:
            node->setSectionModelProperties(value.value<PropertyList>());
            emit dataChanged( index, index );
            return true;
        case ActiveRole:
            if (value.toBool())
            {
                emit nodeActivated( node );
                emit dataChanged( index, index );
                return true;
            }
    }

    return false;
}


void KisNodeModel::beginInsertNodes( KisNode * parent, int index )
{
    kDebug(41007) <<"KisNodeModel::beginInsertNodes parent=" << parent <<", index=" << index;
    beginInsertRows( indexFromNode( parent ), index, index );
}

void KisNodeModel::endInsertNodes( KisNode *, int)
{
    kDebug(41007) <<"KisNodeModel::endInsertNodes";
    endInsertRows();
}

void KisNodeModel::beginRemoveNodes( KisNode * parent, int index )
{
    kDebug(41007) <<"KisNodeModel::beginRemoveNodes parent=" << parent <<", index=" << index;
    beginRemoveRows( indexFromNode( parent ), index, index );
}

void KisNodeModel::endRemoveNodes( KisNode *, int )
{
    kDebug(41007) <<"KisNodeModel::endRemoveNodes";
    endRemoveRows();
}

#if 0
QMimeData * KisNodeModel::mimeData ( const QModelIndexList & indexes ) const
{
    kDebug(41007) <<"KisNodeModel::mimeData";
    QMimeData* data = new QMimeData;
    // TODO: manage the drag

}
#endif

bool KisNodeModel::dropMimeData ( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent )
{

    Q_UNUSED( row );
    Q_UNUSED( column );
    Q_UNUSED( parent );

#ifdef __GNUC__
    #warning "Implement KisNodeModel::dropMimeData"
#endif

// TODO: manage the drop
    kDebug(41007) <<"KisNodeModel::dropMimeData";
    kDebug(41007) <<"KisNodeModel::dropMimeData" << data->formats();
//     const QString format = "application/x-qabstractitemmodeldatalist";
/*    if(not data->hasFormat( format ))
    {
        return false;
    }*/
/*    QByteArray encoded = data->data(format);
    QDataStream stream(&encoded, QIODevice::ReadOnly);*/
    if(action == Qt::CopyAction)
    {
        kDebug(41007) <<"KisNodeModel::dropMimeData copy action";
/*        while (!stream.atEnd()) {
            int r, c;
            QMap<int, QVariant> v;
            stream >> r >> c >> v;
            kDebug(41007) <<"KisNodeModel::dropMimeData copy action" << r <<"" << c;
        }*/
        return true;
    } else if(action == Qt::MoveAction) {
        kDebug(41007) <<"KisNodeModel::dropMimeData move action";
        return true;
    }
    return false;
}

Qt::DropActions KisNodeModel::supportedDragActions () const
{
  return Qt::CopyAction | Qt::MoveAction;
}

#include "kis_node_model.moc"
