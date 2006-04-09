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

#include "kis_vec.h"
#include "kis_brush.h"
#include "kis_global.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_types.h"
#include "kis_paintop.h"
#include "kis_layer.h"
#include "kis_selection.h"
#include "kis_airbrushop.h"

KisPaintOp * KisAirbrushOpFactory::createOp(const KisPaintOpSettings */*settings*/, KisPainter * painter)
{ 
    KisPaintOp * op = new KisAirbrushOp(painter); 
    Q_CHECK_PTR(op);
    return op; 
}


KisAirbrushOp::KisAirbrushOp(KisPainter * painter)
    : super(painter) 
{
}

KisAirbrushOp::~KisAirbrushOp() 
{
}

void KisAirbrushOp::paintAt(const KisPoint &pos, const KisPaintInformation& info)
{
// See: http://www.sysf.physto.se/~klere/airbrush/ for information
// about _real_ airbrushes.
//
// Most graphics apps -- especially the simple ones like Kolourpaint
// and the previous version of this routine in Krita took a brush
// shape -- often a simple ellipse -- and filled that shape with a
// random 'spray' of single pixels.
//
// Other, more advanced graphics apps, like the Gimp or Photoshop,
// take the brush shape and paint just as with the brush paint op,
// only making the initial dab more transparent, and perhaps adding
// extra transparence near the edges. Then, using a timer, when the
// cursor stays in place, dab upon dab is positioned in the same
// place, which makes the result less and less transparent.
//
// What I want to do here is create an airbrush that approaches a real
// one. It won't use brush shapes, instead going for the old-fashioned
// circle. Depending upon pressure, both the size of the dab and the
// rate of paint deposition is determined. The edges of the dab are
// more transparent than the center, with perhaps even some fully
// transparent pixels between the near-transparent pixels.
//
// By pressing some to-be-determined key at the same time as pressing
// mouse-down, one edge of the dab is made straight, to simulate
// working with a shield.
//
// Tilt may be used to make the gradients more realistic, but I don't
// have a tablet that supports tilt.
//
// Anyway, it's exactly twenty years ago that I have held a real
// airbrush, for the first and up to now the last time...
//

    if (!m_painter) return;

    KisPaintDeviceSP device = m_painter->device();

    // For now: use the current brush shape -- it beats calculating
    // ellipes and cones, and it shows the working of the timer.
    if (!device) return;

    KisBrush * brush = m_painter->brush();
    if (! brush->canPaintFor(info) )
        return;
    KisPaintDeviceSP dab = m_painter->dab();

    KisPoint hotSpot = brush->hotSpot(info);
    KisPoint pt = pos - hotSpot;

    qint32 x;
    double xFraction;
    qint32 y;
    double yFraction;

    splitCoordinate(pt.x(), &x, &xFraction);
    splitCoordinate(pt.y(), &y, &yFraction);

    if (brush->brushType() == IMAGE || brush->brushType() == PIPE_IMAGE) {
        dab = brush->image(device->colorSpace(), info, xFraction, yFraction);
    }
    else {
        KisAlphaMaskSP mask = brush->mask(info, xFraction, yFraction);
        dab = computeDab(mask);
    }

    m_painter->setDab(dab); // Cache dab for future paints in the painter.
    m_painter->setPressure(info.pressure); // Cache pressure in the current painter.

    QRect dabRect = QRect(0, 0, brush->maskWidth(info), brush->maskHeight(info));
    QRect dstRect = QRect(x, y, dabRect.width(), dabRect.height());

    KisImage * image = device->image();

    if (image != 0) {
        dstRect &= image->bounds();
    }

    if (dstRect.isNull() || dstRect.isEmpty() || !dstRect.isValid()) return;

    qint32 sx = dstRect.x() - x;
    qint32 sy = dstRect.y() - y;
    qint32 sw = dstRect.width();
    qint32 sh = dstRect.height();

    if (m_source->hasSelection()) {
        m_painter->bltSelection(dstRect.x(), dstRect.y(), m_painter->compositeOp(), dab,
                                m_source->selection(), m_painter->opacity(), sx, sy, sw, sh);
    }
    else {
        m_painter->bitBlt(dstRect.x(), dstRect.y(), m_painter->compositeOp(), dab, m_painter->opacity(), sx, sy, sw, sh);
    }

    m_painter->addDirtyRect(dstRect);
}
