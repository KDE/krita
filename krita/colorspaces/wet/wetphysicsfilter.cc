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
#include <kdebug.h>

#include <kis_iterators_pixel.h>
#include <kis_filter_registry.h>
#include <kis_debug_areas.h>
#include <kis_types.h>
#include <kis_paint_device.h>
#include <kis_debug_areas.h>
#include "wetphysicsfilter.h"


WetPhysicsFilter::WetPhysicsFilter()
#if 0
    : KisFilter(id(), "artistic", i18n("Dry the Paint (25 times)"))
#endif
    : KisFilter(id(), "artistic", i18n("Dry the Paint"))
{
    m_adsorbCount = 0;
}

void WetPhysicsFilter::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* /*config*/, const QRect& rect)
{
    // XXX: It would be nice be able to interleave this, instead of having the same loop over
    //      our pixels three times.
    //flow(src, dst, rect);
    if (m_adsorbCount++ == 2) {
        // XXX I think we could combine dry and adsorb, yes
        adsorb(src, dst, rect);
        dry(src, dst, rect);
        m_adsorbCount = 0;
    }
}


void WetPhysicsFilter::flow(KisPaintDeviceSP src, KisPaintDeviceSP /*dst*/, const QRect & r)
{
    /* XXX: Is this like a convolution operation? BSAR */
    int width = r.width();
    int height = r.height();

    /* width of a line in a layer in pixel units, not in bytes -- used to move to the next
       line in the fluid masks below */
    int rs = width; // rowstride

    double * flow_t  = new double[width * height];
    Q_CHECK_PTR(flow_t);

    double * flow_b  = new double[width * height];
    Q_CHECK_PTR(flow_b);

    double * flow_l  = new double[width * height];
    Q_CHECK_PTR(flow_l);

    double * flow_r  = new double[width * height];
    Q_CHECK_PTR(flow_r);

    double * fluid   = new double[width * height];
    Q_CHECK_PTR(fluid);

    double * outflow = new double[width * height];
    Q_CHECK_PTR(outflow);

    // Height of the paper surface. Do we also increase height because of paint deposits?
    int my_height;

    // Flow to the top, bottom, left, right of the currentpixel
    double ft, fb, fl, fr;

    // Temporary pixel constructs
    WetPixDbl wet_mix, wet_tmp;

    // XXX If the flow touches areas that have not been initialized with a height field yet,
    // create a heigth field.

    // We need three iterators, because we're working on a five-point convolution kernel (no corner pixels are being used)

    // First iteration: compute fluid deposits around the paper.
    Q_INT32 dx, dy;
    dx = r.x();
    dy = r.y();

    int ix = width + 1; // keeps track where we are in the one-dimensional arrays

    for (Q_INT32 y2 = 1; y2 < height - 1; ++y2) {
        KisHLineIteratorPixel srcIt = src->createHLineIterator(dx, dy + y2, width, false);
        KisHLineIteratorPixel upIt = src->createHLineIterator(dx + 1, dy + y2 - 1, width - 2, false);
        KisHLineIteratorPixel downIt = src->createHLineIterator(dx + 1, dy + y2 + 1, width - 2, false);

        // .paint is the first field in our wetpack, so this is ok (even though not nice)
        WetPix left = *(reinterpret_cast<WetPix*>(srcIt.rawData()));
        ++srcIt;
        WetPix current = *(reinterpret_cast<WetPix*>(srcIt.rawData()));
        ++srcIt;
        WetPix right = *(reinterpret_cast<WetPix*>(srcIt.rawData()));
        WetPix up, down;

        while (!srcIt.isDone()) {
            up = *(reinterpret_cast<WetPix*>(upIt.rawData()));
            down = *(reinterpret_cast<WetPix*>(downIt.rawData()));

            if (current.w > 0) {
                my_height = current.h + current.w;
                ft = (up.h + up.w) - my_height;
                fb = (down.h + down.w) - my_height;
                fl = (left.h + left.w) - my_height;
                fr = (right.h + right.w) - my_height;

                fluid[ix] = 0.4 * sqrt(current.w * 1.0 / 255.0);

                /* smooth out the flow a bit */
                flow_t[ix] = CLAMP(0.1 * (10 + ft * 0.75 - fb * 0.25), 0, 1);

                flow_b[ix] = CLAMP(0.1 * (10 + fb * 0.75 - ft * 0.25), 0, 1);

                flow_l[ix] = CLAMP(0.1 * (10 + fl * 0.75 - fr * 0.25), 0, 1);

                flow_r[ix] = CLAMP(0.1 * (10 + fr * 0.75 - fl * 0.25), 0, 1);

                outflow[ix] = 0;
            }

            ++srcIt;
            ++upIt;
            ++downIt;
            ix++;
            left = current;
            current = right;
            right = *(reinterpret_cast<WetPix*>(srcIt.rawData()));
        }
        ix+=2; // one for the last pixel on the line, and one for the first of the next line
    }
    // Second iteration: Reduce flow in dry areas
    ix = width + 1;

    for (Q_INT32 y2 = 1; y2 < height - 1; ++y2) {
        KisHLineIteratorPixel srcIt = src->createHLineIterator(dx + 1, dy + y2, width - 2, false);
        while (!srcIt.isDone()) {
            if ((reinterpret_cast<WetPix*>(srcIt.rawData()))->w > 0) {
                /* reduce flow in dry areas */
                flow_t[ix] *= fluid[ix] * fluid[ix - rs];
                outflow[ix - rs] += flow_t[ix];
                flow_b[ix] *= fluid[ix] * fluid[ix + rs];
                outflow[ix + rs] += flow_b[ix];
                flow_l[ix] *= fluid[ix] * fluid[ix - 1];
                outflow[ix - 1] += flow_l[ix];
                flow_r[ix] *= fluid[ix] * fluid[ix + 1];
                outflow[ix + 1] += flow_r[ix];
            }
            ++srcIt;
            ix++;
        }
        ix += 2;
    }

    // Third iteration: Combine the paint from the flow areas.
    ix = width + 1;
    for (Q_INT32 y2 = 1; y2 < height - 1; ++y2) {
        KisHLineIteratorPixel srcIt = src->createHLineIterator(dx, dy + y2, width, false);
        KisHLineIteratorPixel upIt = src->createHLineIterator(dx + 1, dy + y2 - 1, width - 2, false);
        KisHLineIteratorPixel downIt = src->createHLineIterator(dx + 1, dy + y2 + 1, width - 2, false);

        KisHLineIteratorPixel dstIt = src->createHLineIterator(dx + 1, dy + y2, width - 2, true);

        WetPix left = *(reinterpret_cast<const WetPix*>(srcIt.oldRawData()));
        ++srcIt;
        WetPix current = *(reinterpret_cast<const WetPix*>(srcIt.oldRawData()));
        ++srcIt;
        WetPix right = *(reinterpret_cast<const WetPix*>(srcIt.oldRawData()));
        WetPix up, down;

        while (!srcIt.isDone()) {
            up = *(reinterpret_cast<const WetPix*>(upIt.oldRawData()));
            down = *(reinterpret_cast<const WetPix*>(downIt.oldRawData()));

            if ((reinterpret_cast<WetPix*>(srcIt.rawData()))->w > 0) {
                reducePixel(&wet_mix, &current, 1 - outflow[ix]);
                reducePixel(&wet_tmp, &up, flow_t[ix]);
                combinePixels(&wet_mix, &wet_mix, &wet_tmp);
                reducePixel(&wet_tmp, &down, flow_b[ix]);
                combinePixels(&wet_mix, &wet_mix, &wet_tmp);
                reducePixel(&wet_tmp, &left, flow_l[ix]);
                combinePixels(&wet_mix, &wet_mix, &wet_tmp);
                reducePixel(&wet_tmp, &right, flow_r[ix]);
                combinePixels(&wet_mix, &wet_mix, &wet_tmp);
                WetPix* target = reinterpret_cast<WetPix*>(dstIt.rawData());
                wetPixFromDouble(target, &wet_mix);
            }
            ++srcIt;
            ++dstIt;
            ++upIt;
            ++downIt;
            ix++;

            left = current;
            current = right;
            right = *(reinterpret_cast<const WetPix*>(srcIt.oldRawData()));
        }
        ix += 2;
    }

    delete[] flow_t;
    delete[] flow_b;
    delete[] flow_l;
    delete[] flow_r;
    delete[] fluid;
    delete[] outflow;
}

