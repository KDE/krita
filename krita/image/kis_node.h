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
#include "kis_shared.h"
#include "krita_export.h"

class KisNodeGraphListener;

/**
 * A KisNode is the base class for all components of an image: nodes,
 * masks, selections. A node knows its direct peers, parent and
 * children and whether it can have children.
 *
 * KisNode is a temporary solution while working towards making Krita
 * completely flake based.
 *
 */
class KRITAIMAGE_EXPORT KisNode : public KisShared {

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

public:

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
     * Set the parent of this node.
     */
    void setParent( KisNodeSP parent );

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
     * Adds the node to this group at the specified index.
     * childCount() is a valid index and appends to the end.
     *
     * @return false if the node is already in this node or any other
     * node in the graph. (remove it first.)
     */
    bool add( KisNodeSP newNode, quint32 index );

    /**
     * Add the specified node above the specified node (if aboveThis
     * == 0, the bottom is used)
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
