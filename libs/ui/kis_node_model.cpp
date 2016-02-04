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
#include <QBuffer>

#include <KoColorSpaceConstants.h>

#include <klocalizedstring.h>

#include "kis_mimedata.h"
#include <kis_debug.h>
#include <kis_node.h>
#include <kis_node_progress_proxy.h>
#include <kis_image.h>
#include <kis_selection.h>
#include <kis_selection_mask.h>
#include <kis_undo_adapter.h>
#include <commands/kis_node_property_list_command.h>
#include <kis_paint_layer.h>
#include <kis_group_layer.h>
#include <kis_projection_leaf.h>


#include "kis_dummies_facade_base.h"
#include "kis_node_dummies_graph.h"
#include "kis_model_index_converter.h"
#include "kis_model_index_converter_show_all.h"
#include "kis_node_selection_adapter.h"
#include "kis_node_insertion_adapter.h"

#include "kis_config.h"
#include "kis_config_notifier.h"
#include <QTimer>


struct KisNodeModel::Private
{
    KisImageWSP image;
    KisShapeController *shapeController = 0;
    KisNodeSelectionAdapter *nodeSelectionAdapter = 0;
    KisNodeInsertionAdapter *nodeInsertionAdapter = 0;
    QList<KisNodeDummy*> updateQueue;
    QTimer updateTimer;

    KisModelIndexConverterBase *indexConverter = 0;
    KisDummiesFacadeBase *dummiesFacade = 0;
    bool needFinishRemoveRows = false;
    bool needFinishInsertRows = false;
    bool showRootLayer = false;
    bool showGlobalSelection = false;
    QPersistentModelIndex activeNodeIndex;

    KisNodeDummy* parentOfRemovedNode = 0;
};

KisNodeModel::KisNodeModel(QObject * parent)
        : QAbstractItemModel(parent)
        , m_d(new Private)
{
    updateSettings();
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), this, SLOT(updateSettings()));

    m_d->updateTimer.setSingleShot(true);
    connect(&m_d->updateTimer, SIGNAL(timeout()), SLOT(processUpdateQueue()));
}

KisNodeModel::~KisNodeModel()
{
    delete m_d->indexConverter;
    delete m_d;
}

KisNodeSP KisNodeModel::nodeFromIndex(const QModelIndex &index) const
{
    Q_ASSERT(index.isValid());

    KisNodeDummy *dummy = m_d->indexConverter->dummyFromIndex(index);
    return dummy->node();
}

QModelIndex KisNodeModel::indexFromNode(KisNodeSP node) const
{
    KisNodeDummy *dummy = m_d->dummiesFacade->dummyForNode(node);
    if(dummy)
        return m_d->indexConverter->indexFromDummy(dummy);
    return QModelIndex();
}

bool KisNodeModel::belongsToIsolatedGroup(KisImageSP image, KisNodeSP node, KisDummiesFacadeBase *dummiesFacade)
{
    KisNodeSP isolatedRoot = image->isolatedModeRoot();
    if (!isolatedRoot) return true;

    KisNodeDummy *isolatedRootDummy =
        dummiesFacade->dummyForNode(isolatedRoot);
    KisNodeDummy *dummy =
        dummiesFacade->dummyForNode(node);

    while (dummy) {
        if (dummy == isolatedRootDummy) {
            return true;
        }
        dummy = dummy->parent();
    }

    return false;
}

bool KisNodeModel::belongsToIsolatedGroup(KisNodeSP node) const
{
    return belongsToIsolatedGroup(m_d->image, node, m_d->dummiesFacade);
}

void KisNodeModel::resetIndexConverter()
{
    delete m_d->indexConverter;
    m_d->indexConverter = 0;

    if(m_d->dummiesFacade) {
        m_d->indexConverter = createIndexConverter();
    }
}

KisModelIndexConverterBase *KisNodeModel::createIndexConverter()
{
    if(m_d->showRootLayer) {
        return new KisModelIndexConverterShowAll(m_d->dummiesFacade, this);
    } else {
        return new KisModelIndexConverter(m_d->dummiesFacade, this, m_d->showGlobalSelection);
    }
}

