/*
 *  Copyright (c) 2006 C. Boemann <cbo@boemann.dk>
 *  Copyright (c) 2010 Sven Langkamp <sven.langkamp@gmail.com>
 *  Copyright (c) 2010 Marc Pegon <pe.marc@free.fr>
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
 *  Copyright (c) 2010 Boudewijn Rempt <boud@kde.org>
 *
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

#include "QRect"

#include "klocale.h"

#include "kis_node_visitor.h"
#include "kis_types.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_transform_worker.h"
#include "kis_transaction.h"
#include "kis_external_layer_iface.h"
#include "kis_undo_adapter.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "generator/kis_generator_layer.h"
#include "kis_pixel_selection.h"
#include "kis_transparency_mask.h"
#include "kis_selection_mask.h"
#include "kis_clone_layer.h"
#include "kis_filter_mask.h"

class KoUpdater;
class KisFilterStrategy;

class KisTransformVisitor : public KisNodeVisitor
{

public:

    using KisNodeVisitor::visit;

    KisTransformVisitor(KisImageWSP image, qreal  xscale, qreal  yscale,
                        qreal  /*xshear*/, qreal  /*yshear*/, qreal angle,
                        qint32  tx, qint32  ty, KoUpdater *progress, KisFilterStrategy *filter,
                        bool scaleOnlyShapes = false)
            : KisNodeVisitor()
            , m_sx(xscale)
            , m_sy(yscale)
            , m_tx(tx)
            , m_ty(ty)
            , m_filter(filter)
            , m_angle(angle)
            , m_progress(progress)
            , m_image(image)
            , m_scaleOnlyShapes(scaleOnlyShapes){
    }

    virtual ~KisTransformVisitor() {
    }

    bool visit(KisExternalLayer * layer) {
        KisUndoAdapter* undoAdapter = layer->image()->undoAdapter();

        KisTransformWorker tw(layer->projection(), m_sx, m_sy,
                              0, 0, 0, 0,
                              m_angle, m_tx, m_ty, 0,
                              m_filter, true);

        KUndo2Command* command = layer->transform(tw.transform());
        if (command)
            undoAdapter->addCommand(command);
        visitAll(layer);
        return true;
    }

    /**
     * Crops the specified layer and adds the undo information to the undo adapter of the
     * layer's image.
     */
    bool visit(KisPaintLayer *layer) {
        transformPaintDevice(layer);
        visitAll(layer);
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
        visitAll(layer);
        return true;
    }

    bool visit(KisGeneratorLayer* layer) {
        transformPaintDevice(layer);
        visitAll(layer);
        return true;
    }

    bool visit(KisNode*) {
        return true;
    }
    bool visit(KisCloneLayer* layer) {
        visitAll(layer);
        return true;
    }
    bool visit(KisFilterMask* mask) {
        transformMask(mask);
        return true;
    }
    bool visit(KisTransparencyMask* mask) {
        transformMask(mask);
        return true;
    }
    bool visit(KisSelectionMask* mask) {
        transformMask(mask);
        return true;
    }


private:

    void transformPaintDevice(KisNode * node) {
        if(m_scaleOnlyShapes) {
            return;
        }

        KisPaintDeviceSP dev = node->paintDevice();

        KisTransaction transaction(i18n("Rotate Node"), dev);

        KisTransformWorker tw(dev, m_sx, m_sy, 0.0, 0.0, 0.0, 0.0, m_angle, m_tx, m_ty, m_progress, m_filter, true);
        tw.run();

        transaction.commit(m_image->undoAdapter());
        node->setDirty();
    }

    void transformMask(KisMask* mask) {
        KisSelectionSP selection = mask->selection();
        if(selection->hasPixelSelection() && !m_scaleOnlyShapes) {
            KisSelectionTransaction transaction(QString(), m_image, selection);

            KisPaintDeviceSP dev = selection->getOrCreatePixelSelection().data();
            KisTransformWorker tw(dev, m_sx, m_sy, 0.0, 0.0, 0.0, 0.0, m_angle, m_tx, m_ty, m_progress, m_filter, true);
            tw.run();

            transaction.commit(m_image->undoAdapter());
        }
        if (selection->hasShapeSelection()) {
            KisTransformWorker tw(selection->projection(), m_sx, m_sy, 0.0, 0.0, 0.0, 0.0, m_angle, m_tx, m_ty, 0, m_filter, true);

            KUndo2Command* command = selection->shapeSelection()->transform(tw.transform());
            if (command)
                m_image->undoAdapter()->addCommand(command);
        }

        selection->updateProjection();
    }

private:
    qreal m_sx, m_sy;
    qint32 m_tx, m_ty;
    KisFilterStrategy *m_filter;
    qreal m_angle;
    KoUpdater *m_progress;
    KisImageWSP m_image;
    bool m_scaleOnlyShapes;
};


#endif
