/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
#include <stdlib.h>
#include <string.h>

#include "qbrush.h"
#include "qcolor.h"
#include "qfontinfo.h"
#include "qfontmetrics.h"
#include "qpen.h"
#include "qpointarray.h"
#include "qregion.h"
#include "qwmatrix.h"
#include <qimage.h>
#include <qmap.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpointarray.h>
#include <qrect.h>
#include <qstring.h>

#include <kdebug.h>
#include <kcommand.h>
#include <koColor.h>

#include "kis_brush.h"
#include "kis_colorspace_factory.h"
#include "kis_global.h"
#include "kis_gradient.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_pattern.h"
#include "kis_strategy_colorspace.h"
#include "kis_tile_command.h"
#include "kis_types.h"
#include "kis_vec.h"
#include "kispixeldata.h"
#include "kistile.h"
#include "kistilemgr.h"

KisPainter::KisPainter()
{
	init();
}

KisPainter::KisPainter(KisPaintDeviceSP device)
{
	init();
        begin(device);
}

void KisPainter::init()
{
	m_transaction = 0;

	m_dab = 0;
	m_brush = 0;
	m_pattern= 0;
	m_gradient = 0;
	m_opacity = OPACITY_OPAQUE;
	m_compositeOp = COMPOSITE_OVER;
}

