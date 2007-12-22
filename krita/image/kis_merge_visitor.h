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

#include <QRect>

#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_node_visitor.h"
#include "kis_painter.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_external_layer_iface.h"
#include "kis_paint_layer.h"
#include "kis_filter.h"
#include "kis_filter_configuration.h"
#include "kis_filter_registry.h"
#include "kis_selection.h"
#include "kis_transaction.h"
#include "kis_iterators_pixel.h"
#include "kis_clone_layer.h"
#include "kis_filter_processing_information.h"

class KisMergeVisitor : public KisNodeVisitor {
public:

    using KisNodeVisitor::visit;

    /**
     * Don't even _think_ of creating a merge visitor without a projection; without a projection,
     * the adjustmentlayers won't work.
     */
    KisMergeVisitor(KisPaintDeviceSP projection, const QRect& rc) :
        KisNodeVisitor()
        {
            Q_ASSERT(projection);
            m_projection = projection;
            m_rc = rc;
        }

public:

    bool visit( KisExternalLayer * layer )
        {
            if (m_projection.isNull()) {
                return false;
            }
            if (!layer->visible())
                return true;

            layer->updateProjection( m_rc );
            KisPaintDeviceSP dev = layer->projection();
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
            gc.setChannelFlags( layer->channelFlags() );
            gc.bitBlt(dx, dy, layer->compositeOp() , dev, layer->opacity(), sx, sy, w, h);

            layer->setClean( rc );

            return true;
        }

    bool visit(KisPaintLayer *layer)
        {

            if (m_projection.isNull()) {
                return false;
            }

            if (!layer->visible())
                return true;

            bool first = false;
            if (layer->prevSibling() == 0 and layer->parent() == layer->image()->root())
                first = true;

            qint32 sx, sy, dx, dy, w, h;

            QRect rc = layer->paintDevice()->extent() & m_rc;

            // Indirect painting?
            KisPaintDeviceSP tempTarget = layer->temporaryTarget();
            if (tempTarget) {
                rc = (layer->projection()->extent() | tempTarget->extent()) & m_rc;
            }

            sx = rc.left();
            sy = rc.top();
            w  = rc.width();
            h  = rc.height();
            dx = sx;
            dy = sy;

            KisPainter gc(m_projection);
            gc.setChannelFlags( layer->channelFlags() );

            layer->updateProjection(m_rc);
            KisPaintDeviceSP source = layer->projection();

            if (tempTarget) {
                KisPaintDeviceSP temp = new KisPaintDevice(source->colorSpace());
                source = paintIndirect(source, temp, layer, sx, sy, dx, dy, w, h);
            }

            if (first)
                gc.bitBlt(dx, dy, layer->colorSpace()->compositeOp(COMPOSITE_COPY), source, layer->opacity(), sx, sy, w, h);
            else
                gc.bitBlt(dx, dy, layer->compositeOp(), source, layer->opacity(), sx, sy, w, h);
                

            layer->setClean( rc );

            return true;
        }

    bool visit(KisGroupLayer *layer)
        {

            if (m_projection.isNull()) {
                return false;
            }

            if (!layer->visible())
                return true;

            qint32 sx, sy, dx, dy, w, h;

            layer->updateProjection( m_rc );
            KisPaintDeviceSP dev = layer->projection();

            QRect rc = dev->extent() & m_rc;

            sx = rc.left();
            sy = rc.top();
            w  = rc.width();
            h  = rc.height();
            dx = sx;
            dy = sy;

            KisPainter gc(m_projection);
            gc.setChannelFlags( layer->channelFlags() );
            gc.bitBlt(dx, dy, layer->compositeOp(), dev, layer->opacity(), sx, sy, w, h);

            layer->setClean( rc );

            return true;
        }

