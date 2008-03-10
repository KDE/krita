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

#include <KoCompositeOp.h>

#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_node_visitor.h"
#include "kis_painter.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_external_layer_iface.h"
#include "kis_paint_layer.h"
#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "kis_selection.h"
#include "kis_transaction.h"
#include "kis_iterators_pixel.h"
#include "kis_clone_layer.h"
#include "kis_processing_information.h"
#include "kis_node.h"
#include "kis_image.h"
#include "kis_projection.h"
#include "kis_layer.h"
#include "kis_group_layer.h"

namespace {
/**
 * The class merge visitor works using a bottom-up recomposition strategy. It does
 * not need to update the projection of the nodes, because this strategy updates
 * all projections before starting to merge.
 */
class KisMergeVisitor : public KisNodeVisitor {

public:

    using KisNodeVisitor::visit;

    /**
     * Don't even _think_ of creating a merge visitor without a projection; without a projection,
     * the adjustmentlayers won't work.
     */
    KisMergeVisitor(KisPaintDeviceSP projection, const QRect& rc) :
        KisNodeVisitor()
        {
            Q_ASSERT(projection);
            m_projection = projection;
            m_rc = rc;
        }

public:

    bool visit( KisExternalLayer * layer )
        {
            if (m_projection.isNull()) {
                return false;
            }
            if (!layer->visible())
                return true;

            KisPaintDeviceSP dev = layer->projection();
            if (!dev)
                return true;

            QRect rc = dev->extent() & m_rc;

            KisPainter gc(m_projection);
            gc.setChannelFlags( layer->channelFlags() );
            gc.bitBlt(rc.left(), rc.top(), layer->compositeOp() , dev, layer->opacity(), rc.left(), rc.top(), rc.width(), rc.height());

            return true;
        }

    bool visit(KisPaintLayer *layer)
        {

            if (m_projection.isNull()) {
                return false;
            }

            if (!layer->visible())
                return true;

            bool first = false;
            if (layer->prevSibling() == 0 && layer->parent() == layer->image()->root())
                first = true;

            QRect rc = layer->paintDevice()->extent() & m_rc;

            // Indirect painting?
            KisPaintDeviceSP tempTarget = layer->temporaryTarget();
            if (tempTarget) {
                rc = (layer->projection()->extent() | tempTarget->extent()) & m_rc;
            }

            KisPainter gc(m_projection);
            gc.setChannelFlags( layer->channelFlags() );

            KisPaintDeviceSP source = layer->projection();

            if (tempTarget) {
                KisPaintDeviceSP temp = new KisPaintDevice(source->colorSpace());
                source = paintIndirect(source, temp, layer, rc.left(), rc.top(), rc.left(), rc.top(), rc.width(), rc.height());
            }

            if (first)
                gc.bitBlt(rc.left(), rc.top(), m_projection->colorSpace()->compositeOp(COMPOSITE_COPY), source, layer->opacity(), rc.left(), rc.top(), rc.width(), rc.height());
            else
                gc.bitBlt(rc.left(), rc.top(), layer->compositeOp(), source, layer->opacity(), rc.left(), rc.top(),rc.width(), rc.height());

            return true;
        }

    bool visit(KisGroupLayer *layer)
        {

            if (m_projection.isNull()) {
                return false;
            }

            if (!layer->visible())
                return true;

            KisPaintDeviceSP dev = layer->projection();

            QRect rc = dev->extent() & m_rc;


            KisPainter gc(m_projection);
            gc.setChannelFlags( layer->channelFlags() );
            gc.bitBlt(rc.left(), rc.top(), layer->compositeOp(), dev, layer->opacity(), rc.left(), rc.top(),rc.width(), rc.height());
            
            return true;
        }