KisPainter::~KisPainter()
{
	m_brush = 0;
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

void KisPainter::beginTransaction(const QString& customName)
{
        if (m_transaction)
                delete m_transaction;
        m_transaction = new KisTileCommand(customName, m_device);
}

KCommand *KisPainter::endTransaction()
{
        KCommand *command = m_transaction;

        m_transaction = 0;
        return command;
}

void KisPainter::tileBlt(QUANTUM *dst,
			 KisTileSP dsttile,
			 QUANTUM *src,
			 KisTileSP srctile,
			 QUANTUM opacity,
			 Q_INT32 rows,
			 Q_INT32 cols,
			 CompositeOp op)
{
        Q_INT32 dststride = dsttile -> width() * dsttile -> depth();
        Q_INT32 srcstride = srctile -> width() * srctile -> depth();
        Q_INT32 stride = m_device -> image() -> depth();
        KisColorSpaceFactoryInterface *factory = KisColorSpaceFactoryInterface::singleton();
        KisStrategyColorSpaceSP strategy;

        Q_ASSERT(factory);
        strategy = factory -> create(m_device);

        if (strategy)
                strategy -> tileBlt(stride, dst, dststride, src, srcstride, opacity, rows, cols, op);
}

void KisPainter::tileBlt(QUANTUM *dst,
			 KisTileSP dsttile,
			 QUANTUM *src,
			 KisTileSP srctile,
			 Q_INT32 rows,
			 Q_INT32 cols,
			 CompositeOp op)
{
        Q_INT32 dststride = dsttile -> width() * dsttile -> depth();
        Q_INT32 srcstride = srctile -> width() * srctile -> depth();
        Q_INT32 stride = m_device -> image() -> depth();
        KisColorSpaceFactoryInterface *factory = KisColorSpaceFactoryInterface::singleton();
        KisStrategyColorSpaceSP strategy;

        Q_ASSERT(factory);
        strategy = factory -> create(m_device);

        if (strategy)
                strategy -> tileBlt(stride, dst, dststride, src, srcstride, rows, cols, op);
}

void KisPainter::bitBlt(Q_INT32 dx, Q_INT32 dy, CompositeOp op,
                        KisPaintDeviceSP srcdev,
                        Q_INT32 sx, Q_INT32 sy, Q_INT32 sw, Q_INT32 sh)
{
        bitBlt(dx, dy, op, srcdev, OPACITY_OPAQUE, sx, sy, sw, sh);
}

void KisPainter::bitBlt(Q_INT32 dx, Q_INT32 dy, CompositeOp op,
                        KisPaintDeviceSP srcdev,
                        QUANTUM opacity, Q_INT32 sx, Q_INT32 sy, Q_INT32 sw, Q_INT32 sh)
{
	if (srcdev == 0) {
		kdDebug() << "bitBlt: source is null.\n";
		return;
	}
	if (m_device == 0) {
		kdDebug() << "bitBlt: no device to blt onto.\n";
		return;
	}

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
        Q_INT32 nrows = 0;
        Q_INT32 ncols = 0;
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

        for (y = dy; y <= dy2; y += TILE_HEIGHT) {
                sx = sx2;

                for (x = dx; x <= dx2; x += TILE_WIDTH) {
                        tileno = dsttm -> tileNum(x, y);

                        if (m_transaction && (dsttile = dsttm -> tile(tileno, TILEMODE_NONE)))
                                m_transaction -> addTile(tileno, dsttile);

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
                        cols = QMIN(cols, srctile -> width());
                        rows = QMIN(rows, srctile -> height());
                        tileBlt(dst, dsttile, src, srctile, opacity, rows, cols, op);

                        if (yxtra < 0) {
                                tile = srctm -> tile(sx, sy + TILE_HEIGHT - (sy % TILE_HEIGHT) + 1, TILEMODE_READ);

                                if (tile) {
                                        if (dsttile -> height() == TILE_HEIGHT) {
                                                nrows = QMIN(tile -> height(), symod);
                                        } else {
                                                nrows = dsttile -> height() - rows;
                                                nrows = QMIN(tile -> height(), nrows);
                                        }

                                        tile -> lock();
                                        dst = dsttile -> data(dxmod, rows);
                                        src = tile -> data(sxmod, 0);
                                        tileBlt(dst, dsttile, src, tile, opacity, nrows, cols, op);
                                        tile -> release();
                                }
                        } else if (yxtra > 0) {
                                tileno = dsttm -> tileNum(x, y + TILE_HEIGHT - (y % TILE_HEIGHT) + 1);

                                if (m_transaction && (tile = dsttm -> tile(tileno, TILEMODE_NONE)))
                                        m_transaction -> addTile(tileno, tile);

                                tile = dsttm -> tile(tileno, TILEMODE_RW);

                                if (tile) {
                                        if (srctile -> height() == TILE_HEIGHT) {
                                                nrows = QMIN(tile -> height(), dymod);
                                        } else {
                                                nrows = srctile -> height() - rows;
                                                nrows = QMIN(tile -> height(), nrows);
                                        }

                                        if (nrows > 0) {
                                                tile -> lock();
                                                dst = tile -> data(dxmod, 0);
                                                src = srctile -> data(sxmod, rows);
                                                tileBlt(dst, tile, src, srctile, opacity, nrows, cols, op);
                                                tile -> release();
                                        }
                                }
                        }

                        if (xxtra < 0) {
                                tile = srctm -> tile(sx + TILE_WIDTH - (sx % TILE_WIDTH) + 1, sy, TILEMODE_READ);

                                if (tile) {
                                        if (dsttile -> width() == TILE_WIDTH) {
                                                ncols = QMIN(tile -> width(), sxmod);
                                        } else {
                                                ncols = dsttile -> width() - cols;
                                                ncols = QMIN(tile -> width(), ncols);
                                        }

                                        tile -> lock();
                                        dst = dsttile -> data(cols, dymod);
                                        src = tile -> data(0, symod);
                                        tileBlt(dst, dsttile, src, tile, opacity, rows, ncols, op);
                                        tile -> release();
                                }
                        } else if (xxtra > 0) {
                                tileno = dsttm -> tileNum(x + TILE_WIDTH - (x % TILE_WIDTH) + 1, y);

                                if (m_transaction && (tile = dsttm -> tile(tileno, TILEMODE_NONE)))
                                        m_transaction -> addTile(tileno, tile);

                                tile = dsttm -> tile(tileno, TILEMODE_RW);

                                if (tile) {
                                        if (srctile -> width() == TILE_WIDTH) {
                                                ncols = QMIN(tile -> width(), dxmod);
                                        } else {
                                                ncols = srctile -> width() - cols;
                                                ncols = QMIN(tile -> width(), ncols);
                                        }

                                        if (ncols > 0) {
                                                tile -> lock();
                                                dst = tile -> data(0, dymod);
                                                src = srctile -> data(cols, symod);
                                                tileBlt(dst, tile, src, srctile, opacity, rows, ncols, op);
                                                tile -> release();
                                        }
                                }
                        }

                        if (yxtra > 0 && xxtra > 0) {
                                tileno = dsttm -> tileNum(x + TILE_WIDTH - (x % TILE_WIDTH) + 1,
                                                          y + TILE_HEIGHT - (y % TILE_HEIGHT) + 1);

                                if (m_transaction && (tile = dsttm -> tile(tileno, TILEMODE_NONE)))
                                        m_transaction -> addTile(tileno, tile);

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
                                                tileBlt(dst, tile, src, srctile, opacity, nrows, ncols, op);
                                                tile -> release();
                                        }
                                }
                        } else if (yxtra < 0 && xxtra < 0) {
                                tile = srctm -> tile(sx + TILE_WIDTH - (sx % TILE_WIDTH) + 1,
                                                     sy + TILE_HEIGHT - (sy % TILE_HEIGHT) + 1,
                                                     TILEMODE_READ);

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
                                                tileBlt(dst, dsttile, src, tile, opacity, nrows, ncols, op);
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
        KisColorSpaceFactoryInterface *factory = KisColorSpaceFactoryInterface::singleton();
        KisStrategyColorSpaceSP strategy;

        Q_ASSERT(factory);
        strategy = factory -> create(m_device);
        strategy -> nativeColor(c, opacity, src);
        stride = m_device -> image() -> depth();
        ydiff = y1 - TILE_HEIGHT * (y1 / TILE_HEIGHT);

        for (y = y1; y <= y2; y += TILE_HEIGHT - ydiff) {
                xdiff = x1 - TILE_WIDTH * (x1 / TILE_WIDTH);

                for (x = x1; x <= x2; x += TILE_WIDTH - xdiff) {
                        ymod = (y % TILE_HEIGHT);
                        xmod = (x % TILE_WIDTH);

                        if (m_transaction && (tile = tm -> tile(x, y, TILEMODE_NONE)))
                                m_transaction -> addTile(tm -> tileNum(x, y), tile);

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

QRect KisPainter::dirtyRect() {
	QRect r = m_dirtyRect;
	m_dirtyRect = QRect();
	return r;
}


float KisPainter::paintLine(const enumPaintOp paintOp,
			    const QPoint & pos1,
			    const QPoint & pos2,
			    const Q_INT32 pressure,
			    const Q_INT32 xTilt,
			    const Q_INT32 yTilt,
			    const float inSavedDist)
{
	if (!m_device) return 0;

	float savedDist = inSavedDist;

	if (savedDist < 0) {
		// pos1 is the start of the line, rather than this being a 
		// continuation of an existing line, so always paint it.
		switch(paintOp) {
		case PAINTOP_BRUSH:
			paintAt(pos1, pressure, xTilt, yTilt);
			break;
		case PAINTOP_ERASE:
			eraseAt(pos1, pressure, xTilt, yTilt);
			break;
		default:
			kdDebug() << "Paint operation not implemented yet.\n";
		}
		savedDist = 0;
	}

	Q_INT32 xSpacing = m_brush -> xSpacing(pressure);
	Q_INT32 ySpacing = m_brush -> ySpacing(pressure);

	if (xSpacing <= 0) {
		xSpacing = 1;
	}
	if (ySpacing <= 0) {
		ySpacing = 1;
	}

	double xScale = 1;
	double yScale = 1;
	double spacing;
	// Scale x or y so that we effectively have a square brush
	// and calculate distance in that coordinate space. We reverse this scaling
	// before drawing the brush. This produces the correct spacing in both
	// x and y directions, even if the brush's aspect ratio is not 1:1.
	if (xSpacing > ySpacing) {
		yScale = (double)xSpacing / ySpacing;
		spacing = xSpacing;
	}
	else {
		xScale = (double)ySpacing / xSpacing;
		spacing = ySpacing;
	}

	KisVector2D end(pos2);
	KisVector2D start(pos1);

	KisVector2D dragVec = end - start;

	dragVec.setX(dragVec.x() * xScale);
	dragVec.setY(dragVec.y() * yScale);

	float newDist = dragVec.length();
	float dist = savedDist + newDist;
	float l_savedDist = savedDist;

	if (static_cast<int>(dist) < spacing) {
		return dist;
	}

	dragVec.normalize();
	KisVector2D step(0, 0);

	while (dist >= spacing) {
		if (l_savedDist > 0) {
			step += dragVec * (spacing - l_savedDist);
			l_savedDist -= spacing;
		}
		else {
			step += dragVec * spacing;
		}
		
		KisVector2D sp = start + KisVector2D(step.x() / xScale, step.y() / yScale);

		QPoint p(qRound(sp.x()), qRound(sp.y()));

		switch(paintOp) {
		case PAINTOP_BRUSH:
			paintAt(p, pressure, xTilt, yTilt);
			break;
		case PAINTOP_ERASE:
			eraseAt(p, pressure, xTilt, yTilt);
			break;
		case PAINTOP_AIRBRUSH:
			airBrushAt(p, pressure, xTilt, yTilt);
			break;
		default:
			kdDebug() << "Paint operation not implemented yet.\n";
		}

		dist -= spacing;
	}

	if (dist > 0)
		return dist;
	else
		return 0;
}

void KisPainter::paintAt(const QPoint & pos,
			 const Q_INT32 pressure,
                         const Q_INT32 /*xTilt*/,
                         const Q_INT32 /*yTilt*/)
{
	// Painting should be implemented according to the following algorithm:
	// retrieve brush
	// if brush == mask
	//          retrieve mask
	// else if brush == image
	//          retrieve image
	// subsample (mask | image) for position -- pos should be double!
	// apply filters to mask (colour | gradient | pattern | etc.
	// composite filtered mask into temporary layer
	// composite temporary layer into target layer
	// @see: doc/brush.txt

	// Note that this paint op is not correct: it really should do
	// sub-pixel positioning of the dab to create a soft line
	// effect. What is does now is equivalent to the 'pen' tool in
	// the Gimp and, presumably, Photoshop.


	if (!m_device) return;

	QPoint hotSpot = m_brush -> hotSpot(pressure);
	Q_INT32 x = pos.x() - hotSpot.x();
	Q_INT32 y = pos.y() - hotSpot.y();

	// This is going to be sloooooow!
	if (m_pressure != pressure || m_brush -> brushType() == PIPE_MASK || m_brush -> brushType() == PIPE_IMAGE || m_dab == 0) {

		if (m_brush -> brushType() == IMAGE || m_brush -> brushType() == PIPE_IMAGE) {
			m_dab = m_brush -> image(pressure);
		}
		else {
			KisAlphaMask * mask = m_brush -> mask(pressure);
			computeDab(mask);
		}

		m_pressure = pressure;
	}
        
        // Draw correctly near the left and top edges
        Q_INT32 sx = 0;
        Q_INT32 sy = 0;
        if (x < 0) {
                sx = -x;
                x = 0;
        }
        if (y < 0) {
                sy = -y;
                y = 0;
        }
        
	bitBlt( x,  y,  m_compositeOp, m_dab.data(), m_opacity, sx, sy, m_dab -> width(), m_dab -> height());

	m_dirtyRect |= QRect(x, y, m_dab -> width(), m_dab -> height());
}

void KisPainter::eraseAt(const QPoint &pos,
			 const Q_INT32 pressure,
			 const Q_INT32 /*xTilt*/,
			 const Q_INT32 /*yTilt*/) 
{
// Erasing is traditionally in paint applications one of two things:
// either it is painting in the 'background' color, or it is replacing
// all pixels with transparent (black?) pixels.
//
// That's what this paint op does for now; however, anyone who has
// ever worked with paper and soft pencils knows that a sharp piece of
// eraser rubber is a pretty useful too for making sharp to fuzzy lines
// in the graphite layer, or equally useful: for smudging skin tones.
//
// A smudge tool for Krita is in the making, but when working with 
// a tablet, the eraser tip should be at least as functional as a rubber eraser.
// That means that only after repeated or forceful application should all the
// 'paint' or 'graphite' be removed from the surface -- a kind of pressure
// sensitive, incremental smudge.
//
// And there should be an option to not have the eraser work on certain
// kinds of material. Layers are just a hack for this; putting your ink work
// in one layer and your pencil in another is not the same as really working
// with the combination.
 
	if (!m_device) return;
	
	KisAlphaMask *mask = m_brush -> mask(pressure);
	QPoint hotSpot = m_brush -> hotSpot(pressure);

	QRect r = QRect(pos.x() - hotSpot.x(),
			pos.y() - hotSpot.y(),
			mask -> width(),
			mask -> height());

	m_dab = new KisLayer(mask -> width(),
			     mask -> height(),
			     m_device -> typeWithAlpha(),
			     "eraser_dab");

	if (m_device -> alpha()) {
		//kdDebug() << "Erase to inverted brush transparency.\n";
		m_dab -> setOpacity(OPACITY_OPAQUE);
		for (int y = 0; y < mask -> height(); y++) {
			for (int x = 0; x < mask -> width(); x++) {
				// the color doesn't matter, since we only composite the alpha
				m_dab -> setPixel(x, y, m_paintColor, QUANTUM_MAX - mask -> alphaAt(x, y));
			}
		}
		bitBlt( r.x(), r.y(), COMPOSITE_ERASE, m_dab.data() );
 	} else {
 		//kdDebug() << "Erase to background colour.\n";
		m_dab -> setOpacity(OPACITY_TRANSPARENT);
		for (int y = 0; y < mask -> height(); y++) {
			for (int x = 0; x < mask -> width(); x++) {
				m_dab -> setPixel(x, y, m_backgroundColor, mask -> alphaAt(x, y));
			}
		}
		bitBlt(r.x(), r.y(), COMPOSITE_OVER, m_dab.data() );
 	}

	m_dirtyRect |= r;
}





void KisPainter::airBrushAt(const QPoint &pos,
			    const Q_INT32 pressure,
			    const Q_INT32 /*xTilt*/,
			    const Q_INT32 /*yTilt*/)
{
// See: http://www.sysf.physto.se/~klere/airbrush/ for information
// about _real_ airbrushes.
//
// Most graphics apps -- especially the simple ones like Kolourpaint
// and the previous version of this routine in Krita took a brush
// shape -- often a simple ellipse -- and filled that shape with a
// random 'spray' of single pixels.
//
// Other, more advanced graphics apps, like the Gimp or Photoshop,
// take the brush shape and paint just as with the brush paint op,
// only making the initial dab more transparent, and perhaps adding
// extra transparence near the edges. Then, using a timer, when the
// cursor stays in place, dab upon dab is positioned in the same
// place, which makes the result less and less transparent.
//
// What I want to do here is create an airbrush that approaches a real
// one. It won't use brush shapes, instead going for the old-fashioned
// circle. Depending upon pressure, both the size of the dab and the
// rate of paint deposition is determined. The edges of the dab are
// more transparent than the center, with perhaps even some fully
// transparent pixels between the near-transparent pixels.
//
// By pressing some to-be-determined key at the same time as pressing
// mouse-down, one edge of the dab is made straight, to simulate
// working with a shield.
//
// Tilt may be used to make the gradients more realistic, but I don't
// have a tablet that supports tilt.
//
// Anyway, it's exactly twenty years ago that I have held a real
// airbrush, for the first and up to now the last time...
//
	
	// For now: use the current brush shape -- it beats calculating
	// ellipes and cones, and it shows the working of the timer.
	if (!m_device) return;

	QPoint hotSpot = m_brush -> hotSpot(pressure);
	Q_INT32 x = pos.x() - hotSpot.x();
	Q_INT32 y = pos.y() - hotSpot.y();

	// This is going to be sloooooow!
	if (m_pressure != pressure || m_brush -> brushType() == PIPE_MASK || m_brush -> brushType() == PIPE_IMAGE || m_dab == 0) {

		if (m_brush -> brushType() == IMAGE || m_brush -> brushType() == PIPE_IMAGE) {
			m_dab = m_brush -> image(pressure);
		}
		else {
			KisAlphaMask * mask = m_brush -> mask(pressure);
			computeDab(mask);
		}

		m_pressure = pressure;
	}
        
        // Draw correctly near the left and top edges
        Q_INT32 sx = 0;
        Q_INT32 sy = 0;
        if (x < 0) {
                sx = -x;
                x = 0;
        }
        if (y < 0) {
                sy = -y;
                y = 0;
        }

	bitBlt( x,  y,  COMPOSITE_OVER, m_dab.data(), OPACITY_OPAQUE / 50, sx, sy, m_dab -> width(), m_dab -> height());

	m_dirtyRect |= QRect(x, y, m_dab -> width(), m_dab -> height());

#if 0
	KisView *view = getCurrentView();
	KisImage *img = m_doc->currentImg();

	if (!img)	    return false;

	KisLayer *lay = img->getCurrentLayer();
	if (!lay)       return false;

	if (!m_brush)  return false;

	if (!img->colorMode() == cm_RGB && !img->colorMode() == cm_RGBA)
		return false;

	if(!m_dragging) return false;

	int hotX = m_brush->hotSpot().x();
	int hotY = m_brush->hotSpot().y();

	int startx = pos.x() - hotX;
	int starty = pos.y() - hotY;

	QRect clipRect(startx, starty, m_brush->width(), m_brush->height());
	if (!clipRect.intersects(lay->imageExtents()))
		return false;

	clipRect = clipRect.intersect(lay->imageExtents());

	int sx = clipRect.left() - startx;
	int sy = clipRect.top() - starty;
	int ex = clipRect.right() - startx;
	int ey = clipRect.bottom() - starty;

	uchar *sl;
	uchar bv, invbv;
	uchar r, g, b, a;
	int   v;

	int red   = view->fgColor().R();
	int green = view->fgColor().G();
	int blue  = view->fgColor().B();

	bool alpha = (img->colorMode() == cm_RGBA);

	for (int y = sy; y <= ey; y++) {
		sl = m_brush->scanline(y);

		for (int x = sx; x <= ex; x++) {
			/* get a truly ??? random number and divide it by
			   desired density - if x is that number, paint
			   a pixel from brush there this turn */
			int nRandom = KApplication::random();

			bool paintPoint = false;

			if ((nRandom % density) == ((x - sx) % density))
				paintPoint = true;

			// don't keep painting over points already painted
			// this makes image too dark and grany, eventually -
			// that effect is good with the regular brush tool,
			// but not with an airbrush or with the pen tool
			if (timeout && (m_brushArray[brushWidth * (y-sy) + (x-sx) ] > 0))
				paintPoint = false;

			if (paintPoint) {
				QRgb rgb = lay -> pixel(startx + x, starty + y);

				r = qRed(rgb);
				g = qGreen(rgb);
				b = qBlue(rgb);
				bv = *(sl + x);

				if (bv == 0) 
					continue;

				invbv = 255 - bv;
				b = ((blue * bv) + (b * invbv))/255;
				g = ((green * bv) + (g * invbv))/255;
				r = ((red * bv) + (r * invbv))/255;

				if (alpha) {
					a = qAlpha(rgb);
					v = a + bv;

					if (v < 0) 
						v = 0;

					if (v > 255) 
						v = 255;

					a = (uchar)v;
				}

				rgb = alpha ? qRgba(r, g, b, a) : qRgb(r, g, b);
				lay -> setPixel(startx + x, starty + y, rgb);

				// add this point to points already painted
				if (timeout) {
					m_brushArray[brushWidth * (y-sy) + (x-sx)] = 1;
					nPoints++;
				}
			}
		}
	}

	return true;
#endif
}


void KisPainter::setBrush(KisBrush* brush)
{
	m_brush = brush;
}

void KisPainter::computeDab(KisAlphaMask* mask)
{
	// XXX: According to the SeaShore source, the Gimp uses a temporary layer
	// the size of the layer that is being painted on. Thas layer is cleared
	// between painting actions. Our temporary layer is only just big enough to 
	// contain the brush mask, and for every paintAt, the dab is composited with
	// the target layer.
	m_dab = new KisLayer(mask -> width(),
			     mask -> height(),
			     m_device -> typeWithAlpha(),
			     "dab");
       for (int y = 0; y < mask -> height(); y++) {
		for (int x = 0; x < mask -> width(); x++) {
			m_dab -> setPixel(x, y, m_paintColor, mask -> alphaAt(x, y));
		}
	}
}

