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

#include "kis_node_shapes_graph.h"

#include "kis_node_shape.h"


KisNodeShape* KisNodeShapesGraph::addNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis)
{
    Q_ASSERT(!m_dummiesMap.contains(node));

    KisNodeDummy *parentDummy = 0;
    KisNodeDummy *aboveThisDummy = 0;

    KisNodeShape *parentShape = 0;

    if(parent) {
        parentDummy = nodeToDummy(parent);
        parentShape = parentDummy->nodeShape();
    }

    if(aboveThis) {
        aboveThisDummy = nodeToDummy(aboveThis);
    }

    KisNodeShape *newShape = new KisNodeShape(node);
    ((KoShapeLayer*)newShape)->setParent(parentShape);

    KisNodeDummy *newDummy = new KisNodeDummy(newShape);
    m_dummiesMap[node] = newDummy;

    m_dummiesGraph.addNode(newDummy, parentDummy, aboveThisDummy);
    return newShape;
}

void KisNodeShapesGraph::moveNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis)
{
    KisNodeDummy *nodeDummy = nodeToDummy(node);
    KisNodeDummy *parentDummy = parent ? nodeToDummy(parent) : 0;
    KisNodeDummy *aboveThisDummy = aboveThis ? nodeToDummy(aboveThis) : 0;

    m_dummiesGraph.moveNode(nodeDummy, parentDummy, aboveThisDummy);
}

void KisNodeShapesGraph::removeNode(KisNodeSP node)
{
    KisNodeDummy *nodeDummy = nodeToDummy(node);
    unmapDummyRecursively(nodeDummy);

    m_dummiesGraph.removeNode(nodeDummy);
    delete nodeDummy;
}

void KisNodeShapesGraph::unmapDummyRecursively(KisNodeDummy *dummy)
{
    m_dummiesMap.remove(dummy->nodeShape()->node());

    KisNodeDummy *child = dummy->firstChild();
    while(child) {
        unmapDummyRecursively(child);
        child = child->nextSibling();
    }
}

KisNodeDummy* KisNodeShapesGraph::nodeToDummy(KisNodeSP node)
{
    Q_ASSERT(m_dummiesMap.contains(node));
    return m_dummiesMap[node];
}

KisNodeShape* KisNodeShapesGraph::nodeToShape(KisNodeSP node)
{
    return nodeToDummy(node)->nodeShape();
}

bool KisNodeShapesGraph::containsNode(KisNodeSP node) const
{
    return m_dummiesMap.contains(node);
}

int KisNodeShapesGraph::shapesCount() const
{
    return m_dummiesMap.size();
}
