/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
 *           (C) Dmitry Kazakov <dimula73@gmail.com>, 2009
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
#include <KoUpdater.h>

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

        bool generalCopy(KisLayer *layer, const QRect &rect) {
            Q_ASSERT(m_projection);
            if(!layer->visible()) return true;

            KisPaintDeviceSP device = layer->projection();
            if (!device) return true;

            QRect needRect = rect & device->extent();

            KisPainter gc(m_projection);
            gc.setChannelFlags(layer->channelFlags());
            gc.setCompositeOp(layer->compositeOp());
            gc.setOpacity(layer->opacity());
            gc.bitBlt(needRect.topLeft(), device, needRect);

            return true;
        }

        bool visit(KisExternalLayer *layer) {
            return generalCopy(layer, m_rc);
        }

        bool visit(KisGeneratorLayer * layer) {
            /**
             * FIXME: Pay attention to selection
             * AAA: seems to be done, check it please
             */
            return generalCopy(layer, m_rc);
        }

        bool visit(KisPaintLayer *layer) {
            return generalCopy(layer, m_rc);
        }

        bool visit(KisGroupLayer *layer) {
            return generalCopy(layer, m_rc);
        }

        bool visit(KisAdjustmentLayer* layer) {
            Q_ASSERT(m_projection);
            if(!layer->visible()) return true;

            KisFilterConfiguration *filterConfig = layer->filter();
            if(!filterConfig) return true;

            KisFilterSP filter = KisFilterRegistry::instance()->value(filterConfig->name());
            if(!filter) return false;

            /**
             * FIXME: A set of crutches, until bottom-up
             * merging strategy is ready
             */
            QRect needRect = m_rc;
            QRect changeRect = m_rc;
            QRect applyRect = m_rc;

            /**
             * FIXME: make this like in KisLayer::applyMasks
             */
            KisPaintDeviceSP originalDevice = layer->original();
            KisPaintDeviceSP tempDevice =
                new KisPaintDevice(originalDevice->colorSpace());

            /**
             * Assume that needRect has already been prepared for
             * us by bottom-up update strategy
             */

            /**
             * FIXME: check whether it's right to leave a selection to
             * a projection mechanism, not for the filter
             */
            KisConstProcessingInformation srcCfg(m_projection, applyRect.topLeft(), 0);
            KisProcessingInformation dstCfg(tempDevice, applyRect.topLeft(), 0);

            Q_ASSERT(layer->nodeProgressProxy());

            KoProgressUpdater updater(layer->nodeProgressProxy());
            updater.start(100, filter->name());
            QPointer<KoUpdater> updaterPtr = updater.startSubtask();

            KisTransaction* transaction =
                new KisTransaction("", tempDevice);
            filter->process(srcCfg, dstCfg, applyRect.size(),
                            filterConfig, updaterPtr);
            delete transaction;

            updaterPtr->setProgress(100);

            KisPainter gc(originalDevice);
            gc.setCompositeOp(originalDevice->colorSpace()->compositeOp(COMPOSITE_COPY));
            gc.bitBlt(changeRect.topLeft(), tempDevice, changeRect);
            gc.end();

            /**
             * FIXME: Pay attention to this change rect too
             */
            changeRect = layer->updateProjection(changeRect);

            /**
             * FIXME: another crutch (changeRect)
             */
            return generalCopy(layer, changeRect);
        }


        bool visit(KisCloneLayer * layer) {
            return generalCopy(layer, m_rc);
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
    KisImageWSP image;
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

    QRect dirtyRect = rc;

    if (KisLayer* layer = dynamic_cast<KisLayer*>(m_d->node.data())) {
        dirtyRect |= layer->updateProjection(rc);
    }
    if (m_d->node->parent()) {
        m_d->node->parent()->setDirty(dirtyRect);
    }
    if (m_d->image) {
        m_d->image->slotProjectionUpdated(dirtyRect);
    }
}

void KisTopDownUpdateStrategy::setImage(KisImageWSP image)
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
        // FIXME: No dynamic_cast needed
        child = dynamic_cast<KisLayer*>(child->nextSibling().data());
    }

    m_d->filthyNode = 0;
    return projection;
}

void KisTopDownUpdateStrategy::setFilthyNode(const KisNodeWSP node)
{
    m_d->filthyNode = node;
}
