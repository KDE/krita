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
#include <kcommand.h>
#include <koColor.h>
#include "kis_types.h"
#include "kis_global.h"
#include "kis_image.h"
#include "kistile.h"
#include "kistilemgr.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kispixeldata.h"

namespace {
#	include <qmap.h>

	class KisTileCommand : public KCommand {
		typedef QMap<Q_INT32, KisTileSP> TileMap;

	public:
		KisTileCommand(const QString& name, KisPaintDeviceSP device);
		virtual ~KisTileCommand();

	public:
		virtual void execute();
		virtual void unexecute();
		virtual QString name() const;

	public:
		void addTile(Q_INT32 tileNo, KisTileSP tile);

	private:
		TileMap m_tiles;
		TileMap m_originals;
		QString m_name;
		KisPaintDeviceSP m_device;
	};

	KisTileCommand::KisTileCommand(const QString& name, KisPaintDeviceSP device)
	{
		m_name = name;
		m_device = device;
	}

	KisTileCommand::~KisTileCommand()
	{
	}

	void KisTileCommand::execute()
	{
		KisTileMgrSP tm = m_device -> data();
		KisImageSP img = m_device -> image();

		for (TileMap::iterator it = m_originals.begin(); it != m_originals.end(); it++) {
			(*it) -> valid(false);
			tm -> attach(*it, it.key());
		}

		if (img)
			img -> invalidate(); //it.key());
	}

	void KisTileCommand::unexecute()
	{
		KisTileMgrSP tm = m_device -> data();
		KisImageSP img = m_device -> image();
		KisTileSP tmp;

		if (m_originals.empty()) {
			for (TileMap::iterator it = m_tiles.begin(); it != m_tiles.end(); it++) {
				tmp = tm -> tile(it.key(), TILEMODE_NONE);
				(*it) -> valid(false);
				tm -> attach(*it, it.key());
				m_originals[it.key()] = tmp;
			}
		} else {
			for (TileMap::iterator it = m_tiles.begin(); it != m_tiles.end(); it++) {
				(*it) -> valid(false);
				tm -> attach(*it, it.key());

			}
		}

		if (img)
			img -> invalidate(); //it.key());
	}

	QString KisTileCommand::name() const
	{
		return m_name;
	}

	void KisTileCommand::addTile(Q_INT32 tileNo, KisTileSP tile)
	{
		Q_ASSERT(tile);

		if (m_tiles.count(tileNo) == 0) {
			tile -> shareRef();
			m_tiles[tileNo] = tile;	
		}
	}
}

KisPainter::KisPainter()
{
	m_transaction = 0;
}

KisPainter::KisPainter(KisPaintDeviceSP device)
{
	m_transaction = 0;
	begin(device);
}

KisPainter::~KisPainter()
{
	end();
}

void KisPainter::begin(KisPaintDeviceSP device)
{
	if (m_transaction)
		delete m_transaction;

	m_device = device;
}

KCommand *KisPainter::end()
{
	return endTransaction();
}

void KisPainter::beginTransaction(KMacroCommand *command)
{
	if (m_transaction)
		delete m_transaction;

	m_transaction = command;
}

void KisPainter::beginTransaction(const QString& customName)
{
	if (m_transaction)
		delete m_transaction;

	m_transaction = new KMacroCommand(customName);
}

KCommand *KisPainter::endTransaction()
{
	KCommand *command = m_transaction;

	m_transaction = 0;
	return command;
}

#if 0
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
						d[PIXEL_ALPHA] = s[PIXEL_ALPHA];
						continue;
					}

					if (d[PIXEL_ALPHA] == OPACITY_TRANSPARENT || (d[PIXEL_ALPHA] == OPACITY_OPAQUE && s[PIXEL_ALPHA] == OPACITY_OPAQUE)) {
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

	m_dst -> mgr -> releasePixelData(m_dst);
	m_dst = 0;
}
#endif

