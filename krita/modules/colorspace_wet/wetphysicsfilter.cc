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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <stdlib.h>
#include <vector>

#include <klocale.h>
#include <kdebug.h>

#include <kis_doc.h>
#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_filter_registry.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view.h>
#include <kis_paint_device.h>

#include "wetphysicsfilter.h"


WetPhysicsFilter::WetPhysicsFilter(KisView * view)
	: KisFilter(id(), view)
{
	m_adsorbCount = 0;
}

void WetPhysicsFilter::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* /*config*/, const QRect& rect)
{
	// XXX: It would be nice be able to interleave this, instead of having the same loop over
	//      our pixels three times.
	// XXX: Don't do the flow yet. It loops three times through the paint device and creates an enormous amount
	//      of temporary data -- size arrays of doubles width * height of the paint device, and besides, it's
	//      subject to the same problems as the Wet & Sticky model; the windscreen wiper effect.
	//flow(src, dst, rect);
	if (m_adsorbCount == 2) {
		adsorb(src, dst, rect);
		dry(src, dst, rect);
		m_adsorbCount = 0;
	}
}


void WetPhysicsFilter::flow(KisPaintDeviceSP src, KisPaintDeviceSP dst, const QRect & r)
{
	/* XXX: Is this like a convolution operation? BSAR */
        int x, y;
        int width = r.width();
        int height = r.height();

        /* width of a line in a layer in pixel units, not in bytes -- used to move to the next
	   line in the fluid masks below */
        int rowstride = width;

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

	// ??? I haven't got a clue about this...
        int ix;

	// Flow to the top, bottom, left, right of the currentpixel
        double ft, fb, fl, fr;

	// Temporary pixel constructs
        WetPixDbl wet_mix, wet_tmp;

	// If the flow touches areas that have not been initialized with a height field yet,
	// create a heigth field.

	// We need three iterators, because we're working on a five-point convolution kernel (no corner pixels are being used)

	// First iteration: compute fluid deposits around the paper.
	Q_INT32 dx, dy, dw, dh;
	dx = r.x();
	dy = r.y();
	dw = r.width();
	dh = r.height();

	for (Q_INT32 y2 = dh; y2 < dh - dy; ++y2) {
		KisHLineIteratorPixel srcIt = src->createHLineIterator(dx, y2, dw, false);
		while (!srcIt.isDone()) {
			// XXX
			++srcIt;
		}
	}

	// Second iteration: Reduce flow in dry areas
	for (Q_INT32 y2 = dh; y2 < dh - dy; ++y2) {
		KisHLineIteratorPixel srcIt = src->createHLineIterator(dx, y2, dw, false);
		while (!srcIt.isDone()) {
			// XXX
			++srcIt;
		}
	}

	// Third iteration: Combine the paint from the flow areas.
	for (Q_INT32 y2 = dh; y2 < dh - dy; ++y2) {
		KisHLineIteratorPixel srcIt = src->createHLineIterator(dx, y2, dw, false);
		while (!srcIt.isDone()) {
			// XXX
			++srcIt;
		}
	}

	delete[] flow_t;
	delete[] flow_b;
	delete[] flow_l;
	delete[] flow_r;
	delete[] fluid;
	delete[] outflow;

}

void WetPhysicsFilter::dry(KisPaintDeviceSP src, KisPaintDeviceSP /*dst*/, const QRect & r)
{
	KisRectIteratorPixel srcIt = src->createRectIterator(r.x(), r.y(), r.width(), r.height(), true);

	Q_UINT16 w;
	while (!srcIt.isDone()) {
		// Two wet pixels in one KisColorSpaceWet pixels.

		WetPix * p = (WetPix*)srcIt.rawData();
		// The adsorbtion pixel is p[1]
		w = p[0].w;
		w -= 1;
		if (w > 0) {
			p[0].w = w;
		} else {
			p[0].w = 0;
		}
		++srcIt;

	}

}

void WetPhysicsFilter::adsorb(KisPaintDeviceSP src, KisPaintDeviceSP /*dst*/, const QRect & r)
{
	KisRectIteratorPixel srcIt = src->createRectIterator(r.x(), r.y(), r.width(), r.height(), true);

        double ads;

        WetPixDbl wet_top;
        WetPixDbl wet_bot;

	WetPix * pixels;
	Q_UINT16 w;

	while (!srcIt.isDone()) {
		// Two wet pixels in one KisColorSpaceWet pixels.
		pixels = (WetPix*) srcIt.rawData();

		/* do adsorption */
		w = pixels[0].w;

		if (w == 0)
			continue;

		ads = 0.5 / QMAX(w, 1);

		wetPixToDouble(&wet_top, &pixels[0]);
		wetPixToDouble(&wet_bot, &pixels[1]);

		mergePixel(&wet_bot, &wet_top, ads, &wet_bot);

		wetPixFromDouble(pixels + 1, &wet_bot);

		pixels[0].rd = pixels[0].rd * (1 - ads);
		pixels[0].rw = pixels[0].rw * (1 - ads);
		pixels[0].gd = pixels[0].gd * (1 - ads);
		pixels[0].gw = pixels[0].gw * (1 - ads);
		pixels[0].bd = pixels[0].bd * (1 - ads);
		pixels[0].bw = pixels[0].bw * (1 - ads);

		++srcIt;
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

void WetPhysicsFilter::mergePixel (WetPixDbl *dst, WetPixDbl *src1, double dilution1, WetPixDbl *src2)
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
