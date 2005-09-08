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

#include <kis_brush.h>
#include <kis_global.h>
#include <kis_paint_device_impl.h>
#include <kis_painter.h>
#include <kis_types.h>
#include <kis_paintop.h>
#include <kis_iterators_pixel.h>

#include "kis_wetop.h"
#include "kis_wet_colorspace.h"

KisPaintOp * KisWetOpFactory::createOp(KisPainter * painter)
{
    KisPaintOp * op = new KisWetOp(painter);
    return op;
}

KisWetOp::KisWetOp(KisPainter * painter)
    : super(painter)
{
}

KisWetOp::~KisWetOp()
{
}

void KisWetOp::paintAt(const KisPoint &pos,
             const double pressure,
             const double /*xTilt*/,
             const double /*yTilt*/)
{
    if (!m_painter) return;

    if (!m_painter -> device()) return;
    KisPaintDeviceImplSP device = m_painter -> device();

    int x = pos.floorX(); // XXX subpixel positioning?
    int y = pos.floorY();
    int r = 10; // ### radius afaik, but please make configurable (KisBrush or so?)
    kdDebug(DBG_AREA_CMS) << pressure << endl;

    KisAbstractColorSpace * cs = device -> colorSpace();

    if (cs -> id() != KisID("WET","")) {
        kdDebug(DBG_AREA_CMS) << "You cannot paint wet paint on dry pixels.\n";
        return;
    }

    KisColor paintColor = m_painter -> paintColor();
    paintColor.convertTo(cs);
    // hopefully this does
    // nothing, conversions are bad ( wet -> rgb -> wet gives horrible mismatches, due to
    // the conversion to rgb actually rendering the paint above white

    WetPack* paintPack = reinterpret_cast<WetPack*>(paintColor.data());
    WetPix paint = paintPack -> paint;
    
    // Get the paint info (we store the strength in the otherwise unused (?) height field of
    // the paint
    double wetness = paint.w;
    // strength is a double in the 0 - 2 range, but upscaled to Q_UINT16:
    double strength = 2.0 * static_cast<double>(paint.h) / (double)(0xffff);
    strength  = strength * (strength + pressure) * 0.5;

    // Maybe it wouldn't be a bad idea to use a KisBrush in some way here
    double r_fringe;
    int x0, y0;
    int x1, y1;
    WetPack currentPack;
    WetPix currentPix;
    int xp, yp;
    double xx, yy, rr;
    double eff_height;
    double press, contact;

    r_fringe = r + 1;
    x0 = floor(x - r_fringe);
    y0 = floor(y - r_fringe);
    x1 = ceil(x + r_fringe);
    y1 = ceil(y + r_fringe);

    for (yp = y0; yp < y1; yp++) {
        yy = (yp + 0.5 - y);
        yy *= yy;
        for (xp = x0; xp < x1; xp++) {
            // XXX this only does something with .paint, and not with adsorb I assume?
            KisHLineIteratorPixel it = device -> createHLineIterator(xp, yp, 1, true);
            currentPack = *(reinterpret_cast<WetPack*>(it.rawData()));
            WetPix currentData = currentPack.adsorb;
            currentPix = currentPack.paint;

            xx = (xp + 0.5 - x);
            xx *= xx;
            rr = yy + xx;
            if (rr < r * r) {
                press = pressure * 0.25;
            } else {
                press = -1;
            }

            // XXX - 192 is probably only useful for paper with a texture...
            eff_height = (currentData.h + currentPix.w - 192) * (1.0 / 255);
            contact = (press + eff_height) * 0.2;
            if (contact > 0.5)
                contact = 1 - 0.5 * exp(-2.0 * contact - 1);
            if (contact > 0.0001) {
                int v;
                double rnd = rand() * (1.0 / RAND_MAX);

                v = currentPix.rd;
                currentPix.rd = floor(v + (paint.rd * strength - v) * contact + rnd);
                v = currentPix.rw;
                currentPix.rw = floor(v + (paint.rw * strength - v) * contact + rnd);
                v = currentPix.gd;
                currentPix.gd = floor(v + (paint.gd * strength - v) * contact + rnd);
                v = currentPix.gw;
                currentPix.gw = floor(v + (paint.gw * strength - v) * contact + rnd);
                v = currentPix.bd;
                currentPix.bd = floor(v + (paint.bd * strength - v) * contact + rnd);
                v = currentPix.bw;
                currentPix.bw = floor(v + (paint.bw * strength - v) * contact + rnd);
                v = currentPix.w;
                currentPix.w = floor(v + (paint.w - v) * contact + rnd);

                currentPack.paint = currentPix;
                *(reinterpret_cast<WetPack*>(it.rawData())) = currentPack;
            }
        }
    }
    
    m_painter -> addDirtyRect(QRect(x0, y0, x1 - x0, y1 - y0));
}
