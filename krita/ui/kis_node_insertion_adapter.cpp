/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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
