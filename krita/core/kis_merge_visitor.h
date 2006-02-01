/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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
#ifndef KIS_MERGE_H_
#define KIS_MERGE_H_

#include <qrect.h>

#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_layer_visitor.h"
#include "kis_painter.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include "kis_part_layer_iface.h"
#include "kis_filter.h"
#include "kis_filter_configuration.h"
#include "kis_filter_registry.h"

class KisMergeVisitor : public KisLayerVisitor {
public:
    /**
     * Don't even _think_ of creating a merge visitor without a projection; without a projection,
     * the adjustmentlayers won't work.
     */
    KisMergeVisitor(KisImageSP img, KisPaintDeviceSP projection, const QRect& rc) :
        KisLayerVisitor()
    {
        Q_ASSERT(img);
        Q_ASSERT(projection);

        m_img = img;
        m_projection = projection;
        m_rc = rc;
    }

public:
    virtual bool visit(KisPaintLayer *layer)
    {
        if (!layer -> visible())
            return true;

        Q_INT32 sx, sy, dx, dy, w, h;

        QRect rc = layer ->paintDevice() -> extent() & m_rc;

        sx= rc.left();
        sy = rc.top();
        w = rc.width();
        h = rc.height();
        dx = sx;
        dy = sy;

        KisPainter gc(m_projection);
        gc.bitBlt(dx, dy, layer->compositeOp() , layer->paintDevice(), layer->opacity(), sx, sy, w, h);

        layer->setDirty( false );
        return true;
    }

    virtual bool visit(KisGroupLayer *layer)
    {
        if (!layer -> visible())
            return true;

        // No layers in this group are dirty, we're not
        // dirty, we're not going to recomposite. Our
        // projection is up to date.
        if (!layer->dirty() && m_projection != layer->projection()) {
            KisPainter gc(m_projection);
            gc.bitBlt(m_rc.left(), m_rc.top(), layer->compositeOp(),
                      layer->projection(), OPACITY_OPAQUE,
                      m_rc.left(), m_rc.top(), m_rc.width(), m_rc.height());
            gc.end();

            return true;
        }

        // Get the first layer in this group to start compositing with
        KisLayerSP child = layer->lastChild();
        KisLayerSP startWith = layer->lastChild();
        KisAdjustmentLayerSP adjLayer = 0;

        // Look through all the layer, searching for the first dirty layer
        // if it's found, and if we have an adj. layer, composite from the
        // first adjustment layer searching back from the first dirty layer
        while (child) {
            KisAdjustmentLayerSP tmpAdjLayer = dynamic_cast<KisAdjustmentLayer*>(child.data());
            if (tmpAdjLayer) {

                // If this adjustment layer is dirty, start compositing with the
                // previous adj. layer, if there's one.
                if (tmpAdjLayer->dirty() && adjLayer != 0) {
                    startWith = adjLayer->prevSibling();
                    break;
                }
                else {
                    // This is the first adj. layer that is not dirty -- the perfect starting point
                    adjLayer = tmpAdjLayer;
                }
            }
            else {
                // A non-adjustmentlayer that's dirty; if there's an adjustmentlayer
                // with a cache, we'll start from there.
                if (child->dirty()) {
                    if (adjLayer != 0) {
                        // the first layer on top of the adj. layer
                        startWith = adjLayer->prevSibling();
                    }
                    // break here: if there's no adj layer, we'll start with the layer->lastChild
                    break;
                }
            }
            child = child->prevSibling();
        }

        if (startWith == 0) {
            // The projection is apparently still up to date, but why was it marked dirty, then?
            return true;
        }

        bool first = true; // The first layer in a stack needs special compositing

        // Fill the projection either with the cached data, or erase it.
        KisFillPainter gc(layer->projection());
        if (adjLayer != 0) {
            gc.bitBlt(m_rc.left(), m_rc.top(),
                      COMPOSITE_COPY, adjLayer->cachedPaintDevice(), OPACITY_OPAQUE,
                      m_rc.left(), m_rc.top(), m_rc.width(), m_rc.height());
            first = false;
        }
        else {
            gc.eraseRect(m_rc);
            first = true;
        }
        gc.end();
        
        KisMergeVisitor visitor(m_img, layer->projection(), m_rc);

        child = startWith;

        while(child)
        {
            if(first)
            {
                // Copy the lowest layer rather than compositing it with the background
                // or an empty image. This means the layer's composite op is ignored, 
                // which is consistent with Photoshop and gimp.
                const KisCompositeOp cop = child->compositeOp();
                const bool block = child->signalsBlocked();
                child->blockSignals(true);
                child->setCompositeOp(COMPOSITE_COPY);
                child->blockSignals(block);
                child->accept(visitor);
                child->blockSignals(true);
                child->setCompositeOp(cop);
                child->blockSignals(block);
                first = false;
            }
            else
                child->accept(visitor);

           child = child->prevSibling();
        }

        // If this is the root layer, the entire stack is composited onto the projection of the root layer,
        // else composite the contents of the projection of this group layer
        // onto the projection of the visitor
        if (m_projection != layer->projection()) {
            KisPainter gc2(m_projection);
            gc2.bitBlt(m_rc.left(), m_rc.top(), layer->compositeOp(), layer->projection(), OPACITY_OPAQUE, m_rc.left(),
                      m_rc.top(), m_rc.width(), m_rc.height());
            gc2.end();
        }
        layer->setDirty(false);
        return true;
    }

