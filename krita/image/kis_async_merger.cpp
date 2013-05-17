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


#include <QDebug>
#include <QBitArray>

#include <KoCompositeOp.h>
#include <KoUpdater.h>

#include "kis_paint_device.h"
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
#include "kis_node_progress_proxy.h"


#include "kis_merge_walker.h"
#include "kis_refresh_subtree_walker.h"


//#define DEBUG_MERGER

#ifdef DEBUG_MERGER
#define DEBUG_NODE_ACTION(message, type, node, rect)            \
    qDebug() << message << type << ":" << node->name() << rect
#else
#define DEBUG_NODE_ACTION(message, type, node, rect)
#endif


class KisUpdateOriginalVisitor : public KisNodeVisitor
{
public:
    KisUpdateOriginalVisitor(QRect updateRect, KisPaintDeviceSP projection, QRect cropRect)
        : m_updateRect(updateRect),
          m_cropRect(cropRect),
          m_projection(projection)
        {
        }

    ~KisUpdateOriginalVisitor() {
    }

public:
    using KisNodeVisitor::visit;

    bool visit(KisAdjustmentLayer* layer) {
        if (!layer->visible()) return true;

        if (!m_projection) {
            warnImage << "ObligeChild mechanism has been activated for "
                "an adjustment layer! Do nothing...";
            layer->original()->clear();
            return true;
        }

        KisPaintDeviceSP originalDevice = layer->original();
        originalDevice->clear(m_updateRect);

        QRect applyRect = m_updateRect & m_projection->extent();

        // If the intersection of the updaterect and the projection extent is
        //      null, we are finish here.
        if(applyRect.isNull()) return true;

        KisSafeFilterConfigurationSP filterConfig = layer->filter();
        if (!filterConfig) {
            /**
             * When an adjustment layer is just created, it may have no
             * filter inside. Then the layer has work as a pass-through
             * node. Just copy the merged data to the layer's original.
             */
            KisPainter gc(originalDevice);
            gc.setCompositeOp(COMPOSITE_COPY);
            gc.bitBlt(applyRect.topLeft(), m_projection, applyRect);
            return true;
        }

        KisFilterSP filter = KisFilterRegistry::instance()->value(filterConfig->name());
        if (!filter) return false;

        Q_ASSERT(layer->nodeProgressProxy());

        KoProgressUpdater updater(layer->nodeProgressProxy());
        updater.start(100, filter->name());
        QPointer<KoUpdater> updaterPtr = updater.startSubtask();

        // We do not create a transaction here, as srcDevice != dstDevice
        filter->process(m_projection, originalDevice, 0, applyRect, filterConfig.data(), updaterPtr);

        updaterPtr->setProgress(100);

        return true;
    }

    bool visit(KisExternalLayer*) {
        return true;
    }

    bool visit(KisGeneratorLayer*) {
        return true;
    }

    bool visit(KisPaintLayer*) {
        return true;
    }

    bool visit(KisGroupLayer*) {
        return true;
    }

    bool visit(KisCloneLayer *layer) {
        QRect emptyRect;
        KisRefreshSubtreeWalker walker(emptyRect);
        KisAsyncMerger merger;

        KisLayerSP srcLayer = layer->copyFrom();
        QRect srcRect = m_updateRect.translated(-layer->x(), -layer->y());

        QRegion prepareRegion(srcRect);
        prepareRegion -= m_cropRect;

        foreach(const QRect &rect, prepareRegion.rects()) {
            walker.collectRects(srcLayer, rect);
            merger.startMerge(walker, false);
        }

        return true;
    }

