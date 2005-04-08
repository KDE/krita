/*
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <math.h>

#include <kdebug.h>

#include <kis_global.h>
#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_types.h>

#include "kis_colorspace_wet.h"
#include "kis_texture_painter.h"

KisTexturePainter::KisTexturePainter()
	: super()
{
}

KisTexturePainter::KisTexturePainter(KisPaintDeviceSP device) : super(device)
{
}

void KisTexturePainter::createTexture(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
	// XXX: Make these parameters?
	double height = 1;
	double blurh = 0.7;

	double hscale = 128 * height / RAND_MAX;

        int ibh = (int) floor(256 * blurh + 0.5);

        for (int y2 = h; y2 < h - y; y2++) {
		KisHLineIterator i = m_device -> createHLineIterator(x, y2, w - x, true);
		while (!i.isDone()) {
			WetPix * w = (WetPix*)i.rawData();
			w[0].h = (Q_UINT16)floor(128 + hscale * rand());
			++i;
		}
        }

        int lh;

	// Blur horizontally
        for (int y2 = h; y2 < h - y; y2++) {
		KisHLineIterator i = m_device -> createHLineIterator(x, y2, w - x, true);

		WetPix * w = (WetPix*)i.rawData();
		lh = w[0].h;
		++i;

		while (!i.isDone()) {
			w = (WetPix*)i.rawData();
			w[0].h +=  ((lh - w[0].h) * ibh + 228) >> 8;
			lh = w[0].h;
			w[1].h = w[0].h; // XXX: Do we really need two height values? Krita
					 // combines the paint and absorbtion layers into one pixel.
			++i;
		}

        }
	// Vertical blurring was commented out in wetdreams, the effect seems to be achievable without this.
	//int ibv = floor(256 * blurv + 0.5);
}
