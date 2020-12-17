/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_node_filter_proxy_model.h"

#include <QSet>
#include <boost/optional.hpp>

#include "kis_node.h"
#include "kis_node_model.h"
#include "kis_node_manager.h"
#include "kis_signal_compressor.h"
#include "kis_signal_auto_connection.h"

#include "kis_image.h"


struct KisNodeFilterProxyModel::Private
{
    Private()
        : nodeModel(0),
          activeNodeCompressor(1000, KisSignalCompressor::FIRST_INACTIVE)
    {}

    KisNodeModel *nodeModel;
    KisNodeSP pendingActiveNode;
    KisNodeSP activeNode;
    QSet<int> acceptedColorLabels;
    boost::optional<QString> activeTextFilter;
    KisSignalCompressor activeNodeCompressor;
    bool isUpdatingFilter = false;
    KisSignalAutoConnectionsStore modelConnections;

    bool checkIndexAllowedRecursively(QModelIndex srcIndex);
};

KisNodeFilterProxyModel::KisNodeFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent),
      m_d(new Private)
{
    connect(&m_d->activeNodeCompressor, SIGNAL(timeout()), SLOT(slotUpdateCurrentNodeFilter()), Qt::QueuedConnection);
}

KisNodeFilterProxyModel::~KisNodeFilterProxyModel()
{
}

void KisNodeFilterProxyModel::setNodeModel(KisNodeModel *model)
{
    m_d->modelConnections.clear();
    m_d->modelConnections.addConnection(model, SIGNAL(sigBeforeBeginRemoveRows(const QModelIndex &, int, int)),
                                        this, SLOT(slotBeforeBeginRemoveRows(const QModelIndex &, int, int)));

    m_d->nodeModel = model;
    setSourceModel(model);
}

bool KisNodeFilterProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (m_d->isUpdatingFilter && role == KisNodeModel::ActiveRole) {
        return false;
    }

    return QSortFilterProxyModel::setData(index, value, role);
}

bool KisNodeFilterProxyModel::Private::checkIndexAllowedRecursively(QModelIndex srcIndex)
{
    if (!srcIndex.isValid()) return false;

    KisNodeSP node = nodeModel->nodeFromIndex(srcIndex);
    const bool nodeTextFilterMatch = (!activeTextFilter || node->name().contains(activeTextFilter.get(), Qt::CaseInsensitive));

    // directParentTextFilterMatch -- There's an argument to be made that searching for a parent name should show
    // all of the direct children of said text-search. For now, it will remain unused.
    const bool directParentTextFilterMatch =  (!activeTextFilter || (node->parent() && node->parent()->name().contains(activeTextFilter.get(), Qt::CaseInsensitive)));
    Q_UNUSED(directParentTextFilterMatch);

    const bool nodeColorMatch = (acceptedColorLabels.count() == 0 || acceptedColorLabels.contains(node->colorLabelIndex()));
    if ( node == activeNode ||
         ( nodeColorMatch && nodeTextFilterMatch )) {
        return true;
    }

    bool result = false;

    const int numChildren = srcIndex.model()->rowCount(srcIndex);
    for (int i = 0; i < numChildren; i++) {
        QModelIndex child = nodeModel->index(i, 0, srcIndex);
        if (checkIndexAllowedRecursively(child)) {
            result = true;
            break;
        }
    }

    return result;
}

bool KisNodeFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    KIS_ASSERT_RECOVER(m_d->nodeModel) { return true; }

    const QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    if (!index.isValid()) return false;

    KisNodeSP node = m_d->nodeModel->nodeFromIndex(index);

    return !node ||
        (m_d->acceptedColorLabels.isEmpty() && !m_d->activeTextFilter) ||
        m_d->checkIndexAllowedRecursively(index);
}

KisNodeSP KisNodeFilterProxyModel::nodeFromIndex(const QModelIndex &index) const
{
    KIS_ASSERT_RECOVER(m_d->nodeModel) { return 0; }

    QModelIndex srcIndex = mapToSource(index);
    return m_d->nodeModel->nodeFromIndex(srcIndex);
}

QModelIndex KisNodeFilterProxyModel::indexFromNode(KisNodeSP node) const
{
    KIS_ASSERT_RECOVER(m_d->nodeModel) { return QModelIndex(); }

    QModelIndex srcIndex = m_d->nodeModel->indexFromNode(node);
    return mapFromSource(srcIndex);
}

void KisNodeFilterProxyModel::setAcceptedLabels(const QSet<int> &value)
{
    m_d->acceptedColorLabels = value;
    invalidateFilter();
}

void KisNodeFilterProxyModel::setTextFilter(const QString &text)
{
    m_d->activeTextFilter = !text.isEmpty() ? boost::make_optional(text) : boost::none;
    invalidateFilter();
}

void KisNodeFilterProxyModel::setActiveNode(KisNodeSP node)
{
    // NOTE: the current node might change due to beginRemoveRows, in such case
    //       we must ensure we don't trigger recursive model invalidation.

    // the new node may temporary become null when the last layer
    // of the document in removed
    m_d->pendingActiveNode = node;
    m_d->activeNodeCompressor.start();
}

void KisNodeFilterProxyModel::slotUpdateCurrentNodeFilter()
{
    m_d->activeNode = m_d->pendingActiveNode;

    /**
     * During the filter update the model might emit "current changed" signals,
     * which (in their turn) will issue setData(..., KisNodeModel::ActiveRole)
     * call, leading to a double recursion. Which, obviously, crashes Krita.
     *
     * Right now, just blocking the KisNodeModel::ActiveRole call is the
     * most obvious solution for the problem.
     */
    m_d->isUpdatingFilter = true;
    invalidateFilter();
    m_d->isUpdatingFilter = false;
}

void KisNodeFilterProxyModel::slotBeforeBeginRemoveRows(const QModelIndex &parent, int start, int end)
{
    for (int row = start; row <= end; row++) {
        const QModelIndex sourceIndex = sourceModel()->index(row, 0, parent);
        const QModelIndex mappedIndex = mapFromSource(sourceIndex);

        if (mappedIndex.isValid()) {
            emit sigBeforeBeginRemoveRows(mappedIndex.parent(), mappedIndex.row(), mappedIndex.row());
        }
    }
}

void KisNodeFilterProxyModel::unsetDummiesFacade()
{
    m_d->nodeModel->setDummiesFacade(0, 0, 0, 0, 0);
    m_d->pendingActiveNode = 0;
    m_d->activeNode = 0;
}
