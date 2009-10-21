/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef _KIS_NODE_H
#define _KIS_NODE_H

#include "kis_types.h"
#include "kis_base_node.h"
#include "krita_export.h"

class QStringList;
class KoProperties;

class KisNodeVisitor;
class KisNodeGraphListener;
class KisNodeProgressProxy;

/**
 * A KisNode is a KisBaseNode that knows about its direct peers, parent
 * and children and whether it can have children.
 *
 * NOTE: your subclasses must have the Q_OBJECT declaration, even if
 * you do not define new signals or slots.
 */
class KRITAIMAGE_EXPORT KisNode : public KisBaseNode
{
    friend class KisFilterMaskTest;
    Q_OBJECT

public:

    /**
     * Create an empty node without a parent.
     */
    KisNode();

    /**
     * Create a copy of this node. The copy will not have a parent
     * node.
     */
    KisNode(const KisNode & rhs);

    /**
     * Delete this node
     */
    virtual ~KisNode();

    virtual KisNodeSP clone() const = 0;

    virtual bool accept(KisNodeVisitor &v);

    /**
     * Re-implement this method to add constraints for the node
     * subclasses that can be added as to this subclass of KisNode.
     *
     * @return false if the given node is not allowed as a subclass to
     * this node
     */
    virtual bool allowAsChild(KisNodeSP) const = 0;

    /**
     * Set the entire node extent dirty; this percolates up to parent
     * nodes all the way to the root node. By default this is the
     * empty rect (through KisBaseNode::extent())
     */
    virtual void setDirty() {}

    /**
     * Add the given rect to the set of dirty rects for this node;
     * this percolates up to parent nodes all the way to the root
     * node.
     */
    virtual void setDirty(const QRect & rect) {
        Q_UNUSED(rect);
    }

    /**
     * Add the given region to the set of dirty rects for this node;
     * this percolates up to parent nodes all the way to the root
     * node, if propagate is true;
     */
    virtual void setDirty(const QRegion & region) {
        Q_UNUSED(region);
    }

    /**
     * Some filters will cause a change of pixels those are outside
     * a requested rect. E.g. we change a rect of 2x2, then we want to
     * apply a convolution filter with kernel 4x4 (changeRect is
     * (2+2*3)x(2+2*3)=8x8) to that area. The rect that should be updated
     * on the layer will be exaclty 8x8. More than that the needRect for
     * that update will be 14x14. See \ref needeRect.
     */
    virtual QRect changeRect(const QRect &rect) const;

    /**
     * Some filters need pixels outside the current processing rect to
     * compute the new value (for instance, convolution filters)
     * See \ref changeRect
     */
    virtual QRect needRect(const QRect &rect) const;

public: // Graph methods

    /**
     * @return the graph listener this node belongs to. 0 if the node
     * does not belong to a grap listener.
     */
    KisNodeGraphListener * graphListener() const;

    /**
     * Set the graph listener for this node. The graphlistener will be
     * informed before and after the list of child nodes has changed.
     */
    void setGraphListener(KisNodeGraphListener * graphListener);

    /**
     * Returns the parent node of this node. This is 0 only for a root
     * node; otherwise this will be an actual Node
     */
    KisNodeSP parent() const;

    /**
     * Returns the first child node of this node, or 0 if there are no
     * child nodes.
     */
    KisNodeSP firstChild() const;

    /**
     * Returns the last child node of this node, or 0 if there are no
     * child nodes.
     */
    KisNodeSP lastChild() const;

    /**
     * Returns the previous sibling of this node in the parent's list.
     * This is the node *above* this node in the composition stack. 0
     * is returned if this child has no more previous siblings (==
     * firstChild())
     */
    KisNodeSP prevSibling() const;

    /**
     * Returns the next sibling of this node in the parent's list.
     * This is the node *below* this node in the composition stack. 0
     * is returned if this child has no more next siblings (==
     * lastChild())
     */
    KisNodeSP nextSibling() const;

    /**
     * Returns how many direct child nodes this node has (not
     * recursive).
     */
    quint32 childCount() const;

    /**
     * Retrieve the child node at the specified index.
     *
     * @return 0 if there is no node at this index.
     */
    KisNodeSP at(quint32 index) const;

    /**
     * Retrieve the index of the specified child node.
     *
     * @return -1 if the specified node is not a child node of this
     * node.
     */
    int index(const KisNodeSP node) const;


    /**
     * Return a list of child nodes of the current node that conform
     * to the specified constraints. There are no guarantees about the
     * order of the nodes in the list. The function is not recursive.
     *
     * @param nodeTypes. if not empty, only nodes that inherit the
     * classnames in this stringlist will be returned.
     * @param properties. if not empty, only nodes for which
     * KisNodeBase::check(properties) returns true will be returned.
     */
    QList<KisNodeSP> childNodes(const QStringList & nodeTypes, const KoProperties & properties) const;

public:

    /**
     * @return the node progress proxy used by this node, if this node has no progress
     *         proxy, it will return the proxy of its parent, if the parent has no progress proxy
     *         it will return 0
     */
    KisNodeProgressProxy* nodeProgressProxy() const;

private:

    /**
     * Create a node progress proxy for this node. You need to create a progress proxy only
     * if the node is going to appear in the layerbox, and it needs to be created before
     * the layer box is made aware of the proxy.
     */
    void createNodeProgressProxy();

protected:

    /**
     * Re-implement this method if your node type has to do something
     * before it is removed.
     */
    virtual void prepareForRemoval() {}

    /**
     * Re-implement this method if your node type has to do something
     * before being added to the stack.
     */
    virtual void prepareForAddition() {}

    /**
     * Re-implement this method if your node type has to do something
     * right after being added to the stack.
     */
    virtual void initAfterAddition() {}


private:

    friend class KisNodeFacade;
    friend class KisNodeTest;
    friend class KisLayer; // Note: only for setting the preview mask!
    /**
     * Set the parent of this node.
     */
    void setParent(KisNodeSP parent);

    /**
     * Add the specified node above the specified node. If aboveThis
     * is 0, the node is added at the bottom.
     */
    bool add(KisNodeSP newNode, KisNodeSP aboveThis);

    /**
     * Removes the node at the specified index from the child nodes.
     *
     * @return false if there is no node at this index
     */
    bool remove(quint32 index);

    /**
     * Removes the node from the child nodes.
     *
     * @return false if there's no such node in this node.
     */
    bool remove(KisNodeSP node);


private:

    class Private;
    Private * const m_d;

};

Q_DECLARE_METATYPE(KisNodeSP)

#endif
