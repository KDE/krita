/*
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@ideasandassociates.com>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <string.h>
#include <kdebug.h>
#include <koColor.h>
#include "kis_types.h"
#include "kis_global.h"
#include "kistile.h"
#include "kistilemgr.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kispixeldata.h"

KisPainter::KisPainter()
{
}

KisPainter::KisPainter(KisTileSP tile)
{
	begin(tile);
}

KisPainter::KisPainter(KisPixelDataSP pd)
{
	begin(pd);
}
	
KisPainter::KisPainter(KisPaintDeviceSP device)
{
	begin(device);
}

KisPainter::KisPainter(KisPaintDeviceSP device, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
	begin(device, x, y, w, h);
}

KisPainter::KisPainter(KisPaintDeviceSP device, const QRect& rc)
{
	begin(device, rc);
}

KisPainter::~KisPainter()
{
	end();
}

void KisPainter::begin(KisTileSP tile)
{
	m_dst = new KisPixelData;
	m_dst -> mgr = 0;
	m_dst -> mode = TILEMODE_RW;
	m_dst -> x1 = 0;
	m_dst -> y1 = 0;
	m_dst -> x2 = tile -> width();
	m_dst -> y2 = tile -> height();
	m_dst -> width = tile -> width();
	m_dst -> height = tile -> height();
	m_dst -> depth = tile -> depth();
	m_dst -> tile = tile;
	m_dst -> data = tile -> data();
	m_dst -> stride = tile -> depth() * tile -> width();
	m_dst -> owner = false;
	m_dst -> tile -> lock();
}

void KisPainter::begin(KisPixelDataSP pd)
{
	m_dst = pd;
}

void KisPainter::begin(KisPaintDeviceSP device)
{
	begin(device, 0, 0, device -> width(), device -> height());
}

void KisPainter::begin(KisPaintDeviceSP device, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
	KisTileMgrSP tm = device -> data();

	m_dst = tm -> pixelData(x, y, x + w - 1, y + h - 1, TILEMODE_RW);
}

void KisPainter::begin(KisPaintDeviceSP device, const QRect& rc)
{
	KisTileMgrSP tm = device -> data();

	m_dst = tm -> pixelData(rc.x(), rc.y(), rc.width(), rc.height(), TILEMODE_RW);
}

void KisPainter::end()
{
	if (m_dst && m_dst -> mgr == 0)
		m_dst -> tile -> release();

	if (m_dst && m_dst -> mgr)
		m_dst -> mgr -> releasePixelData(m_dst);

	m_dst = 0;
}

void KisPainter::bitBlt(Q_INT32 dx, Q_INT32 dy, CompositeOp op, KisTileSP src, Q_INT32 sx, Q_INT32 sy, Q_INT32 sw, Q_INT32 sh)
{
	KisPixelDataSP pd;

	src -> lock();

	if (sw == -1)
		sw = src -> width();

	if (sh == -1)
		sh = src -> height();

	pd = new KisPixelData;
	pd -> mgr = 0;
	pd -> mode = TILEMODE_RW;
	pd -> x1 = 0;
	pd -> y1 = 0;
	pd -> x2 = src -> width() - 1;
	pd -> y2 = src -> height() - 1;
	pd -> width = src -> width();
	pd -> height = src -> height();
	pd -> depth = src -> depth();
	pd -> tile = src;
	pd -> data = src -> data();
	pd -> stride = src -> depth() * src -> width();
	pd -> owner = false;
	bitBlt(dx, dy, op, pd, sx, sy, sw, sh);
	src -> release();
}

void KisPainter::drawTile(Q_INT32 dx, Q_INT32 dy, CompositeOp op, KisTileSP src, QUANTUM opacity, Q_INT32 sx, Q_INT32 sy, Q_INT32 sw, Q_INT32 sh)
{
	Q_INT32 x;
	Q_INT32 y;
	QUANTUM *d;
	QUANTUM *s;
	QUANTUM alpha;
	QUANTUM invAlpha;

	if (sw == -1)
		sw = src -> width();

	if (sh == -1)
		sh = src -> height();

	if (sw < 0 || sh < 0)
		return;

	if (opacity == OPACITY_TRANSPARENT)
		return;

	switch (op) {
		case COMPOSITE_OVER:
			for (y = 0; y < sh; y++) {
				for (x = 0; x < sw; x++) {
					s = src -> data() + ((y + sy) * src -> width() + (sx + x)) * src -> depth();
					d = m_dst -> data + ((y + dy) * m_dst -> width + (dx + x)) * m_dst -> depth;

					if (s[PIXEL_ALPHA] == OPACITY_TRANSPARENT)
						continue;

					if (d[PIXEL_ALPHA] == OPACITY_TRANSPARENT) {
						d[PIXEL_RED] = s[PIXEL_RED];
						d[PIXEL_GREEN] = s[PIXEL_GREEN];
						d[PIXEL_BLUE] = s[PIXEL_BLUE];
						continue;
					}

					alpha = (s[PIXEL_ALPHA] * opacity) / QUANTUM_MAX;
					invAlpha = QUANTUM_MAX - alpha;

					d[PIXEL_RED] = (d[PIXEL_RED] * invAlpha + s[PIXEL_RED] * alpha) / QUANTUM_MAX;
					d[PIXEL_GREEN] = (d[PIXEL_GREEN] * invAlpha + s[PIXEL_GREEN] * alpha) / QUANTUM_MAX;
					d[PIXEL_BLUE] = (d[PIXEL_BLUE] * invAlpha + s[PIXEL_BLUE] * alpha) / QUANTUM_MAX;
					alpha = (d[PIXEL_ALPHA] * (QUANTUM_MAX - s[PIXEL_ALPHA]) + s[PIXEL_ALPHA]) / QUANTUM_MAX;
					d[PIXEL_ALPHA] = (d[PIXEL_ALPHA] * (QUANTUM_MAX - alpha) + s[PIXEL_ALPHA]) / QUANTUM_MAX;
				}
			}
			break;
		default:
			break;
	}
}

void KisPainter::bitBlt(Q_INT32 dx, Q_INT32 dy, CompositeOp op, KisPixelDataSP src, Q_INT32 sx, Q_INT32 sy, Q_INT32 sw, Q_INT32 sh)
{
	Q_INT32 x;
	Q_INT32 y;
	QUANTUM *d;
	QUANTUM *s;
	QUANTUM alpha;

	if (sw == -1)
		sw = src -> width;

	if (sh == -1)
		sh = src -> height;

	if (sw < 0 || sh < 0)
		return;

	// TODO switch on the image type then go for the composite
	// TODO Implement all composites for all image depths
	switch (op) {
		case COMPOSITE_OVER:
			for (y = 0; y < sh; y++) {
				for (x = 0; x < sw; x++) {
					s = src -> data + ((y + sy) * src -> width + (sx + x)) * src -> depth;
					d = m_dst -> data + ((y + dy) * m_dst -> width + (dx + x)) * m_dst -> depth;

					if (s[PIXEL_ALPHA] == OPACITY_TRANSPARENT)
						continue;

					if (d[PIXEL_ALPHA] == OPACITY_TRANSPARENT) {
						d[PIXEL_RED] = s[PIXEL_RED];
						d[PIXEL_GREEN] = s[PIXEL_GREEN];
						d[PIXEL_BLUE] = s[PIXEL_BLUE];
						continue;
					}

					d[PIXEL_RED] = (d[PIXEL_RED] * (QUANTUM_MAX - s[PIXEL_ALPHA]) + s[PIXEL_RED] * s[PIXEL_ALPHA]) / QUANTUM_MAX;
					d[PIXEL_GREEN] = (d[PIXEL_GREEN] * (QUANTUM_MAX - s[PIXEL_ALPHA]) + s[PIXEL_GREEN] * s[PIXEL_ALPHA]) / QUANTUM_MAX;
					d[PIXEL_BLUE] = (d[PIXEL_BLUE] * (QUANTUM_MAX - s[PIXEL_ALPHA]) + s[PIXEL_BLUE] * s[PIXEL_ALPHA]) / QUANTUM_MAX;
					alpha = (d[PIXEL_ALPHA] * (QUANTUM_MAX - s[PIXEL_ALPHA]) + s[PIXEL_ALPHA]) / QUANTUM_MAX;
					d[PIXEL_ALPHA] = (d[PIXEL_ALPHA] * (QUANTUM_MAX - alpha) + s[PIXEL_ALPHA]) / QUANTUM_MAX;
				}
			}
			break;
		case COMPOSITE_COPY:
			for (y = 0; y < sh; y++) {
				for (x = 0; x < sw; x++) {
					s = src -> data + ((y + sy) * src -> width + (sx + x)) * src -> depth;
					d = m_dst -> data + ((y + dy) * m_dst -> width + (dx + x)) * m_dst -> depth;
					d[PIXEL_RED] = s[PIXEL_RED];
					d[PIXEL_GREEN] = s[PIXEL_GREEN];
					d[PIXEL_BLUE] = s[PIXEL_BLUE];
					d[PIXEL_ALPHA] = s[PIXEL_ALPHA];
				}
			}
			break;
		default:
			kdDebug() << "Not Implemented.\n";
			break;
	}
}

void KisPainter::bitBlt(Q_INT32 dx, Q_INT32 dy, CompositeOp op, KisPaintDeviceSP src, Q_INT32 sx, Q_INT32 sy, Q_INT32 sw, Q_INT32 sh)
{
	KisTileMgrSP tm = src -> data();
	KisPixelDataSP pd;

	if (sw == -1)
		sw = tm -> width();

	if (sh == -1)
		sh = tm -> height();

	pd = tm -> pixelData(sx, sy, sx + sw - 1, sy + sh - 1, TILEMODE_RW);
	bitBlt(dx, dy, op, pd, sx, sy, sw, sh);
}

void KisPainter::fillRect(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h, const KoColor& c)
{
	fillRect(x, y, w, h, c, OPACITY_OPAQUE);
}

void KisPainter::fillRect(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h, const KoColor& c, QUANTUM opacity)
{
	Q_INT32 dx = x + w;
	Q_INT32 dy = y + h;
	Q_INT32 x1;
	Q_INT32 y1;
	QUANTUM r = upscale(c.R());
	QUANTUM g = upscale(c.G());
	QUANTUM b = upscale(c.B());
	QUANTUM *d = m_dst -> data;

	for (y1 = y; y1 < dy; y1++) {
		for (x1 = x; x1 < dx; x1++) {
			d[PIXEL_RED] = r;
			d[PIXEL_GREEN] = g;
			d[PIXEL_BLUE] = b;
			d[PIXEL_ALPHA] = opacity;
			d += m_dst -> depth;
		}
	}
}

