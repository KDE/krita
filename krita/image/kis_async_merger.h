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

#ifndef __KIS_ASYNC_MERGER_H
#define __KIS_ASYNC_MERGER_H

#include <QDebug>

//#include <KoColorSpace.h>
#include <KoCompositeOp.h>
#include <KoUpdater.h>

#include "kis_types.h"
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
#include "kis_transaction.h"
//#include "kis_iterators_pixel.h"
#include "kis_clone_layer.h"
#include "kis_processing_information.h"
//#include "kis_node.h"
//#include "kis_projection.h"
#include "kis_node_progress_proxy.h"


#include "kis_merge_walkers.h"


class KisUpdateOriginalVisitor : public KisNodeVisitor
{
public:
    KisUpdateOriginalVisitor(QRect updateRect, KisPaintDeviceSP projection)
        : m_updateRect(updateRect), m_projection(projection)
    {
    }

    ~KisUpdateOriginalVisitor() {
    }

public:
    using KisNodeVisitor::visit;

    bool visit(KisAdjustmentLayer* layer) {
        Q_ASSERT(m_projection);
        if (!layer->visible()) return true;

        KisFilterConfiguration *filterConfig = layer->filter();
        if (!filterConfig) return true;

        KisFilterSP filter = KisFilterRegistry::instance()->value(filterConfig->name());
        if (!filter) return false;

        QRect applyRect = m_updateRect;
        KisPaintDeviceSP originalDevice = layer->original();

//        qDebug()<<"PR:"<<m_projection->exactBounds();
//        qDebug()<<"AR:"<<applyRect;
//        qDebug()<<"NR:"<<layer->needRect(applyRect);

        /**
         * FIXME: check whether it's right to leave a selection to
         * a projection mechanism, not for the filter
         */
        KisConstProcessingInformation srcCfg(m_projection, applyRect.topLeft(), 0);
        KisProcessingInformation dstCfg(originalDevice, applyRect.topLeft(), 0);

        Q_ASSERT(layer->nodeProgressProxy());

        KoProgressUpdater updater(layer->nodeProgressProxy());
        updater.start(100, filter->name());
        QPointer<KoUpdater> updaterPtr = updater.startSubtask();

        KisTransaction* transaction =
            new KisTransaction("", originalDevice);
        filter->process(srcCfg, dstCfg, applyRect.size(),
                        filterConfig, updaterPtr);
        delete transaction;

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

    bool visit(KisCloneLayer*) {
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
    bool visit(KisTransformationMask*) {
        return true;
    }
    bool visit(KisSelectionMask*) {
        return true;
    }

private:
    QRect m_updateRect;
    KisPaintDeviceSP m_projection;
};


class KisAsyncMerger
{
public:
    KisAsyncMerger() {
    }

    ~KisAsyncMerger() {
    }

    /**
     * FIXME: Check node<->layer transitions
     */

    void startMerge(KisMergeWalker &walker) {
        KisMergeWalker::NodeStack &nodeStack = walker.nodeStack();

        const bool useTempProjections = walker.needRectVaries();

        while(!nodeStack.isEmpty()) {
            KisMergeWalker::JobItem item = nodeStack.pop();
            if(isRootNode(item.m_node)) continue;

            if(!m_currentProjection) {
                bool obligeChild = setupProjection(item.m_node,
                                                   useTempProjections);
                if(obligeChild) continue;
            }

            KisLayerSP currentNode = dynamic_cast<KisLayer*>(item.m_node.data());
            QRect applyRect = item.m_applyRect;

            KisUpdateOriginalVisitor originalVisitor(applyRect,
                                                     m_currentProjection);

            switch(item.m_position) {
            case KisGraphWalker::N_TOPMOST:
                currentNode->accept(originalVisitor);
                currentNode->updateProjection(applyRect);
                compositeWithProjection(currentNode, applyRect);
                writeProjection(currentNode, useTempProjections, applyRect);
                resetProjection();
                break;
            case KisGraphWalker::N_NORMAL:
                currentNode->accept(originalVisitor);
                currentNode->updateProjection(applyRect);
            case KisGraphWalker::N_LOWER:
            case KisGraphWalker::N_BOTTOMMOST:
                compositeWithProjection(currentNode, applyRect);
            }

        }
    }

private:
    static inline bool isRootNode(KisNodeSP node) {
        return !node->parent();
    }

    void resetProjection() {
        m_currentProjection = 0;
    }

    bool setupProjection(KisNodeSP currentNode, bool useTempProjection) {
        KisPaintDeviceSP parentOriginal = currentNode->parent()->original();

        bool obligeChild = parentOriginal == currentNode->projection();

        if (!obligeChild) {
            m_currentProjection = !useTempProjection ? parentOriginal :
                new KisPaintDevice(parentOriginal->colorSpace());
        }

        return obligeChild;
    }

    void writeProjection(KisNodeSP topmostNode, bool useTempProjection, QRect rect) {
        Q_UNUSED(useTempProjection);
        KisPaintDeviceSP parentOriginal = topmostNode->parent()->original();

        if(m_currentProjection != parentOriginal) {
            KisPainter gc(parentOriginal);
            gc.setCompositeOp(parentOriginal->colorSpace()->compositeOp(COMPOSITE_COPY));
            gc.bitBlt(rect.topLeft(), m_currentProjection, rect);
        }
    }

    bool compositeWithProjection(KisLayerSP layer, const QRect &rect) {

        Q_ASSERT(m_currentProjection);
        if (!layer->visible()) return true;

        KisPaintDeviceSP device = layer->projection();
        if (!device) return true;

        /**
         * FIXME: Check whether this cropping is still needed
         * Probable case: selections on layers
         */
//        QRect needRect = rect & device->extent();
//        QRect needRect = rect & device->exactBounds();
        QRect needRect = rect;
        Q_ASSERT(device->exactBounds().contains(needRect));

        KisPainter gc(m_currentProjection);
        gc.setChannelFlags(layer->channelFlags());
        gc.setCompositeOp(layer->compositeOp());
        gc.setOpacity(layer->opacity());
        gc.bitBlt(needRect.topLeft(), device, needRect);

        return true;
    }

private:
    KisPaintDeviceSP m_currentProjection;
};


#endif /* __KIS_ASYNC_MERGER_H */

