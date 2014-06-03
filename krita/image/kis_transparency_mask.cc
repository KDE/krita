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

#include <KoIcon.h>
#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoCompositeOpRegistry.h>
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_node_visitor.h"
#include "kis_processing_visitor.h"


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
        KisPainter gc(dst);
        gc.setCompositeOp(src->colorSpace()->compositeOp(COMPOSITE_COPY));
        gc.bitBlt(rc.topLeft(), src, rc);
        src->fill(rc, KoColor(Qt::transparent, src->colorSpace()));
    }

    return rc;
}

QRect KisTransparencyMask::extent() const
{
    return parent() ? parent()->extent() : QRect();
}

QRect KisTransparencyMask::exactBounds() const
{
    return parent() ? parent()->exactBounds() : QRect();
}

QRect KisTransparencyMask::changeRect(const QRect &rect, PositionToFilthy pos) const
{
    /**
     * Selection on transparency masks have no special meaning:
     * They do crop both: change and need area
     */
    return KisMask::changeRect(rect, pos);
}

QRect KisTransparencyMask::needRect(const QRect &rect, PositionToFilthy pos) const
{
    /**
     * Selection on transparency masks have no special meaning:
     * They do crop both: change and need area
     */
    return KisMask::needRect(rect, pos);
}

QIcon KisTransparencyMask::icon() const
{
    return koIcon("view-filter");
}

bool KisTransparencyMask::accept(KisNodeVisitor &v)
{
    return v.visit(this);
}

void KisTransparencyMask::accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter)
{
    return visitor.visit(this, undoAdapter);
}

#include "kis_transparency_mask.moc"