    bool visit(KisNode*) {
        return true;
    }
    bool visit(KisFilterMask*) {
        return true;
    }
    bool visit(KisTransparencyMask*) {
        return true;
    }
    bool visit(KisSelectionMask*) {
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

/**
 * FIXME: Check node<->layer transitions
 */
void KisAsyncMerger::startMerge(KisBaseRectsWalker &walker, bool notifyClones) {
    KisMergeWalker::NodeStack &nodeStack = walker.nodeStack();

    const bool useTempProjections = walker.needRectVaries();

    while(!nodeStack.isEmpty()) {
        KisMergeWalker::JobItem item = nodeStack.pop();
        if(isRootNode(item.m_node)) continue;

        KisLayerSP currentNode = dynamic_cast<KisLayer*>(item.m_node.data());
        // All the masks should be filtered by the walkers
        Q_ASSERT(currentNode);

        QRect applyRect = item.m_applyRect;

        if(item.m_position & KisMergeWalker::N_EXTRA) {
            // The type of layers that will not go to projection.

            DEBUG_NODE_ACTION("Updating", "N_EXTRA", currentNode, applyRect);
            KisUpdateOriginalVisitor originalVisitor(applyRect,
                                                     m_currentProjection,
                                                     walker.cropRect());
            currentNode->accept(originalVisitor);
            currentNode->updateProjection(applyRect);

            continue;
        }


        if(!m_currentProjection)
            setupProjection(currentNode, applyRect, useTempProjections);

        KisUpdateOriginalVisitor originalVisitor(applyRect,
                                                 m_currentProjection,
                                                 walker.cropRect());

        if(item.m_position & KisMergeWalker::N_FILTHY) {
            DEBUG_NODE_ACTION("Updating", "N_FILTHY", currentNode, applyRect);
            currentNode->accept(originalVisitor);
            currentNode->updateProjection(applyRect);
        }
        else if(item.m_position & KisMergeWalker::N_ABOVE_FILTHY) {
            DEBUG_NODE_ACTION("Updating", "N_ABOVE_FILTHY", currentNode, applyRect);
            if(dependOnLowerNodes(currentNode)) {
                currentNode->accept(originalVisitor);
                currentNode->updateProjection(applyRect);
            }
        }
        else if(item.m_position & KisMergeWalker::N_FILTHY_PROJECTION) {
            DEBUG_NODE_ACTION("Updating", "N_FILTHY_PROJECTION", currentNode, applyRect);
            currentNode->updateProjection(applyRect);
        }
        else /*if(item.m_position & KisMergeWalker::N_BELOW_FILTHY)*/ {
            DEBUG_NODE_ACTION("Updating", "N_BELOW_FILTHY", currentNode, applyRect);
            /* nothing to do */
        }

        compositeWithProjection(currentNode, applyRect);

        if(item.m_position & KisMergeWalker::N_TOPMOST) {
            writeProjection(currentNode, useTempProjections, applyRect);
            resetProjection();
        }
    }

    if(notifyClones) {
        doNotifyClones(walker);
    }

    if(m_currentProjection) {
        warnImage << "BUG: The walker hasn't reached the root layer!";
        warnImage << "     Start node:" << walker.startNode() << "Requested rect:" << walker.requestedRect();
        warnImage << "     There must be an inconsistency in the walkers happened!";
        warnImage << "     Please report a bug describing how you got this message.";
        // reset projection to avoid artefacts in next merges and allow people to work further
        resetProjection();
    }
}

bool KisAsyncMerger::isRootNode(KisNodeSP node) {
    return !node->parent();
}

bool KisAsyncMerger::dependOnLowerNodes(KisNodeSP node) {
    return qobject_cast<KisAdjustmentLayer*>(node.data());
}

void KisAsyncMerger::resetProjection() {
    m_currentProjection = 0;
    m_finalProjection = 0;
}

void KisAsyncMerger::setupProjection(KisNodeSP currentNode, const QRect& rect, bool useTempProjection) {
    KisPaintDeviceSP parentOriginal = currentNode->parent()->original();

    if (parentOriginal != currentNode->projection()) {
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

void KisAsyncMerger::writeProjection(KisNodeSP topmostNode, bool useTempProjection, QRect rect) {
    Q_UNUSED(useTempProjection);
    Q_UNUSED(topmostNode);
    if (!m_currentProjection) return;

    if(m_currentProjection != m_finalProjection) {
        KisPainter gc(m_finalProjection);
        gc.setCompositeOp(m_finalProjection->colorSpace()->compositeOp(COMPOSITE_COPY));
        gc.bitBlt(rect.topLeft(), m_currentProjection, rect);
    }
    DEBUG_NODE_ACTION("Writing projection", "", topmostNode->parent(), rect);
}

bool KisAsyncMerger::compositeWithProjection(KisLayerSP layer, const QRect &rect) {

    if (!m_currentProjection) return true;
    if (!layer->visible()) return true;

    KisPaintDeviceSP device = layer->projection();
    if (!device) return true;

    QRect needRect = rect & device->extent();
    if(needRect.isEmpty()) return true;

    QBitArray channelFlags = layer->channelFlags();


    // if the color spaces don't match we will have a problem with the channel flags
    // because the channel flags from the source layer doesn't match with the colorspace of the projection device
    // this leads to the situation that the wrong channels will be enabled/disabled
    if(!channelFlags.isEmpty() && m_currentProjection->colorSpace() != device->colorSpace()) {
        const KoColorSpace* src = device->colorSpace();
        const KoColorSpace* dst = m_currentProjection->colorSpace();

        bool alphaFlagIsSet        = (src->channelFlags(false,true) & channelFlags) == src->channelFlags(false,true);
        bool allColorFlagsAreSet   = (src->channelFlags(true,false) & channelFlags) == src->channelFlags(true,false);
        bool allColorFlagsAreUnset = (src->channelFlags(true,false) & channelFlags).count(true) == 0;

        if(allColorFlagsAreSet) {
            channelFlags = dst->channelFlags(true, alphaFlagIsSet);
        }
        else if(allColorFlagsAreUnset) {
            channelFlags = dst->channelFlags(false, alphaFlagIsSet);
        }
        else {
            //TODO: convert the cannel flags properly
            //      for now just the alpha channel bit is copied and the other channels are left alone
            for(quint32 i=0; i<dst->channelCount(); ++i) {
                if(dst->channels()[i]->channelType() == KoChannelInfo::ALPHA) {
                    channelFlags.setBit(i, alphaFlagIsSet);
                    break;
                }
            }
        }
    }
    KisPainter gc(m_currentProjection);
    gc.setChannelFlags(channelFlags);

    gc.setCompositeOp(layer->compositeOp());
    gc.setOpacity(layer->opacity());
    gc.bitBlt(needRect.topLeft(), device, needRect);

    DEBUG_NODE_ACTION("Compositing projection", "", layer, needRect);
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
