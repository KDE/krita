/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
#include "kis_paint_device_visitor.h"
#include "kis_painter.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_selection.h"

struct All {
    const bool operator()(const KisPaintDeviceImplSP) const
    {
        return true;
    }
};

struct isVisible {
    const bool operator()(const KisPaintDeviceImplSP dev) const
    {
        return dev -> visible();
    }
};

struct isLinked {
    const bool operator()(const KisPaintDeviceImplSP dev) const
    {
        const KisLayer *layer = dynamic_cast<const KisLayer*>(dev.data());

        return layer && layer -> linked();
    }
};

template <typename merge_cond_t, typename remove_cond_t>
class KisMerge : public KisPaintDeviceImplVisitor {
public:
    KisMerge(KisImageSP img)
    {
        m_img = img;
        m_insertMergedAboveLayer = 0;
        m_haveFoundInsertionPlace = false;
    }

public:
    virtual bool visit(KisPainter&, KisPaintDeviceImplSP)
    {
        return false;
    }

    virtual bool visit(KisPainter&, vKisPaintDeviceImplSP&)
    {
        return false;
    }

    virtual bool visit(KisPainter& gc, vKisLayerSP& layers)
    {
         for (Q_INT32 i = layers.size() - 1; i >= 0; i--) {
            KisLayerSP& layer = layers[i];
            visit(gc, layer);
        }

        return true;
    }

    virtual bool visit(KisPainter& gc, KisLayerSP layer)
    {
        if (m_img -> index(layer) < 0)
            return false;

        if (m_mergeTest(layer.data())) {
            Q_INT32 sx, sy, dx, dy, w, h;

            layer -> extent(sx,sy,w,h);
            dx = sx;
            dy = sy;
            
            gc.bitBlt(dx, dy, layer -> compositeOp() , layer.data(), layer -> opacity(), sx, sy, w, h);

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

        return true;
    }



    virtual bool visit(KisPainter&, KisSelectionSP)
    {
        return false;
    }

    // The layer the merged layer should be inserted above, or 0 if
    // the merged layer should go to the bottom of the stack.
    KisLayerSP insertMergedAboveLayer() const { return m_insertMergedAboveLayer; }

private:
    KisImageSP m_img;
    merge_cond_t m_mergeTest;
    remove_cond_t m_removeTest;
    QRect m_rc;
    KisLayerSP m_insertMergedAboveLayer;
    bool m_haveFoundInsertionPlace;
};

#endif // KIS_MERGE_H_

