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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <qrect.h>

#include <kdebug.h>

#include <kis_brush.h>
#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_types.h>
#include <kis_paintop.h>

#include "kis_wetop.h"
#include "kis_colorspace_wet.h"

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

	KisPaintDeviceSP device = m_painter -> device();

	// Get the paint
	double wetness = (double)m_painter -> backgroundColor().red();
	double strength = (double)m_painter -> backgroundColor().green();

	KisStrategyColorSpaceSP cs = device -> colorStrategy();

	if (cs -> id() != KisID("WET","")) {
		kdDebug() << "You cannot paint wet paint on dry pixels.\n";
	}

	WetPix paint;

	cs -> nativeColor(m_painter -> paintColor(), (QUANTUM*)(&paint), 0);



/*
	//
	double r_fringe;
	int x0, y0;
	int x1, y1;
	WetPix *wet_line;
	int xp, yp;
	double xx, yy, rr;
	double eff_height;
	double press, contact;
	WetPixDbl wet_tmp, wet_tmp2;

	r_fringe = r + 1;
	x0 = floor(x - r_fringe);
	y0 = floor(y - r_fringe);
	x1 = ceil(x + r_fringe);
	y1 = ceil(y + r_fringe);
	if (x0 < 0)
		x0 = 0;
	if (y0 < 0)
		y0 = 0;
	if (x1 >= layer->width)
		x1 = layer->width;
	if (y1 >= layer->height)
		y1 = layer->height;

	wet_line = layer->buf + y0 * layer->rowstride;
	for (yp = y0; yp < y1; yp++) {
		yy = (yp + 0.5 - y);
		yy *= yy;
		for (xp = x0; xp < x1; xp++) {
			xx = (xp + 0.5 - x);
			xx *= xx;
			rr = yy + xx;
			if (rr < r * r)
				press = pressure * 0.25;
			else
				press = -1;
			eff_height =
			    (wet_line[xp].h + wet_line[xp].w -
			     192) * (1.0 / 255);
			contact = (press + eff_height) * 0.2;
			if (contact > 0.5)
				contact =
				    1 - 0.5 * exp(-2.0 * contact - 1);
			if (contact > 0.0001) {
				int v;
				double rnd = rand() * (1.0 / RAND_MAX);

				v = wet_line[xp].rd;
				wet_line[xp].rd =
				    floor(v +
					  (paint->rd * strength -
					   v) * contact + rnd);
				v = wet_line[xp].rw;
				wet_line[xp].rw =
				    floor(v +
					  (paint->rw * strength -
					   v) * contact + rnd);
				v = wet_line[xp].gd;
				wet_line[xp].gd =
				    floor(v +
					  (paint->gd * strength -
					   v) * contact + rnd);
				v = wet_line[xp].gw;
				wet_line[xp].gw =
				    floor(v +
					  (paint->gw * strength -
					   v) * contact + rnd);
				v = wet_line[xp].bd;
				wet_line[xp].bd =
				    floor(v +
					  (paint->bd * strength -
					   v) * contact + rnd);
				v = wet_line[xp].bw;
				wet_line[xp].bw =
				    floor(v +
					  (paint->bw * strength -
					   v) * contact + rnd);
				v = wet_line[xp].w;
				wet_line[xp].w =
				    floor(v + (paint->w - v) * contact +
					  rnd);

			}
		}
		wet_line += layer->rowstride;
	}
*/
}