void WetPhysicsFilter::dry(KisPaintDeviceSP src, KisPaintDeviceSP dst, const QRect & r)
{
    for (Q_INT32 y = 0; y < r.height(); y++) {
        KisHLineIteratorPixel srcIt = src->createHLineIterator(r.x(), r.y() + y, r.width(), false);
        KisHLineIteratorPixel dstIt = dst->createHLineIterator(r.x(), r.y() + y, r.width(), true);

        Q_UINT16 w;
        while (!srcIt.isDone()) {
            // Two wet pixels in one KisWetColorSpace pixels.

            WetPack pack = *(reinterpret_cast<WetPack*>(srcIt.rawData()));
            WetPix* p = &(pack.paint);

            w = p->w; // no -1 here because we work on unsigned ints!

            if (w > 0)
                p->w = w - 1;
            else
                p->w = 0;

            *(reinterpret_cast<WetPack*>(dstIt.rawData())) = pack;

            ++dstIt;
            ++srcIt;
        }
    }
}

void WetPhysicsFilter::adsorb(KisPaintDeviceSP src, KisPaintDeviceSP /*dst*/, const QRect & r)
{
    for (Q_INT32 y = 0; y < r.height(); y++) {
        KisHLineIteratorPixel srcIt = src->createHLineIterator(r.x(), r.y() + y, r.width(), true);

        double ads;

        WetPixDbl wet_top;
        WetPixDbl wet_bot;

        WetPack * pack;
        Q_UINT16 w;

        while (!srcIt.isDone()) {
            // Two wet pixels in one KisWetColorSpace pixels.
            pack = reinterpret_cast<WetPack*>(srcIt.rawData());
            WetPix* paint = &pack->paint;
            WetPix* adsorb = &pack->adsorb;

            /* do adsorption */
            w = paint->w;

            if (w == 0) {
                ++srcIt;
            }
            else {

                ads = 0.5 / QMAX(w, 1);

                wetPixToDouble(&wet_top, paint);
                wetPixToDouble(&wet_bot, adsorb);

                mergePixel(&wet_bot, &wet_top, ads, &wet_bot);
                wetPixFromDouble(adsorb, &wet_bot);

                paint->rd *= (Q_UINT16)(1 - ads);
                paint->rw *= (Q_UINT16)(1 - ads);
                paint->gd *= (Q_UINT16)(1 - ads);
                paint->gw *= (Q_UINT16)(1 - ads);
                paint->bd *= (Q_UINT16)(1 - ads);
                paint->bw *= (Q_UINT16)(1 - ads);

                ++srcIt;
            }
        }
    }
}

