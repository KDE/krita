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

#include "kis_layer_visitor.h"
#include "kis_types.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_transaction.h"
#include "kis_transform_worker.h"
#include <kis_selected_transaction.h>

class KisProgressDisplayInterface;
class KisFilterStrategy;

class KisTransformVisitor : public KisLayerVisitor {

public:

    KisTransformVisitor(KisImageSP img, double  xscale, double  yscale,
        double  /*xshear*/, double  /*yshear*/, double angle,
        Q_INT32  tx, Q_INT32  ty, KisProgressDisplayInterface *progress, KisFilterStrategy *filter) 
        : KisLayerVisitor()
        , m_sx(xscale)
        , m_sy(yscale)
        , m_tx(tx)
        , m_ty(ty)
        , m_filter(filter)
        , m_angle(angle)
        , m_progress(progress)
        , m_img(img)
    {
    }

    virtual ~KisTransformVisitor()
    {
    }

    /**
     * Crops the specified layer and adds the undo information to the undo adapter of the
     * layer's image.
     */
    bool visit(KisPaintLayer *layer) 
    {
        KisPaintDeviceSP dev = layer->paintDevice();

        KisTransaction * t = 0;
        if (m_img->undo()) {
            t = new KisTransaction(i18n("Rotate Layer"), dev);
            Q_CHECK_PTR(t);
	}

        KisTransformWorker tw(dev, m_sx, m_sy, 0.0, 0.0, m_angle, m_tx, m_ty, m_progress, m_filter);
        tw.run();

        if (m_img->undo()) {
            m_img->undoAdapter()->addCommand(t);
	}
        layer->setDirty();
        return true;
    };

    bool visit(KisGroupLayer *layer)
    {
	layer->resetProjection();

        KisLayerSP child = layer->firstChild();
        while (child) {
            child->accept(*this);
            child = child->nextSibling();
        }
        layer->setDirty();
        return true;
    };

    bool visit(KisPartLayer */*layer*/)
    {
        return true;
    };

    virtual bool visit(KisAdjustmentLayer* layer)
    {
        KisPaintDeviceSP dev = layer->selection().data();
        
        KisTransaction * t = 0;

        if (m_img->undo()) {
            t = new KisTransaction(i18n("Rotate Layer"), dev);
            Q_CHECK_PTR(t);
        }

        KisTransformWorker tw(dev, m_sx, m_sy, 0.0, 0.0, m_angle, m_tx, m_ty, m_progress, m_filter);
        tw.run();

        if (m_img->undo()) {
            m_img->undoAdapter()->addCommand(t);
        }
        layer->setDirty();
        
        layer->resetCache();
        return true;
    }
    

private:
    double m_sx, m_sy;
    Q_INT32 m_tx, m_ty;
    KisFilterStrategy *m_filter;
    double m_angle;
    KisProgressDisplayInterface *m_progress;
    KisImageSP m_img;
};


#endif
