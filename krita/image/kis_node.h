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
class KRITAIMAGE_EXPORT KisNode {

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
    virtual KisNode* parentNode();

    virtual const KisNode * parentNode() const;

    /**
     * Returns the previous sibling of this node in the parent's
     * list. This is the node *above* this node. 0 is returned if
     * there is no parent, or if this child has no more previous
     * siblings (== firstChild())
     */
    virtual KisNode * prevSiblingNode() const;

    /**
     * Returns the next sibling of this node in the parent's list.
     * This is the node *below* this node. 0 is returned if there is
     * no parent, or if this child has no more next siblings (==
     * lastChild())
     */
    virtual KisNode * nextSiblingNode() const;

    /// Returns how many direct child nodes this node has (not
    /// recursive). The childcount can include masks.
    virtual uint childCount() const { return 0; }

    virtual KisNode * atNode(int /*index*/) const { return 0; }

    /// Returns the first child node of this node (if it supports that).
    virtual KisNode * firstChildNode() const { return 0; }

    /// Returns the last child node of this node (if it supports that).
    virtual KisNode * lastChildNode() const { return 0; }

    /// Recursively searches this node and any child nodes for a node with the specified name.
    virtual KisNode * findNode(const QString& name) const;

    /// Recursively searches this node and any child nodes for a node with the specified ID.
    virtual KisNode * findNode(int id) const;

    /// Returns the total number of nodes in this node, its child
    /// nodes, and their child nodes recursively, optionally ones
    /// with the specified properties Visible or Locked, which you can
    /// OR together.
    virtual int numNodes(int type = 0) const;

    /**
     * Called whenever the settings are changed.
     */
    virtual void updateSettings();
};

#endif
