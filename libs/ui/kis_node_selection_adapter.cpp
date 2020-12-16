/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_node_selection_adapter.h"

#include "kis_node_manager.h"
#include "kis_node.h"

struct KisNodeSelectionAdapter::Private
{
    KisNodeManager *nodeManager;
};

KisNodeSelectionAdapter::KisNodeSelectionAdapter(KisNodeManager *nodeManager)
    : m_d(new Private)
{
    m_d->nodeManager = nodeManager;
}

KisNodeSelectionAdapter::~KisNodeSelectionAdapter()
{
}

KisNodeSP KisNodeSelectionAdapter::activeNode() const
{
    return m_d->nodeManager->activeNode();
}

void KisNodeSelectionAdapter::setActiveNode(KisNodeSP node)
{
    m_d->nodeManager->slotUiActivatedNode(node);
}
