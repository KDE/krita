/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _KIS_NODE_FACADE_H
#define _KIS_NODE_FACADE_H

#include <QScopedPointer>

#include "kis_types.h"
#include "kis_node.h"
#include "kritaimage_export.h"
/**
 * KisNodeFacade is the public interface to adding and removing nodes.
 */
class KRITAIMAGE_EXPORT KisNodeFacade
{

public:

    /**
     * Create a new, empty KisNodeFacade
     */
    KisNodeFacade();

    /**
     * Create a new kisnodefacade for the given root.
     */
    KisNodeFacade(KisNodeSP root);

    virtual ~KisNodeFacade();

    /**
     * Set the rootnode for this facade
     */
    void setRoot(KisNodeSP root);

    /**
     * Return the root node for the graph this facade managed
     */
    const KisNodeSP root() const;

    /**
     * Move the given node to specified position. If the node already
     * has a parent, it is removed from the parent's node list.
     */
    bool moveNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis);

    /**
     * Move the givent node at the specified index. If the node already
     * has a parent, it is removed from the parent's node list.
     *
     * childCount() is a valid index and appends to the end.
     */
    bool moveNode(KisNodeSP node, KisNodeSP parent, quint32 index);
    /**
     * Add an already existing node to the image. The node is put on top
     * of the nodes in the specified nodegroup. If parent is 0, then
     * the root is used as parent.
     *
     * @param node the node to be added
     * @param parent the parent node
     */
    bool addNode(KisNodeSP node, KisNodeSP parent = KisNodeSP());

    /**
     * Add already existing node to the graph.
     *
     * @param node the node to be added
     * @param parent the parent node
     * @param aboveThis in the list with child nodes of the specified
     *                  parent, add this node above the specified sibling.
     *                  if 0, the node is put in the lowermost position in
     *                  its group.
     * returns false if adding the node didn't work, true if the node got added
     */
    bool addNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis);

    /**
     * Adds the node as a child of the given parent at the specified
     * index.
     *
     * childCount() is a valid index and appends to the end. Fails and
     * returns false if the node is already in this group or any
     * other (remove it first.)
     */
    bool addNode(KisNodeSP node,  KisNodeSP parent, quint32 index);

    /**
     * Remove the specified node.
     *
     * @return false if removing the node failed
     */
    bool removeNode(KisNodeSP node);

    /**
     * Move node up one slot, i.e., nextSibling becomes prevSibling
     */
    bool raiseNode(KisNodeSP node);

    /**
     * Move node down one slot -- i.e, prevSibling becomes
     * nextSibling.
     *
     * @return false if moving the node failed
     */
    bool lowerNode(KisNodeSP node);

    /**
     * Move the given node to the top-most position among its
     * siblings.
     *
     * @return false if moving the node failed.
     */
    bool toTop(KisNodeSP node);

    /**
     * Move the given node to bottom-most position among its siblings.
     *
     * @return false if moving the node failed.
     */
    bool toBottom(KisNodeSP node);

private:

    struct Private;
    QScopedPointer<Private> m_d;
};
#endif
