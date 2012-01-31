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
#include <QDataStream>
#include <QBuffer>

#include <klocale.h>

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

#include "kis_dummies_facade_base.h"
#include "kis_node_dummies_graph.h"

#include "kis_config.h"
#include "kis_config_notifier.h"
#include <QTimer>

struct KisNodeModel::Private
{
public:
    Private() : dummiesFacade(0), needFinishRemoveRows(false) {}

    KisImageWSP image;
    bool showRootLayer;
    QList<KisNodeDummy*> updateQueue;
    QTimer* updateTimer;

    KisDummiesFacadeBase *dummiesFacade;
    bool needFinishRemoveRows;
};

KisNodeModel::KisNodeModel(QObject * parent)
        : KoDocumentSectionModel(parent)
        , m_d(new Private)
{
    updateSettings();
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), this, SLOT(updateSettings()));
    m_d->updateTimer = new QTimer(this);
    m_d->updateTimer->setSingleShot(true);
    connect(m_d->updateTimer, SIGNAL(timeout()), SLOT(processUpdateQueue()));
}

KisNodeModel::~KisNodeModel()
{
    delete m_d;
}

inline KisNodeSP KisNodeModel::nodeFromDummy(KisNodeDummy *dummy) {
    return dummy->node();
}

inline KisNodeDummy* KisNodeModel::dummyFromIndex(const QModelIndex &index) {
    Q_ASSERT(index.internalPointer());
    return static_cast<KisNodeDummy*>(index.internalPointer());
}

KisNodeSP KisNodeModel::nodeFromIndex(const QModelIndex &index) const
{
    return nodeFromDummy(dummyFromIndex(index));
}

QModelIndex KisNodeModel::indexFromNode(KisNodeSP node) const
{
    return indexFromDummy(m_d->dummiesFacade->dummyForNode(node));
}

void KisNodeModel::updateSettings()
{
    KisConfig cfg;
    m_d->showRootLayer = cfg.showRootLayer();
    reset();
}

void KisNodeModel::progressPercentageChanged(int, const KisNodeSP node)
{
    QModelIndex index = indexFromNode(node);

    emit dataChanged(index, index);
}