    virtual bool visit(KisPartLayer* layer)
    {
        if (m_projection == 0) {
            return false;
        }
        if (!layer -> visible())
            return true;

        KisPaintDeviceSP dev(layer -> prepareProjection(m_projection));
        if (!dev)
            return true;

        Q_INT32 sx, sy, dx, dy, w, h;

        QRect rc = dev -> extent() & m_rc;

        sx= rc.left();
        sy = rc.top();
        w = rc.width();
        h = rc.height();
        dx = sx;
        dy = sy;

        KisPainter gc(m_projection);
        gc.bitBlt(dx, dy, layer->compositeOp() , dev, layer->opacity(), sx, sy, w, h);

        return true;
    }

    virtual bool visit(KisAdjustmentLayer* layer)
    {
        Q_ASSERT(m_projection);
        Q_ASSERT(layer->cachedPaintDevice());

        if (m_projection == 0) {
            return true;
        }
        
        if (!layer->visible())
            return true;

        KisFilterConfiguration * cfg = layer->filter();
        if (!cfg) return false;

        kdDebug() << "Filter: " << cfg->name() << "\n" << kdBacktrace() << "\n";
        
        
        KisFilter * f = KisFilterRegistry::instance()->get( cfg->name() );
        if (!f) return false;
        
        KisSelectionSP selection = layer->selection();
        kdDebug() << "Do we have a selection: " << selection << "?\n";

        // Copy of the projection -- use the copy-on-write trick.
        KisPaintDeviceSP tmp = new KisPaintDevice(*m_projection);

        // If there's a selection, only keep the selected bits
        if (selection != 0) {
            tmp->setSelection(selection);
        }
        
        // Filter the temporary paint device -- remember, these are only the selected bits,
        // if there was a selection.
        f->process(tmp, tmp, cfg, m_rc);

        // Copy the filtered bits onto the projection 
        KisPainter gc(m_projection);
        if (selection)
            gc.bltSelection(m_rc.left(), m_rc.top(),
                            COMPOSITE_OVER, tmp, selection, layer->opacity(),
                            m_rc.left(), m_rc.top(), m_rc.width(), m_rc.height());
        else
            gc.bitBlt(m_rc.left(), m_rc.top(),
                      COMPOSITE_OVER, tmp, layer->opacity(),
                      m_rc.left(), m_rc.top(), m_rc.width(), m_rc.height());
        gc.end();

        // Copy the finished projection onto the cache
        gc.begin(layer->cachedPaintDevice());
        gc.bitBlt(m_rc.left(), m_rc.top(),
                  COMPOSITE_COPY, m_projection, OPACITY_OPAQUE,
                  m_rc.left(), m_rc.top(), m_rc.width(), m_rc.height());

        layer->setDirty(false);

        return true;
    }

private:
    KisImageSP m_img;
    KisPainter *m_gc;
    KisPaintDeviceSP m_projection;
    QRect m_rc;
};

#endif // KIS_MERGE_H_

