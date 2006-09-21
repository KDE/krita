/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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
#include <stdlib.h>
#include <vector>

#include <klocale.h>

#include <kis_iterators_pixel.h>
#include <kis_filter_registry.h>
#include <kis_types.h>
#include <kis_paint_device.h>
#include "wetphysicsfilter.h"


WetPhysicsFilter::WetPhysicsFilter()
#if 0
    : KisFilter(id(), "artistic", i18n("Dry the Paint (25 times)"))
#endif
    : KisFilter(id(), "artistic", i18n("Dry the Paint"))
{
    m_adsorbCount = 0;
}


void WetPhysicsFilter::process(KisPaintDeviceSP /*src*/, KisPaintDeviceSP dst, KisFilterConfiguration* /*config*/, const QRect& r)
{
    /*
      This is actually a kind of convolution filter. Use the slow but clear way of convolving
      until I get the physics right; then move to the faster way of convolving from the convolution
      painter.
    */

    // Loop through all pixels
    KisHLineIterator topIt = dst->createHLineIterator(r.x(), r.y(), r.width(), true);
    KisHLineIterator midIt = dst->createHLineIterator(r.x() + 1, r.y(), r.width(), true);
    KisHLineIterator botIt = dst->createHLineIterator(r.x() + 2, r.y(), r.width(), true);


    // Old pixel values
    const WetPack * topLeftOld = reinterpret_cast<const WetPack*>(topIt.oldRawData());
    WetPack * topLeft = reinterpret_cast<WetPack*>(topIt.rawData());
    ++topIt;
    const WetPack * topMidOld = reinterpret_cast<const WetPack*>(topIt.oldRawData());
    WetPack * topMid = reinterpret_cast<WetPack*>(topIt.rawData());
    ++topIt;
    const WetPack * topRightOld = reinterpret_cast<const WetPack*>(topIt.oldRawData());
    WetPack * topRight = reinterpret_cast<WetPack*>(topIt.rawData());

    const WetPack * curLeftOld = reinterpret_cast<const WetPack*>(midIt.oldRawData());
    WetPack * curLeft = reinterpret_cast<WetPack*>(midIt.rawData());
    ++midIt;
    const WetPack * currentOld = reinterpret_cast<const WetPack*>(midIt.oldRawData());
    WetPack * current = reinterpret_cast<WetPack*>(midIt.rawData());
    ++midIt;
    const WetPack * curRightOld = reinterpret_cast<const WetPack*>(midIt.oldRawData());
    WetPack * curRight = reinterpret_cast<WetPack*>(midIt.rawData());

    const WetPack * botLeftOld = reinterpret_cast<const WetPack*>(botIt.oldRawData());
    WetPack * botLeft = reinterpret_cast<WetPack*>(botIt.rawData());
    ++botIt;
    const WetPack * botMidOld = reinterpret_cast<const WetPack*>(botIt.oldRawData());
    WetPack * botMid = reinterpret_cast<WetPack*>(botIt.rawData());
    ++botIt;
    const WetPack * botRightOld = reinterpret_cast<const WetPack*>(botIt.oldRawData());
    WetPack * botRight = reinterpret_cast<WetPack*>(botIt.rawData());

    int x = r.x();
    int y = r.y();
    while (y < r.height()) {
        WetPix paint = current->paint;
        WetPix adsorb = current->adsorb;

        // Only wet pixels can attract wetness -- this simulates the boundary
        // between wet and dry areas and the surface tension of waterdrops. I hope.
        if (paint.w > 0) {

            // Dry a little: just subtract 1 from the wetness, if there's still wetness. Whether this is
            // accurate remains to be seen. This simulates drying to the air, not adsorbing wetness into
            // the paper.
            paint.w -= 1;

            // Adsorb: this means that the pigment is glued onto the paper. In real terms: pigment gets
            // transfered to the adsorp layer. We don't yet simulate the reverse, although that's possible
            // with real watercolor paint.
            adsorbPixel(&paint, &adsorb);

            // Flow to the lower parts
            if (paint.h < topLeft->paint.h) {
            }
            if (paint.h < topMid->paint.h) {
            }
            if (paint.h < topRight->paint.h) {
            }
            if (paint.h < curLeft->paint.h) {
            }
            if (paint.h < curRight->paint.h) {
            }
            if (paint.h < botLeft->paint.h) {
            }
            if (paint.h < botMid->paint.h) {
            }
            if (paint.h < botRight->paint.h) {
            }

        }

        ++x;

        if (x == r.width() - r.x()) {
            topIt.nextRow();
            midIt.nextRow();
            botIt.nextRow();
            x = 0;
            ++y;
        }

        ++topIt;
        ++midIt;
        ++botIt;

        topLeftOld = topMidOld;
        topLeft = topMid;
        topMidOld = topRightOld;
        topMid = topRight;
        topRightOld = reinterpret_cast<const WetPack*>(topIt.oldRawData());
        topRight = reinterpret_cast<WetPack*>(topIt.rawData());

        curLeftOld = currentOld;
        curLeft = current;
        currentOld = curRightOld;
        current = curRight;
        curRightOld = reinterpret_cast<const WetPack*>(midIt.oldRawData());
        curRight = reinterpret_cast<WetPack*>(midIt.rawData());

        botLeftOld = botMidOld;
        botLeft = botMid;
        botMidOld = botRightOld;
        botMid = botRight;
        botRightOld = reinterpret_cast<const WetPack*>(botIt.oldRawData());
        botRight = reinterpret_cast<WetPack*>(botIt.rawData());


    }

}

