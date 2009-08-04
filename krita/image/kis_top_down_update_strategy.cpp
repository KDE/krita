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

#include <QDebug>

#include <KoCompositeOp.h>

#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_node_visitor.h"
#include "kis_painter.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_adjustment_layer.h"
#include "generator/kis_generator_layer.h"
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
#include "kis_projection.h"
#include "kis_node_progress_proxy.h"

namespace
{
/**
 * The class merge visitor works using a bottom-up recomposition strategy. It does
 * not need to update the projection of the nodes, because this strategy updates
 * all projections before starting to merge.
 */
    class KisMergeVisitor : public KisNodeVisitor
    {

    public:

        using KisNodeVisitor::visit;

        KisMergeVisitor(KisPaintDeviceSP projection, const QRect& rc) :
            KisNodeVisitor() {
            Q_ASSERT(projection);
            m_projection = projection;
            m_rc = rc;
        }

    public:

        bool visit(KisExternalLayer * layer) {
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
            gc.setChannelFlags(layer->channelFlags());
            gc.setCompositeOp(layer->compositeOp());
            gc.setOpacity(layer->opacity());
            gc.bitBlt(rc.topLeft(), dev, rc);

            return true;
        }

        bool visit(KisGeneratorLayer * layer) {
            if (m_projection.isNull()) {
                return false;
            }
            if (!layer->visible())
                return true;

            layer->updateProjection(m_rc);
            KisPaintDeviceSP dev = layer->projection();
            if (!dev)
                return true;

            QRect rc = dev->extent() & m_rc;

            KisPainter gc(m_projection);
            gc.setChannelFlags(layer->channelFlags());


            return true;
        }

        bool visit(KisPaintLayer *layer) {

            if (m_projection.isNull()) {
                return false;
            }

            bool first = false;
            if (layer->prevSibling() == 0 && layer->parent() == layer->image()->root())
                first = true;

            if (!layer->visible()) {
                if (first) {
                    m_projection->clear(m_rc);
                }
                return true;
            }

            QRect rc = layer->extent() & m_rc;

            KisPainter gc(m_projection);
            QBitArray flags = layer->channelFlags();
            gc.setChannelFlags(flags);

            KisPaintDeviceSP source = layer->projection();
            gc.setOpacity(layer->opacity());
            
            if (first) {
                gc.setCompositeOp(m_projection->colorSpace()->compositeOp(COMPOSITE_COPY));
                gc.bitBlt(rc.topLeft(), source, rc);
            }
            else {
                gc.setCompositeOp(layer->compositeOp());
                gc.bitBlt(rc.topLeft(), source, rc);
            }
            return true;
        }

        bool visit(KisGroupLayer *layer) {

            if (m_projection.isNull()) {
                return false;
            }

            if (!layer->visible())
                return true;

            KisPaintDeviceSP dev = layer->projection();

            QRect rc = dev->extent() & m_rc;

            KisPainter gc(m_projection);
            gc.setChannelFlags(layer->channelFlags());
            gc.setCompositeOp(layer->compositeOp());
            gc.setOpacity(layer->opacity());
            gc.bitBlt(rc.topLeft(), dev, rc);
            
            return true;
        }

        bool visit(KisAdjustmentLayer* layer) {

            Q_ASSERT(layer);

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
            if (!cfg) return true;

            KisFilterSP f = KisFilterRegistry::instance()->value(cfg->name());
            if (!f) return false;

            // Possibly enlarge the rect that changed (like for convolution filters)
            tmpRc = f->changedRect(tmpRc, cfg);

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

            Q_ASSERT( layer->nodeProgressProxy() );
            KoProgressUpdater updater( layer->nodeProgressProxy() );
            updater.start( 100, f->name() );
            KoUpdaterPtr up = updater.startSubtask();
            // Some filters will require usage of oldRawData, which is not available without
            // a transaction!
            KisTransaction* cmd = new KisTransaction("", layerProjection);
            f->process(srcCfg, dstCfg, tmpRc.size(), cfg, up);
            delete cmd;
            layer->nodeProgressProxy()->setValue( layer->nodeProgressProxy()->maximum() );

            // Copy the filtered bits onto the projection
            KisPainter gc(m_projection);
            if (selection) {
                gc.setSelection(selection);
                gc.setCompositeOp(layer->compositeOp());
                gc.setOpacity(layer->opacity());
                gc.bitBlt(tmpRc.topLeft(), layerProjection, tmpRc);
            }
            else {
                gc.setCompositeOp(layer->compositeOp());
                gc.setOpacity(layer->opacity());
                gc.bitBlt(tmpRc.topLeft(), layerProjection, tmpRc);
            }
            gc.end();

            return true;
        }


        bool visit(KisCloneLayer * layer) {

            if (m_projection.isNull()) {
                return false;
            }

            if (!layer->visible())
                return true;
            KisPaintDeviceSP dev = layer->projection();

            if (!dev) return false;

            QRect rc = dev->extent() & m_rc;

            KisPainter gc(m_projection);
            gc.setCompositeOp(layer->compositeOp());
            gc.setOpacity(layer->opacity());
            gc.setChannelFlags(layer->channelFlags());

            gc.bitBlt(rc.topLeft(), dev, rc);

            return true;

        }

        bool visit(KisNode*) { return true; }
        bool visit(KisFilterMask*) { return true; }
        bool visit(KisTransparencyMask*) { return true; }
        bool visit(KisTransformationMask*) { return true; }
        bool visit(KisSelectionMask*) { return true; }


    private:
        KisPaintDeviceSP m_projection;
        QRect m_rc;
    };
}


class KisTopDownUpdateStrategy::Private
{
public:

    KisNodeWSP node;
    KisNodeWSP filthyNode;
    KisImageSP image;
};

KisTopDownUpdateStrategy::KisTopDownUpdateStrategy(KisNodeWSP node)
    : m_d(new Private)
{
    m_d->node = node;
}


KisTopDownUpdateStrategy::~KisTopDownUpdateStrategy()
{
    delete m_d;
}


void KisTopDownUpdateStrategy::setDirty(const QRect & rc)
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
        layer->updateProjection(rc);
    }
    if (m_d->node->parent()) {
        m_d->node->parent()->setDirty(rc);
    }
    if (m_d->image) {
        m_d->image->slotProjectionUpdated(rc);
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
    m_d->node.data()->setSystemLocked(true);
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
    m_d->node.data()->setSystemLocked(false);
}

KisPaintDeviceSP KisTopDownUpdateStrategy::updateGroupLayerProjection(const QRect & rc, KisPaintDeviceSP projection)
{
    /*
      Grouplayer are special since they can contain nodes that contain a projection
      of part of the stack.

      if filthynode is above an adjustment layer
      start recomposition from the adjustment layer
      else
      start recomposition from the bottom
    */
    KisNodeSP startWith = m_d->node->firstChild();
    if (m_d->filthyNode) {
        KisNodeSP node = m_d->node->firstChild();
        while (node) {
            if (node.data() == m_d->filthyNode.data())
                break;
            if (node->inherits("KisAdjustmentLayer")) {
                startWith = node;
            }
            node = node->nextSibling();
        }
    }

    KisMergeVisitor visitor(projection, rc);

    KisNodeSP child = startWith;

    while (child) {
        child->accept(visitor);
        child = dynamic_cast<KisLayer*>(child->nextSibling().data());
    }

//     return projection;

    m_d->filthyNode = 0;
    return projection;
}

void KisTopDownUpdateStrategy::setFilthyNode(const KisNodeWSP node)
{
    m_d->filthyNode = node;
}
