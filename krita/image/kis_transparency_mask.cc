/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_transparency_mask.h"

#include "kis_debug.h"

#include <KoColorSpace.h>
#include <KoCompositeOp.h>
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_node_visitor.h"


KisTransparencyMask::KisTransparencyMask()
        : KisEffectMask()
{
}

KisTransparencyMask::KisTransparencyMask(const KisTransparencyMask& rhs)
        : KisEffectMask(rhs)
{
}

KisTransparencyMask::~KisTransparencyMask()
{
}

bool KisTransparencyMask::allowAsChild(KisNodeSP node) const
{
    Q_UNUSED(node);
    return false;
}

QRect KisTransparencyMask::decorateRect(KisPaintDeviceSP &src,
                                        KisPaintDeviceSP &dst,
                                        const QRect & rc) const
{
    if (src != dst) {
        //dst = new KisPaintDevice(*src);
        KisPainter gc(dst);
        gc.setCompositeOp(src->colorSpace()->compositeOp(COMPOSITE_COPY));
        gc.bitBlt(rc.topLeft(), src, rc);
        src->clear(rc);
    }

    return rc;
}

QRect KisTransparencyMask::changeRect(const QRect &rect) const
{
    /**
     * Selection on transparency masks have special meaning:
     * They do not crop change area, they crop need area.
     * It doesn't need any area outside a selection
     */
    return rect;
}

QRect KisTransparencyMask::needRect(const QRect &rect) const
{
    /**
     * Selection on transparency masks have special meaning:
     * They do not crop change area, they crop need area.
     * It doesn't need any area outside a selection
     */
    return KisMask::needRect(rect);
}

QIcon KisTransparencyMask::icon() const
{
    return KIcon("view-filter");
}

bool KisTransparencyMask::accept(KisNodeVisitor &v)
{
    return v.visit(this);
}


#include "kis_transparency_mask.moc"
