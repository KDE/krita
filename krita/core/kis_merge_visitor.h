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
#include "kis_adjustment_layer.h"
#include "kis_paint_layer.h"
#include "kis_part_layer_iface.h"
#include "kis_filter.h"
#include "kis_filter_configuration.h"
#include "kis_filter_registry.h"
#include "kis_selection.h"

class KisMergeVisitor : public KisLayerVisitor {
public:
    /**
     * Don't even _think_ of creating a merge visitor without a projection; without a projection,
     * the adjustmentlayers won't work.
     */
    KisMergeVisitor(KisPaintDeviceSP projection, const QRect& rc) :
        KisLayerVisitor()
    {
        Q_ASSERT(projection);

        m_projection = projection;
        m_rc = rc;
    }

public:
    virtual bool visit(KisPaintLayer *layer)
    {
        
        if (m_projection.isNull()) {
            return false;
        }
        
        kDebug(41010) << "Visiting on paint layer " << layer->name() << ", visible: " << layer->visible()
                << ", temporary: " << layer->temporary() << ", extent: "
                << layer->extent() << ", dirty: " << layer->dirtyRect() << ", paint rect: " << m_rc << endl;
        if (!layer->visible())
            return true;

        qint32 sx, sy, dx, dy, w, h;

        QRect rc = layer->paintDevice()->extent() & m_rc;

        sx = rc.left();
        sy = rc.top();
        w  = rc.width();
        h  = rc.height();
        dx = sx;
        dy = sy;

        KisPainter gc(m_projection);
        gc.bitBlt(dx, dy, layer->compositeOp(), layer->paintDevice(), layer->opacity(), sx, sy, w, h);

        layer->setClean( rc );
        return true;
    }

    virtual bool visit(KisGroupLayer *layer)
    {
        
        if (m_projection.isNull()) {
            return false;
        }
        
        kDebug(41010) << "Visiting on group layer " << layer->name() << ", visible: " << layer->visible() << ", extent: "
                << layer->extent() << ", dirty: " << layer->dirtyRect() << ", paint rect: " << m_rc << endl;
                
        if (!layer->visible())
            return true;
        
        qint32 sx, sy, dx, dy, w, h;

        // This automatically makes sure the projection is up-to-date for the specified rect.
        KisPaintDeviceSP dev = layer->projection(m_rc);
        QRect rc = dev->extent() & m_rc;

        sx = rc.left();
        sy = rc.top();
        w  = rc.width();
        h  = rc.height();
        dx = sx;
        dy = sy;

        KisPainter gc(m_projection);
        gc.bitBlt(dx, dy, layer->compositeOp(), dev, layer->opacity(), sx, sy, w, h);

        return true;
    }

    virtual bool visit(KisPartLayer* layer)
    {

        kDebug(41010) << "Visiting on part layer " << layer->name() << ", visible: " << layer->visible() << ", extent: "
                << layer->extent() << ", dirty: " << layer->dirtyRect() << ", paint rect: " << m_rc << endl;
        
        if (m_projection.isNull()) {
            return false;
        }
        if (!layer->visible())
            return true;

        KisPaintDeviceSP dev(layer->prepareProjection(m_projection, m_rc));
        if (!dev)
            return true;

        qint32 sx, sy, dx, dy, w, h;

        QRect rc = dev->extent() & m_rc;

        sx= rc.left();
        sy = rc.top();
        w = rc.width();
        h = rc.height();
        dx = sx;
        dy = sy;

        KisPainter gc(m_projection);
        gc.bitBlt(dx, dy, layer->compositeOp() , dev, layer->opacity(), sx, sy, w, h);

        layer->setClean(rc);
        return true;
    }

    virtual bool visit(KisAdjustmentLayer* layer)
    {
        kDebug(41010) << "Visiting on adjustment layer " << layer->name() << ", visible: " << layer->visible() << ", extent: "
                << layer->extent() << ", dirty: " << layer->dirtyRect() << ", paint rect: " << m_rc << endl;
        
        if (m_projection.isNull()) {
            return true;
        }
        
        if (!layer->visible())
            return true;

        KisFilterConfiguration * cfg = layer->filter();
        if (!cfg) return false;

        
        KisFilterSP f = KisFilterRegistry::instance()->get( cfg->name() );
        if (!f) return false;
        
        KisSelectionSP selection = layer->selection();

        // Copy of the projection -- use the copy-on-write trick.
        KisPaintDeviceSP tmp = KisPaintDeviceSP(new KisPaintDevice(*m_projection));

        // If there's a selection, only keep the selected bits
        if (!selection.isNull()) {
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

        layer->setClean(m_rc);

        return true;
    }

private:
    KisPaintDeviceSP m_projection;
    QRect m_rc;
};

#endif // KIS_MERGE_H_

