/* Copyright (c) Dmitry Kazakov <dimula73@gmail.com>, 2009
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

#include "kis_async_merger.h"


#include <kis_debug.h>
#include <QBitArray>

#include <KoChannelInfo.h>
#include <KoCompositeOpRegistry.h>

#include "kis_node_visitor.h"
#include "kis_painter.h"
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
#include "kis_clone_layer.h"
#include "kis_processing_information.h"
#include "kis_busy_progress_indicator.h"


#include "kis_merge_walker.h"
#include "kis_refresh_subtree_walker.h"

#include "kis_abstract_projection_plane.h"


//#define DEBUG_MERGER

#ifdef DEBUG_MERGER
#define DEBUG_NODE_ACTION(message, type, leaf, rect)            \
    qDebug() << message << type << ":" << leaf->node()->name() << rect
#else
#define DEBUG_NODE_ACTION(message, type, leaf, rect)
#endif


class KisUpdateOriginalVisitor : public KisNodeVisitor
{
public:
    KisUpdateOriginalVisitor(const QRect &updateRect, KisPaintDeviceSP projection, const QRect &cropRect)
        : m_updateRect(updateRect),
          m_cropRect(cropRect),
          m_projection(projection)
        {
        }

    ~KisUpdateOriginalVisitor() override {
    }

public:
    using KisNodeVisitor::visit;

    bool visit(KisAdjustmentLayer* layer) override {
        if (!layer->visible()) return true;

        if (!m_projection) {
            warnImage << "ObligeChild mechanism has been activated for "
                "an adjustment layer! Do nothing...";
            layer->original()->clear();
            return true;
        }

        const QRect originalUpdateRect =
            layer->projectionPlane()->needRectForOriginal(m_updateRect);

        KisPaintDeviceSP originalDevice = layer->original();
        originalDevice->clear(originalUpdateRect);

        const QRect applyRect = originalUpdateRect & m_projection->extent();

        // If the intersection of the updaterect and the projection extent is
        //      null, we are finish here.
        if(applyRect.isNull()) return true;

        KisFilterConfigurationSP filterConfig = layer->filter();
        if (!filterConfig) {
            /**
             * When an adjustment layer is just created, it may have no
             * filter inside. Then the layer has work as a pass-through
             * node. Just copy the merged data to the layer's original.
             */
            KisPainter::copyAreaOptimized(applyRect.topLeft(), m_projection, originalDevice, applyRect);
            return true;
        }

        KisSelectionSP selection = layer->fetchComposedInternalSelection(applyRect);
        const QRect filterRect = selection ? applyRect & selection->selectedRect() : applyRect;

        KisFilterSP filter = KisFilterRegistry::instance()->value(filterConfig->name());
        if (!filter) return false;

        KisPaintDeviceSP dstDevice = originalDevice;

        if (selection) {
            dstDevice = new KisPaintDevice(originalDevice->colorSpace());
        }

        if (!filterRect.isEmpty()) {
            KIS_ASSERT_RECOVER_NOOP(layer->busyProgressIndicator());
            layer->busyProgressIndicator()->update();

            // We do not create a transaction here, as srcDevice != dstDevice
            filter->process(m_projection, dstDevice, 0, filterRect, filterConfig.data(), 0);
        }

        if (selection) {
            KisPainter::copyAreaOptimized(applyRect.topLeft(), m_projection, originalDevice, applyRect);
            KisPainter::copyAreaOptimized(filterRect.topLeft(), dstDevice, originalDevice, filterRect, selection);
        }

        return true;
    }

    bool visit(KisExternalLayer*) override {
        return true;
    }

    bool visit(KisGeneratorLayer*) override {
        return true;
    }

    bool visit(KisPaintLayer*) override {
        return true;
    }

    bool visit(KisGroupLayer*) override {
        return true;
    }

    bool visit(KisCloneLayer *layer) override {
        QRect emptyRect;
        KisRefreshSubtreeWalker walker(emptyRect);
        KisAsyncMerger merger;

        KisLayerSP srcLayer = layer->copyFrom();
        QRect srcRect = m_updateRect.translated(-layer->x(), -layer->y());

        QRegion prepareRegion(srcRect);
        prepareRegion -= m_cropRect;


        /**
         * If a clone has complicated masks, we should prepare additional
         * source area to ensure the rect is prepared.
         */
        QRect needRectOnSource = layer->needRectOnSourceForMasks(srcRect);
        if (!needRectOnSource.isEmpty()) {
            prepareRegion += needRectOnSource;
        }

        if (srcLayer.isNull()) {
            return true;
        }

        Q_FOREACH (const QRect &rect, prepareRegion.rects()) {
            walker.collectRects(srcLayer, rect);
            merger.startMerge(walker, false);
        }

        return true;
    }

    bool visit(KisNode*) override {
        return true;
    }
    bool visit(KisFilterMask*) override {
        return true;
    }
    bool visit(KisTransformMask*) override {
        return true;
    }
    bool visit(KisTransparencyMask*) override {
        return true;
    }
    bool visit(KisSelectionMask*) override {
        return true;
    }
    bool visit(KisColorizeMask*) override {
        return true;
    }