void KisNodeModel::regenerateItems(KisNodeDummy *dummy)
{
    const QModelIndex &index = m_d->indexConverter->indexFromDummy(dummy);
    emit dataChanged(this->index(index.row(), 0, index.parent()),
                     this->index(index.row(), 1, index.parent()));

    dummy = dummy->firstChild();
    while (dummy) {
        regenerateItems(dummy);
        dummy = dummy->nextSibling();
    }
}

void KisNodeModel::slotIsolatedModeChanged()
{
    regenerateItems(m_d->dummiesFacade->rootDummy());
}

bool KisNodeModel::showGlobalSelection() const
{
    KisConfig cfg;
    return cfg.showGlobalSelection();
}

void KisNodeModel::setShowGlobalSelection(bool value)
{
    KisConfig cfg;
    cfg.setShowGlobalSelection(value);
    updateSettings();
}

void KisNodeModel::updateSettings()
{
    KisConfig cfg;
    bool oldShowRootLayer = m_d->showRootLayer;
    bool oldShowGlobalSelection = m_d->showGlobalSelection;
    m_d->showRootLayer = cfg.showRootLayer();
    m_d->showGlobalSelection = cfg.showGlobalSelection();
    if (m_d->showRootLayer != oldShowRootLayer || m_d->showGlobalSelection != oldShowGlobalSelection) {
        resetIndexConverter();
        reset();
    }
}

void KisNodeModel::progressPercentageChanged(int, const KisNodeSP node)
{
    if(!m_d->dummiesFacade) return;

    // Need to check here as the node might already be removed, but there might
    // still be some signals arriving from another thread
    if (m_d->dummiesFacade->hasDummyForNode(node)) {
        QModelIndex index = indexFromNode(node);

        // no need to update 0st column!
        emit dataChanged(index, index);
    }
}

KisModelIndexConverterBase * KisNodeModel::indexConverter() const
{
    return m_d->indexConverter;
}

KisDummiesFacadeBase *KisNodeModel::dummiesFacade() const
{
    return m_d->dummiesFacade;
}

void KisNodeModel::connectDummy(KisNodeDummy *dummy, bool needConnect)
{
    KisNodeSP node = dummy->node();
    KisNodeProgressProxy *progressProxy = node->nodeProgressProxy();
    if(progressProxy) {
        if(needConnect) {
            connect(progressProxy, SIGNAL(percentageChanged(int,KisNodeSP)),
                    SLOT(progressPercentageChanged(int,KisNodeSP)));
        } else {
            progressProxy->disconnect(this);
        }
    }
}

void KisNodeModel::connectDummies(KisNodeDummy *dummy, bool needConnect)
{
    connectDummy(dummy, needConnect);

    dummy = dummy->firstChild();
    while(dummy) {
        connectDummies(dummy, needConnect);
        dummy = dummy->nextSibling();
    }
}

