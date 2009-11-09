/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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
#include "kis_node_model.h"

#include <iostream>

#include <QMimeData>

#include <klocale.h>

#include <kis_debug.h>
#include <kis_node.h>
#include <kis_node_progress_proxy.h>
#include <kis_image.h>
#include <kis_selection.h>
#include <kis_undo_adapter.h>
#include <commands/kis_node_property_list_command.h>
#include <kis_paint_layer.h>

#include "kis_config.h"
#include "kis_config_notifier.h"

class KisNodeModel::Private
{
public:
    KisImageWSP image;
    bool showRootLayer;
};

KisNodeModel::KisNodeModel(QObject * parent)
        : KoDocumentSectionModel(parent)
        , m_d(new Private)
{
    updateSettings();
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), this, SLOT(updateSettings()));
}

KisNodeModel::~KisNodeModel()
{
    delete m_d;
}

void KisNodeModel::setImage(KisImageWSP image)
{
    dbgUI << "KisNodeModel::setImage " << image << ": number of layers " << image->nlayers();
    if (m_d->image) {
        m_d->image->disconnect(this);
    }
    m_d->image = image;
    connect(m_d->image, SIGNAL(sigPostLayersChanged(KisGroupLayerSP)), SLOT(layersChanged()));

    connect(m_d->image, SIGNAL(sigAboutToAddANode(KisNode*, int)),
            SLOT(beginInsertNodes(KisNode*, int)));
    connect(m_d->image, SIGNAL(sigNodeHasBeenAdded(KisNode*, int)),
            SLOT(endInsertNodes(KisNode*, int)));
    connect(m_d->image, SIGNAL(sigAboutToRemoveANode(KisNode*, int)),
            SLOT(beginRemoveNodes(KisNode*, int)));
    connect(m_d->image, SIGNAL(sigNodeHasBeenRemoved(KisNode*, int)),
            SLOT(endRemoveNodes(KisNode*, int)));
}

KisNodeSP KisNodeModel::nodeFromIndex(const QModelIndex &index)
{
    //dbgUI <<"KisNodeModel::nodeFromIndex " << index;
    if (!index.isValid())
        return KisNodeSP(0);

    Q_ASSERT(index.model() == this);
    Q_ASSERT(index.internalPointer());

    return static_cast<KisNode*>(index.internalPointer());
}

vKisNodeSP KisNodeModel::nodesFromIndexes(const QModelIndexList &indexes)
{
    //dbgUI <<"KisNodeModel::nodesFromIndexes " << indexes.count();
    vKisNodeSP out;
    for (int i = 0, n = indexes.count(); i < n; ++i)
        if (KisNodeSP node = nodeFromIndex(indexes.at(i)))
            out << node;
    return out;
}

QModelIndex KisNodeModel::indexFromNode(const KisNodeSP node) const
{
    //dbgUI << "KisNodeModel::indexFromNode " << node;
    Q_ASSERT(node);
    if (node->parent()) {
        int rowCount = node->parent()->childCount() - 1;
        int index = node->parent()->index(node);
        int row = rowCount - index;
        //dbgUI << "Node index in image: " << index << ", parent has " << rowCount + 1 << " rows, inverted index becomes " << row;
        return createIndex(row, 0, (void*)node.data());
    } else {
        if (m_d->showRootLayer) {
            // if no parent then it is the root layer
            return createIndex(0, 0, (void*)node.data());
        } else {
            return QModelIndex();
        }

    }
}


int KisNodeModel::rowCount(const QModelIndex &parent) const
{
    //dbgUI <<"KisNodeModel::rowCount" << parent;

    if (!parent.isValid()) {
        if (m_d->image) {
            if (m_d->showRootLayer) {
                return 1; // <- this means that it is the "parent" of the root
            } else {
                if (m_d->image && m_d->image->root()) {
                    //dbgUI <<"Root node:" << m_d->image->root() <<", childcount:" << m_d->image->root()->childCount();;  the root
                    return m_d->image->root()->childCount();
                }
            }
        }
    } else  {
        return static_cast<KisNode*>(parent.internalPointer())->childCount();
    }

    return 0;
}

int KisNodeModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QModelIndex KisNodeModel::index(int row, int column, const QModelIndex &parent) const
{
    //dbgUI <<"KisNodeModel::index(row =" << row <<", column=" << column <<", parent=" << parent <<" parent is valid:" << parent.isValid();

    if (!hasIndex(row, column, parent)) {
        //dbgUI << "Does not have index";
        return QModelIndex();
    }
    KisNodeSP parentNode;

    if (!parent.isValid()) {
        if (m_d->showRootLayer) {
            Q_ASSERT(row == 0);
            if (m_d->image) {
                //dbgUI << "root, row: " << row << ", node: " << m_d->image->root();
                return indexFromNode(m_d->image->root());
            } else {
                return QModelIndex();
            }
        } else {
            int rowCount = m_d->image->root()->childCount() - 1;
            //dbgUI << "row count: " << rowCount << ", row: " << row << ", node: " << m_d->image->root()->at( rowCount - row );
            return indexFromNode(m_d->image->root()->at(rowCount - row));
        }

    }

    Q_ASSERT(parent.model() == this);
    Q_ASSERT(parent.internalPointer());
    parentNode = static_cast<KisNode*>(parent.internalPointer());

    int rowCount = parentNode->childCount() - 1;
    // Now invert!
    //dbgUI << "row count: " << rowCount << ", row: " << row << ", node: " << parentNode->at(rowCount - row);
    return createIndex(row, column, parentNode->at(rowCount - row).data());

}

QModelIndex KisNodeModel::parent(const QModelIndex &index) const
{
    //dbgUI <<"KisNodeModel::parent " << index;
    if (!index.isValid())
        return QModelIndex();

    Q_ASSERT(index.model() == this);
    Q_ASSERT(index.internalPointer());

    KisNode * l = static_cast<KisNode*>(index.internalPointer());
    //dbgUI <<" node:" << l <<", name:" << l->name() <<", parent:" << l->parent();

    KisNode *p = l->parent().data();

    // If the parent is the root node, we want to return an invalid
    // parent, because the qt model shouldn't know about our root node.
    if (m_d->showRootLayer) {
        if (p) {
            return indexFromNode(p);
        }
    } else {
        if (p && p->parent().data()) {
            //dbgUI <<"parent node:" << p <<", name:" << p->name() <<", parent:" << p->parent();
            return indexFromNode(p);
        }
    }
    return QModelIndex();

}

QVariant KisNodeModel::data(const QModelIndex &index, int role) const
{
    //dbgUI <<"KisNodeModel::data(index=" << index <<", role=" << role;
    if (!index.isValid())
        return QVariant();

    Q_ASSERT(index.model() == this);
    Q_ASSERT(index.internalPointer());

    KisNode *node = static_cast<KisNode*>(index.internalPointer());

    switch (role) {
    case Qt::DisplayRole: return node->name();
    case Qt::DecorationRole: return node->icon();
    case Qt::EditRole: return node->name();
    case Qt::SizeHintRole: return m_d->image->size();
    case PropertiesRole: return QVariant::fromValue(node->sectionModelProperties());
    case AspectRatioRole: return double(m_d->image->width()) / m_d->image->height();
    case ProgressRole: {
        if (node->nodeProgressProxy()) {
            return node->nodeProgressProxy()->percentage();
        } else {
            return -1;
        }
    }
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
    //dbgUI <<"KisNodeModel::flags" << index;
    if (!index.isValid())
        return Qt::ItemIsDropEnabled;

    Q_ASSERT(index.model() == this);
    Q_ASSERT(index.internalPointer());

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEditable | Qt::ItemIsDropEnabled;
    //dbgUI << flags << (flags & Qt::ItemIsDropEnabled);
    return flags;
}

bool KisNodeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    //dbgUI <<"KisNodeModel::setData( index=" << index <<", value=" << value <<", role=" << role;
    if (!index.isValid()) {
        if (role == ActiveRole) {
            emit nodeActivated(0);
        }
        return false;
    }

    Q_ASSERT(index.model() == this);
    Q_ASSERT(index.internalPointer());

    KisNode *node = static_cast<KisNode*>(index.internalPointer());
    PropertyList proplist;

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        node->setName(value.toString());
        emit dataChanged(index, index);
        return true;
    case PropertiesRole:
        {
            // don't record undo/redo for visibility, locked or alpha locked changes

            proplist = value.value<PropertyList>();
            bool undo = true;
            foreach(KoDocumentSectionModel::Property prop, proplist) {
                if (prop.name == i18n("Visible") && node->visible() !=prop.state.toBool()) undo = false;
                if (prop.name == i18n("Locked") && node->userLocked() != prop.state.toBool()) undo = false;
                if (prop.name == i18n("Alpha Locked")) {
                    if (KisPaintLayer* l = dynamic_cast<KisPaintLayer*>(node)) {
                        if (l->alphaLocked() != prop.state.toBool()) {
                            undo = false;
                        }
                    }
                }
            }

            if (undo) {
                m_d->image->undoAdapter()->addCommand(new KisNodePropertyListCommand(node, proplist));
            }
            else {
                node->setSectionModelProperties(proplist);
                node->setDirty();
            }

            emit dataChanged(index, index);
            return true;
        }
    case ActiveRole:
        if (value.toBool()) {
            emit nodeActivated(node);
            emit dataChanged(index, index);
            return true;
        }
    }

    return false;
}


