/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_node_facade.h"
#include "kis_node_graph_listener.h"
#include <kis_debug.h>

struct Q_DECL_HIDDEN KisNodeFacade::Private
{
public:
    KisNodeWSP root;
};

KisNodeFacade::KisNodeFacade()
        : m_d(new Private())
{
}

KisNodeFacade::KisNodeFacade(KisNodeSP root)
        : m_d(new Private())
{
    m_d->root = root;
}

KisNodeFacade::~KisNodeFacade()
{
}

void KisNodeFacade::setRoot(KisNodeSP root)
{
    m_d->root = root;
}

const KisNodeSP KisNodeFacade::root() const
{
    return m_d->root;
}

bool KisNodeFacade::moveNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis)
{
    dbgImage << "moveNode " << node << " " << parent << " " << aboveThis;
    if (!node) {
        dbgImage << "cannot move null node"; return false;
    }
    if (!parent)  {
        dbgImage << "cannot move to null parent"; return false;
    }
    if (node == parent)  {
        dbgImage << "cannot move self inside self"; return false;
    }
    if (node == aboveThis)  {
        dbgImage << "cannot move self above self"; return false;
    }
    if (parent == aboveThis)  {
        dbgImage << "cannot move above parent"; return false;
    }
    if (!node->parent())  {
        dbgImage << "node does not have a parent"; return false;
    }

    if (aboveThis && aboveThis->parent() != parent)  {
        dbgImage << "above this parent is not the parent"; return false;
    }

    int newIndex = aboveThis ? parent->index(aboveThis) + 1 : 0;
    return moveNode(node, parent, newIndex);
}

bool KisNodeFacade::moveNode(KisNodeSP node, KisNodeSP parent, quint32 newIndex)
{
    dbgImage << "moveNode " << node << " " << parent << " " << newIndex;
    int oldIndex = node->parent()->index(node);

    if (node->graphListener())
        node->graphListener()->aboutToMoveNode(node.data(), oldIndex, newIndex);
    KisNodeSP aboveThis = parent->at(newIndex - 1);
    if (aboveThis == node) return false;
    if (node->parent()) {
        if (!node->parent()->remove(node)) return false;
    }
    dbgImage << "moving node to " << newIndex;
    bool success = addNode(node, parent, aboveThis);
    if (node->graphListener())
        node->graphListener()->nodeHasBeenMoved(node.data(), oldIndex, newIndex);
    return success;
}


bool KisNodeFacade::addNode(KisNodeSP node, KisNodeSP parent, KisNodeAdditionFlags flags)
{
    dbgImage << "Add node " << node << " to " << parent;
    if (!node) return false;
    if (!parent && !m_d->root) return false;

    if (parent)
        return parent->add(node, parent->lastChild(), flags);
    else
        return m_d->root->add(node, m_d->root->lastChild(), flags);
}

bool KisNodeFacade::addNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis, KisNodeAdditionFlags flags)
{
    if (!node) return false;
    if (!parent) return false;

    return parent->add(node, aboveThis, flags);
}

bool KisNodeFacade::addNode(KisNodeSP node,  KisNodeSP parent, quint32 index, KisNodeAdditionFlags flags)
{
    if (!node) return false;
    if (!parent) return false;

    if (index == parent->childCount())
        return parent->add(node, parent->lastChild(), flags);
    else if (index != 0)
        return parent->add(node, parent->at(index), flags);
    else
        return parent->add(node, 0, flags);
}

bool KisNodeFacade::removeNode(KisNodeSP node)
{
    if (!node) return false;
    if (!node->parent()) return false;

    return node->parent()->remove(node);

}