void KisNodeModel::setDummiesFacade(KisDummiesFacadeBase *dummiesFacade, KisImageWSP image, KisShapeController *shapeController, KisNodeSelectionAdapter *nodeSelectionAdapter, KisNodeInsertionAdapter *nodeInsertionAdapter)
{
    KisDummiesFacadeBase *oldDummiesFacade;
    KisShapeController *oldShapeController;
    oldShapeController = m_d->shapeController;
    oldDummiesFacade = m_d->dummiesFacade;

    m_d->shapeController = shapeController;
    m_d->nodeSelectionAdapter = nodeSelectionAdapter;
    m_d->nodeInsertionAdapter = nodeInsertionAdapter;

    if(oldDummiesFacade && m_d->image) {
        m_d->image->disconnect(this);
        oldDummiesFacade->disconnect(this);
        connectDummies(m_d->dummiesFacade->rootDummy(), false);
    }

    m_d->image = image;
    m_d->dummiesFacade = dummiesFacade;
    m_d->parentOfRemovedNode = 0;
    resetIndexConverter();

    if(m_d->dummiesFacade) {
        KisNodeDummy *rootDummy = m_d->dummiesFacade->rootDummy();
        if(rootDummy) {
            connectDummies(rootDummy, true);
        }

        connect(m_d->dummiesFacade, SIGNAL(sigBeginInsertDummy(KisNodeDummy*,int,QString,bool)),
                SLOT(slotBeginInsertDummy(KisNodeDummy*,int,QString,bool)));
        connect(m_d->dummiesFacade, SIGNAL(sigEndInsertDummy(KisNodeDummy*)),
                SLOT(slotEndInsertDummy(KisNodeDummy*)));
        connect(m_d->dummiesFacade, SIGNAL(sigBeginRemoveDummy(KisNodeDummy*)),
                SLOT(slotBeginRemoveDummy(KisNodeDummy*)));
        connect(m_d->dummiesFacade, SIGNAL(sigEndRemoveDummy()),
                SLOT(slotEndRemoveDummy()));

        connect(m_d->dummiesFacade, SIGNAL(sigDummyChanged(KisNodeDummy*)),
                SLOT(slotDummyChanged(KisNodeDummy*)));

        if(m_d->image.isValid()) {
            connect(m_d->image, SIGNAL(sigIsolatedModeChanged()), SLOT(slotIsolatedModeChanged()));
        }
    }

    if(m_d->dummiesFacade != oldDummiesFacade || m_d->shapeController != oldShapeController) {
        reset();
    }
}

void KisNodeModel::slotBeginInsertDummy(KisNodeDummy *parent, int index, const QString &metaObjectType, bool isAnimated)
{
    int row = 0;
    QModelIndex parentIndex;

    bool willAdd =
        m_d->indexConverter->indexFromAddedDummy(parent, index,
                                                 metaObjectType,
                                                 isAnimated,
                                                 parentIndex, row);

    if(willAdd) {
        beginInsertRows(parentIndex, row, row);
        m_d->needFinishInsertRows = true;
    }
}

void KisNodeModel::slotEndInsertDummy(KisNodeDummy *dummy)
{
    if(m_d->needFinishInsertRows) {
        connectDummy(dummy, true);
        endInsertRows();
        m_d->needFinishInsertRows = false;
    }
}

void KisNodeModel::slotBeginRemoveDummy(KisNodeDummy *dummy)
{
    if (!dummy) return;

    // FIXME: is it really what we want?
    m_d->updateTimer.stop();
    m_d->updateQueue.clear();

    m_d->parentOfRemovedNode = dummy->parent();

    QModelIndex parentIndex;
    if (m_d->parentOfRemovedNode) {
        parentIndex = m_d->indexConverter->indexFromDummy(m_d->parentOfRemovedNode);
    }

    QModelIndex itemIndex = m_d->indexConverter->indexFromDummy(dummy);

    if (itemIndex.isValid()) {
        connectDummy(dummy, false);
        beginRemoveRows(parentIndex, itemIndex.row(), itemIndex.row());
        m_d->needFinishRemoveRows = true;
    }
}

void KisNodeModel::slotEndRemoveDummy()
{
    if(m_d->needFinishRemoveRows) {
        endRemoveRows();
        m_d->needFinishRemoveRows = false;
    }
}

void KisNodeModel::slotDummyChanged(KisNodeDummy *dummy)
{
    if (!m_d->updateQueue.contains(dummy)) {
        m_d->updateQueue.append(dummy);
    }
    m_d->updateTimer.start(1000);
}

void KisNodeModel::processUpdateQueue()
{
    Q_FOREACH (KisNodeDummy *dummy, m_d->updateQueue) {
        QModelIndex index = m_d->indexConverter->indexFromDummy(dummy);
        emit dataChanged(this->index(index.row(), 0, index.parent()),
                         this->index(index.row(), 1, index.parent()));
    }
    m_d->updateQueue.clear();
}

