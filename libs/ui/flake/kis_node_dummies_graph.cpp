/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_node_dummies_graph.h"
#include "kis_node_shape.h"
#include "kis_selection_mask.h"


/********************************************************************/
/*                 KisNodeDummy                                     */
/********************************************************************/

KisNodeDummy::KisNodeDummy(KisNodeShape *nodeShape, KisNodeSP node)
    : m_nodeShape(nodeShape),
      m_node(node)
{
}

KisNodeDummy::~KisNodeDummy()
{
    qDeleteAll(m_children);
}

KisNodeDummy* KisNodeDummy::firstChild() const
{
    return !m_children.isEmpty() ? m_children.first() : 0;
}

KisNodeDummy* KisNodeDummy::lastChild() const
{
    return !m_children.isEmpty() ? m_children.last() : 0;
}

KisNodeDummy* KisNodeDummy::nextSibling() const
{
    if(!parent()) return 0;

    int index = parent()->m_children.indexOf(const_cast<KisNodeDummy*>(this));
    Q_ASSERT(index >= 0);

    index++;
    return index < parent()->m_children.size() ?
        parent()->m_children[index] : 0;
}

KisNodeDummy* KisNodeDummy::prevSibling() const
{
    if(!parent()) return 0;

    int index = parent()->m_children.indexOf(const_cast<KisNodeDummy*>(this));
    Q_ASSERT(index >= 0);

    index--;
    return index >= 0 ? parent()->m_children[index] : 0;
}

KisNodeDummy* KisNodeDummy::parent() const
{
    return qobject_cast<KisNodeDummy*>(QObject::parent());
}

KisNodeShape* KisNodeDummy::nodeShape() const
{
    return m_nodeShape;
}

KisNodeSP KisNodeDummy::node() const
{
    return m_node;
}

bool KisNodeDummy::isGUIVisible(bool showGlobalSelection) const
{
    if (!showGlobalSelection &&
        parent() && !parent()->parent() &&
        dynamic_cast<const KisSelectionMask*>(m_node.data())) {

        return false;
    }

    return parent() && !m_node->isFakeNode();
}

KisNodeDummy* KisNodeDummy::at(int index) const
{
    return m_children.at(index);
}

int KisNodeDummy::childCount() const
{
    return m_children.size();
}

int KisNodeDummy::indexOf(KisNodeDummy *child) const
{
    return m_children.indexOf(child);
}

/********************************************************************/
/*                 KisNodeDummiesGraph                              */
/********************************************************************/

KisNodeDummiesGraph::KisNodeDummiesGraph()
    : m_rootDummy(0)
{
}

KisNodeDummy* KisNodeDummiesGraph::rootDummy() const
{
    return m_rootDummy;
}

void KisNodeDummiesGraph::addNode(KisNodeDummy *node, KisNodeDummy *parent, KisNodeDummy *aboveThis)
{
    Q_ASSERT(!containsNode(node->node()));

    node->setParent(parent);

    Q_ASSERT_X(parent || !m_rootDummy, "KisNodeDummiesGraph::addNode", "Trying to add second root dummy");
    Q_ASSERT_X(!parent || m_rootDummy, "KisNodeDummiesGraph::addNode", "Trying to add non-orphan child with no root dummy set");

    if(!parent) {
        m_rootDummy = node;
    }
    else {
        int insertionIndex = parent->m_children.size();

        insertionIndex = aboveThis ?
            parent->m_children.indexOf(aboveThis) + 1: 0;

        Q_ASSERT(!aboveThis || parent->m_children.indexOf(aboveThis) >= 0);

        parent->m_children.insert(insertionIndex, node);
    }

    m_dummiesMap[node->node()] = node;
}

void KisNodeDummiesGraph::removeNode(KisNodeDummy *node)
{
    Q_ASSERT(containsNode(node->node()));
    unmapDummyRecursively(node);

    KisNodeDummy *parent = node->parent();
    Q_ASSERT_X(m_rootDummy, "KisNodeDummiesGraph::removeNode", "Trying to remove a dummy with no root dummy");

    if(!parent) {
        m_rootDummy = 0;
    }
    else {
        parent->m_children.removeOne(node);
    }
}

void KisNodeDummiesGraph::unmapDummyRecursively(KisNodeDummy *dummy)
{
    m_dummiesMap.remove(dummy->node());

    KisNodeDummy *child = dummy->firstChild();
    while(child) {
        unmapDummyRecursively(child);
        child = child->nextSibling();
    }
}


void KisNodeDummiesGraph::moveNode(KisNodeDummy *node, KisNodeDummy *parent, KisNodeDummy *aboveThis)
{
    removeNode(node);
    addNode(node, parent, aboveThis);
}

bool KisNodeDummiesGraph::containsNode(KisNodeSP node) const
{
    return m_dummiesMap.contains(node);
}

int KisNodeDummiesGraph::dummiesCount() const
{
    return m_dummiesMap.size();
}

KisNodeDummy* KisNodeDummiesGraph::nodeToDummy(KisNodeSP node)
{
    if (!m_dummiesMap.contains(node)) {
        return 0;
    }

    return m_dummiesMap[node];
}
