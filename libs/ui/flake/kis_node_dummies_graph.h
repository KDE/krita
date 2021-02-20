/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_NODE_DUMMIES_GRAPH_H
#define __KIS_NODE_DUMMIES_GRAPH_H

#include <QList>
#include <QMap>

#include "kritaui_export.h"
#include "kis_types.h"
#include "kis_node.h"

class KisNodeShape;

/**
 * KisNodeDummy is a simplified representation of a node
 * in the node stack. It stores all the hierarchy information
 * about the node, so you needn't access from the node
 * directly (actually, you cannot do it usually, because UI
 * works in a different thread and race conditions are possible).
 *
 * The dummy stores a KisNodeShape which can store a pointer to
 * to node. You can access to the node data through it, but take
 * care -- not all the information is accessible in
 * multithreading environment.
 *
 * The ownership on the KisNodeShape is taken by the dummy.
 * The ownership on the children of the dummy is taken as well.
 */

class KRITAUI_EXPORT KisNodeDummy : public QObject
{
    Q_OBJECT

public:
    /**
     * Take care that KisNodeDummy does not take ownership over
     * the \p nodeShape since the handling of the removal of the
     * children of the shape is done by flake. So please handle it
     * manually.
     *
     * The children dummies of the dummy are still owned by the
     * dummy and are deleted automatically.
     */
    KisNodeDummy(KisNodeShape *nodeShape, KisNodeSP node);
    ~KisNodeDummy() override;

    KisNodeDummy* firstChild() const;
    KisNodeDummy* lastChild() const;
    KisNodeDummy* nextSibling() const;
    KisNodeDummy* prevSibling() const;
    KisNodeDummy* parent() const;

    KisNodeDummy* at(int index) const;
    int childCount() const;
    int indexOf(KisNodeDummy *child) const;

    KisNodeSP node() const;

    bool isGUIVisible(bool showGlobalSelection) const;

private:
    friend class KisNodeShapesGraph; // for ::nodeShape() method
    friend class KisNodeShapesGraphTest;
    KisNodeShape* nodeShape() const;

    friend class KisNodeDummiesGraph;
    QList<KisNodeDummy*> m_children;

    KisNodeShape *m_nodeShape;
    KisNodeSP m_node;
};

/**
 * KisNodeDummiesGraph manages the hierarchy of dummy objects
 * representing nodes in the UI environment.
 */

class KRITAUI_EXPORT KisNodeDummiesGraph
{
public:
    KisNodeDummiesGraph();

    KisNodeDummy* rootDummy() const;

    KisNodeDummy* nodeToDummy(KisNodeSP node);
    bool containsNode(KisNodeSP node) const;
    int dummiesCount() const;

    /**
     * Adds a dummy \p node to the position specified
     * by \p parent and \p aboveThis.
     *
     * It is not expected that you would add a dummy twice.
     */
    void addNode(KisNodeDummy *node, KisNodeDummy *parent, KisNodeDummy *aboveThis);

    /**
     * Moves a dummy \p node from its current position to
     * the position specified by \p parent and \p aboveThis.
     *
     * It is expected that the dummy \p node has been added
     * to the graph with addNode() before calling this function.
     */
    void moveNode(KisNodeDummy *node, KisNodeDummy *parent, KisNodeDummy *aboveThis);

    /**
     * Removes the dummy \p node from the graph.
     *
     * WARNING: The dummy is only "unlinked" from the graph. Neither
     * deletion of the node nor deletion of its children happens.
     * The dummy keeps maintaining its children so after unlinking
     * it from the graph you can just type to free memory recursively:
     * \code
     * graph.removeNode(node);
     * delete node;
     * \endcode
     */
    void removeNode(KisNodeDummy *node);

private:
    void unmapDummyRecursively(KisNodeDummy *dummy);

private:
    typedef QMap<KisNodeSP, KisNodeDummy*> NodeMap;

private:
    KisNodeDummy *m_rootDummy;
    NodeMap m_dummiesMap;
};

#endif /* __KIS_NODE_DUMMIES_GRAPH_H */
