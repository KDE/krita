/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
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
#include <stdlib.h>
#include <string.h>
#include <cfloat>

#include "qbrush.h"
#include "qcolor.h"
#include "qfontinfo.h"
#include "qfontmetrics.h"
#include "qpen.h"
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
#include <klocale.h>

#include <koColor.h>

#include "kis_brush.h"
#include "kis_global.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_pattern.h"
#include "kis_rect.h"
#include "kis_strategy_colorspace.h"
#include "kis_tile_command.h"
#include "kis_types.h"
#include "kis_vec.h"
#include "kispixeldata.h"
#include "kistile.h"
#include "kistilemgr.h"
#include "kis_iterators_pixel.h"
#include "kis_paintop.h"

// Maximum distance from a Bezier control point to the line through the start
// and end points for the curve to be considered flat.
#define BEZIER_FLATNESS_THRESHOLD 0.5

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
	m_paintOp = 0;
	m_filter = 0;
	m_brush = 0;
	m_pattern= 0;
	m_opacity = OPACITY_OPAQUE;
	m_compositeOp = COMPOSITE_OVER;
	m_dab = 0;
}

KisPainter::~KisPainter()
{
	m_brush = 0;
	delete m_paintOp;
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

void KisPainter::beginTransaction( KisTileCommand* command)
{
	if (m_transaction)
		delete m_transaction;
	m_transaction = command;
}


KCommand *KisPainter::endTransaction()
{
        KCommand *command = m_transaction;

        m_transaction = 0;
        return command;
}


QRect KisPainter::dirtyRect() {
	QRect r = m_dirtyRect;
	m_dirtyRect = QRect();
	return r;
}


void KisPainter::tileBlt(QUANTUM *dst, KisTileSP dsttile, 
			 KisStrategyColorSpaceSP srcSpace, QUANTUM *src, KisTileSP srctile, 
			 Q_INT32 rows, Q_INT32 cols,
			 QUANTUM opacity, 
			 CompositeOp op)
{
        Q_INT32 dststride = dsttile -> width() * dsttile -> depth();
        Q_INT32 srcstride = srctile -> width() * srctile -> depth();
        Q_INT32 stride = m_device -> depth();
        m_device -> colorStrategy() -> bitBlt(stride, dst, dststride, srcSpace, src, srcstride, opacity, rows, cols, op);
}

void KisPainter::bitBlt(Q_INT32 dx, Q_INT32 dy, 
			CompositeOp op,
                        KisPaintDeviceSP srcdev,
                        QUANTUM opacity,
			Q_INT32 sx, Q_INT32 sy, 
			Q_INT32 sw, Q_INT32 sh)
{
	if (srcdev == 0) {
		kdDebug() << "bitBlt: source is null.\n";
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
        KisTileMgrSP dsttm = m_device -> tiles();
        KisTileMgrSP srctm = srcdev -> tiles();
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

                        tileBlt(dst, dsttile, 
				srcdev -> colorStrategy(), 
				src, 
				srctile, 
				rows, cols, 
				opacity,
				op);

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
                                        tileBlt(dst, 
						dsttile, 
						srcdev -> colorStrategy(), 
						src, 
						tile, 
						nrows, cols, 
						opacity,
						op);
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
                                                tileBlt(dst, 
							tile, 
							srcdev -> colorStrategy(), 
							src, 
							srctile, 
							nrows, cols,
							opacity, 
							op);
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
                                        tileBlt(dst, 
						dsttile, 
						srcdev -> colorStrategy(), 
						src, 
						tile, 
						rows, ncols, 
						opacity, 
						op);
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
                                                tileBlt(dst, 
							tile, 
							srcdev -> colorStrategy(), 
							src, 
							srctile, 
							rows, ncols,
							opacity, 
							op);
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
                                                tileBlt(dst, 
							tile, 
							srcdev -> colorStrategy(), 
							src, 
							srctile, 
							nrows, ncols,
							opacity, 
							op);
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
                                                tileBlt(dst, 
							dsttile, 
							srcdev -> colorStrategy(), 
							src, 
							tile, 
							nrows, ncols,
							opacity, 
							op);
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

double KisPainter::paintLine(const KisPoint & pos1,
			     const double pressure1,
			     const double xTilt1,
			     const double yTilt1,
			     const KisPoint & pos2,
			     const double pressure2,
			     const double xTilt2,
			     const double yTilt2,
			     const double inSavedDist)
{
	if (!m_device) return 0;
	if (!m_paintOp) return 0;

	double savedDist = inSavedDist;

	if (savedDist < 0) {
		m_paintOp -> paintAt(pos1, pressure1, xTilt1, yTilt1);
		savedDist = 0;
	}

	// XXX: The spacing should vary as the pressure changes along the line.
	// This is a quick simplification.
	double xSpacing = m_brush -> xSpacing((pressure1 + pressure2) / 2);
	double ySpacing = m_brush -> ySpacing((pressure1 + pressure2) / 2);

	if (xSpacing < 0.5) {
		xSpacing = 0.5;
	}
	if (ySpacing < 0.5) {
		ySpacing = 0.5;
	}

	double xScale = 1;
	double yScale = 1;
	double spacing;
	// Scale x or y so that we effectively have a square brush
	// and calculate distance in that coordinate space. We reverse this scaling
	// before drawing the brush. This produces the correct spacing in both
	// x and y directions, even if the brush's aspect ratio is not 1:1.
	if (xSpacing > ySpacing) {
		yScale = xSpacing / ySpacing;
		spacing = xSpacing;
	}
	else {
		xScale = ySpacing / xSpacing;
		spacing = ySpacing;
	}

	KisVector2D end(pos2);
	KisVector2D start(pos1);

	KisVector2D dragVec = end - start;

	dragVec.setX(dragVec.x() * xScale);
	dragVec.setY(dragVec.y() * yScale);

	double newDist = dragVec.length();
	double dist = savedDist + newDist;
	double l_savedDist = savedDist;

	if (dist < spacing) {
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

		KisPoint p(start.x() + (step.x() / xScale), start.y() + (step.y() / yScale));

		double distanceMoved = step.length();
		double t = 0;

		if (newDist > DBL_EPSILON) {
			t = distanceMoved / newDist;
		}

		double pressure = (1 - t) * pressure1 + t * pressure2;
		double xTilt = (1 - t) * xTilt1 + t * xTilt2;
		double yTilt = (1 - t) * yTilt1 + t * yTilt2;
		
		m_paintOp -> paintAt(p, pressure, xTilt, yTilt);
		dist -= spacing;
	}

	if (dist > 0)
		return dist;
	else
		return 0;
}

void KisPainter::paintPolyline (const QValueVector <KisPoint> &points,
                                int index, int numPoints)
{
	if (index >= (int) points.count ())
		return;

	if (numPoints < 0)
		numPoints = points.count ();

	if (index + numPoints > (int) points.count ())
		numPoints = points.count () - index;


	for (int i = index; i < index + numPoints - 1; i++)
	{
		paintLine (points [index], 0/*pressure*/, 0, 0, points [index + 1],
			   0/*pressure*/, 0, 0);
	}
}

double KisPainter::paintBezierCurve(const KisPoint &pos1,
				    const double pressure1,
				    const double xTilt1,
				    const double yTilt1,
				    const KisPoint &control1,
				    const KisPoint &control2,
				    const KisPoint &pos2,
				    const double pressure2,
				    const double xTilt2,
				    const double yTilt2,
				    const double savedDist)
{
	double newDistance;
	double d1 = pointToLineDistance(control1, pos1, pos2);
	double d2 = pointToLineDistance(control2, pos1, pos2);

	if (d1 < BEZIER_FLATNESS_THRESHOLD && d2 < BEZIER_FLATNESS_THRESHOLD) {
		newDistance = paintLine(pos1, pressure1, xTilt1, yTilt1, pos2, pressure2, xTilt2, yTilt2, savedDist);
	} else {
		// Midpoint subdivision. See Foley & Van Dam Computer Graphics P.508
		KisVector2D p1 = pos1;
		KisVector2D p2 = control1;
		KisVector2D p3 = control2;
		KisVector2D p4 = pos2;

		KisVector2D l2 = (p1 + p2) / 2;
		KisVector2D h = (p2 + p3) / 2;
		KisVector2D l3 = (l2 + h) / 2;
		KisVector2D r3 = (p3 + p4) / 2;
		KisVector2D r2 = (h + r3) / 2;
		KisVector2D l4 = (l3 + r2) / 2;
		KisVector2D r1 = l4;
		KisVector2D l1 = p1;
		KisVector2D r4 = p4;

		double midPressure = (pressure1 + pressure2) / 2;
		double midXTilt = (xTilt1 + xTilt2) / 2;
		double midYTilt = (yTilt1 + yTilt2) / 2;

		newDistance = paintBezierCurve(l1.toKisPoint(), pressure1, xTilt1, yTilt1, 
					       l2.toKisPoint(), l3.toKisPoint(), 
					       l4.toKisPoint(), midPressure, midXTilt, midYTilt,
					       savedDist);
		newDistance = paintBezierCurve(r1.toKisPoint(), midPressure, midXTilt, midYTilt, 
					       r2.toKisPoint(), 
					       r3.toKisPoint(), 
					       r4.toKisPoint(), pressure2, xTilt2, yTilt2, newDistance);
	}

	return newDistance;
}

void KisPainter::paintRect (const KisPoint &startPoint,
                            const KisPoint &endPoint,
                            const double pressure,
			    const double xTilt,
			    const double yTilt)
{
	KoRect normalizedRect = KisRect (startPoint, endPoint).normalize ();

	paintLine (normalizedRect.topLeft (),
		   pressure,
		   xTilt,
		   yTilt,
		   normalizedRect.topRight (),
		   pressure,
		   xTilt, 
		   yTilt);
	paintLine (normalizedRect.topRight (),
		   pressure,
		   xTilt,
		   yTilt,
		   normalizedRect.bottomRight (),
		   pressure,
		   xTilt,
		   yTilt);
	paintLine (normalizedRect.bottomRight (),
		   pressure,
		   xTilt,
		   yTilt,
		   normalizedRect.bottomLeft (),
		   pressure,
		   xTilt,
		   yTilt);
	paintLine (normalizedRect.bottomLeft (),
		   pressure,
		   xTilt,
		   yTilt,
		   normalizedRect.topLeft (),
		   pressure,
		   xTilt,
		   yTilt);
}


//
// Ellipse code derived from zSprite2 Game Engine. 
// XXX: copyright attribution needed? BSAR.
//

void KisPainter::paintEllipsePixel (bool invert,
                                    int xc, int yc, int x1, int y1, int x2, int y2,
                                    const double pressure)
{
	if (!m_paintOp) return;

	if (invert)
	{
		paintAt (QPoint (y1 + xc, x1 + yc), pressure, 0, 0);
		paintAt (QPoint (y2 + xc, x2 + yc), pressure, 0, 0);

	}
	else
	{
		paintAt (QPoint (x1 + xc, y1 + yc), pressure, 0, 0);
		paintAt (QPoint (x2 + xc, y2 + yc), pressure, 0, 0);
	}
}

void KisPainter::paintEllipseSymmetry(double ratio, bool invert,
				      int x, int y, int xc, int yc,
				      const double pressure)
{
	int x_start, x_end, x_out;
	int y_start, y_end, y_out;

	x_start = (int) (x * ratio);
	x_end = (int) ((x + 1) * ratio);
	y_start = (int) (y * ratio);
	y_end = (int) ((y + 1) * ratio);

	for (x_out = x_start; x_out < x_end; x_out++)
	{
		paintEllipsePixel (invert, xc, yc, -x_out, -y, x_out, -y, pressure);
		paintEllipsePixel (invert, xc, yc, -x_out, y, x_out, y, pressure);
	}

	for (y_out = y_start; y_out < y_end; y_out++)
	{
		paintEllipsePixel (invert, xc, yc, -y_out, -x, y_out, -x, pressure);
		paintEllipsePixel (invert, xc, yc, -y_out, x, y_out, x, pressure);
	}
}

void KisPainter::paintEllipseInternal (double ratio, bool invert,
                                       int xc, int yc, int radius,
                                       const double pressure)
{
	int x, y, d;
	//unsigned char mask, exist_color;

	y = radius;
	d = 3 - 2 * radius;

	for (x = 0; x < y;)
	{
		paintEllipseSymmetry (ratio, invert, x, y, xc, yc, pressure);

		if  (d < 0)
		{
			d += (4 * x + 6);
		}
		else
		{
			d += (4 * (x - y) + 10);
			y--;
		}

		x++;
	}

	if (x == y)
		paintEllipseSymmetry (ratio, invert, x, y, xc, yc, pressure);
}

void KisPainter::paintEllipse (const KisPoint &startPoint,
                               const KisPoint &endPoint,
                               const double pressure,
			       const double xTilt,
			       const double yTilt)
{
#if 1
	KisRect r = KisRect(startPoint, endPoint).normalize();

	// See http://www.whizkidtech.redprince.net/bezier/circle/ for explanation.
	// kappa = (4/3*(sqrt(2)-1))
	const double kappa = 0.5522847498;
	const double lx = (r.width() / 2) * kappa;
	const double ly = (r.height() / 2) * kappa;

	KisPoint center = r.center();

	KisPoint p0(r.left(), center.y());
	KisPoint p1(r.left(), center.y() - ly);
	KisPoint p2(center.x() - lx, r.top());
	KisPoint p3(center.x(), r.top());

	double distance = paintBezierCurve(p0, pressure, xTilt, yTilt, p1, p2, p3, pressure, xTilt, yTilt);

	KisPoint p4(center.x() + lx, r.top());
	KisPoint p5(r.right(), center.y() - ly);
	KisPoint p6(r.right(), center.y());

	distance = paintBezierCurve(p3, pressure, xTilt, yTilt, p4, p5, p6, pressure, xTilt, yTilt, distance);

	KisPoint p7(r.right(), center.y() + ly);
	KisPoint p8(center.x() + lx, r.bottom());
	KisPoint p9(center.x(), r.bottom());

	distance = paintBezierCurve(p6, pressure, xTilt, yTilt, p7, p8, p9, pressure, xTilt, yTilt, distance);

	KisPoint p10(center.x() - lx, r.bottom());
	KisPoint p11(r.left(), center.y() + ly);

	paintBezierCurve(p9, pressure, xTilt, yTilt, p10, p11, p0, pressure, xTilt, yTilt, distance);
#else
	QRect normalizedRect = QRect (startPoint.floorQPoint(), endPoint.floorQPoint()).normalize ();

	const int x1 = normalizedRect.left (),
		x2 = normalizedRect.right (),
		y1 = normalizedRect.top (),
		y2 = normalizedRect.bottom ();

	const double ratio = (double) (x2 - x1 + 1) / (y2 - y1 + 1);
	bool invert = false;

	if (x1 == x2 || y1 == y2)
	{
		paintLine (normalizedRect.topLeft (),
			   pressure, 0, 0,
			   normalizedRect.bottomLeft (),
			   pressure, 0, 0);
		return;
	}

	if (x2 - x1 < y2 - y1)
	{
		paintEllipseInternal (1 / ratio, true,
				      (x1 + x2) / 2, (y1 + y2) / 2, (x2 - x1 + 1) / 2,
				      pressure);
	}
	else
	{
		paintEllipseInternal (ratio, false,
				      (x1 + x2) / 2, (y1 + y2) / 2, (y2 - y1 + 1) / 2,
				      pressure);
	}
#endif
}

void KisPainter::paintAt(const KisPoint & pos,
			 const double pressure,
                         const double xTilt,
                         const double yTilt)
{
	if (!m_paintOp) return;
	m_paintOp -> paintAt(pos, pressure, xTilt, yTilt);
}

double KisPainter::pointToLineDistance(const KisPoint& p, const KisPoint& l0, const KisPoint& l1)
{
	double lineLength = sqrt((l1.x() - l0.x()) * (l1.x() - l0.x()) + (l1.y() - l0.y()) * (l1.y() - l0.y()));
	double distance = 0;

	if (lineLength > DBL_EPSILON) {
		distance = ((l0.y() - l1.y()) * p.x() + (l1.x() - l0.x()) * p.y() + l0.x() * l1.y() - l1.x() * l0.y()) / lineLength;
		distance = fabs(distance);
	}

	return distance;
}

