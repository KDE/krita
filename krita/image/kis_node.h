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
#include "KoDocumentSectionModel.h"

/**
 * A KisNode is the base class for all components of an image: nodes,
 * masks, selections. A node knows its direct peers, parent and
 * children and whether it can have children. The node implementations
 * must calll the sigAboutToRemoveNode and sigAboutToAddNode signals
 * on the parent image in order to keep the various models updated.
 *
 * KisNode is a temporary solution while working towards making Krita
 * completely flake based.
 *
 */
class KRITAIMAGE_EXPORT KisNode : public KisShared {

public:

    enum { Visible = 1, Hidden = 2, Locked = 4, Unlocked = 8 };


    KisNode();
    KisNode( KisImageWSP image, const QString & name );
    KisNode( const KisNode & rhs );
    virtual ~KisNode();

    virtual QString nodeType() = 0;
    virtual bool canHaveChildren() = 0;
    virtual QIcon icon() const = 0;

    virtual KoDocumentSectionModel::PropertyList properties() const;
    virtual void setProperties( const KoDocumentSectionModel::PropertyList &properties  );

    virtual const bool visible() const { return false; }
    virtual bool locked() const { return false; }

    /**
     * Returns the index of the node in its parent's list of child
     * nodes. Indices increase from 0, which is the topmost node in
     * the list, to the bottommost.
     */
    virtual int index() const;

    /**
     * Returns the parent node of a node. This is 0 only for a root
     * node; otherwise this will be an actual Node
     */
    virtual KisNodeSP parentNode();

    virtual const KisNodeSP parentNode() const;

    /**
     * Returns the previous sibling of this node in the parent's
     * list. This is the node *above* this node. 0 is returned if
     * there is no parent, or if this child has no more previous
     * siblings (== firstChild())
     */
    virtual KisNodeSP prevSiblingNode() const;

    /**
     * Returns the next sibling of this node in the parent's list.
     * This is the node *below* this node. 0 is returned if there is
     * no parent, or if this child has no more next siblings (==
     * lastChild())
     */
    virtual KisNodeSP nextSiblingNode() const;

    /**
     * Returns how many direct child nodes this node has (not
     * recursive). The childcount can include masks.
     */
    virtual uint childCount() const;

    /**
     * Retrieve the child node at the specified index. Returns 0 if
     * there is no node at this index.
     */
    virtual KisNodeSP atNode(int index) const;

    /**
     * Returns the first child node of this node (if it supports
     * that).
     */
    virtual KisNodeSP firstChildNode() const;

    /**
     * Returns the last child node of this node (if it supports that).
     */
    virtual KisNodeSP lastChildNode() const;

    /**
     * Recursively searches this node and any child nodes for a node
     * with the specified name.
     */
    virtual KisNodeSP findNode(const QString& name) const;

    /**
     * Recursively searches this node and any child nodes for a node
     * with the specified ID.
     */
    virtual KisNodeSP findNode(int id) const;

    /**
     * Returns the total number of nodes in this node, its child
     * nodes, and their child nodes recursively, optionally ones
     * with the specified properties Visible or Locked, which you can
     * OR together.
     */
    virtual int numNodes(int type = 0) const;

    /**
     * Called whenever the settings are changed. The whole graph below
     * this node is called.
     */
    virtual void updateSettings();


    /**
     * Adds the node to this group at the specified index.
     * childCount() is a valid index and appends to the end. Fails and
     * returns false if the node is already in this group or any
     * other (remove it first.)
    */
    bool addNode(KisNodeSP newNode, int index);

    /**
     * Add the specified node above the specified node (if aboveThis
     * == 0, the bottom is used)
     */
    bool addNode(KisNodeSP newNode, KisNodeSP aboveThis);

    /**
     * Removes the node at the specified index from the group.
     */
    bool removeNode(int index);

    /**
     * Removes the node from this group. Fails if there's no such node
     * in this group.
     */
    bool removeNode(KisNodeSP node);

protected:

    bool useProjections();


private:

    void init();

    /**
     * Set the parent of this node. This method is private because a
     * node can only gain a parent through being added to the node
     * stack.
     */
    void setParent( KisNodeSP parent );

    class Private;
    Private * m_d;

};

#endif