private:
    QRect m_updateRect;
    QRect m_cropRect;
    KisPaintDeviceSP m_projection;
};


/*********************************************************************/
/*                     KisAsyncMerger                                */
/*********************************************************************/

void KisAsyncMerger::startMerge(KisBaseRectsWalker &walker, bool notifyClones) {
    KisMergeWalker::LeafStack &leafStack = walker.leafStack();

    const bool useTempProjections = walker.needRectVaries();

    while(!leafStack.isEmpty()) {
        KisMergeWalker::JobItem item = leafStack.pop();
        KisProjectionLeafSP currentLeaf = item.m_leaf;

        /**
         * In some unidentified cases teh nodes might be removed
         * while the updates are still running. We have no proof
         * of it yet, so just add a safety assert here.
         */
        KIS_SAFE_ASSERT_RECOVER_RETURN(currentLeaf);
        KIS_SAFE_ASSERT_RECOVER_RETURN(currentLeaf->node());

        // All the masks should be filtered by the walkers
        KIS_SAFE_ASSERT_RECOVER_RETURN(currentLeaf->isLayer());

        QRect applyRect = item.m_applyRect;

        if (currentLeaf->isRoot()) {
            currentLeaf->projectionPlane()->recalculate(applyRect, walker.startNode());
            continue;
        }

        if(item.m_position & KisMergeWalker::N_EXTRA) {
            // The type of layers that will not go to projection.

            DEBUG_NODE_ACTION("Updating", "N_EXTRA", currentLeaf, applyRect);
            KisUpdateOriginalVisitor originalVisitor(applyRect,
                                                     m_currentProjection,
                                                     walker.cropRect());
            currentLeaf->accept(originalVisitor);
            currentLeaf->projectionPlane()->recalculate(applyRect, currentLeaf->node());

            continue;
        }


        if (!m_currentProjection) {
            setupProjection(currentLeaf, applyRect, useTempProjections);
        }

        KisUpdateOriginalVisitor originalVisitor(applyRect,
                                                 m_currentProjection,
                                                 walker.cropRect());

        if(item.m_position & KisMergeWalker::N_FILTHY) {
            DEBUG_NODE_ACTION("Updating", "N_FILTHY", currentLeaf, applyRect);
            if (currentLeaf->visible() || currentLeaf->hasClones()) {
                currentLeaf->accept(originalVisitor);
                currentLeaf->projectionPlane()->recalculate(applyRect, walker.startNode());
            }
        }
        else if(item.m_position & KisMergeWalker::N_ABOVE_FILTHY) {
            DEBUG_NODE_ACTION("Updating", "N_ABOVE_FILTHY", currentLeaf, applyRect);
            if(currentLeaf->dependsOnLowerNodes()) {
                if (currentLeaf->visible() || currentLeaf->hasClones()) {
                    currentLeaf->accept(originalVisitor);
                    currentLeaf->projectionPlane()->recalculate(applyRect, currentLeaf->node());
                }
            }
        }
        else if(item.m_position & KisMergeWalker::N_FILTHY_PROJECTION) {
            DEBUG_NODE_ACTION("Updating", "N_FILTHY_PROJECTION", currentLeaf, applyRect);
            if (currentLeaf->visible() || currentLeaf->hasClones()) {
                currentLeaf->projectionPlane()->recalculate(applyRect, walker.startNode());
            }
        }
        else /*if(item.m_position & KisMergeWalker::N_BELOW_FILTHY)*/ {
            DEBUG_NODE_ACTION("Updating", "N_BELOW_FILTHY", currentLeaf, applyRect);
            /* nothing to do */
        }

        compositeWithProjection(currentLeaf, applyRect);

        if(item.m_position & KisMergeWalker::N_TOPMOST) {
            writeProjection(currentLeaf, useTempProjections, applyRect);
            resetProjection();
        }

        // FIXME: remove it from the inner loop and/or change to a warning!
        Q_ASSERT(currentLeaf->projection()->defaultBounds()->currentLevelOfDetail() ==
                 walker.levelOfDetail());
    }

    if(notifyClones) {
        doNotifyClones(walker);
    }

    if(m_currentProjection) {
        warnImage << "BUG: The walker hasn't reached the root layer!";
        warnImage << "     Start node:" << walker.startNode() << "Requested rect:" << walker.requestedRect();
        warnImage << "     An inconsistency in the walkers occurred!";
        warnImage << "     Please report a bug describing how you got this message.";
        // reset projection to avoid artifacts in next merges and allow people to work further
        resetProjection();
    }
}