void WetPhysicsFilter::adsorbPixel(WetPix * paint, WetPix * adsorb)
{
    WetPixDbl wet_top;
    WetPixDbl wet_bot;
    double ads;

    ads = 0.5 / qMax(paint->w, (quint16)1);

    wetPixToDouble(&wet_top, paint);
    wetPixToDouble(&wet_bot, adsorb);

    double d1, w1, d2, w2;
    double ed1, ed2;

    if (wet_top.rd < 0.0001) {
        // wet_bot.rd = wet_bot.rd;
        // wet_bot.rw = wet_bot.rw;
    } else if (wet_bot.rd < 0.0001) {
        wet_bot.rd = wet_top.rd * ads;
        wet_bot.rw = wet_top.rw * ads;
    } else {
        d1 = wet_top.rd;
        w1 = wet_top.rw;
        d2 = wet_bot.rd;
        w2 = wet_bot.rw;
        wet_bot.rd = d1 * ads + d2;
        ed1 = exp(-d1 * ads);
        ed2 = exp(-d2);
        wet_bot.rw = wet_bot.rd * ((1 - ed1) * w1 / d1 + ed1 * (1 - ed2) * w2 / d2) / (1 - ed1 * ed2);
    }

    if (wet_top.gd < 0.0001) {
        //wet_bot.gd = wet_bot.gd;
        //wet_bot.gw = wet_bot.gw;
    } else if (wet_bot.gd < 0.0001) {
        wet_bot.gd = wet_top.gd * ads;
        wet_bot.gw = wet_top.gw * ads;
    } else {
        d1 = wet_top.gd;
        w1 = wet_top.gw;
        d2 = wet_bot.gd;
        w2 = wet_bot.gw;
        wet_bot.gd = d1 * ads + d2;
        ed1 = exp(-d1 * ads);
        ed2 = exp(-d2);
        wet_bot.gw = wet_bot.gd * ((1 - ed1) * w1 / d1 + ed1 * (1 - ed2) * w2 / d2) / (1 - ed1 * ed2);
    }

    if (wet_top.bd < 0.0001) {
        //wet_bot.bd = wet_bot.bd;
        //wet_bot.bw = wet_bot.bw;
    } else if (wet_bot.bd < 0.0001) {
        wet_bot.bd = wet_top.bd * ads;
        wet_bot.bw = wet_top.bw * ads;
    } else {
        d1 = wet_top.bd;
        w1 = wet_top.bw;
        d2 = wet_bot.bd;
        w2 = wet_bot.bw;
        wet_bot.bd = d1 * ads + d2;
        ed1 = exp(-d1 * ads);
        ed2 = exp(-d2);
        wet_bot.bw = wet_bot.bd * ((1 - ed1) * w1 / d1 + ed1 * (1 - ed2) * w2 / d2) / (1 - ed1 * ed2);
    }

    wetPixFromDouble(adsorb, &wet_bot);

    paint->rd *= (quint16)(1 - ads);
    paint->rw *= (quint16)(1 - ads);
    paint->gd *= (quint16)(1 - ads);
    paint->gw *= (quint16)(1 - ads);
    paint->bd *= (quint16)(1 - ads);
    paint->bw *= (quint16)(1 - ads);

}
