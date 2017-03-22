/*
 *  Copyright (c) 2017 Eugene Ingerman
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

#include "kis_inpaint_mask.h"

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
#include "KoColorSpaceRegistry.h"

KisInpaintMask::KisInpaintMask()
        : KisTransparencyMask()
{
}

KisInpaintMask::KisInpaintMask(const KisInpaintMask& rhs)
        : KisTransparencyMask(rhs)
{
}

KisInpaintMask::~KisInpaintMask()
{
}

QRect KisInpaintMask::decorateRect(KisPaintDeviceSP &src,
                                        KisPaintDeviceSP &dst,
                                        const QRect & rc,
                                        PositionToFilthy maskPos) const
{
    Q_UNUSED(maskPos);
    KIS_ASSERT(dst != src);

    if (src != dst) {
        KisPainter::copyAreaOptimized(rc.topLeft(), src, dst, rc);
        src->fill(rc, KoColor(Qt::magenta, src->colorSpace()));
    }

    return rc;
}