void KisNodeModel::connectDummy(KisNodeDummy *dummy, bool needConnect)
{
    KisNodeSP node = nodeFromDummy(dummy);
    KisNodeProgressProxy *progressProxy = node->nodeProgressProxy();
    if(progressProxy) {
        if(needConnect) {
            connect(progressProxy, SIGNAL(percentageChanged(int, const KisNodeSP&)),
                    SLOT(progressPercentageChanged(int, const KisNodeSP&)));
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

void KisNodeModel::setDummiesFacade(KisDummiesFacadeBase *dummiesFacade, KisImageWSP image)
{
    m_d->image = image;

    if(m_d->dummiesFacade) {
        m_d->dummiesFacade->disconnect(this);
        connectDummies(m_d->dummiesFacade->rootDummy(), false);
    }

    m_d->dummiesFacade = dummiesFacade;

    if(m_d->dummiesFacade) {
        KisNodeDummy *rootDummy = m_d->dummiesFacade->rootDummy();
        if(rootDummy) {
            connectDummies(rootDummy, true);
        }

        connect(m_d->dummiesFacade, SIGNAL(sigBeginInsertDummy(KisNodeDummy*, int)),
                SLOT(slotBeginInsertDummy(KisNodeDummy*, int)));
        connect(m_d->dummiesFacade, SIGNAL(sigEndInsertDummy(KisNodeDummy*)),
                SLOT(slotEndInsertDummy(KisNodeDummy*)));
        connect(m_d->dummiesFacade, SIGNAL(sigBeginRemoveDummy(KisNodeDummy*)),
                SLOT(slotBeginRemoveDummy(KisNodeDummy*)));
        connect(m_d->dummiesFacade, SIGNAL(sigEndRemoveDummy()),
                SLOT(slotEndRemoveDummy()));

        connect(m_d->dummiesFacade, SIGNAL(sigDummyChanged(KisNodeDummy*)),
                SLOT(slotDummyChanged(KisNodeDummy*)));
    }

    reset();
}

void KisNodeModel::slotBeginInsertDummy(KisNodeDummy *parent, int index)
{
    int row = 0;
    QModelIndex parentIndex;

    if(parent) {
        int rowCount = parent->childCount();
        row = rowCount - index;
        parentIndex = indexFromDummy(parent);
    }

    if(parent || m_d->showRootLayer) {
        beginInsertRows(parentIndex, row, row);
    }
}

void KisNodeModel::slotEndInsertDummy(KisNodeDummy *dummy)
{
    connectDummy(dummy, true);

    if(dummy->parent() || m_d->showRootLayer) {
        endInsertRows();
    }
}

void KisNodeModel::slotBeginRemoveDummy(KisNodeDummy *dummy)
{
    connectDummy(dummy, false);

    // FIXME: is it really what we want?
    m_d->updateTimer->stop();
    m_d->updateQueue.clear();

    KisNodeDummy *parentDummy = dummy->parent();

    int row = 0;
    QModelIndex parentIndex;

    if(parentDummy) {
        int rowCount = parentDummy->childCount();
        int index = parentDummy->indexOf(dummy);

        row = rowCount - index - 1;
        parentIndex = indexFromDummy(parentDummy);
    }

    if(parentDummy || m_d->showRootLayer) {
        beginRemoveRows(parentIndex, row, row);
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
    m_d->updateTimer->start(1000);
}

void KisNodeModel::processUpdateQueue()
{
    foreach(KisNodeDummy *dummy, m_d->updateQueue) {
        QModelIndex index = indexFromDummy(dummy);
        emit dataChanged(index, index);
    }
    m_d->updateQueue.clear();
}

QModelIndex KisNodeModel::indexFromDummy(KisNodeDummy *dummy) const
{
    if(!dummy->parent()) {
        return m_d->showRootLayer ?
            createIndex(0, 0, dummy) : QModelIndex();
    }
    else if (!m_d->showRootLayer &&
             !dummy->parent()->parent() &&
             nodeFromDummy(dummy)->inherits("KisSelectionMask")) {

        // Don't show the global selection mask if we don't show the root layer
        return QModelIndex();
    }
    else {
        int rowCount = dummy->parent()->childCount();
        int index = dummy->parent()->indexOf(dummy);
        int row = rowCount - index - 1;
        return createIndex(row, 0, (void*)dummy);
    }
}

QModelIndex KisNodeModel::index(int row, int col, const QModelIndex &parent) const
{
    if(!m_d->dummiesFacade || !hasIndex(row, col, parent)) return QModelIndex();

    KisNodeDummy *parentDummy;

    if(parent.isValid()) {
        parentDummy = dummyFromIndex(parent);
    }
    else if(!m_d->showRootLayer) {
        parentDummy = m_d->dummiesFacade->rootDummy();
    }
    else {
        Q_ASSERT(row == 0);
        KisNodeDummy *rootDummy = m_d->dummiesFacade->rootDummy();
        return rootDummy ? indexFromDummy(rootDummy) : QModelIndex();
    }

    QModelIndex modelIndex;
    if(parentDummy) {
        int rowCount = parentDummy->childCount();
        int index = rowCount - row - 1;
        modelIndex = createIndex(row, col, parentDummy->at(index));
    }
    return modelIndex;
}

int KisNodeModel::rowCount(const QModelIndex &parent) const
{
    if(!m_d->dummiesFacade) return 0;

    KisNodeDummy *parentDummy;

    if(parent.isValid()) {
        parentDummy = dummyFromIndex(parent);
    }
    else if(!m_d->showRootLayer) {
        parentDummy = m_d->dummiesFacade->rootDummy();
    }
    else {
        return 1;
    }

    return parentDummy ? parentDummy->childCount() : 0;
}

int KisNodeModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QModelIndex KisNodeModel::parent(const QModelIndex &index) const
{
    if(!m_d->dummiesFacade || !index.isValid()) return QModelIndex();

    KisNodeDummy *dummy = dummyFromIndex(index);

    Q_ASSERT(m_d->showRootLayer || dummy->parent());
    return dummy->parent() ? indexFromDummy(dummy->parent()) : QModelIndex();
}

QVariant KisNodeModel::data(const QModelIndex &index, int role) const
{
    if(!m_d->dummiesFacade || !index.isValid()) return QVariant();

    KisNodeSP node = nodeFromIndex(index);

    switch (role) {
    case Qt::DisplayRole: return node->name();
    case Qt::DecorationRole: return node->icon();
    case Qt::EditRole: return node->name();
    case Qt::SizeHintRole: return m_d->image->size(); // FIXME
    case PropertiesRole: return QVariant::fromValue(node->sectionModelProperties());
    case AspectRatioRole: return double(m_d->image->width()) / m_d->image->height();
    case ProgressRole: {
        KisNodeProgressProxy *proxy = node->nodeProgressProxy();
        return proxy ? proxy->percentage() : -1;
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
    if(!m_d->dummiesFacade || !index.isValid()) return Qt::ItemIsDropEnabled;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEditable | Qt::ItemIsDropEnabled;
    return flags;
}

bool KisNodeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(role == ActiveRole) {
        KisNodeSP activatedNode =
            index.isValid() && value.toBool() ? nodeFromIndex(index) : 0;
        emit nodeActivated(activatedNode);
        emit dataChanged(index, index);
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
    case PropertiesRole:
        {
            // don't record undo/redo for visibility, locked or alpha locked changes
            PropertyList proplist = value.value<PropertyList>();
            bool undo = true;
            foreach(KoDocumentSectionModel::Property prop, proplist) {
                if (prop.name == i18n("Visible") && node->visible() !=prop.state.toBool()) undo = false;
                if (prop.name == i18n("Locked") && node->userLocked() != prop.state.toBool()) undo = false;
                if (prop.name == i18n("Active")) {
                    if (KisSelectionMask *m = dynamic_cast<KisSelectionMask*>(node.data())) {
                        if (m->active() != prop.state.toBool()) {
                            undo = false;
                        }
                    }
                }
                if (prop.name == i18n("Alpha Locked")) {
                    if (KisPaintLayer* l = dynamic_cast<KisPaintLayer*>(node.data())) {
                        if (l->alphaLocked() != prop.state.toBool()) {
                            undo = false;
                        }
                    }
                }
            }

            KUndo2Command *cmd = new KisNodePropertyListCommand(node, proplist);

            if (undo) {
                m_d->image->undoAdapter()->addCommand(cmd);
            }
            else {
                cmd->redo();
                delete cmd;
            }

            break;
        }
    default:
        result = false;
    }

    if(result) {
        emit dataChanged(index, index);
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

QStringList KisNodeModel::mimeTypes() const
{
    QStringList types;
    types << QLatin1String("application/x-krita-node");
    return types;
}

QMimeData * KisNodeModel::mimeData(const QModelIndexList &indexes) const
{
    Q_ASSERT(indexes.count() == 1); // we only allow one node at a time to be stored as mimedata

    KisNodeSP node = nodeFromIndex(indexes.first());
    KisMimeData* data = new KisMimeData();
    data->setNode(node);
    return data;
}

bool KisNodeModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent)
{
    Q_UNUSED(column);

    const KisMimeData *mimedata = qobject_cast<const KisMimeData*>(data);
    KisNodeSP node = mimedata ? mimedata->node() : 0;

    if(!node) return false;

    KisNodeSP activeNode = parent.isValid() ? nodeFromIndex(parent) : 0;

    KisNodeSP parentNode = 0;
    if (activeNode && activeNode->parent()) {
        parentNode = activeNode->parent();
    } else {
        parentNode = m_d->image->root();
    }
    dbgUI << activeNode << " " << parentNode;
    if (action == Qt::CopyAction) {
        dbgUI << "KisNodeModel::dropMimeData copy action on " << activeNode;
        if (row >= 0) {
            emit requestAddNode(node->clone(), parentNode, parentNode->childCount() - row);
        } else if (activeNode) {
            emit requestAddNode(node->clone(), activeNode);
        } else {
            emit requestAddNode(node->clone(), parentNode, 0);
        }
        return true;
    }
    else if (action == Qt::MoveAction) {
        dbgUI << "KisNodeModel::dropMimeData move action on " << activeNode;
        if (row >= 0) {
            emit requestMoveNode(node, parentNode, parentNode->childCount() - row);
        } else if (activeNode) {
            emit requestMoveNode(node, activeNode);
        } else {
            emit requestMoveNode(node, parentNode, 0);
        }
        return true;
    }
    dbgUI << "Action was not Copy or Move " << action;
    return false;
}

#include "kis_node_model.moc"
