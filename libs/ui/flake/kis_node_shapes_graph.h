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

#ifndef __KIS_NODE_SHAPES_GRAPH_H
#define __KIS_NODE_SHAPES_GRAPH_H


#include "kis_node.h"
#include "kis_types.h"
#include "kis_node_dummies_graph.h"

class KisNodeShape;
class KisNodeDummy;


/**
 * KisNodeShapesGraph is a highlevel simplified representation
 * of nodes in the node stack. It stores hierarchy information
 * about graph without accessing real node graph.
 *
 * Takes ownership on both dummies and node shapes.
 *
 * \see KisNodeDummy, KisNodeDummyGraph
 */

class KRITAUI_EXPORT KisNodeShapesGraph
{
public:
    /**
     * Adds a node to the graph
     *
     * WARNING: this method does *not* add all the children recursively
     * because the shapes graph has no right to access the hierarchy
     * information from the UI thread, so you should do it manually.
     */
    KisNodeShape* addNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis);

    /**
     * Moves a node inside the graph. It is supposed that the node has
     * previously been added to the graph using addNode().
     *
     * The node is moved together with all its children.
     */
    void moveNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis);

    /**
     * Removes the node from the graph
     *
     * Removing the node from the graph removes both the node itself
     * and all its children, because all this information is accessible
     * to the shapes graph without referring to the node's hierarchy
     * information owned by the image.
     */
    void removeNode(KisNodeSP node);

    KisNodeShape* nodeToShape(KisNodeSP node);
    KisNodeDummy* nodeToDummy(KisNodeSP node);
    KisNodeDummy* rootDummy() const;

    bool containsNode(KisNodeSP node) const;
    int shapesCount() const;

private:
    void unmapDummyRecursively(KisNodeDummy *dummy);

private:
    KisNodeDummiesGraph m_dummiesGraph;
};

#endif /* __KIS_NODE_SHAPES_GRAPH_H */
