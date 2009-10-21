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

#include <QDebug>

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
#include "kis_iterators_pixel.h"
#include "kis_clone_layer.h"
#include "kis_processing_information.h"
#include "kis_node.h"
#include "kis_projection.h"
#include "kis_node_progress_proxy.h"


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

    bool visit(KisExternalLayer *layer) {
        return compositeWithProjection(layer, m_rc);
    }

    bool visit(KisGeneratorLayer * layer) {
        /**
         * FIXME: Pay attention to selection
         * AAA: seems to be done, check it please
         */
        return compositeWithProjection(layer, m_rc);
    }

    bool visit(KisPaintLayer *layer) {
        return compositeWithProjection(layer, m_rc);
    }

    bool visit(KisGroupLayer *layer) {
        return compositeWithProjection(layer, m_rc);
    }

    bool visit(KisAdjustmentLayer* layer) {
        Q_ASSERT(m_projection);
        if (!layer->visible()) return true;

        KisFilterConfiguration *filterConfig = layer->filter();
        if (!filterConfig) return true;

        KisFilterSP filter = KisFilterRegistry::instance()->value(filterConfig->name());
        if (!filter) return false;

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
        return compositeWithProjection(layer, changeRect);
    }


    bool visit(KisCloneLayer * layer) {
        return compositeWithProjection(layer, m_rc);
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

    bool compositeWithProjection(KisLayer *layer, const QRect &rect) {

        Q_ASSERT(m_projection);
        if (!layer->visible()) return true;

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


    KisPaintDeviceSP m_projection;
    QRect m_rc;
};

