/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_node_filter_proxy_model.h"

#include <QSet>
#include "kis_node.h"
#include "kis_node_model.h"
#include "kis_node_manager.h"
#include "kis_signal_compressor.h"

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
    QSet<int> acceptedLabels;
    KisSignalCompressor activeNodeCompressor;
    bool isUpdatingFilter = false;

    bool checkIndexAllowedRecursively(QModelIndex srcIndex);
};

KisNodeFilterProxyModel::KisNodeFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent),
      m_d(new Private)
{
    connect(&m_d->activeNodeCompressor, SIGNAL(timeout()), SLOT(slotUpdateCurrentNodeFilter()));
}

KisNodeFilterProxyModel::~KisNodeFilterProxyModel()
{
}

void KisNodeFilterProxyModel::setNodeModel(KisNodeModel *model)
{
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
    if (node == activeNode ||
        acceptedLabels.contains(node->colorLabelIndex())) {

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
        m_d->acceptedLabels.isEmpty() ||
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

void KisNodeFilterProxyModel::setAcceptedLabels(const QList<int> &value)
{
    m_d->acceptedLabels = QSet<int>::fromList(value);
    invalidateFilter();
}

void KisNodeFilterProxyModel::setActiveNode(KisNodeSP node)
{
    // the new node may temporary become null when the last layer
    // of the document in removed
    m_d->pendingActiveNode = node;

    if (node && indexFromNode(node).isValid()) {
        m_d->activeNodeCompressor.start();
    } else {
        slotUpdateCurrentNodeFilter();
    }
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

void KisNodeFilterProxyModel::unsetDummiesFacade()
{
    m_d->nodeModel->setDummiesFacade(0, 0, 0, 0, 0);
    m_d->pendingActiveNode = 0;
    m_d->activeNode = 0;
}
