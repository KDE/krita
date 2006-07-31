/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
 *  Copyright (c) 2006 Bart Coppens <kde@bartcoppens.be>
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
#include "kis_transaction.h"

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
        
        if (m_projection == 0) {
            return false;
        }
        
        kdDebug(41010) << "Visiting on paint layer " << layer->name() << ", visible: " << layer->visible()
                << ", temporary: " << layer->temporary() << ", extent: "
                << layer->extent() << ", dirty: " << layer->dirtyRect() << ", paint rect: " << m_rc << endl;
        if (!layer->visible())
            return true;

        Q_INT32 sx, sy, dx, dy, w, h;

        QRect rc = layer->paintDevice()->extent() & m_rc;

        sx = rc.left();
        sy = rc.top();
        w  = rc.width();
        h  = rc.height();
        dx = sx;
        dy = sy;

        KisPainter gc(m_projection);

        if (!layer->hasMask()) {
            gc.bitBlt(dx, dy, layer->compositeOp(), layer->paintDevice(), layer->opacity(), sx, sy, w, h);
        } else {
            if (layer->renderMask()) {
                // To display the mask, we don't do things with composite op and opacity
                // This is like the gimp does it, I guess that's ok?
                gc.bitBlt(dx, dy, COMPOSITE_OVER, layer->getMask(), OPACITY_OPAQUE,
                          sx, sy, w, h);
            } else {
                gc.bltSelection(dx, dy,
                                layer->compositeOp(),
                                layer->paintDevice(),
                                layer->getMaskAsSelection(),
                                layer->opacity(), sx, sy, w, h);
            }
        }

        layer->setClean( rc );
        return true;
    }

    virtual bool visit(KisGroupLayer *layer)
    {
        
        if (m_projection == 0) {
            return false;
        }
        
        kdDebug(41010) << "Visiting on group layer " << layer->name() << ", visible: " << layer->visible() << ", extent: "
                << layer->extent() << ", dirty: " << layer->dirtyRect() << ", paint rect: " << m_rc << endl;
                
        if (!layer->visible())
            return true;
        
        Q_INT32 sx, sy, dx, dy, w, h;

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

        kdDebug(41010) << "Visiting on part layer " << layer->name() << ", visible: " << layer->visible() << ", extent: "
                << layer->extent() << ", dirty: " << layer->dirtyRect() << ", paint rect: " << m_rc << endl;
        
        if (m_projection == 0) {
            return false;
        }
        if (!layer->visible())
            return true;

        KisPaintDeviceSP dev(layer->prepareProjection(m_projection, m_rc));
        if (!dev)
            return true;

        Q_INT32 sx, sy, dx, dy, w, h;

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
        kdDebug(41010) << "Visiting on adjustment layer " << layer->name() << ", visible: " << layer->visible() << ", extent: "
                << layer->extent() << ", dirty: " << layer->dirtyRect() << ", paint rect: " << m_rc << endl;

        if (m_projection == 0) {
            return true;
        }

        if (!layer->visible())
            return true;

        if (m_rc.width() == 0 || m_rc.height() == 0) // Don't even try
            return true;

        KisFilterConfiguration * cfg = layer->filter();
        if (!cfg) return false;


        KisFilter * f = KisFilterRegistry::instance()->get( cfg->name() );
        if (!f) return false;

        // Possibly enlarge the rect that changed (like for convolution filters)
        // m_rc = f->enlargeRect(m_rc, cfg);
        KisSelectionSP selection = layer->selection();

        // Copy of the projection -- use the copy-on-write trick. XXX NO COPY ON WRITE YET =(
        //KisPaintDeviceSP tmp = new KisPaintDevice(*m_projection);
        KisPaintDeviceSP tmp = 0;
        // If there's a selection, only keep the selected bits
        if (selection != 0) {
            tmp = new KisPaintDevice(m_projection->colorSpace());
            KisPainter gc(tmp);
            QRect selectedRect = selection->selectedRect();
            selectedRect &= m_rc;

            if (selectedRect.width() == 0 || selectedRect.height() == 0) // Don't even try
                return true;

            // Don't forget that we need to take into account the extended sourcing area as well
            //selectedRect = f->enlargeRect(selectedRect, cfg);

            //kdDebug() << k_funcinfo << selectedRect << endl;
            tmp->setX(selection->getX());
            tmp->setY(selection->getY());
            gc.bitBlt(selectedRect.x(), selectedRect.y(), COMPOSITE_COPY, m_projection,
                      selectedRect.x(), selectedRect.y(),
                      selectedRect.width(), selectedRect.height());
            gc.end();
        } else {
            tmp = new KisPaintDevice(*m_projection);
        }

        // Some filters will require usage of oldRawData, which is not available without
        // a transaction!
        KisTransaction* cmd = new KisTransaction("", tmp);

        // Filter the temporary paint device -- remember, these are only the selected bits,
        // if there was a selection.
        f->process(tmp, tmp, cfg, m_rc);

        delete cmd;
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

