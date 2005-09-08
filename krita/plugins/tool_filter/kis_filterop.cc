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
#include "kis_painter.h"
#include "kis_types.h"
#include "kis_iterators_pixel.h"
#include "kis_paintop.h"

#include "kis_filterop.h"


KisPaintOp * KisFilterOpFactory::createOp(KisPainter * painter)
{
    KisPaintOp * op = new KisFilterOp(painter);
    Q_CHECK_PTR(op);
    return op;
}


KisFilterOp::KisFilterOp(KisPainter * painter)
    : super(painter)
{
}

KisFilterOp::~KisFilterOp()
{
}

void KisFilterOp::paintAt(const KisPoint &pos,
              const double pressure,
              const double /*xTilt*/,
              const double /*yTilt*/)
{
    if (!m_painter) return;

    KisFilterSP filter = m_painter -> filter();
    if (!filter) return;

    if ( ! m_source ) return;

    KisBrush * brush = m_painter -> brush();
    if (!brush) return;

    KisAbstractColorSpace * colorSpace = m_source -> colorSpace();

    KisPoint hotSpot = brush -> hotSpot(pressure);
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

    // Filters always work with a mask, never with an image; that
    // wouldn't be useful at all.
    KisAlphaMaskSP mask = brush -> mask(pressure, xFraction, yFraction);

    m_painter -> setPressure(pressure);

    Q_INT32 maskWidth = mask -> width();
    Q_INT32 maskHeight = mask -> height();

    // Create a temporary paint device
    KisPaintDeviceImplSP tmpDev = new KisPaintDeviceImpl(colorSpace, "temp");
    Q_CHECK_PTR(tmpDev);

    // Copy the layer data onto the new paint device

    KisPainter p( tmpDev );
    p.bitBlt( 0,  0,  COMPOSITE_COPY, m_source, OPACITY_OPAQUE, x, y, maskWidth, maskHeight );

    // Filter the paint device
    filter -> disableProgress();
    filter -> process( tmpDev,  tmpDev, m_filterConfiguration, QRect( 0, 0, maskWidth, maskHeight ));
    filter -> enableProgress();

    // Apply the mask on the paint device (filter before mask because edge pixels may be important)

    for (int y = 0; y < maskHeight; y++)
    {
        KisHLineIteratorPixel hiter = tmpDev->createHLineIterator(0, y, maskWidth, false);
        int x=0;
        while(! hiter.isDone())
        {
            // XXX: QUANTUM should be Q_UINT8
            QUANTUM alpha = mask -> alphaAt( x++, y );
            KisPixel p = colorSpace -> toKisPixel( hiter.rawData(), 0);
            p.alpha() = alpha;

            ++hiter;
        }
    }

    // Blit the paint device onto the layer

    QRect dabRect = QRect(0, 0, maskWidth, maskHeight);
    QRect dstRect = QRect(x, y, dabRect.width(), dabRect.height());

    KisImage * image = m_painter -> device() -> image();

    if (image != 0) {
        dstRect &= image -> bounds();
    }

    if (dstRect.isNull() || dstRect.isEmpty() || !dstRect.isValid()) return;

    Q_INT32 sx = dstRect.x() - x;
    Q_INT32 sy = dstRect.y() - y;
    Q_INT32 sw = dstRect.width();
    Q_INT32 sh = dstRect.height();

    m_painter -> bltSelection(dstRect.x(), dstRect.y(), m_painter -> compositeOp(), tmpDev, m_painter -> opacity(), sx, sy, sw, sh);
    m_painter -> addDirtyRect(dstRect);
}
