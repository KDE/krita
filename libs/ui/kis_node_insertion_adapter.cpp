/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_node_insertion_adapter.h"

struct KisNodeInsertionAdapter::Private
{
    KisNodeManager *nodeManager;
};

KisNodeInsertionAdapter::KisNodeInsertionAdapter(KisNodeManager *nodeManager)
    : m_d(new Private)
{
    m_d->nodeManager = nodeManager;
}

KisNodeInsertionAdapter::~KisNodeInsertionAdapter()
{
}

void KisNodeInsertionAdapter::moveNodes(KisNodeList nodes, KisNodeSP parent, KisNodeSP aboveThis)
{
    m_d->nodeManager->moveNodesDirect(nodes, parent, aboveThis);
}

void KisNodeInsertionAdapter::copyNodes(KisNodeList nodes, KisNodeSP parent, KisNodeSP aboveThis)
{
    m_d->nodeManager->copyNodesDirect(nodes, parent, aboveThis);
}

void KisNodeInsertionAdapter::addNodes(KisNodeList nodes, KisNodeSP parent, KisNodeSP aboveThis)
{
    m_d->nodeManager->addNodesDirect(nodes, parent, aboveThis);
}