void KisPainter::tileBlt(QUANTUM *dst, KisTileSP dsttile, QUANTUM *src, KisTileSP srctile, QUANTUM opacity, Q_INT32 rows, Q_INT32 cols, CompositeOp op)
{
	if (opacity == OPACITY_OPAQUE)
		return tileBlt(dst, dsttile, src, srctile, rows, cols, op);

	Q_INT32 dststride = dsttile -> width() * dsttile -> depth();
	Q_INT32 srcstride = srctile -> width() * srctile -> depth();
	Q_INT32 stride = m_device -> image() -> depth();
	QUANTUM *d;
	QUANTUM *s;
	QUANTUM alpha;
	QUANTUM invAlpha;
	Q_INT32 i;

	if (rows <= 0 || cols <= 0)
		return;

	switch (op) {
		case COMPOSITE_OVER:
			while (rows-- > 0) {
				d = dst;
				s = src;

				for (i = cols; i > 0; i--, d += stride, s += stride) {
					if (s[PIXEL_ALPHA] == OPACITY_TRANSPARENT)
						continue;

					alpha = (s[PIXEL_ALPHA] * opacity) / QUANTUM_MAX;
					invAlpha = QUANTUM_MAX - alpha;
					d[PIXEL_RED] = (d[PIXEL_RED] * invAlpha + s[PIXEL_RED] * alpha) / QUANTUM_MAX;
					d[PIXEL_GREEN] = (d[PIXEL_GREEN] * invAlpha + s[PIXEL_GREEN] * alpha) / QUANTUM_MAX;
					d[PIXEL_BLUE] = (d[PIXEL_BLUE] * invAlpha + s[PIXEL_BLUE] * alpha) / QUANTUM_MAX;
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

void KisPainter::tileBlt(QUANTUM *dst, KisTileSP dsttile, QUANTUM *src, KisTileSP srctile, Q_INT32 rows, Q_INT32 cols, CompositeOp op)
{
	Q_INT32 dststride = dsttile -> width() * dsttile -> depth();
	Q_INT32 srcstride = srctile -> width() * srctile -> depth();
	Q_INT32 stride = m_device -> image() -> depth();
	QUANTUM *d;
	QUANTUM *s;
	QUANTUM alpha;
	Q_INT32 i;

	if (rows <= 0 || cols <= 0)
		return;

	switch (op) {
		case COMPOSITE_COPY:
			while (rows-- > 0) {
				d = dst;
				s = src;

				for (i = cols; i > 0; i--, d += stride, s += stride)
					memcpy(d, s, stride * sizeof(QUANTUM));

				dst += dststride;
				src += srcstride;
			}
			break;
		case COMPOSITE_CLEAR:
			while (rows-- > 0) {
				d = dst;
				s = src;

				for (i = cols; i > 0; i--, d += stride, s += stride)
					memset(d, 0, stride * sizeof(QUANTUM));

				dst += dststride;
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
	bitBlt(dx, dy, op, srcdev, OPACITY_OPAQUE, sx, sy, sw, sh);
}

#if 0
void KisPainter::bitBlt(Q_INT32 dx, Q_INT32 dy, CompositeOp op, KisPixelDataSP src, QUANTUM opacity, Q_INT32 sx, Q_INT32 sy, Q_INT32 sw, Q_INT32 sh)
{
	Q_INT32 x;
	Q_INT32 y;
	QUANTUM *d;
	QUANTUM *s;
	QUANTUM alpha;
	QUANTUM invAlpha;

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
#endif

void KisPainter::bitBlt(Q_INT32 dx, Q_INT32 dy, CompositeOp op, KisPaintDeviceSP srcdev, QUANTUM opacity, Q_INT32 sx, Q_INT32 sy, Q_INT32 sw, Q_INT32 sh)
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
	KisTileCommand *tc;
	Q_INT32 tileno;

	if (dx < 0 || dy < 0 || sx < 0 || sy < 0)
		return;

	if (sw == -1)
		sw = dsttm -> width();

	if (sh == -1)
		sh = dsttm -> height();

	dx2 = dx + sw - 1;
	dy2 = dy + sh - 1;
	dymod = dy % TILE_HEIGHT;
	dxmod = dx % TILE_WIDTH;
	symod = sy % TILE_HEIGHT;
	sxmod = sx % TILE_WIDTH;

	if (m_transaction) {
		tc = new KisTileCommand(m_transaction -> name(), m_device);
		m_transaction -> addCommand(tc);
	}

	for (y = dy; y <= dy2; y += TILE_HEIGHT) {
		sx = sx2;

		for (x = dx; x <= dx2; x += TILE_WIDTH) {
			tileno = dsttm -> tileNum(x, y);

			if (m_transaction && (dsttile = dsttm -> tile(tileno, TILEMODE_NONE)))
				tc -> addTile(tileno, dsttile);

			dsttile = dsttm -> tile(tileno, TILEMODE_RW);
			srctile = srctm -> tile(sx, sy, TILEMODE_READ);

			if (!dsttile || !srctile)
				continue;

			if (dymod > symod) {
				rows = dsttile -> height() - dymod;
				yxtra = dsttile -> height() - rows;
			} else {
				rows = srctile -> height() - symod;
				yxtra = rows - srctile -> height();
			}

			if (rows > dy2 - y + 1)
				rows = dy2 - y + 1;

			if (dxmod > sxmod) {
				cols = dsttile -> width() - dxmod;
				xxtra = dsttile -> width() - cols;
			} else {
				cols = srctile -> width() - sxmod;
				xxtra = cols - srctile -> width();
			}

			if (cols > dx2 - x + 1)
				cols = dx2 - x + 1;

			if (cols <= 0 || rows <= 0)
				continue;

			dsttile -> lock();
			srctile -> lock();
			dst = dsttile -> data(dxmod, dymod);
			src = srctile -> data(sxmod, symod);
			tileBlt(dst, dsttile, src, srctile, opacity, rows, cols, op);

			if (yxtra < 0) {
				tile = srctm -> tile(sx, sy + TILE_HEIGHT - (sy % TILE_HEIGHT) + 1, TILEMODE_READ);

				if (tile) {
					if (dsttile -> height() == TILE_HEIGHT)
						nrows = QMIN(tile -> height(), symod);
					else
						nrows = dsttile -> height() - rows;

					tile -> lock();
					dst = dsttile -> data(dxmod, rows);
					src = tile -> data(sxmod, 0);
					tileBlt(dst, dsttile, src, tile, nrows, cols, op);
					tile -> release();
				}
			} else if (yxtra > 0) {
				tileno = dsttm -> tileNum(x, y + TILE_HEIGHT - (y % TILE_HEIGHT) + 1);

				if (m_transaction && (tile = dsttm -> tile(tileno, TILEMODE_NONE)))
					tc -> addTile(tileno, tile);

				tile = dsttm -> tile(tileno, TILEMODE_RW);

				if (tile) {
					if (srctile -> height() == TILE_HEIGHT)
						nrows = QMIN(tile -> height(), dymod);
					else
						nrows = srctile -> height() - rows;

					tile -> lock();
					dst = tile -> data(dxmod, 0);
					src = srctile -> data(sxmod, rows);
					tileBlt(dst, tile, src, srctile, nrows, cols, op);
					tile -> release();
				}
			}

			if (xxtra < 0) {
				tile = srctm -> tile(sx + TILE_WIDTH - (sx % TILE_WIDTH) + 1, sy, TILEMODE_READ);

				if (tile) {
					if (dsttile -> width() == TILE_WIDTH)
						ncols = QMIN(tile -> width(), sxmod);
					else
						ncols = dsttile -> width() - cols;

					tile -> lock();
					dst = dsttile -> data(cols, dymod);
					src = tile -> data(0, symod);
					tileBlt(dst, dsttile, src, tile, rows, ncols, op);
					tile -> release();
				}
			} else if (xxtra > 0) {
				tileno = dsttm -> tileNum(x + TILE_WIDTH - (x % TILE_WIDTH) + 1, y);

				if (m_transaction && (tile = dsttm -> tile(tileno, TILEMODE_NONE)))
					tc -> addTile(tileno, tile);

				tile = dsttm -> tile(tileno, TILEMODE_RW);

				if (tile) {
					if (srctile -> width() == TILE_WIDTH)
						ncols = QMIN(tile -> width(), dxmod);
					else
						ncols = srctile -> width() - cols;

					tile -> lock();
					dst = tile -> data(0, dymod);
					src = srctile -> data(cols, symod);
					tileBlt(dst, tile, src, srctile, rows, ncols, op);
					tile -> release();
				}
			}

			if (yxtra > 0 && xxtra > 0) {
				tileno = dsttm -> tileNum(x + TILE_WIDTH - (x % TILE_WIDTH) + 1, y + TILE_HEIGHT - (y % TILE_HEIGHT) + 1);

				if (m_transaction && (tile = dsttm -> tile(tileno, TILEMODE_NONE)))
					tc -> addTile(tileno, tile);

				tile = dsttm -> tile(tileno, TILEMODE_RW);

				if (tile) {
					if (tile -> height() == TILE_HEIGHT)
						nrows = QMIN(dsttile -> height(), dymod);
					else if (nrows > tile -> height())
						nrows = tile -> height();

					if (tile -> width() == TILE_WIDTH)
						ncols = QMIN(dsttile -> width(), dxmod);
					else if (ncols > tile -> width())
						ncols = tile -> width();

					if (rows + nrows > srctile -> height())
						nrows = srctile -> height() - rows;

					if (cols + ncols > srctile -> width())
						ncols = srctile -> width() - cols;

					if (ncols > 0 && nrows > 0) {
						nrows = QMIN(nrows, tile -> height());
						ncols = QMIN(ncols, tile -> width());
						nrows = QMIN(nrows, dsttile -> height());
						ncols = QMIN(ncols, dsttile -> width());
						Q_ASSERT(ncols <= tile -> width());
						Q_ASSERT(ncols <= srctile -> width());
						Q_ASSERT(cols + ncols <= srctile -> width());
						Q_ASSERT(nrows <= tile -> height());
						Q_ASSERT(nrows <= srctile -> height());
						Q_ASSERT(nrows + rows <= srctile -> height());
						tile -> lock();
						dst = tile -> data();
						src = srctile -> data(cols, rows);
						tileBlt(dst, tile, src, srctile, nrows, ncols, op);
						tile -> release();
					}
				}
			} else if (yxtra < 0 && xxtra < 0) {
				tile = srctm -> tile(sx + TILE_WIDTH - (sx % TILE_WIDTH) + 1, sy + TILE_HEIGHT - (sy % TILE_HEIGHT) + 1, TILEMODE_READ);

				if (tile) {
					if (dsttile -> height() == TILE_HEIGHT) {
						nrows = QMIN(tile -> height(), symod);
					} else {
						nrows = dsttile -> height() - rows;
						nrows = QMIN(tile -> height(), nrows);
					}

					if (dsttile -> width() == TILE_WIDTH) {
						ncols = QMIN(tile -> width(), sxmod);
					} else {
						ncols = dsttile -> width() - cols;
						ncols = QMIN(tile -> width(), ncols);
					}

					if (ncols > 0 && nrows > 0) {
						nrows = QMIN(nrows, tile -> height());
						ncols = QMIN(ncols, tile -> width());
						nrows = QMIN(nrows, dsttile -> height());
						ncols = QMIN(ncols, dsttile -> width());
						Q_ASSERT(ncols <= tile -> width());
						Q_ASSERT(ncols <= dsttile -> width());
						Q_ASSERT(cols + ncols <= dsttile -> width());
						Q_ASSERT(nrows <= tile -> height());
						Q_ASSERT(nrows <= dsttile -> height());
						Q_ASSERT(nrows + rows <= dsttile -> height());
						tile -> lock();
						dst = dsttile -> data(cols, rows);
						src = tile -> data();
						tileBlt(dst, dsttile, src, tile, nrows, ncols, op);
						tile -> release();
					}
				}
			}

			dsttile -> release();
			srctile -> release();
			sx += TILE_WIDTH;
		}

		sy += TILE_HEIGHT;
	}
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
	Q_INT32 xdiff;
	Q_INT32 ydiff;
	KisTileCommand *tc;

	switch (m_device -> image() -> imgType()) {
	case IMAGE_TYPE_GREY:
	case IMAGE_TYPE_GREYA:
		src[PIXEL_GRAY] = upscale(c.R());
		src[PIXEL_GRAY_ALPHA] = opacity;
		break;
	case IMAGE_TYPE_INDEXED:
	case IMAGE_TYPE_INDEXEDA:
		src[PIXEL_INDEXED] = upscale(c.R());
		src[PIXEL_INDEXED_ALPHA] = opacity;
		break;
	case IMAGE_TYPE_RGB:
	case IMAGE_TYPE_RGBA:
		src[PIXEL_RED] = upscale(c.R());
		src[PIXEL_GREEN] = upscale(c.G());
		src[PIXEL_BLUE] = upscale(c.B());
		src[PIXEL_ALPHA] = opacity;
		break;
	case IMAGE_TYPE_CMYK:
	case IMAGE_TYPE_CMYKA:
		src[PIXEL_CYAN] = upscale(c.C());
		src[PIXEL_MAGENTA] = upscale(c.M());
		src[PIXEL_YELLOW] = upscale(c.Y());
		src[PIXEL_BLACK] = upscale(c.K());
		src[PIXEL_CMYK_ALPHA] = opacity;
		break;
	default:
		kdDebug() << "Not Implemented.\n";
		return;
	}

	stride = m_device -> image() -> depth();
	ydiff = y1 - TILE_HEIGHT * (y1 / TILE_HEIGHT);

	if (m_transaction) {
		tc = new KisTileCommand(m_transaction -> name(), m_device);
		m_transaction -> addCommand(tc);
	}

	for (y = y1; y <= y2; y += TILE_HEIGHT - ydiff) {
		xdiff = x1 - TILE_WIDTH * (x1 / TILE_WIDTH);

		for (x = x1; x <= x2; x += TILE_WIDTH - xdiff) {
			ymod = (y % TILE_HEIGHT);
			xmod = (x % TILE_WIDTH);

			if (m_transaction && (tile = tm -> tile(x, y, TILEMODE_NONE)))
				tc -> addTile(tm -> tileNum(x, y), tile);

			if (!(tile = tm -> tile(x, y, TILEMODE_WRITE)))
				continue;
		
			if (xmod > tile -> width())
				continue;

			if (ymod > tile -> height())
				continue;

			rows = tile -> height() - ymod;
			cols = tile -> width() - xmod;

			if (rows > y2 - y + 1)
				rows = y2 - y + 1;

			if (cols > x2 - x + 1)
				cols = x2 - x + 1;

			dststride = tile -> width() * tile -> depth();
			tile -> lock();
			dst = tile -> data(xmod, ymod);

			while (rows-- > 0) {
				QUANTUM *d = dst;

				for (Q_INT32 i = cols; i > 0; i--) {
					memcpy(d, src, stride * sizeof(QUANTUM));
					d += stride;
				}

				dst += dststride;
			}

			tile -> release();

			if (x > x1)
				xdiff = 0;
		}

		if (y > y1)
			ydiff = 0;
	}
}

