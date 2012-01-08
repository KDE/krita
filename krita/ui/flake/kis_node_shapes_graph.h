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

#include <QMap>
#include "KoShapeLayer.h"

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
    KisNodeShape* addNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis);
    void moveNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis);
    void removeNode(KisNodeSP node);

    KisNodeShape* nodeToShape(KisNodeSP node);
    KisNodeDummy* nodeToDummy(KisNodeSP node);

    bool containsNode(KisNodeSP node) const;
    int shapesCount() const;

private:
    typedef QMap<KisNodeSP, KisNodeDummy*> NodeMap;

private:
    void unmapDummyRecursively(KisNodeDummy *dummy);

private:
    KisNodeDummiesGraph m_dummiesGraph;
    NodeMap m_dummiesMap;
};

#endif /* __KIS_NODE_SHAPES_GRAPH_H */