    bool visit(KisAdjustmentLayer* layer)
        {
            if (m_projection.isNull()) {
                return true;
            }

            if (!layer->visible())
                return true;

            QRect tmpRc = m_rc;
            
            KisPaintDeviceSP tempTarget = layer->temporaryTarget();
            
            if (tempTarget) {
                tmpRc = (layer->extent() | tempTarget->extent()) & tmpRc;
            }

            if (tmpRc.width() == 0 || tmpRc.height() == 0) // Don't even try
                return true;


            KisFilterConfiguration * cfg = layer->filter();
            if (!cfg) return false;

            KisFilterSP f = KisFilterRegistry::instance()->value( cfg->name() );
            if (!f) return false;

            // Possibly enlarge the rect that changed (like for convolution filters)
            tmpRc = f->enlargeRect(tmpRc, cfg);

            KisSelectionSP selection = layer->selection();
            KisPaintDeviceSP layerProjection = layer->projection();

            // It's necessary to copy the unselected pixels to the projection cache inside
            // the adjustment layer to make the merge optimization in the grouplayer work.
            KisPainter gc1(layerProjection);
            gc1.setCompositeOp(layerProjection->colorSpace()->compositeOp(COMPOSITE_COPY));
            gc1.bitBlt(m_rc.topLeft(), m_projection, m_rc);
            gc1.end();
            
            KisConstProcessingInformation srcCfg(m_projection, tmpRc .topLeft(), 0);
            KisProcessingInformation dstCfg(layerProjection, tmpRc .topLeft(), 0);

            // Some filters will require usage of oldRawData, which is not available without
            // a transaction!
            KisTransaction* cmd = new KisTransaction("", layerProjection);
            f->process(srcCfg, dstCfg, tmpRc.size(), cfg);
            delete cmd;

            // Copy the filtered bits onto the projection
            KisPainter gc(m_projection);
            if (selection)
                gc.bltSelection(tmpRc.left(), tmpRc.top(),
                                layer->compositeOp(), layerProjection, selection, layer->opacity(),
                                tmpRc.left(), tmpRc.top(), tmpRc.width(), tmpRc.height());
            else
                gc.bitBlt(tmpRc.left(), tmpRc.top(),
                          layer->compositeOp(), layerProjection, layer->opacity(),
                          tmpRc.left(), tmpRc.top(), tmpRc.width(), tmpRc.height());
            gc.end();

            return true;
        }


    bool visit( KisCloneLayer * layer )
        {

            if (m_projection.isNull()) {
                return false;
            }

            if (!layer->visible())
                return true;
            KisPaintDeviceSP dev = layer->projection();

            if ( !dev ) return false;

            QRect rc = dev->extent() & m_rc;

            KisPainter gc(m_projection);
            gc.setCompositeOp( layer->compositeOp() );
            gc.setOpacity( layer->opacity() );
            gc.setChannelFlags( layer->channelFlags() );

            gc.bitBlt(rc.topLeft(), dev, rc);

            return true;

        }

private:
    // Helper for the indirect painting
    template<class Target>
    KisSharedPtr<Target> paintIndirect(KisPaintDeviceSP source,
                                       KisSharedPtr<Target> target,
                                       KisIndirectPaintingSupport* layer,
                                       qint32 sx, qint32 sy, qint32 dx, qint32 dy,
                                       qint32 w, qint32 h) {
        KisPainter gc2(target.data());
        gc2.bitBlt(dx, dy, COMPOSITE_COPY, source,
                   OPACITY_OPAQUE, sx, sy, w, h);
        gc2.bitBlt(dx, dy, layer->temporaryCompositeOp(), layer->temporaryTarget(),
                   layer->temporaryOpacity(), sx, sy, w, h);
        gc2.end();
        return target;
    }
    KisPaintDeviceSP m_projection;
    QRect m_rc;
};
}
class KisTopDownUpdateStrategy::Private
{
public:

    KisNodeSP node;
    KisNodeSP filthyNode;
    KisImageSP image;
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


void KisTopDownUpdateStrategy::setDirty( const QRect & rc )
{
   /*
        if the filthyNode is the node
            set the parent dirty
        if the filtyNode is not the node
            schedule a projection update for the current node
                if the projection update is done, set the parent dirty
                    until the root is done
    */ 
    if (m_d->node && m_d->node->parent())
        static_cast<KisTopDownUpdateStrategy*>(m_d->node->parent()->updateStrategy())->setFilthyNode(m_d->node);
    
    if (KisLayer* layer = dynamic_cast<KisLayer*>(m_d->node.data())) {
        layer->updateProjection( rc );
    }
    if (m_d->node->parent()) {
        m_d->node->parent()->setDirty( rc );
    }
    if (m_d->image) {
        m_d->image->slotProjectionUpdated( rc );
    }
}
 
void KisTopDownUpdateStrategy::setImage(KisImageSP image)
{
    m_d->image = image;
}
     
void KisTopDownUpdateStrategy::lock()
{
    /*
       lock is called on the root layer's updateStrategy.
       this strategy locks recursively all children. Lock
       has two meanings: the update process is stopped until
       unlock is called and the nodes are set locked. In contrast
       with the bottom-up strategy that does per-node book-keeping,
       we have to lock the nodes themselves to avoid redirtying while
       updating the projection.
     */
    // XXX: huh? how come m_d->node is const here?
    m_d->node.data()->setLocked( true );
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
    m_d->node.data()->setLocked( false );
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
    KisMergeVisitor visitor(projection, rc);

    KisNodeSP child = m_d->node->firstChild();

    while( child )
    {
        child->accept(visitor);
        child = dynamic_cast<KisLayer*>( child->nextSibling().data() );
    }
    
    return projection;
     
    m_d->filthyNode = 0;
    return projection;
}

void KisTopDownUpdateStrategy::setFilthyNode( const KisNodeSP node )
{
    m_d->filthyNode = node;
}
