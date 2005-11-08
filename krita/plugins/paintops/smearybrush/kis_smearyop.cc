/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_iterator.h"
#include "kis_iterators_pixel.h"
#include "kis_smearyop.h"
#include "kis_point.h"


const Q_INT32 STARTING_PAINTLOAD = 100;
const Q_INT32 CANVAS_WETNESS = 10;
const Q_INT32 NUMBER_OF_TUFTS = 16;

class SmearyTuft {

public:

    KisPoint m_previousPos;
    Q_INT32 m_paintLoad;
    KisColor m_color;

};

/**
 * The smeary brush implements bidirectional paint transfer. It takes the
 * color at the footprint of the brush (unless it's the image color or
 * transparent and mixes it with the paint color, creating a new paint
 * color.
 *
 * A brush contains a number of tufts. Depending on pressure, the tufts
 * will be more or less concentrated around the paint position. Tufts
 * mix with the colour under each tuft and load the tuft with the mixture
 * for the next paint operation. The mixture is also dependent upon pressure.
 *
 * The paint load will run out after a certain number of paintAt's, depending
 * on pressure.
 */
KisPaintOp * KisSmearyOpFactory::createOp(KisPainter * painter)
{
    KisPaintOp * op = new KisSmearyOp(painter);
    Q_CHECK_PTR(op);
    return op;
}

KisSmearyOp::KisSmearyOp(KisPainter * painter)
    : super(painter)
{
}

KisSmearyOp::~KisSmearyOp()
{
}

void KisSmearyOp::paintAt(const KisPoint &pos, const KisPaintInformation& info)
{
    if (!m_painter -> device()) return;

    KisBrush *brush = m_painter -> brush();

    Q_ASSERT(brush);

    if (!brush) return;

    if (! brush -> canPaintFor(info) )
        return;

    KisPaintDeviceImplSP device = m_painter -> device();
    KisColorSpace * colorSpace = device -> colorSpace();
    Q_INT32 pixelSize = colorSpace->pixelSize();
    KisColor kc = m_painter -> paintColor();
    kc.convertTo(colorSpace);


    m_painter -> setPressure(info.pressure);

    //kdDebug() << " Previous point: " << info.movement << "\n";

    // Compute the position of the tufts. The tufts are arranged in a line
    // perpendicular to the motion of the brush, i.e, the straight line between
    // the current position and the previous position.
    // The tufts are spread out through the pressure
    KisPoint previousPoint = info.movement.toKisPoint();
    KisVector2D brushVector(-previousPoint.y(), previousPoint.x());
    KisVector2D currentPointVector = KisVector2D(pos);
    brushVector.normalize();
    
    for (int i = 0; i < (NUMBER_OF_TUFTS / 2); ++i) {
        // Compute the positions on the new vector...
        KisVector2D vl = currentPointVector + i * brushVector;
        KisVector2D vr = currentPointVector - i * brushVector;
        KisPoint pl = vl.toKisPoint();
        device->setPixel(pl.roundX(), pl.roundY(), Qt::red, OPACITY_OPAQUE);
        KisPoint pr = vr.toKisPoint();
        device->setPixel(pr.roundX(), pr.roundY(), Qt::red, OPACITY_OPAQUE);
    }
    


//    QRect dabRect = QRect(0, 0, brush -> maskWidth(info), brush -> maskHeight(info));
//    QRect dstRect = QRect(x, y, dabRect.width(), dabRect.height());
#if 0
    KisImage * image = device -> image();

    if (image != 0) {
        dstRect &= image -> bounds();
    }

    if (dstRect.isNull() || dstRect.isEmpty() || !dstRect.isValid()) return;

    Q_INT32 sx = dstRect.x() - x;
    Q_INT32 sy = dstRect.y() - y;
    Q_INT32 sw = dstRect.width();
    Q_INT32 sh = dstRect.height();

    //m_painter -> bltSelection(dstRect.x(), dstRect.y(), m_painter -> compositeOp(), dab.data(), m_painter -> opacity(), sx, sy, sw, sh);
    m_painter -> addDirtyRect(dstRect);
#endif    
}


