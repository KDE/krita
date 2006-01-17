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
#include "kis_filter.h"
#include "kis_filter_configuration.h"
#include "kis_filter_registry.h"

class KisMergeVisitor : public KisLayerVisitor {
public:
    KisMergeVisitor(KisImageSP img,KisPainter *gc, const QRect& rc) :
        KisLayerVisitor()
    {
        m_img = img;
        m_gc = gc;
        m_projection = 0; // XXX: Is this the full projection of all groups, or just the projection for the current layer group?
        m_rc = rc;
/*
        m_insertMergedAboveLayer = 0;
        m_haveFoundInsertionPlace = false;
*/
    }

    void setProjection(KisPaintDeviceImplSP proj) { m_projection = proj; }

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
            
        m_gc->bitBlt(dx, dy, layer->compositeOp() , layer->paintDevice(), layer->opacity(), sx, sy, w, h);

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
        return true;
    }

    virtual bool visit(KisGroupLayer *layer)
    {
        if (!layer -> visible())
            return true;

        KisPaintDeviceImplSP dst;
        if (m_projection)
            dst = m_projection;
        else
            dst = new KisPaintDeviceImpl(m_img, m_img->colorSpace());
        KisPainter painter(dst);

        KisMergeVisitor visitor(m_img, &painter, m_rc);
        visitor.setProjection(dst);
        bool first = true;

        KisLayerSP child = layer->lastChild();

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

        if (!m_projection) {
            Q_INT32 sx, sy, dx, dy, w, h;

            QRect rc = dst ->extent() & rc;
            sx= rc.left();
            sy = rc.top();
            w = rc.width();
            h = rc.height();
            dx = sx;
            dy = sy;
            m_gc->bitBlt(dx, dy, layer->compositeOp() , dst, layer->opacity(), sx, sy, w, h);
        }

        return true;
    }

    virtual bool visit(KisPartLayer */*layer*/)
    {
        return true;
    }

    virtual bool visit(KisAdjustmentLayer* layer)
    {
        KisFilterConfiguration * cfg = layer->filter();
        kdDebug() << "Going to do adjustment layer magick! " << cfg->name() << endl;
        KisFilter * f = KisFilterRegistry::instance()->get( cfg->name() );
        if (KisSelectionSP selection = layer->selection())
            KisSelectionSP oldSelection = m_projection->setSelection(selection);
        f->process(m_projection, m_projection, cfg, m_rc);
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

