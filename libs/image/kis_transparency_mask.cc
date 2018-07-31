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
#include <kis_icon.h>
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

QRect KisTransparencyMask::decorateRect(KisPaintDeviceSP &src,
                                        KisPaintDeviceSP &dst,
                                        const QRect & rc,
                                        PositionToFilthy maskPos) const
{
    Q_UNUSED(maskPos);

    if (src != dst) {
        KisPainter::copyAreaOptimized(rc.topLeft(), src, dst, rc);
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

bool KisTransparencyMask::paintsOutsideSelection() const
{
    return true;
}

QIcon KisTransparencyMask::icon() const
{
    return KisIconUtils::loadIcon("transparencyMask");
}

bool KisTransparencyMask::accept(KisNodeVisitor &v)
{
    return v.visit(this);
}

void KisTransparencyMask::accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter)
{
    return visitor.visit(this, undoAdapter);
}

