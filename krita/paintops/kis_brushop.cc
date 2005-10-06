/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#include <qrect.h>

#include <kdebug.h>

#include "kis_brush.h"
#include "kis_global.h"
#include "kis_paint_device_impl.h"
#include "kis_layer.h"
#include "kis_painter.h"
#include "kis_types.h"
#include "kis_paintop.h"

#include "kis_brushop.h"

KisPaintOp * KisBrushOpFactory::createOp(KisPainter * painter)
{
    KisPaintOp * op = new KisBrushOp(painter);
    Q_CHECK_PTR(op);
    return op;
}

KisBrushOp::KisBrushOp(KisPainter * painter)
    : super(painter)
{
}

KisBrushOp::~KisBrushOp()
{
}

void KisBrushOp::paintAt(const KisPoint &pos, const KisPaintInformation& info)
{
    // Painting should be implemented according to the following algorithm:
    // retrieve brush
    // if brush == mask
    //          retrieve mask
    // else if brush == image
    //          retrieve image
    // subsample (mask | image) for position -- pos should be double!
    // apply filters to mask (colour | gradient | pattern | etc.
    // composite filtered mask into temporary layer
    // composite temporary layer into target layer
    // @see: doc/brush.txt

    if (!m_painter -> device()) return;

    KisBrush *brush = m_painter -> brush();
    
    Q_ASSERT(brush);
    if (!brush) return;
    if (! brush -> canPaintFor(info) )
        return;
    
    KisPaintDeviceImplSP device = m_painter -> device();

    KisPoint hotSpot = brush -> hotSpot(info);
    KisPoint pt = pos - hotSpot;

    // Split the coordinates into integer plus fractional parts. The integer
    // is where the dab will be positioned and the fractional part determines
    // the sub-pixel positioning.
    Q_INT32 x;
    double xFraction;
    Q_INT32 y;
    double yFraction;

    splitCoordinate(pt.x(), &x, &xFraction);
    splitCoordinate(pt.y(), &y, &yFraction);

    KisLayerSP dab = 0;

    if (brush -> brushType() == IMAGE || brush -> brushType() == PIPE_IMAGE) {
        dab = brush -> image(device -> colorSpace(), info, xFraction, yFraction);
    }
    else {
        KisAlphaMaskSP mask = brush -> mask(info, xFraction, yFraction);
        dab = computeDab(mask);
    }
    m_painter -> setPressure(info.pressure);

    QRect dabRect = QRect(0, 0, brush -> maskWidth(info), brush -> maskHeight(info));
    QRect dstRect = QRect(x, y, dabRect.width(), dabRect.height());

    KisImage * image = device -> image();
    
    if (image != 0) {
        dstRect &= image -> bounds();
    }
    
    if (dstRect.isNull() || dstRect.isEmpty() || !dstRect.isValid()) return;

    Q_INT32 sx = dstRect.x() - x;
    Q_INT32 sy = dstRect.y() - y;
    Q_INT32 sw = dstRect.width();
    Q_INT32 sh = dstRect.height();

    m_painter -> bltSelection(dstRect.x(), dstRect.y(), m_painter -> compositeOp(), dab.data(), m_painter -> opacity(), sx, sy, sw, sh);
    m_painter -> addDirtyRect(dstRect);
}