QModelIndex KisNodeModel::index(int row, int col, const QModelIndex &parent) const
{
    if(!m_d->dummiesFacade || !hasIndex(row, col, parent)) return QModelIndex();

    QModelIndex itemIndex;

    KisNodeDummy *dummy = m_d->indexConverter->dummyFromRow(row, parent);
    if(dummy) {
        itemIndex = m_d->indexConverter->indexFromDummy(dummy);
    }

    if (itemIndex.isValid() && itemIndex.column() != col) {
        itemIndex = createIndex(row, col, (void*)dummy);
    }

    return itemIndex;
}

int KisNodeModel::rowCount(const QModelIndex &parent) const
{
    if(!m_d->dummiesFacade) return 0;
    return m_d->indexConverter->rowCount(parent);
}

int KisNodeModel::columnCount(const QModelIndex&) const
{
    /**
     * We have a bit weird layout of columns in the tree view. That
     * happens because we need to paint visibility icon *before* the
     * actual layer tree. Therefore we put a separate column in the
     * initial position and tell the tree that the position of the
     * hierarchy is in the second column! So the QTreeView reserves us
     * a bit of space at the left to put the visibility icon in.
     *
     * Later we paint this icon using KisNodeDelegate. There is a hack
     *  actually: we paint it in a position of the first column using
     *  the delegate of the second one. That should be fixed in the
     *  future, but it needs a bit of work, because quite a lot of
     *  code will need to be shared between the two delegates.
     */

    return 2;
}

QModelIndex KisNodeModel::parent(const QModelIndex &index) const
{
    if(!m_d->dummiesFacade || !index.isValid()) return QModelIndex();

    KisNodeDummy *dummy = m_d->indexConverter->dummyFromIndex(index);
    KisNodeDummy *parentDummy = dummy->parent();

    QModelIndex parentIndex;

    if(parentDummy) {
        parentIndex = m_d->indexConverter->indexFromDummy(parentDummy);
    }

    return parentIndex;
}

QVariant KisNodeModel::data(const QModelIndex &index, int role) const
{
    if (!m_d->dummiesFacade || !index.isValid() || !m_d->image.isValid()) return QVariant();

    KisNodeSP node = nodeFromIndex(index);

    if (index.column() == 0) {
        /**
         * We have no data to return for the zero column, since it is
         * hi-jacked by the delegate of the first column...
         */
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole: return node->name();
    case Qt::DecorationRole: return node->icon();
    case Qt::EditRole: return node->name();
    case Qt::SizeHintRole: return m_d->image->size(); // FIXME
    case Qt::TextColorRole:
        return belongsToIsolatedGroup(node) &&
            !node->projectionLeaf()->isDroppedMask() ? QVariant() : QVariant(QColor(Qt::gray));
    case Qt::FontRole: {
        QFont baseFont;
        if (node->projectionLeaf()->isDroppedMask()) {
            baseFont.setStrikeOut(true);
        }
        if (m_d->activeNodeIndex == index) {
            baseFont.setBold(true);
        }
        return baseFont;
    }
    case KisBaseNode::PropertiesRole: return QVariant::fromValue(node->sectionModelProperties());
    case KisBaseNode::AspectRatioRole: return double(m_d->image->width()) / m_d->image->height();
    case KisBaseNode::ProgressRole: {
        KisNodeProgressProxy *proxy = node->nodeProgressProxy();
        return proxy ? proxy->percentage() : -1;
    }
    case KisBaseNode::ActiveRole: {
        return m_d->activeNodeIndex == index;
    }
    case KisBaseNode::ShouldGrayOutRole: {
        return !node->visible(true);
    }
    default:
        if (role >= int(KisBaseNode::BeginThumbnailRole) && belongsToIsolatedGroup(node))
            return node->createThumbnail(role - int(KisBaseNode::BeginThumbnailRole), role - int(KisBaseNode::BeginThumbnailRole));
        else
            return QVariant();
    }

    return QVariant();
}

Qt::ItemFlags KisNodeModel::flags(const QModelIndex &index) const
{
    if(!m_d->dummiesFacade || !index.isValid()) return Qt::ItemIsDropEnabled;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEditable | Qt::ItemIsDropEnabled;
    return flags;
}

