/*
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
#include <stdlib.h>
#include <string.h>
#include <kdebug.h>
#include <koColor.h>
#include "kis_types.h"
#include "kis_global.h"
#include "kis_image.h"
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
	m_device = device;
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

void KisPainter::bitBlt(Q_INT32 dx, Q_INT32 dy, CompositeOp op, KisPixelDataSP src, Q_INT32 sx, Q_INT32 sy, Q_INT32 sw, Q_INT32 sh)
{
	Q_INT32 x;
	Q_INT32 y;
	QUANTUM *d;
	QUANTUM *s;
	QUANTUM alpha;

	prepareEzPaint();

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
						d[PIXEL_ALPHA] = s[PIXEL_ALPHA];
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

void KisPainter::tileBlt(QUANTUM *dst, KisTileSP dsttile, QUANTUM *src, KisTileSP srctile, Q_INT32 rows, Q_INT32 cols, CompositeOp op)
{
	Q_INT32 dststride = dsttile -> width() * dsttile -> depth();
	Q_INT32 srcstride = srctile -> width() * srctile -> depth();
	Q_INT32 stride = m_device -> image() -> depth();
	QUANTUM *d;
	QUANTUM *s;
	QUANTUM alpha;
	Q_INT32 i;

	switch (op) {
		case COMPOSITE_COPY:
			while (rows-- > 0) {
				d = dst;
				s = src;

				for (i = cols; i > 0; i--, d += stride, s += stride)
					memcpy(d, s, stride * sizeof(QUANTUM));
			}
			break;
		case COMPOSITE_OVER:
			while (rows-- > 0) {
				d = dst;
				s = src;

				for (i = cols; i > 0; i--, d += stride, s += stride) {
					if (s[PIXEL_ALPHA] == OPACITY_TRANSPARENT)
						continue;

					if (d[PIXEL_ALPHA] == OPACITY_TRANSPARENT || (d[PIXEL_ALPHA] == OPACITY_OPAQUE && s[PIXEL_ALPHA] == OPACITY_OPAQUE)) {
						memcpy(d, s, stride * sizeof(QUANTUM));
						continue;
					}

					d[PIXEL_RED] = (d[PIXEL_RED] * (QUANTUM_MAX - s[PIXEL_ALPHA]) + s[PIXEL_RED] * s[PIXEL_ALPHA]) / QUANTUM_MAX;
					d[PIXEL_GREEN] = (d[PIXEL_GREEN] * (QUANTUM_MAX - s[PIXEL_ALPHA]) + s[PIXEL_GREEN] * s[PIXEL_ALPHA]) / QUANTUM_MAX;
					d[PIXEL_BLUE] = (d[PIXEL_BLUE] * (QUANTUM_MAX - s[PIXEL_ALPHA]) + s[PIXEL_BLUE] * s[PIXEL_ALPHA]) / QUANTUM_MAX;
					alpha = (d[PIXEL_ALPHA] * (QUANTUM_MAX - s[PIXEL_ALPHA]) + s[PIXEL_ALPHA]) / QUANTUM_MAX;
					d[PIXEL_ALPHA] = (d[PIXEL_ALPHA] * (QUANTUM_MAX - alpha) + s[PIXEL_ALPHA]) / QUANTUM_MAX;
				}

				dst += dststride;
				src += srcstride;
			}

			break;
		default:
			kdDebug() << "Not Implemented.\n";
			return;
	}
}

void KisPainter::bitBlt(Q_INT32 dx, Q_INT32 dy, CompositeOp op, KisPaintDeviceSP srcdev, Q_INT32 sx, Q_INT32 sy, Q_INT32 sw, Q_INT32 sh)
{
	Q_INT32 x;
	Q_INT32 y;
	Q_INT32 sx2 = sx;
	Q_INT32 dx2;
	Q_INT32 dy2;
	Q_INT32 rows;
	Q_INT32 cols;
	KisTileSP tile;
	KisTileSP dsttile;
	KisTileSP srctile;
	QUANTUM *dst;
	QUANTUM *src;
	KisTileMgrSP dsttm = m_device -> data();
	KisTileMgrSP srctm = srcdev -> data();
	Q_INT32 sxmod;
	Q_INT32 symod;
	Q_INT32 dxmod;
	Q_INT32 dymod;
	Q_INT32 xxtra;
	Q_INT32 yxtra;
	Q_INT32 nrows;
	Q_INT32 ncols;

	if (sw == -1)
		sw = dsttm -> width();

	if (sh == -1)
		sh = dsttm -> height();

	dx2 = dx + sw - 1;
	dy2 = dy + sh - 1;
	symod = (sy % TILE_HEIGHT);
	sxmod = (sx % TILE_WIDTH);

	for (y = dy; y <= dy2; y += TILE_HEIGHT - (y % TILE_HEIGHT)) {
		sx = sx2;

		for (x = dx; x <= dx2; x += TILE_WIDTH - (x % TILE_WIDTH)) {
			dsttile = dsttm -> tile(x, y, TILEMODE_WRITE);
			srctile = srctm -> tile(sx, sy, TILEMODE_READ);

			if (!dsttile || !srctile)
				continue;

			dsttile -> lock();
			srctile -> lock();
			dymod = dy % TILE_HEIGHT;
			dxmod = dx % TILE_WIDTH;
			symod = sy % TILE_HEIGHT;
			sxmod = sx % TILE_WIDTH;

			if (dymod > symod) {
				rows = dsttile -> height() - dymod;
				yxtra = dsttile -> height() - rows;
			} else {
				rows = dsttile -> height() - symod;
				yxtra = rows - dsttile -> height();
			}

			if (rows > dy2 - y + 1)
				rows = dy2 - y + 1;

			if (dxmod > sxmod) {
				cols = dsttile -> width() - dxmod;
				xxtra = dsttile -> width() - cols;
			} else {
				cols = dsttile -> width() - sxmod;
				xxtra = cols - dsttile -> width();
			}

			if (cols > dx2 - x + 1)
				cols = dx2 - x + 1;

			if (cols < 0 || rows < 0) {
				dsttile -> release();
				srctile -> release();
				continue;
			}

			dst = dsttile -> data(dxmod, dymod);
			src = srctile -> data(sxmod, symod);
			tileBlt(dst, dsttile, src, srctile, rows, cols, op);

			if (yxtra > 0) {
				tile = dsttm -> tile(x, y + TILE_HEIGHT - (y % TILE_HEIGHT) + 1, TILEMODE_WRITE);

				if (tile) {
					tile -> lock();
					nrows = QMIN(tile -> height(), dymod);
					dst = tile -> data(dxmod, 0);
					src = srctile -> data(abs(QMIN(xxtra, 0)), rows);
					tileBlt(dst, tile, src, srctile, nrows, cols, op);
					tile -> release();
				}
			} else if (yxtra < 0) {
				tile = srctm -> tile(sx, sy + TILE_HEIGHT - (sy % TILE_HEIGHT) + 1, TILEMODE_READ);

				if (tile) {
					tile -> lock();
					nrows = QMIN(tile -> height(), symod);
					dst = dsttile -> data(dxmod, rows);
					src = tile -> data(sxmod, 0);
					tileBlt(dst, dsttile, src, tile, nrows, cols, op);
					tile -> release();
				}
			}

			if (xxtra > 0) {
				tile = dsttm -> tile(x + TILE_WIDTH - (x % TILE_WIDTH) + 1, y, TILEMODE_WRITE);

				if (tile) {
					tile -> lock();
					ncols = QMIN(tile -> width(), dxmod);
					dst = tile -> data(0, dymod);
					src = srctile -> data(cols, abs(QMIN(yxtra, 0)));
					tileBlt(dst, tile, src, srctile, rows, ncols, op);
					tile -> release();
				}
			} else if (xxtra < 0) {
				tile = srctm -> tile(sx + TILE_WIDTH - (sx % TILE_WIDTH) + 1, sy, TILEMODE_READ);

				if (tile) {
					tile -> lock();
					ncols = QMIN(tile -> width(), sxmod);
					dst = dsttile -> data(cols, dymod);
					src = tile -> data(0, symod);
					tileBlt(dst, dsttile, src, tile, rows, ncols, op);
					tile -> release();
				}
			}

			if (yxtra > 0 && xxtra > 0) {
				tile = dsttm -> tile(x + TILE_WIDTH - (x % TILE_WIDTH) + 1, y + TILE_HEIGHT - (y % TILE_HEIGHT) + 1, TILEMODE_WRITE);

				if (tile) {
					tile -> lock();
					dymod = QMIN(tile -> height(), dymod);
					dxmod = QMIN(tile -> width(), dxmod);
					dst = tile -> data();
					src = srctile -> data(cols, rows);
					tileBlt(dst, tile, src, srctile, dymod, dxmod, op);
					tile -> release();
				}
			} else if (yxtra < 0 && xxtra < 0) {
				tile = srctm -> tile(sx + TILE_WIDTH - (sx % TILE_WIDTH) + 1, sy + TILE_HEIGHT - (sy % TILE_HEIGHT) + 1, TILEMODE_READ);

				if (tile) {
					tile -> lock();
					symod = QMIN(tile -> height(), symod);
					sxmod = QMIN(tile -> width(), sxmod);
					dst = dsttile -> data(cols, rows);
					src = tile -> data();
					tileBlt(dst, dsttile, src, tile, symod, sxmod, op);
					tile -> release();
				}
			} else if (yxtra > 0 && xxtra < 0) {
				KisTileSP srctile = srctm -> tile(sx + TILE_WIDTH - (sx % TILE_WIDTH) + 1, sy, TILEMODE_READ);
				tile = dsttm -> tile(x, y + TILE_HEIGHT - (y % TILE_HEIGHT) + 1, TILEMODE_WRITE);

				if (srctile && tile) {
					srctile -> lock();
					tile -> lock();
					yxtra = QMIN(tile -> height(), yxtra);
					xxtra = QMIN(tile -> width(), -xxtra);
					dst = tile -> data(tile -> width() - xxtra, 0);
					src = srctile -> data(0, srctile -> height() - yxtra);
					tileBlt(dst, tile, src, srctile, yxtra, QMIN(xxtra, srctile -> width()), op);
					tile -> release();
					srctile -> release();
				}
			} else if (yxtra < 0 && xxtra > 0) {
				KisTileSP srctile = srctm -> tile(sx, sy + TILE_HEIGHT - (sy % TILE_HEIGHT) + 1, TILEMODE_READ);
				tile = dsttm -> tile(x + TILE_WIDTH - (x % TILE_WIDTH) + 1, y, TILEMODE_WRITE);

				if (srctile && tile) {
					srctile -> lock();
					tile -> lock();
					yxtra = QMIN(tile -> height(), -yxtra);
					xxtra = QMIN(srctile -> width(), xxtra);
					dst = tile -> data(0, tile -> height() - yxtra);
					src = srctile -> data(srctile -> width() - xxtra, 0);
					tileBlt(dst, tile, src, srctile, yxtra, QMIN(xxtra, tile -> width()), op);
					tile -> release();
					srctile -> release();
				}
			}

			dsttile -> valid(false);
			dsttile -> release();
			srctile -> release();
			sx += TILE_WIDTH;
		}

		sy += TILE_HEIGHT;
	}
}

void KisPainter::bitBlt(Q_INT32 dx, Q_INT32 dy, CompositeOp op, KisTileSP src, QUANTUM opacity, Q_INT32 sx, Q_INT32 sy, Q_INT32 sw, Q_INT32 sh)
{
	Q_INT32 x;
	Q_INT32 y;
	QUANTUM *d;
	QUANTUM *s;
	QUANTUM alpha;
	QUANTUM invAlpha;

	prepareEzPaint();

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
			kdDebug() << "Not Implemented.\n";
			break;
	}
}

void KisPainter::bitBlt(Q_INT32 dx, Q_INT32 dy, CompositeOp op, KisPixelDataSP src, QUANTUM opacity, Q_INT32 sx, Q_INT32 sy, Q_INT32 sw, Q_INT32 sh)
{
	Q_INT32 x;
	Q_INT32 y;
	QUANTUM *d;
	QUANTUM *s;
	QUANTUM alpha;
	QUANTUM invAlpha;

	prepareEzPaint();

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
						d[PIXEL_ALPHA] = s[PIXEL_ALPHA];
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
			kdDebug() << "Not Implemented.\n";
			break;
	}
}

void KisPainter::bitBlt(Q_INT32 dx, Q_INT32 dy, CompositeOp op, KisPaintDeviceSP src, QUANTUM opacity, Q_INT32 sx, Q_INT32 sy, Q_INT32 sw, Q_INT32 sh)
{
	KisTileMgrSP tm = src -> data();
	KisPixelDataSP pd;

	if (!tm)
		return;

	if (sw == -1)
		sw = tm -> width();

	if (sh == -1)
		sh = tm -> height();

	pd = tm -> pixelData(sx, sy, sx + sw - 1, sy + sh - 1, TILEMODE_READ);
	Q_ASSERT(pd);
	bitBlt(dx, dy, op, pd, opacity, 0, 0, pd -> width, pd -> height);
}

void KisPainter::fillRect(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h, const KoColor& c)
{
	fillRect(x, y, w, h, c, OPACITY_OPAQUE);
}

void KisPainter::fillRect(const QRect& rc, const KoColor& c)
{
	fillRect(rc.x(), rc.y(), rc.width(), rc.height(), c, OPACITY_OPAQUE);
}

void KisPainter::fillRect(Q_INT32 x1, Q_INT32 y1, Q_INT32 w, Q_INT32 h, const KoColor& c, QUANTUM opacity)
{
	Q_INT32 x;
	Q_INT32 y;
	Q_INT32 x2 = x1 + w - 1;
	Q_INT32 y2 = y1 + h - 1;
	Q_INT32 rows;
	Q_INT32 cols;
	Q_INT32 dststride;
	Q_INT32 stride;
	KisTileSP tile;
	QUANTUM src[MAXCHANNELS];
	QUANTUM *dst;
	KisTileMgrSP tm = m_device -> data();
	Q_INT32 xmod;
	Q_INT32 ymod;

	switch (m_device -> image() -> imgType()) {
	case IMAGE_TYPE_RGB:
	case IMAGE_TYPE_RGBA:
		src[PIXEL_RED] = upscale(c.R());
		src[PIXEL_GREEN] = upscale(c.G());
		src[PIXEL_BLUE] = upscale(c.B());
		src[PIXEL_ALPHA] = opacity;
		break;
	default:
		kdDebug() << "Not Implemented.\n";
		return;
	}

	stride = m_device -> image() -> depth();
	ymod = (y1 % TILE_HEIGHT);
	xmod = (x1 % TILE_WIDTH);

	for (y = y1; y <= y2; y += TILE_HEIGHT - (y % TILE_HEIGHT)) {
		for (x = x1; x <= x2; x += TILE_WIDTH - (x % TILE_WIDTH)) {
			ymod = y % TILE_HEIGHT;
			xmod = x % TILE_WIDTH;
			tile = tm -> tile(x, y, TILEMODE_WRITE);
			tile -> lock();
			dst = tile -> data(xmod, ymod);
			rows = tile -> height() - ymod;

			if (rows > y2 - y + 1)
				rows = y2 - y + 1;

			cols = tile -> width() - xmod;

			if (cols > x2 - x + 1)
				cols = x2 - x + 1;

			dststride = tile -> width() * tile -> depth();

			while (rows-- > 0) {
				QUANTUM *d = dst;

				for (Q_INT32 i = cols; i > 0; i--) {
					memcpy(d, src, stride * sizeof(QUANTUM));
					d += stride;
				}

				dst += dststride;
			}

			tile -> valid(false);
			tile -> release();
		}
	}
}

void KisPainter::fillRect(const QRect& rc, const KoColor& c, QUANTUM opacity)
{
	fillRect(rc.x(), rc.y(), rc.width(), rc.height(), c, opacity);
}

void KisPainter::prepareEzPaint()
{
	if (m_device && !m_dst) {
		KisTileMgrSP tm = m_device -> data();

		m_dst = tm -> pixelData(0, 0, m_device -> width() - 1, m_device -> height() - 1, TILEMODE_RW);
	}

	Q_ASSERT(m_dst);
}

