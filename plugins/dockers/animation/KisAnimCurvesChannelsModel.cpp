/*
 *  SPDX-FileCopyrightText: 2016 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisAnimCurvesChannelsModel.h"

#include "KisAnimCurvesModel.h"
#include "kis_dummies_facade_base.h"
#include "kis_node_dummies_graph.h"
#include "kis_node.h"
#include "kis_node_view_color_scheme.h"
#include "kis_scalar_keyframe_channel.h"
#include "kis_signal_auto_connection.h"
#include "krita_utils.h"
#include "kis_image.h"
#include "KisAnimUtils.h"

#include <QApplication>

const quintptr ID_NODE = 0xffffffff;

struct NodeListItem
{
    NodeListItem(KisNodeDummy *dummy)
        : dummy(dummy)
    {}

    KisNodeDummy *dummy;
    QList<KisAnimationCurve*> curves;
};

struct KisAnimCurvesChannelsModel::Private
{
    KisAnimCurvesModel *curvesModel;
    KisDummiesFacadeBase *dummiesFacade = 0;
    KisSignalAutoConnectionsStore dummiesFacadeConnections;

    QList<NodeListItem*> items;

    Private(KisAnimCurvesModel *curvesModel)
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

KisAnimCurvesChannelsModel::KisAnimCurvesChannelsModel(KisAnimCurvesModel *curvesModel, QObject *parent)
    : QAbstractItemModel(parent)
    , m_d(new Private(curvesModel))
{}

KisAnimCurvesChannelsModel::~KisAnimCurvesChannelsModel()
{
    qDeleteAll(m_d->items);
    m_d->items.clear();
}

void KisAnimCurvesChannelsModel::setDummiesFacade(KisDummiesFacadeBase *facade)
{
    m_d->dummiesFacadeConnections.clear();
    m_d->dummiesFacade = facade;
    m_d->dummiesFacadeConnections.addConnection(m_d->dummiesFacade, SIGNAL(sigBeginRemoveDummy(KisNodeDummy*)),
                                                this, SLOT(slotNotifyDummyRemoved(KisNodeDummy*)));
}

void KisAnimCurvesChannelsModel::selectedNodesChanged(const KisNodeList &nodes)
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
                    this, &KisAnimCurvesChannelsModel::keyframeChannelAddedToNode);

            endInsertRows();
        }
    }
}

void KisAnimCurvesChannelsModel::reset(const QModelIndex &index)
{
    const quintptr parentRow = index.internalId();
    const bool indexIsNode = (parentRow == ID_NODE);

    if (indexIsNode) {
        NodeListItem *item = m_d->itemForRow(index.row());

        KisNodeSP node = item->dummy->node();
        if (!node || !node->image())
            return;

        KisImageSP image = node->image().toStrongRef();

        QList<KisAnimationCurve*> curves = item->curves;
        QList<QString> ids;
        Q_FOREACH( const KisAnimationCurve* curve, curves ) {
            ids << curve->channel()->id();
        }

        KisAnimUtils::resetChannels(image, node, ids);

    } else {
        NodeListItem *item = m_d->itemForRow(parentRow);

        KisAnimationCurve* curve = item->curves.at(index.row());

        if (!curve)
            return;

        KisNodeSP node = item->dummy->node();

        if (!node || !node->image())
            return;

        KisImageSP image = node->image().toStrongRef();
        KisAnimUtils::resetChannel(image, node, curve->channel()->id());
    }
}

void KisAnimCurvesChannelsModel::keyframeChannelAddedToNode(KisKeyframeChannel *channel)
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

void KisAnimCurvesChannelsModel::slotNotifyDummyRemoved(KisNodeDummy *dummy)
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

QModelIndex KisAnimCurvesChannelsModel::index(int row, int column, const QModelIndex &parent) const
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

QModelIndex KisAnimCurvesChannelsModel::parent(const QModelIndex &child) const
{
    quintptr parentIndex = child.internalId();
    if (parentIndex == ID_NODE) return QModelIndex();
    return createIndex(parentIndex, 0, ID_NODE);
}

int KisAnimCurvesChannelsModel::rowCount(const QModelIndex &parent) const
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

int KisAnimCurvesChannelsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant KisAnimCurvesChannelsModel::data(const QModelIndex &index, int role) const
{
    const quintptr parentRow = index.internalId();
    const bool indexIsNode = (parentRow == ID_NODE);
    NodeListItem *item = m_d->itemForRow(indexIsNode ? index.row() : parentRow);

    switch (role) {
    case Qt::DisplayRole: {
        if (indexIsNode) {
            return item->dummy->node()->name();
        } else {
            KisKeyframeChannel *channel = item->curves.at(index.row())->channel();
            return channel->name();
        }
    } break;

    case CurveRole:
        return !indexIsNode;

    case CurveColorRole:
        return indexIsNode ? QVariant() : item->curves.at(index.row())->color();

    case CurveVisibilityRole:
        return indexIsNode ? QVariant() : item->curves.at(index.row())->visible();

    case CurveIsIsolatedRole: {
        const bool isVisible = item->curves.at(index.row())->visible();
        if (!isVisible)
            return false;

        int numVisible = 0;
        for (int i = 0; i < item->curves.size(); i++) {
            if ( numVisible > 1)
                return false;

            if (item->curves.at(i)->visible())
                numVisible++;
        }

        return (numVisible == 1);
    }
    case NodeColorRole: {

        if (!indexIsNode)
            return QVariant();

        KisNodeViewColorScheme nodeColorScheme;
        const QColor backgroundColor = qApp->palette().color(QPalette::Button);
        const int colorLabelIndex = item->dummy->node()->colorLabelIndex();
        const QColor nodeColor = nodeColorScheme.colorFromLabelIndex(colorLabelIndex);
        return colorLabelIndex > 0 ? KritaUtils::blendColors(nodeColor, backgroundColor, 0.3) : backgroundColor;
    }
    default:
        break;
    }

    return QVariant();
}

bool KisAnimCurvesChannelsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    quintptr parentRow = index.internalId();
    bool indexIsNode = (parentRow == ID_NODE);
    NodeListItem *item = m_d->itemForRow(indexIsNode ? index.row() : parentRow);

    switch (role) {
    case CurveVisibilityRole:
        KIS_ASSERT_RECOVER_BREAK(!indexIsNode);
        m_d->curvesModel->setCurveVisible(item->curves.at(index.row()), value.toBool());
        this->dataChanged(index, index);
        return true;
    }

    return false;
}


void KisAnimCurvesChannelsModel::clear()
{
    qDeleteAll(m_d->items);
    m_d->items.clear();
}