    bool visit(KisAdjustmentLayer* layer)
        {
            if (m_projection.isNull()) {
                return true;
            }

            if (!layer->visible())
                return true;

            QRect tmpRc = m_rc;
            
            KisPaintDeviceSP tempTarget = layer->temporaryTarget();
            kDebug() << "tempTarget: " << tempTarget;
            if (tempTarget) {
                tmpRc = (layer->extent() | tempTarget->extent()) & tmpRc;
            }

            if (tmpRc.width() == 0 || tmpRc.height() == 0) // Don't even try
                return true;

            kDebug() << "Filtering on " << tmpRc;

            KisFilterConfiguration * cfg = layer->filter();
            if (!cfg) return false;

            KisFilterSP f = KisFilterRegistry::instance()->value( cfg->name() );
            if (!f) return false;

            // Possibly enlarge the rect that changed (like for convolution filters)
            tmpRc = f->enlargeRect(tmpRc, cfg);

            KisSelectionSP selection = layer->selection();
            KisPaintDeviceSP layerProjection = layer->projection();

            // It's necessary to copy the unselected pixels to the projection cache inside
            // the adjustment layer to make the merge optimization in the grouplayer work.
            KisPainter gc1(layerProjection);
            gc1.setCompositeOp(layerProjection->colorSpace()->compositeOp(COMPOSITE_COPY));
            gc1.bitBlt(m_rc.topLeft(), m_projection, m_rc);
            gc1.end();
            
            KisFilterConstProcessingInformation srcCfg(m_projection, tmpRc .topLeft(), 0);
            KisFilterProcessingInformation dstCfg(layerProjection, tmpRc .topLeft(), 0);

            // Some filters will require usage of oldRawData, which is not available without
            // a transaction!
            KisTransaction* cmd = new KisTransaction("", layerProjection);
            f->process(srcCfg, dstCfg, tmpRc.size(), cfg);
            delete cmd;

            // Copy the filtered bits onto the projection
            KisPainter gc(m_projection);
            if (selection)
                gc.bltSelection(tmpRc.left(), tmpRc.top(),
                                layer->compositeOp(), layerProjection, selection, layer->opacity(),
                                tmpRc.left(), tmpRc.top(), tmpRc.width(), tmpRc.height());
            else
                gc.bitBlt(tmpRc.left(), tmpRc.top(),
                          layer->compositeOp(), layerProjection, layer->opacity(),
                          tmpRc.left(), tmpRc.top(), tmpRc.width(), tmpRc.height());
            gc.end();

            layer->setClean( tmpRc  );

            return true;
        }


    bool visit( KisCloneLayer * layer )
        {

            if (m_projection.isNull()) {
                return false;
            }

            if (!layer->visible())
                return true;

            qint32 sx, sy, dx, dy, w, h;

            layer->updateProjection( m_rc );
            KisPaintDeviceSP dev = layer->projection();

            if ( !dev ) return false;

            QRect rc = dev->extent() & m_rc;

            sx = rc.left();
            sy = rc.top();
            w  = rc.width();
            h  = rc.height();
            dx = sx;
            dy = sy;

            KisPainter gc(m_projection);
            gc.setCompositeOp( layer->compositeOp() );
            gc.setOpacity( layer->opacity() );
            gc.setChannelFlags( layer->channelFlags() );

            gc.bitBlt(rc.topLeft(), dev, rc);

            layer->setClean( rc );

            return true;

        }

private:
    // Helper for the indirect painting
    template<class Target>
    KisSharedPtr<Target> paintIndirect(KisPaintDeviceSP source,
                                       KisSharedPtr<Target> target,
                                       KisIndirectPaintingSupport* layer,
                                       qint32 sx, qint32 sy, qint32 dx, qint32 dy,
                                       qint32 w, qint32 h) {
        KisPainter gc2(target.data());
        gc2.bitBlt(dx, dy, COMPOSITE_COPY, source,
                   OPACITY_OPAQUE, sx, sy, w, h);
        gc2.bitBlt(dx, dy, layer->temporaryCompositeOp(), layer->temporaryTarget(),
                   layer->temporaryOpacity(), sx, sy, w, h);
        gc2.end();
        return target;
    }
    KisPaintDeviceSP m_projection;
    QRect m_rc;
};

#endif // KIS_MERGE_H_

