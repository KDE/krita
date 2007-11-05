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
#include <KoProperties.h>

class KisNodeVisitor;
class KisNodeGraphListener;

/**
 * A KisNode is a KisBaseNode that knows about its direct peers, parent
 * and children and whether it can have children.
 *
 * NOTE: your subclasses must have the Q_OBJECT declaration, even if
 * you do not define new signals or slots.
 */
class KRITAIMAGE_EXPORT KisNode : public KisBaseNode {

    Q_OBJECT

public:

    /**
     * Create an empty node without a parent.
     *
     * @param the image that owns this node. The image will be
     * informed before and after changes in this node's node list.
     */
    KisNode();

    /**
     * Create a copy of this node. The copy will not have a parent
     * node.
     */
    KisNode( const KisNode & rhs );

    /**
     * Delete this node
     */
    virtual ~KisNode();

    virtual KisNodeSP clone() = 0;

    virtual bool accept(KisNodeVisitor &v);

protected:

    /**
     * Re-implement this method to add constraints for the node
     * subclasses that can be added as to this subclass of KisNode.
     *
     * @return false if the given node is not allowed as a subclass to
     * this node
     */
    virtual bool allowAsChild( KisNodeSP ) = 0;

public: // dirty region methods. XXX: Make these slots?

    /**
     * Set the entire node extent dirty; this percolates up to parent
     * nodes all the way to the root node. By default this is the
     * empty rect (through KisBaseNode::extent())
     */
    virtual void setDirty();

    /**
     * Add the given rect to the set of dirty rects for this node;
     * this percolates up to parent nodes all the way to the root
     * node.
     */
    virtual void setDirty(const QRect & rect);

    /**
     * Add the given region to the set of dirty rects for this node;
     * this percolates up to parent nodes all the way to the root
     * node, if propagate is true;
     */
    virtual void setDirty( const QRegion & region);

public:

    /**
     * @return true if any part of this layer has been marked dirty
     */
    bool isDirty();

    /**
     *  @return true if the given rect overlaps with the dirty region
     *  of this node
     */
    bool isDirty( const QRect & rect );

    /**
     * Mark the specified area as clean
     */
    void setClean( QRect rc );

    /**
     * Mark the whole layer as clean
     */
    void setClean();

    /**
     * @return the region from the given rect that is dirty.
     */
    QRegion dirtyRegion( const QRect & rc );

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
    void setGraphListener( KisNodeGraphListener * graphListener );

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
    KisNodeSP at( quint32 index ) const;

    /**
     * Retrieve the index of the specified child node.
     *
     * @return -1 if the specified node is not a child node of this
     * node.
     */
    int index( const KisNodeSP node ) const;


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
    QList<KisNodeSP> childNodes( QStringList nodeTypes, const KoProperties & properties ) const;


protected:

    /**
     * Re-implement this method if your node type has to do something
     * before it is removed.
     */
    virtual void prepareForRemoval() {};

    /**
     * Re-implement this method if your node type has to do something
     * before being added to the stack.
     */
    virtual void prepareForAddition() {};

    /**
     * Re-implement this method if your node type has to do something
     * right after being added to the stack.
     */
    virtual void initAfterAddition() {};


private:

    friend class KisNodeFacade;
    friend class KisNodeTest;
    friend class KisLayer; // Note: only for setting the preview mask!
    /**
     * Set the parent of this node.
     */
    void setParent( KisNodeSP parent );

    /**
     * Add the specified node above the specified node. If aboveThis
     * is 0, the node is added at the bottom.
     */
    bool add( KisNodeSP newNode, KisNodeSP aboveThis );

    /**
     * Removes the node at the specified index from the child nodes.
     *
     * @return false if there is no node at this index
     */
    bool remove( quint32 index );

    /**
     * Removes the node from the child nodes.
     *
     * @return false if there's no such node in this node.
     */
    bool remove( KisNodeSP node );


private:

    class Private;
    Private * const m_d;

};

#endif
