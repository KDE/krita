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
#include "kis_model_index_converter.h"
#include "kis_model_index_converter_show_all.h"

#include "kis_config.h"
#include "kis_config_notifier.h"
#include <QTimer>


struct KisNodeModel::Private
{
public:
    Private() : indexConverter(0),
                dummiesFacade(0),
                needFinishRemoveRows(false),
                needFinishInsertRows(false) {}

    KisImageWSP image;
    bool showRootLayer;
    QList<KisNodeDummy*> updateQueue;
    QTimer* updateTimer;

    KisModelIndexConverterBase *indexConverter;
    KisDummiesFacadeBase *dummiesFacade;
    bool needFinishRemoveRows;
    bool needFinishInsertRows;
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
    return m_d->indexConverter->indexFromDummy(dummy);
}

void KisNodeModel::resetIndexConverter()
{
    delete m_d->indexConverter;
    m_d->indexConverter = 0;

    if(m_d->dummiesFacade) {
        if(m_d->showRootLayer) {
            m_d->indexConverter =
                new KisModelIndexConverterShowAll(m_d->dummiesFacade, this);
        }
        else {
            m_d->indexConverter =
                new KisModelIndexConverter(m_d->dummiesFacade, this);
        }
    }
}

void KisNodeModel::updateSettings()
{
    KisConfig cfg;
    m_d->showRootLayer = cfg.showRootLayer();
    resetIndexConverter();
    reset();
}

void KisNodeModel::progressPercentageChanged(int, const KisNodeSP node)
{
    if(!m_d->dummiesFacade) return;

    // Need to check here as the node might already be removed, but there might
    // still be some signals arriving from another thread
    if (m_d->dummiesFacade->hasDummyForNode(node)) {
        QModelIndex index = indexFromNode(node);
        emit dataChanged(index, index);
    }
}

void KisNodeModel::connectDummy(KisNodeDummy *dummy, bool needConnect)
{
    KisNodeSP node = dummy->node();
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
    resetIndexConverter();

    if(m_d->dummiesFacade) {
        KisNodeDummy *rootDummy = m_d->dummiesFacade->rootDummy();
        if(rootDummy) {
            connectDummies(rootDummy, true);
        }

        connect(m_d->dummiesFacade, SIGNAL(sigBeginInsertDummy(KisNodeDummy*, int, const QString&)),
                SLOT(slotBeginInsertDummy(KisNodeDummy*, int, QString)));
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

void KisNodeModel::slotBeginInsertDummy(KisNodeDummy *parent, int index, const QString &metaObjectType)
{
    int row = 0;
    QModelIndex parentIndex;

    bool willAdd =
        m_d->indexConverter->indexFromAddedDummy(parent, index,
                                                 metaObjectType,
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
    // FIXME: is it really what we want?
    m_d->updateTimer->stop();
    m_d->updateQueue.clear();

    KisNodeDummy *parentDummy = dummy->parent();

    QModelIndex parentIndex;
    if(parentDummy) {
        parentIndex = m_d->indexConverter->indexFromDummy(parentDummy);
    }

    QModelIndex itemIndex = m_d->indexConverter->indexFromDummy(dummy);

    if(itemIndex.isValid()) {
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
    m_d->updateTimer->start(1000);
}

void KisNodeModel::processUpdateQueue()
{
    foreach(KisNodeDummy *dummy, m_d->updateQueue) {
        QModelIndex index = m_d->indexConverter->indexFromDummy(dummy);
        emit dataChanged(index, index);
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

    return itemIndex;
}

int KisNodeModel::rowCount(const QModelIndex &parent) const
{
    if(!m_d->dummiesFacade) return 0;
    return m_d->indexConverter->rowCount(parent);
}

int KisNodeModel::columnCount(const QModelIndex&) const
{
    return 1;
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
    KisMimeData* data = new KisMimeData(node);

    return data;
}

bool KisNodeModel::correctNewNodeLocation(KisNodeSP node,
                                          KisNodeDummy* &parentDummy,
                                          KisNodeDummy* &aboveThisDummy)
{
    KisNodeSP parentNode = parentDummy->node();
    bool result = true;

    if(!parentDummy->node()->allowAsChild(node)) {
        aboveThisDummy = parentDummy;
        parentDummy = parentDummy->parent();

        result = (!parentDummy) ? false :
            correctNewNodeLocation(node, parentDummy, aboveThisDummy);
    }

    return result;
}

bool KisNodeModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent)
{
    Q_UNUSED(column);

    const KisMimeData *mimedata = qobject_cast<const KisMimeData*>(data);
    KisNodeSP node = mimedata ? mimedata->node() : 0;

    if (!node) return false;

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

    if (!correctNewNodeLocation(node, parentDummy, aboveThisDummy)) {
        return false;
    }

    Q_ASSERT(parentDummy);
    KisNodeSP aboveThisNode = aboveThisDummy ? aboveThisDummy->node() : 0;


    bool result = true;

    if (action == Qt::CopyAction) {
        emit requestAddNode(node->clone(), parentDummy->node(), aboveThisNode);
    }
    else if (action == Qt::MoveAction) {
        emit requestMoveNode(node, parentDummy->node(), aboveThisNode);
    }
    else {
        result = false;
    }

    return result;
}

#include "kis_node_model.moc"
