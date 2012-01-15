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


/********************************************************************/
/*                 KisNodeDummy                                     */
/********************************************************************/

KisNodeDummy::KisNodeDummy(KisNodeShape *nodeShape)
    : m_parent(0),
      m_nodeShape(nodeShape)
{
}

KisNodeDummy::~KisNodeDummy()
{
    delete m_nodeShape;
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
    if(!m_parent) return 0;

    int index = m_parent->m_children.indexOf(const_cast<KisNodeDummy*>(this));
    Q_ASSERT(index >= 0);

    index++;
    return index < m_parent->m_children.size() ?
        m_parent->m_children[index] : 0;
}

KisNodeDummy* KisNodeDummy::prevSibling() const
{
    if(!m_parent) return 0;

    int index = m_parent->m_children.indexOf(const_cast<KisNodeDummy*>(this));
    Q_ASSERT(index >= 0);

    index--;
    return index >= 0 ? m_parent->m_children[index] : 0;
}

KisNodeDummy* KisNodeDummy::parent() const
{
    return m_parent;
}

KisNodeShape* KisNodeDummy::nodeShape() const
{
    return m_nodeShape;
}


/********************************************************************/
/*                 KisNodeDummiesGraph                              */
/********************************************************************/

void KisNodeDummiesGraph::addNode(KisNodeDummy *node, KisNodeDummy *parent, KisNodeDummy *aboveThis)
{
    node->m_parent = parent;
    if(!parent) return;

    int insertionIndex = parent->m_children.size();

    insertionIndex = aboveThis ?
        parent->m_children.indexOf(aboveThis) + 1: 0;

    Q_ASSERT(!aboveThis || parent->m_children.indexOf(aboveThis) >= 0);

    parent->m_children.insert(insertionIndex, node);
}

void KisNodeDummiesGraph::removeNode(KisNodeDummy *node)
{
    KisNodeDummy *parent = node->m_parent;
    if(!parent) return;

    parent->m_children.removeOne(node);
}

void KisNodeDummiesGraph::moveNode(KisNodeDummy *node, KisNodeDummy *parent, KisNodeDummy *aboveThis)
{
    removeNode(node);
    addNode(node, parent, aboveThis);
}
