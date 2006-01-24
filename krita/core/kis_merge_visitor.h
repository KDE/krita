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
#include "kis_paint_device_impl.h"
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
    KisMergeVisitor(KisImageSP img, KisPaintDeviceImplSP projection, const QRect& rc) :
        KisLayerVisitor()
    {
        Q_ASSERT(img);
        Q_ASSERT(projection);

        m_img = img;
        m_projection = projection;
        m_rc = rc;
/*
        m_insertMergedAboveLayer = 0;
        m_haveFoundInsertionPlace = false;
*/
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

/*
            if (!m_haveFoundInsertionPlace) {

                if (m_img -> index(layer) != m_img -> nlayers() - 1) {
                    m_insertMergedAboveLayer = m_img -> layer(m_img -> index(layer) + 1);
                }
                else {
                    m_insertMergedAboveLayer = 0;
                }

                m_haveFoundInsertionPlace = true;
            }
        }


        if (m_removeTest(layer.data())) {
            m_img -> rm(layer);
        }
*/
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
        if (!layer->dirty())
            return true;
        
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
            if(true)//test(child)) // LAYERREMOVE
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

            }
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

        KisPaintDeviceImplSP dev(layer -> prepareProjection(m_projection));
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
        Q_ASSERT(cfg);
        KisFilter * f = KisFilterRegistry::instance()->get( cfg->name() );
        Q_ASSERT(f);
        
        KisSelectionSP selection = layer->selection();
        if (selection != 0) {
            m_projection->setSelection(selection);
        }
        
        // Filter onto the cached paint device

        f->process(m_projection, m_projection, cfg, m_rc);

        KisPainter gc(layer->cachedPaintDevice());

        // Cache the projection
        gc.bitBlt(m_rc.left(), m_rc.top(),
                  COMPOSITE_COPY, m_projection, OPACITY_OPAQUE,
                  m_rc.left(), m_rc.top(), m_rc.width(), m_rc.height());

        m_projection->deselect();
        layer->setDirty(false);
        
        return true;
    }
    
    
/*
    // The layer the merged layer should be inserted above, or 0 if
    // the merged layer should go to the bottom of the stack.
    KisLayerSP insertMergedAboveLayer() const { return m_insertMergedAboveLayer; }
*/
private:
    KisImageSP m_img;
    KisPainter *m_gc;
    KisPaintDeviceImplSP m_projection;
    QRect m_rc;
/*
    KisLayerSP m_insertMergedAboveLayer;
    bool m_haveFoundInsertionPlace;
*/
};

#endif // KIS_MERGE_H_

