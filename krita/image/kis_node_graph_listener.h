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
#ifndef KIS_NODE_GRAPH_LISTENER_H_
#define KIS_NODE_GRAPH_LISTENER_H_

#include "krita_export.h"
class KisNode;

/**
 * Implementations of this class are called by nodes whenever the node
 * graph changes. These implementations can then emit the right
 * signals so Qt interview models can be updated before and after
 * changes.
 *
 * The reason for this go-between is that we don't want our nodes to
 * be QObjects, nor to have sig-slot connections between every node
 * and every mode.
 */
class KRITAIMAGE_EXPORT KisNodeGraphListener
{

public:

    virtual ~KisNodeGraphListener() {}

    /**
     * Inform the model that we're going to add a node.
     */
    virtual void aboutToAddANode(KisNode *parent, int index) = 0;

    /**
     * Inform the model we're done adding a node.
     */
    virtual void nodeHasBeenAdded(KisNode *parent, int index) = 0;

    /**
     * Inform the model we're going to remove a node.
     */
    virtual void aboutToRemoveANode(KisNode *parent, int index) = 0;

    /**
     * Inform the model we're done removing a node.
     */
    virtual void nodeHasBeenRemoved(KisNode *parent, int index) = 0;

    /**
     * Inform the model we're about to start moving a node (which
     * includes removing and adding the same node)
     */
    virtual void aboutToMoveNode(KisNode * parent, int oldIndex, int newIndex) = 0;

    /**
     * Inform the model we're done moving the node: it has been
     * removed and added successfully
     */
    virtual void nodeHasBeenMoved(KisNode * parent, int oldIndex, int newIndex) = 0;
};

#endif