void KisNodeModel::beginInsertNodes(KisNode * parent, int index)
{
    //dbgUI <<"KisNodeModel::beginInsertNodes parent=" << parent << ", childcount: " << parent->childCount() << ", index=" << index;

    beginInsertRows(indexFromNode(parent), parent->childCount() - index, parent->childCount() - index);
}

void KisNodeModel::endInsertNodes(KisNode * parent, int index)
{
    KisNodeSP node = parent->at(index);
    if (node->nodeProgressProxy()) {
        connect(node->nodeProgressProxy(), SIGNAL(percentageChanged(int, const KisNodeSP&)), SLOT(progressPercentageChanged(int, const KisNodeSP&)));

    }
    //dbgUI <<"KisNodeModel::endInsertNodes";
    endInsertRows();
}

void KisNodeModel::beginRemoveNodes(KisNode * parent, int index)
{
    //dbgUI <<"KisNodeModel::beginRemoveNodes parent=" << parent << ", index=" << index;
    beginRemoveRows(indexFromNode(parent), parent->childCount() - 1 - index, parent->childCount() - 1 - index);
}

void KisNodeModel::endRemoveNodes(KisNode *, int)
{
    //dbgUI <<"KisNodeModel::endRemoveNodes";
    endRemoveRows();
}

Qt::DropActions KisNodeModel::supportedDropActions() const
{
    return Qt::MoveAction | Qt::CopyAction;
}

QStringList KisNodeModel::mimeTypes() const
{
    QStringList types;
    types << QLatin1String("application/x-kritalayermodeldatalist");
    return types;
}

QMimeData * KisNodeModel::mimeData(const QModelIndexList & indexes) const
{
    //dbgUI <<"KisNodeModel::mimeData";
    QMimeData* data = new QMimeData;
    QByteArray encoded;
    QDataStream stream(&encoded, QIODevice::WriteOnly);

    // encode the data
    QModelIndexList::ConstIterator it = indexes.begin();
    for (; it != indexes.end(); ++it) {
        stream << qVariantFromValue(qulonglong(it->internalPointer()));
    }

    data->setData("application/x-kritalayermodeldatalist", encoded);
    return data;
}

bool KisNodeModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent)
{

    Q_UNUSED(row);
    Q_UNUSED(column);
    dbgUI << "KisNodeModel::dropMimeData" << data->formats() << " row = " << row << " column = " << column;
    if (! data->hasFormat("application/x-kritalayermodeldatalist")) {
        dbgUI << "Does not have 'application/x-kritalayermodeldatalist'";
        return false;
    }
    QByteArray encoded = data->data("application/x-kritalayermodeldatalist");
    QDataStream stream(&encoded, QIODevice::ReadOnly);
    QList<KisNode*> nodes;
    while (! stream.atEnd()) {
        QVariant v;
        stream >> v;
        nodes.push_back(static_cast<KisNode*>((void*)v.value<qulonglong>()));
    }


    /*    QByteArray encoded = data->data(format);
        QDataStream stream(&encoded, QIODevice::ReadOnly);*/
    KisNodeSP activeNode = static_cast<KisNode*>(parent.internalPointer());
    KisNodeSP parentNode = 0;
    if (activeNode && activeNode->parent()) {
        parentNode = activeNode->parent();
    } else {
        parentNode = m_d->image->root();
    }
    if (action == Qt::CopyAction) {
        dbgUI << "KisNodeModel::dropMimeData copy action on " << activeNode;
        foreach(KisNode* n, nodes) {
            if (row >= 0) {
                emit requestAddNode(n->clone(), parentNode, parentNode->childCount() - row);
            } else {
                emit requestAddNode(n->clone(), activeNode);
            }
        }
        return true;
    } else if (action == Qt::MoveAction) {
        dbgUI << "KisNodeModel::dropMimeData move action on " << activeNode;
        foreach(KisNode* n, nodes) {
            if (row >= 0) {
                emit requestMoveNode(n, parentNode, parentNode->childCount() - row);
            } else {
                emit requestMoveNode(n, activeNode);
            }
        }
        return true;
    }
    dbgUI << "Action was not Copy or Move " << action;
    return false;
}

Qt::DropActions KisNodeModel::supportedDragActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

void KisNodeModel::updateSettings()
{
    KisConfig cfg;
    m_d->showRootLayer = cfg.showRootLayer();
    reset();
}

void KisNodeModel::progressPercentageChanged(int, const KisNodeSP _node)
{
    QModelIndex index = indexFromNode(_node);
    emit(dataChanged(index, index));
}

void KisNodeModel::layersChanged()
{
    reset();
}


#include "kis_node_model.moc"
