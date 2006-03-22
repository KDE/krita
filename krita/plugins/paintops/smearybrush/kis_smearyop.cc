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
#include "kis_paint_device.h"
#include "kis_layer.h"
#include "kis_painter.h"
#include "kis_types.h"
#include "kis_paintop.h"
#include "kis_iterator.h"
#include "kis_iterators_pixel.h"
#include "kis_selection.h"
#include "kis_smearyop.h"
#include "kis_point.h"


const Q_INT32 STARTING_PAINTLOAD = 100;
const Q_INT32 CANVAS_WETNESS = 10;
const Q_INT32 NUMBER_OF_TUFTS = 16;

class KisSmearyOp::SmearyTuft {

public:

    /**
     * Create a new smeary tuft.
     *
     * @param distanceFromCenter how far this tuft is from the center of the brush
     * @param paintload the initial amount of paint this tuft holds. May be replenished
     *        by picking up paint from the canvas
     * @param color the initial paint color. Will change through contact with color on the canvas
     */
    SmearyTuft(Q_INT32 distanceFromCenter, Q_INT32 paintload, KisColor color)
        : m_distanceFromCenter(distanceFromCenter)
        , m_paintload(paintload)
        , m_color(color) {};

    /**
     * Mix the current paint color with the color found
     * at pos in dev.
     */
        void mixAt(const KisPoint & /*pos*/, double /*pressure*/, KisPaintDeviceSP /*dev*/)
    {
        // Get the image background color
        // Get the color at pos
        // if the color at pos is the same as the background color, don't mix
        // else mix the color; the harder the pressure, the more mixed the color will be
        // if the pressure is really hard, take the color from all layers under this layer
        // if there's little paint on the brush, pick up some from the canvas
    };

    /**
     * Paint the tuft footprint (calculated from the pressure) at the given position
     */
    void paintAt(const KisPoint & /*pos*/, double /*pressure*/, KisPaintDeviceSP /*dev*/)
    {
        //
    };

public:
    Q_INT32 m_distanceFromCenter;
    Q_INT32 m_paintload;
    KisColor m_color;

};



KisSmearyOp::KisSmearyOp(KisPainter * painter)
    : super(painter)
{
    for (int i = 0; i < NUMBER_OF_TUFTS / 2; ++i) {
        m_leftTufts.append(new SmearyTuft(i, STARTING_PAINTLOAD, painter->paintColor()));
        m_rightTufts.append(new SmearyTuft(i, STARTING_PAINTLOAD, painter->paintColor()));
    }

}

KisSmearyOp::~KisSmearyOp()
{
}

void KisSmearyOp::paintAt(const KisPoint &pos, const KisPaintInformation& info)
{
    if (!m_painter->device()) return;

    KisBrush *brush = m_painter->brush();

    Q_ASSERT(brush);

    if (!brush) return;

    if (! brush->canPaintFor(info) )
        return;

    KisPaintDeviceSP device = m_painter->device();
    KisColorSpace * colorSpace = device->colorSpace();
    KisColor kc = m_painter->paintColor();
    kc.convertTo(colorSpace);

    KisPoint hotSpot = brush->hotSpot(info);
    KisPoint pt = pos - hotSpot;

    // Split the coordinates into integer plus fractional parts. The integer
    // is where the dab will be positioned and the fractional part determines
    // the sub-pixel positioning.
    Q_INT32 x, y;
    double xFraction, yFraction;

    splitCoordinate(pt.x(), &x, &xFraction);
    splitCoordinate(pt.y(), &y, &yFraction);

    KisPaintDeviceSP dab = new KisPaintDevice(colorSpace, "smeary dab");
    Q_CHECK_PTR(dab);

    m_painter->setPressure(info.pressure);

    // Compute the position of the tufts. The tufts are arranged in a line
    // perpendicular to the motion of the brush, i.e, the straight line between
    // the current position and the previous position.
    // The tufts are spread out through the pressure

    KisPoint previousPoint = info.movement.toKisPoint();
    KisVector2D brushVector(-previousPoint.y(), previousPoint.x());
    KisVector2D currentPointVector = KisVector2D(pos);
    brushVector.normalize();

    KisVector2D vl, vr;

    for (int i = 0; i < (NUMBER_OF_TUFTS / 2); ++i) {
        // Compute the positions on the new vector.
        vl = currentPointVector + i * brushVector;
        KisPoint pl = vl.toKisPoint();
        dab->setPixel(pl.roundX(), pl.roundY(), kc);

        vr = currentPointVector - i * brushVector;
        KisPoint pr = vr.toKisPoint();
        dab->setPixel(pr.roundX(), pr.roundY(), kc);
    }

    vr = vr - vl;
    vr.normalize();

    if (m_source->hasSelection()) {
        m_painter->bltSelection(x - 32, y - 32, m_painter->compositeOp(), dab.data(),
                                m_source->selection(), m_painter->opacity(), x - 32, y -32, 64, 64);
    }
    else {
        m_painter->bitBlt(x - 32, y - 32, m_painter->compositeOp(), dab.data(), m_painter->opacity(), x - 32, y -32, 64, 64);
    }

    m_painter->addDirtyRect(QRect(x -32, y -32, 64, 64));
}


KisPaintOp * KisSmearyOpFactory::createOp(const KisPaintOpSettings */*settings*/, KisPainter * painter)
{
    KisPaintOp * op = new KisSmearyOp(painter);
    Q_CHECK_PTR(op);
    return op;
}
