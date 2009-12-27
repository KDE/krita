/*
 *  Copyright (c) 2006 Casper Boemann <cbr@boemann.dk>
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
#ifndef KIS_TRANSFORM_VISITOR_H_
#define KIS_TRANSFORM_VISITOR_H_

#include "qrect.h"

#include "klocale.h"

#include "kis_node_visitor.h"
#include "kis_types.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_transaction.h"
#include "kis_transform_worker.h"
#include "kis_selected_transaction.h"
#include "kis_external_layer_iface.h"
#include "kis_undo_adapter.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "generator/kis_generator_layer.h"

class KoUpdater;
class KisFilterStrategy;

class KisTransformVisitor : public KisNodeVisitor
{

public:

    using KisNodeVisitor::visit;

    KisTransformVisitor(KisImageWSP image, double  xscale, double  yscale,
                        double  /*xshear*/, double  /*yshear*/, double angle,
                        qint32  tx, qint32  ty, KoUpdater *progress, KisFilterStrategy *filter)
            : KisNodeVisitor()
            , m_sx(xscale)
            , m_sy(yscale)
            , m_tx(tx)
            , m_ty(ty)
            , m_filter(filter)
            , m_angle(angle)
            , m_progress(progress)
            , m_image(image) {
    }

    virtual ~KisTransformVisitor() {
    }

    bool visit(KisExternalLayer * layer) {
        KisUndoAdapter* undoAdapter = layer->image()->undoAdapter();
        
        QUndoCommand* cmd = layer->transform(m_sx, m_sy, 0.0, 0.0, m_angle, m_tx, m_ty);
        if (cmd && undoAdapter && undoAdapter->undo()) {
            undoAdapter->addCommand(cmd);
        }
        return true;
    }

    /**
     * Crops the specified layer and adds the undo information to the undo adapter of the
     * layer's image.
     */
    bool visit(KisPaintLayer *layer) {
        transformPaintDevice(layer);
        return true;
    }

    bool visit(KisGroupLayer *layer) {
        layer->resetCache();

        KisNodeSP child = layer->firstChild();
        while (child) {
            child->accept(*this);
            child = child->nextSibling();
        }
        layer->setDirty();
        return true;
    }

    virtual bool visit(KisAdjustmentLayer* layer) {
        transformPaintDevice(layer);
        layer->resetCache();
        return true;
    }

    bool visit(KisGeneratorLayer* layer) {
        transformPaintDevice(layer);
        return true;
    }

    bool visit(KisNode*) {
        return true;
    }
    bool visit(KisCloneLayer*) {
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

    void transformPaintDevice(KisNode * node) {
        KisPaintDeviceSP dev = node->paintDevice();

        KisTransaction * t = 0;
        if (m_image->undo()) {
            t = new KisTransaction(i18n("Rotate Node"), dev);
            Q_CHECK_PTR(t);
        }

        KisTransformWorker tw(dev, m_sx, m_sy, 0.0, 0.0, m_angle, m_tx, m_ty, m_progress, m_filter, true);
        tw.run();

        if (m_image->undo()) {
            m_image->undoAdapter()->addCommand(t);
        }
        node->setDirty();
    }

private:
    double m_sx, m_sy;
    qint32 m_tx, m_ty;
    KisFilterStrategy *m_filter;
    double m_angle;
    KoUpdater *m_progress;
    KisImageWSP m_image;
};


#endif
