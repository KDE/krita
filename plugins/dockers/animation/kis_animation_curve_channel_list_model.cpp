/*
 *  Copyright (c) 2016 Jouni Pentikäinen <joupent@gmail.com>
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

#include "kis_animation_curve_channel_list_model.h"

#include "kis_animation_curves_model.h"
#include "kis_dummies_facade_base.h"
#include "kis_node_dummies_graph.h"
#include "kis_node.h"
#include "kis_scalar_keyframe_channel.h"
#include "kis_signal_auto_connection.h"

const quintptr ID_NODE = 0xffffffff;

struct NodeListItem
{
    NodeListItem(KisNodeDummy *dummy)
        : dummy(dummy)
    {}

    KisNodeDummy *dummy;
    QList<KisAnimationCurve*> curves;
};

struct KisAnimationCurveChannelListModel::Private
{
    KisAnimationCurvesModel *curvesModel;
    KisDummiesFacadeBase *dummiesFacade = 0;
    KisSignalAutoConnectionsStore dummiesFacadeConnections;

    QList<NodeListItem*> items;

    Private(KisAnimationCurvesModel *curvesModel)
        : curvesModel(curvesModel)
    {}

    NodeListItem * itemForRow(int row) {
        if (row < 0 || row >= items.count())
            return nullptr;
        return items.at(row);
    }

    int rowForDummy(KisNodeDummy *dummy) {
        for (int row=0; row < items.count(); row++) {
            if (items.at(row)->dummy == dummy) return row;
        }

        return -1;
    }

    void addCurveForChannel(NodeListItem *nodeItem, KisKeyframeChannel *channel) {
        KisScalarKeyframeChannel *scalarChannel = dynamic_cast<KisScalarKeyframeChannel*>(channel);

        if (scalarChannel) {
            KisAnimationCurve *curve = curvesModel->addCurve(scalarChannel);
            nodeItem->curves.append(curve);
        }
    }
};

KisAnimationCurveChannelListModel::KisAnimationCurveChannelListModel(KisAnimationCurvesModel *curvesModel, QObject *parent)
    : QAbstractItemModel(parent)
    , m_d(new Private(curvesModel))
{}

KisAnimationCurveChannelListModel::~KisAnimationCurveChannelListModel()
{
    qDeleteAll(m_d->items);
    m_d->items.clear();
}

void KisAnimationCurveChannelListModel::setDummiesFacade(KisDummiesFacadeBase *facade)
{
    m_d->dummiesFacadeConnections.clear();
    m_d->dummiesFacade = facade;
    m_d->dummiesFacadeConnections.addConnection(m_d->dummiesFacade, SIGNAL(sigBeginRemoveDummy(KisNodeDummy*)),
                                                this, SLOT(slotNotifyDummyRemoved(KisNodeDummy*)));
}

void KisAnimationCurveChannelListModel::selectedNodesChanged(const KisNodeList &nodes)
{
    // Remove unselected nodes
    for (int i = m_d->items.count()-1; i >= 0; i--) {
        NodeListItem *item = m_d->items.at(i);
        if (item && item->dummy) {
            if (!nodes.contains(item->dummy->node())) {
                beginRemoveRows(QModelIndex(), i, i);
                m_d->items.removeAt(i);
                endRemoveRows();

                Q_FOREACH(KisAnimationCurve *curve, item->curves) {
                    m_d->curvesModel->removeCurve(curve);
                }

                item->dummy->node()->disconnect(this);
                delete item;
            }
        }
    }

    // Add newly selected nodes
    Q_FOREACH(KisNodeSP node, nodes) {
        KisNodeDummy *dummy = m_d->dummiesFacade->dummyForNode(node);
        if (!dummy) continue;

        if (m_d->rowForDummy(dummy) == -1) {
            beginInsertRows(QModelIndex(), m_d->items.count(), m_d->items.count());

            NodeListItem *item = new NodeListItem(dummy);
            m_d->items.append(item);

            Q_FOREACH(KisKeyframeChannel *channel, dummy->node()->keyframeChannels()) {
                m_d->addCurveForChannel(item, channel);
            }

            connect(node.data(), &KisNode::keyframeChannelAdded,
                    this, &KisAnimationCurveChannelListModel::keyframeChannelAddedToNode);

            endInsertRows();
        }
    }
}

void KisAnimationCurveChannelListModel::keyframeChannelAddedToNode(KisKeyframeChannel *channel)
{
    KisNodeDummy *dummy = m_d->dummiesFacade->dummyForNode(KisNodeSP(channel->node()));
    int row = m_d->rowForDummy(dummy);
    KIS_ASSERT_RECOVER_RETURN(row >= 0);

    NodeListItem *item = m_d->itemForRow(row);

    int newCurveRow = item->curves.count();
    beginInsertRows(index(row, 0, QModelIndex()), newCurveRow, newCurveRow);

    m_d->addCurveForChannel(item, channel);

    endInsertRows();
}

void KisAnimationCurveChannelListModel::slotNotifyDummyRemoved(KisNodeDummy *dummy)
{
    bool shouldChangeSelection = false;
    KisNodeList newSelectedNodes;

    Q_FOREACH (NodeListItem *item, m_d->items) {
        if (item->dummy == dummy) {
            shouldChangeSelection = true;
            break;
        }

        newSelectedNodes << item->dummy->node();
    }

    if (shouldChangeSelection) {
        selectedNodesChanged(newSelectedNodes);
    }
}

QModelIndex KisAnimationCurveChannelListModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(column);

    if (!parent.isValid()) {
        // Node

        NodeListItem *item = m_d->itemForRow(row);
        if (!item) return QModelIndex();

        return createIndex(row, column, ID_NODE);
    } else {
        // Channel
        if (parent.parent().isValid()) return QModelIndex();

        NodeListItem *parentItem = m_d->itemForRow(parent.row());
        if (!parentItem) return QModelIndex();

        if (row >= parentItem->curves.count()) return QModelIndex();

        return createIndex(row, column, parent.row());
    }
}

QModelIndex KisAnimationCurveChannelListModel::parent(const QModelIndex &child) const
{
    quintptr parentIndex = child.internalId();
    if (parentIndex == ID_NODE) return QModelIndex();
    return createIndex(parentIndex, 0, ID_NODE);
}

int KisAnimationCurveChannelListModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        // Root
        return m_d->items.count();
    } else if (parent.internalId() == ID_NODE) {
        // Node
        NodeListItem *item = m_d->itemForRow(parent.row());
        return item->curves.count();
    } else {
        // Channel
        return 0;
    }
}

int KisAnimationCurveChannelListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant KisAnimationCurveChannelListModel::data(const QModelIndex &index, int role) const
{
    quintptr parentRow = index.internalId();
    bool indexIsNode = (parentRow == ID_NODE);
    NodeListItem *item = m_d->itemForRow(indexIsNode ? index.row() : parentRow);

    switch (role) {
    case Qt::DisplayRole:
    {
        if (indexIsNode) {
            return item->dummy->node()->name();
        } else {
            KisKeyframeChannel *channel = item->curves.at(index.row())->channel();
            return channel->name();
        }
    }
        break;
    case CurveColorRole:
        return indexIsNode ? QVariant() : item->curves.at(index.row())->color();
    case CurveVisibleRole:
        return indexIsNode ? QVariant() : item->curves.at(index.row())->visible();
    }

    return QVariant();
}

bool KisAnimationCurveChannelListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    quintptr parentRow = index.internalId();
    bool indexIsNode = (parentRow == ID_NODE);
    NodeListItem *item = m_d->itemForRow(indexIsNode ? index.row() : parentRow);

    switch (role) {
    case CurveVisibleRole:
        KIS_ASSERT_RECOVER_BREAK(!indexIsNode);
        m_d->curvesModel->setCurveVisible(item->curves.at(index.row()), value.toBool());
        break;
    }

    return false;
}


void KisAnimationCurveChannelListModel::clear()
{
    qDeleteAll(m_d->items);
    m_d->items.clear();
}