void KisAsyncMerger::resetProjection() {
    m_currentProjection = 0;
    m_finalProjection = 0;
}

void KisAsyncMerger::setupProjection(KisProjectionLeafSP currentLeaf, const QRect& rect, bool useTempProjection) {
    KisPaintDeviceSP parentOriginal = currentLeaf->parent()->original();

    if (parentOriginal != currentLeaf->projection()) {
        if (useTempProjection) {
            if(!m_cachedPaintDevice)
                m_cachedPaintDevice = new KisPaintDevice(parentOriginal->colorSpace());
            m_currentProjection = m_cachedPaintDevice;
            m_currentProjection->prepareClone(parentOriginal);
            m_finalProjection = parentOriginal;
        }
        else {
            parentOriginal->clear(rect);
            m_finalProjection = m_currentProjection = parentOriginal;
        }
    }
    else {
        /**
         * It happened so that our parent uses our own projection as
         * its original. It means obligeChild mechanism works.
         * We won't initialise m_currentProjection. This will cause
         * writeProjection() and compositeWithProjection() do nothing
         * when called.
         */
        /* NOP */
    }
}

void KisAsyncMerger::writeProjection(KisProjectionLeafSP topmostLeaf, bool useTempProjection, const QRect &rect) {
    Q_UNUSED(useTempProjection);
    Q_UNUSED(topmostLeaf);
    if (!m_currentProjection) return;

    if(m_currentProjection != m_finalProjection) {
        KisPainter::copyAreaOptimized(rect.topLeft(), m_currentProjection, m_finalProjection, rect);
    }
    DEBUG_NODE_ACTION("Writing projection", "", topmostLeaf->parent(), rect);
}

bool KisAsyncMerger::compositeWithProjection(KisProjectionLeafSP leaf, const QRect &rect) {

    if (!m_currentProjection) return true;
    if (!leaf->visible()) return true;

    KisPainter gc(m_currentProjection);
    leaf->projectionPlane()->apply(&gc, rect);

    DEBUG_NODE_ACTION("Compositing projection", "", leaf, rect);
    return true;
}

void KisAsyncMerger::doNotifyClones(KisBaseRectsWalker &walker) {
    KisBaseRectsWalker::CloneNotificationsVector &vector =
        walker.cloneNotifications();

    KisBaseRectsWalker::CloneNotificationsVector::iterator it;

    for(it = vector.begin(); it != vector.end(); ++it) {
        (*it).notify();
    }
}