void WetPhysicsFilter::combinePixels (WetPixDbl *dst, WetPixDbl *src1, WetPixDbl *src2)
{
    dst->rd = src1->rd + src2->rd;
    dst->rw = src1->rw + src2->rw;
    dst->gd = src1->gd + src2->gd;
    dst->gw = src1->gw + src2->gw;
    dst->bd = src1->bd + src2->bd;
    dst->bw = src1->bw + src2->bw;
    dst->w = src1->w + src2->w;
}

void WetPhysicsFilter::dilutePixel (WetPixDbl *dst, WetPix *src, double dilution)
{
    double scale = dilution * (1.0 / 8192.0);

    dst->rd = src->rd * scale;
    dst->rw = src->rw * scale;
    dst->gd = src->gd * scale;
    dst->gw = src->gw * scale;
    dst->bd = src->bd * scale;
    dst->bw = src->bw * scale;
    dst->w = src->w * (1.0 / 8192.0);
    dst->h = src->h * (1.0 / 8192.0);
}


void WetPhysicsFilter::reducePixel (WetPixDbl *dst, WetPix *src, double dilution)
{
    dilutePixel(dst, src, dilution);
    dst->w *= dilution;
}

void WetPhysicsFilter::mergePixel (WetPixDbl *dst, WetPixDbl *src1, double dilution1,
                                   WetPixDbl *src2)
{
    double d1, w1, d2, w2;
    double ed1, ed2;

    if (src1->rd < 1e-4) {
        dst->rd = src2->rd;
        dst->rw = src2->rw;
    } else if (src2->rd < 1e-4) {
        dst->rd = src1->rd * dilution1;
        dst->rw = src1->rw * dilution1;
    } else {
        d1 = src1->rd;
        w1 = src1->rw;
        d2 = src2->rd;
        w2 = src2->rw;
        dst->rd = d1 * dilution1 + d2;
        ed1 = exp(-d1 * dilution1);
        ed2 = exp(-d2);
        dst->rw = dst->rd * ((1 - ed1) * w1 / d1 + ed1 * (1 - ed2) * w2 / d2) /  (1 - ed1 * ed2);
    }

    if (src1->gd < 1e-4) {
        dst->gd = src2->gd;
        dst->gw = src2->gw;
    } else if (src2->gd < 1e-4) {
        dst->gd = src1->gd * dilution1;
        dst->gw = src1->gw * dilution1;
    } else {
        d1 = src1->gd;
        w1 = src1->gw;
        d2 = src2->gd;
        w2 = src2->gw;
        dst->gd = d1 * dilution1 + d2;
        ed1 = exp(-d1 * dilution1);
        ed2 = exp(-d2);
        dst->gw = dst->gd * ((1 - ed1) * w1 / d1 + ed1 * (1 - ed2) * w2 / d2) / (1 - ed1 * ed2);
    }

    if (src1->bd < 1e-4) {
        dst->bd = src2->bd;
        dst->bw = src2->bw;
    } else if (src2->bd < 1e-4) {
        dst->bd = src1->bd * dilution1;
        dst->bw = src1->bw * dilution1;
    } else {
        d1 = src1->bd;
        w1 = src1->bw;
        d2 = src2->bd;
        w2 = src2->bw;
        dst->bd = d1 * dilution1 + d2;
        ed1 = exp(-d1 * dilution1);
        ed2 = exp(-d2);
        dst->bw = dst->bd * ((1 - ed1) * w1 / d1 + ed1 * (1 - ed2) * w2 / d2) / (1 - ed1 * ed2);
    }
}
