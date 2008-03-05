/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_top_down_update_strategy.h"
#include "kis_node.h"
#include "kis_paint_device.h"
#include "kis_projection.h"

class KisTopDownUpdateStrategy::Private
{
public:

    KisNodeSP node;
    KisNodeSP filthyNode;
};

KisTopDownUpdateStrategy::KisTopDownUpdateStrategy( KisNodeSP node )
    : m_d( new Private)
{
    m_d->node = node;

    // XXX: how about nesting masks? Then masks need to get a projection, too.
    if (node->inherits("KisLayer")) {
    }
}


KisTopDownUpdateStrategy::~KisTopDownUpdateStrategy()
{
    delete m_d;
}


void KisTopDownUpdateStrategy::setDirty( const QRect & rc, const KisNodeSP filthyNode )
{
   /*
        if the filthyNode is the node
            set the parent dirty
        if the filtyNode is not the node
            schedule a projection update for the current node
                if the projection update is done, set the parent dirty
                    until the root is done
    */ 
    if (m_d->node->inherits("KisGroupLayer")) {
        m_d->filthyNode = filthyNode;
    }
}
 
void KisTopDownUpdateStrategy::setImage(KisImageSP image)
{
}
     
void KisTopDownUpdateStrategy::lock()
{
    /*
       lock is called on the root layer's updateStrategy.
       this strategy locks recursively all children. Lock
       has two meanings: the update process is stopped until
       unlock is called and the nodes are set locked.
     */
    // XXX: huh? how come m_d->node is const here?
    const_cast<KisNode*>(m_d->node.data())->setLocked( true );
    KisNodeSP child = m_d->node->firstChild();
    while (child) {
        static_cast<KisTopDownUpdateStrategy*>(child->updateStrategy())->lock();
        child = child->nextSibling();
    }
}

void KisTopDownUpdateStrategy::unlock()
{
    /*
      unlock is called on the root layer's updatestrategy.
      this strategy unlocks recursively all children. Unlock
      has two meanings: the update process is restarted
      and all nodes are unlocked.
     */
    KisNodeSP child = m_d->node->firstChild();
    while (child) {
        static_cast<KisTopDownUpdateStrategy*>(child->updateStrategy())->unlock();
        child = child->nextSibling();
    }
    const_cast<KisNode*>(m_d->node.data())->setLocked( false );
}

KisPaintDeviceSP KisTopDownUpdateStrategy::updateGroupLayerProjection( const QRect & rc, KisPaintDeviceSP projection )
{
    /*
        Grouplayer are special since they can contain nodes that contain a projection
        of part of the stack. 

        if filthynode is above an adjustment layer
            start recomposition from the adjustment layer
        else
            start recomposition from the bottom
     */
    m_d->filthyNode = 0;
    return projection;
}
