/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004-2008 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_airbrushop.h"

#include <QRect>

#include <kis_image.h>
#include <kis_debug.h>

#include <KoColorTransformation.h>
#include <KoColor.h>
#include <KoInputDevice.h>

#include <kis_brush.h>
#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_paintop.h>
#include <kis_properties_configuration.h>
#include <kis_selection.h>
#include <kis_brush_option.h>
#include <kis_paintop_options_widget.h>

#include "kis_airbrushop_settings.h"
#include "kis_airbrushop_settings_widget.h"


KisAirbrushOp::KisAirbrushOp(const KisAirbrushOpSettings *settings, KisPainter *painter)
        : KisBrushBasedPaintOp(painter)
        , settings(settings)
{
    Q_ASSERT(settings);
    Q_ASSERT(painter);
    if (settings->m_options) {
        Q_ASSERT(settings->m_options->m_brushOption);
        m_brush = settings->m_options->m_brushOption->brush();
    }
}

KisAirbrushOp::~KisAirbrushOp()
{
}

void KisAirbrushOp::paintAt(const KisPaintInformation& info)
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

    if (!painter()->device()) return;

    KisBrushSP brush = m_brush;
    if (!brush) {
        if (settings->m_options) {
            m_brush = settings->m_options->m_brushOption->brush();
            brush = m_brush;
        } else {
            return;
        }
    }

    if (! brush->canPaintFor(info))
        return;

    KisPaintDeviceSP device = painter()->device();

    // TODO: why is there scale and pScale that seems to contains the same things ?
    QPointF hotSpot = brush->hotSpot(1.0, 1.0);
    QPointF pt = info.pos() - hotSpot;

    // Split the coordinates into integer plus fractional parts. The integer
    // is where the dab will be positioned and the fractional part determines
    // the sub-pixel positioning.
    qint32 x;
    double xFraction;
    qint32 y;
    double yFraction;

    splitCoordinate(pt.x(), &x, &xFraction);
    splitCoordinate(pt.y(), &y, &yFraction);

    KisFixedPaintDeviceSP dab = KisFixedPaintDeviceSP(0);

    QRect dabRect = QRect(0, 0, brush->maskWidth(1.0, 0.0), brush->maskHeight(1.0, 0.0));
    QRect dstRect = QRect(x, y, dabRect.width(), dabRect.height());


    if (painter()->bounds().isValid()) {
        dstRect &= painter()->bounds();
    }

    if (dstRect.isNull() || dstRect.isEmpty() || !dstRect.isValid()) return;

    qint32 sx = dstRect.x() - x;
    qint32 sy = dstRect.y() - y;
    qint32 sw = dstRect.width();
    qint32 sh = dstRect.height();

    if (brush->brushType() == IMAGE || brush->brushType() == PIPE_IMAGE) {
        dab = brush->image(device->colorSpace(), 1.0, 0.0, info, xFraction, yFraction);
    } else {
        dab = cachedDab();
        KoColor color = painter()->paintColor();
        color.convertTo(dab->colorSpace());
        brush->mask(dab, color, 1.0, 1.0, 0.0, info, xFraction, yFraction);
    }

    painter()->bltFixed(dstRect.x(), dstRect.y(), dab, sx, sy, sw, sh);


}

double KisAirbrushOp::paintLine(const KisPaintInformation &pi1,
                                const KisPaintInformation &pi2,
                                double savedDist)
{
    KisPaintInformation adjustedInfo1(pi1);
    KisPaintInformation adjustedInfo2(pi2);
    adjustedInfo1.setPressure(PRESSURE_DEFAULT);
    adjustedInfo2.setPressure(PRESSURE_DEFAULT);
    return KisPaintOp::paintLine(adjustedInfo1, adjustedInfo2, savedDist);
}
