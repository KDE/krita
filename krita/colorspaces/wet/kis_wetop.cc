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
#include <kis_debug_areas.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_types.h>
#include <kis_paintop.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_meta_registry.h>
#include <kis_colorspace_factory_registry.h>

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

void KisWetOp::paintAt(const KisPoint &pos, const KisPaintInformation& info)
{
    if (!m_painter) return;

    if (!m_painter -> device()) return;
    KisPaintDeviceSP device = m_painter -> device();

    if (!m_painter -> device()) return;

    KisBrush *brush = m_painter -> brush();
    Q_ASSERT(brush);

    if (! brush -> canPaintFor(info) )
        return;

    KisPaintDeviceSP dab = 0;

    if (brush -> brushType() == IMAGE || brush -> brushType() == PIPE_IMAGE) {
        dab = brush -> image(KisMetaRegistry::instance() -> csRegistry() -> getAlpha8(), info);
    }
    else {
        KisAlphaMaskSP mask = brush -> mask(info);
        dab = computeDab(mask, KisMetaRegistry::instance() -> csRegistry() -> getAlpha8());
    }

    KisColorSpace * cs = device -> colorSpace();

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
    strength  = strength * (strength + info.pressure) * 0.5;

    WetPack currentPack;
    WetPix currentPix;
    double eff_height;
    double press, contact;

    int maskW = brush -> maskWidth(info);
    int maskH = brush -> maskHeight(info);
    KoPoint dest = (pos - (brush -> hotSpot(info)));
    int xStart = dest.x();
    int yStart = dest.y();

    for (int y = 0; y < maskH; y++) {
        KisHLineIteratorPixel dabIt = dab -> createHLineIterator(0, y, maskW, false);
        KisHLineIteratorPixel it = device -> createHLineIterator(xStart, yStart+y, maskW, true);

        while (!dabIt.isDone()) {
            // This only does something with .paint, and not with adsorb.
            currentPack = *(reinterpret_cast<WetPack*>(it.rawData()));
            WetPix currentData = currentPack.adsorb;
            currentPix = currentPack.paint;

            // Hardcoded threshold for the dab 'strength': above it, it will get painted
            if (*dabIt.rawData() > 125)
                press = info.pressure * 0.25;
            else
                press = -1;

            // XXX - 192 is probably only useful for paper with a texture...
            eff_height = (currentData.h + currentData.w - 192.0) * (1.0 / 255.0);
            contact = (press + eff_height) * 0.2;
            if (contact > 0.5)
                contact = 1.0 - 0.5 * exp(-2.0 * contact - 1.0);
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
            ++dabIt;
            ++it;
        }
    }

    m_painter -> addDirtyRect(QRect(xStart, yStart, maskW, maskH));
}
