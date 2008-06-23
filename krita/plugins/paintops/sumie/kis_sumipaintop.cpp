/*
 *  Copyright (c) 2008 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_sumipaintop.h"

#include <QRect>

#include <KoColor.h>

#include <kis_image.h>
#include <kis_debug.h>

#include "kis_brush.h"
#include "kis_global.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_types.h"
#include "kis_paintop.h"
#include "kis_selection.h"

#include "kis_datamanager.h"

KisPaintOp * KisSumiPaintOpFactory::createOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisImageSP image)
{
    Q_UNUSED( settings );
    Q_UNUSED( image );
    KisPaintOp * op = new KisSumiPaintOp(painter);
    Q_CHECK_PTR(op);
    return op;
}


KisSumiPaintOp::KisSumiPaintOp(KisPainter * painter)
    : KisPaintOp(painter)
{
}

KisSumiPaintOp::~KisSumiPaintOp()
{
}

void KisSumiPaintOp::paintAt(const KisPaintInformation& info)
{
    if (!painter()) return;

    KisPaintDeviceSP device = painter()->device();

    if (!device) return;

    KisBrush * brush = painter()->brush();
    if (! brush->canPaintFor(info) )
        return;
    KisPaintDeviceSP dab = 0;

    double scale = KisPaintOp::scaleForPressure( info.pressure() );
    QPointF hotSpot = brush->hotSpot(scale, scale);
    QPointF pt = info.pos() - hotSpot;

    qint32 x;
    double xFraction;
    qint32 y;
    double yFraction;

    splitCoordinate(pt.x(), &x, &xFraction);
    splitCoordinate(pt.y(), &y, &yFraction);

    if (brush->brushType() == IMAGE || brush->brushType() == PIPE_IMAGE) {
        dab = brush->image(device->colorSpace(), scale, 0.0 , info, xFraction, yFraction);
    }
    else {
        dab = cachedDab( );
        KoColor color = painter()->paintColor();
        color.convertTo( dab->colorSpace() );
        brush->mask(dab, color, scale, scale, 0.0, info, xFraction, yFraction);
    }

    painter()->setDab(dab); // Cache dab for future paints in the painter.
    painter()->setPressure(info.pressure()); // Cache pressure in the current painter.

    QRect dabRect = QRect(0, 0, brush->maskWidth(scale, 0.0), brush->maskHeight(scale, 0.0));
    QRect dstRect = QRect(x, y, dabRect.width(), dabRect.height());

    if ( painter()->bounds().isValid() ) {
        dstRect &= painter()->bounds();
    }

    if (dstRect.isNull() || dstRect.isEmpty() || !dstRect.isValid()) return;

    qint32 sx = dstRect.x() - x;
    qint32 sy = dstRect.y() - y;
    qint32 sw = dstRect.width();
    qint32 sh = dstRect.height();

    painter()->bltSelection(dstRect.x(), dstRect.y(), painter()->compositeOp(), dab, painter()->opacity(), sx, sy, sw, sh);
}