bool KisNodeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{

    if (role == KisBaseNode::ActiveRole || role == KisBaseNode::AlternateActiveRole) {
        QModelIndex parentIndex;
        if (!index.isValid() && m_d->parentOfRemovedNode && m_d->dummiesFacade && m_d->indexConverter) {
            parentIndex = m_d->indexConverter->indexFromDummy(m_d->parentOfRemovedNode);
            m_d->parentOfRemovedNode = 0;
        }

        KisNodeSP activatedNode;

        if (index.isValid() && value.toBool()) {
            activatedNode = nodeFromIndex(index);
        }
        else if (parentIndex.isValid() && value.toBool()) {
            activatedNode = nodeFromIndex(parentIndex);
        }
        else {
            activatedNode = 0;
        }

        QModelIndex newActiveNode = activatedNode ? indexFromNode(activatedNode) : QModelIndex();
        if (role == KisBaseNode::ActiveRole && value.toBool() &&
            m_d->activeNodeIndex == newActiveNode) {

            return true;
        }

        m_d->activeNodeIndex = newActiveNode;

        if (m_d->nodeSelectionAdapter) {
            m_d->nodeSelectionAdapter->setActiveNode(activatedNode);
        }

        if (role == KisBaseNode::AlternateActiveRole) {
            emit toggleIsolateActiveNode();
        }

        emit dataChanged(this->index(index.row(), 0, index.parent()),
                         this->index(index.row(), 1, index.parent()));
        return true;
    }

    if(!m_d->dummiesFacade || !index.isValid()) return false;

    bool result = true;
    KisNodeSP node = nodeFromIndex(index);

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        node->setName(value.toString());
        break;
    case KisBaseNode::PropertiesRole:
        {
            // don't record undo/redo for visibility, locked or alpha locked changes
            KisBaseNode::PropertyList proplist = value.value<KisBaseNode::PropertyList>();
            KisNodePropertyListCommand::setNodePropertiesNoUndo(node, m_d->image, proplist);

            break;
        }
    default:
        result = false;
    }

    if(result) {
        emit dataChanged(this->index(index.row(), 0, index.parent()),
                         this->index(index.row(), 1, index.parent()));
    }

    return result;
}

Qt::DropActions KisNodeModel::supportedDragActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

Qt::DropActions KisNodeModel::supportedDropActions() const
{
    return Qt::MoveAction | Qt::CopyAction;
}

bool KisNodeModel::hasDummiesFacade()
{
    return m_d->dummiesFacade != 0;
}

QStringList KisNodeModel::mimeTypes() const
{
    QStringList types;
    types << QLatin1String("application/x-krita-node");
    types << QLatin1String("application/x-qt-image");
    return types;
}

QMimeData * KisNodeModel::mimeData(const QModelIndexList &indexes) const
{
    KisNodeList nodes;
    Q_FOREACH (const QModelIndex &idx, indexes) {
        if (idx.column() != 1) continue;
        nodes << nodeFromIndex(idx);
    }

    return KisMimeData::mimeForLayers(nodes, m_d->image->rootLayer());
}

bool KisNodeModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent)
{
    Q_UNUSED(column);

    bool copyNode = (action == Qt::CopyAction);

    KisNodeDummy *parentDummy = 0;
    KisNodeDummy *aboveThisDummy = 0;

    parentDummy = parent.isValid() ?
        m_d->indexConverter->dummyFromIndex(parent) :
        m_d->dummiesFacade->rootDummy();

    if (row == -1) {
        aboveThisDummy = parent.isValid() ? parentDummy->lastChild() : 0;
    }
    else {
        aboveThisDummy = row < m_d->indexConverter->rowCount(parent) ? m_d->indexConverter->dummyFromRow(row, parent) : 0;
    }

    return KisMimeData::insertMimeLayers(data,
                                         m_d->image,
                                         m_d->shapeController,
                                         parentDummy,
                                         aboveThisDummy,
                                         copyNode,
                                         m_d->nodeInsertionAdapter);
}

