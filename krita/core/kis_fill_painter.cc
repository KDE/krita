/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Bart Coppens <kde@bartcoppens.be>
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
#include "kis_selection.h"
#include "kis_fill_painter.h"
#include "kis_iterators_pixel.h"
#include "kis_iterators_infinite.h"

namespace {
}

KisFillPainter::KisFillPainter() 
	: super()
{ 
}

KisFillPainter::KisFillPainter(KisPaintDeviceSP device) : super(device)
{
}

// 'regular' filling

void KisFillPainter::fillRect(Q_INT32 x1, Q_INT32 y1, Q_INT32 w, Q_INT32 h, const KoColor& c, QUANTUM opacity)
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
        KisTileMgrSP tm = m_device -> tiles();
        Q_INT32 xmod;
        Q_INT32 ymod;
        Q_INT32 xdiff;
        Q_INT32 ydiff;

        m_device -> colorStrategy() -> nativeColor(c, opacity, src);
        stride = m_device -> depth();
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

void KisFillPainter::fillRect(const QRect& rc, KisIteratorInfiniteLinePixel src) {
	KisIteratorLinePixel lineIt(m_device, 0, rc.y(), rc.x(), rc.x() + rc.width() - 1);
	KisIteratorLinePixel stopLine(m_device, 0, rc.y() + rc.height() - 1);
	Q_INT32 depth = m_device -> depth();

	while (lineIt < stopLine) {
		KisIteratorPixel it = lineIt.begin();
		// We know src is an InfiniteLine
		KisIteratorPixel* srcLine = (KisIteratorPixel*)src;
		KisIteratorPixel stop = lineIt.end();
		while (it <= stop) {
			KisPixelRepresentation data = it;
			KisPixelRepresentation source = *srcLine;
			for(int i = 0; i < depth; i++) {
				data[i] = (QUANTUM) source[i];
			}
			++it;
			srcLine->inc();
		}
		++src;
		++lineIt;
		delete srcLine;
	}
}

// flood filling

void KisFillPainter::fillColor(int startX, int startY) {
	genericFillStart(startX, startY);

	// Now create a layer and fill it
	// XXX: size of selection, not parent layer
	KisLayerSP filled = new KisLayer(m_layer->width(), m_layer->height(),
		m_layer->colorStrategy(), "Fill Temporary Layer");
	KisFillPainter painter(filled.data());
	painter.fillRect(0, 0, m_layer->width(), m_layer->height(), m_paintColor); // XXX
	painter.end();

	genericFillEnd(filled);
}

void KisFillPainter::fillPattern(int startX, int startY) {
	genericFillStart(startX, startY);

	// Now create a layer and fill it
	// XXX: size of selection, not parent layer
	KisLayerSP filled = new KisLayer(m_layer->width(), m_layer->height(),
		m_layer->colorStrategy(), "Fill Temporary Layer");
	KisFillPainter painter(filled.data());
	painter.fillRect(QRect(0, 0, m_layer->width(), m_layer->height()), 
		KisIteratorInfiniteLinePixel(m_pattern->image(m_layer->colorStrategy()).data(),
			0, 0, 0) ); // XXX
	painter.end();

	genericFillEnd(filled);
}

void KisFillPainter::genericFillStart(int startX, int startY) {
	KisLayerSP lay = dynamic_cast<KisLayer*>(m_device.data());
	Q_ASSERT(lay); // XXX: isn't this too agressive? maybe just a return; ?

	m_layer = lay;

	if (lay -> hasSelection()) {
		m_selection = lay -> selection();
	} else {
		// Create a selection from the surrounding area
		m_selection = new KisSelection(lay, "Fill Temporary Selection");
		m_selection -> clear(QRect(0, 0, lay -> width(), lay -> height()));
		m_oldColor = new QUANTUM[m_device->depth()];
		KisIteratorPixel pixel(m_device, 0, startY, startX);
		for (int i = 0; i < lay -> depth(); i++)
			m_oldColor[i] = pixel[i];

		m_map = new bool[m_device -> width() * m_device -> height()];
		floodLine(startX, startY);
		delete[] m_map;

		delete m_oldColor;
	}
}

void KisFillPainter::genericFillEnd(KisLayerSP filled) {
	// use the selection as mask over our fill
	for (int y = 0; y < m_layer -> height(); y++) {
		for (int x = 0; x < m_layer -> width(); x++) {
			KoColor c;
			QUANTUM opacity;
			filled -> pixel(x, y, &c, &opacity);
			opacity = ((OPACITY_OPAQUE - m_selection -> selected(x, y)) * opacity)
				/ QUANTUM_MAX;
			filled -> setPixel(x, y, c, opacity); // XXX
		}
	}

	bitBlt(0, 0, m_compositeOp, filled.data(), m_opacity, 0, 0,
		m_layer -> width(), m_layer -> height());

	if (!m_layer -> hasSelection())
		delete m_selection;
}

void KisFillPainter::floodLine(int x, int y) {
	int mostRight, mostLeft = x;
	
	KisIteratorLinePixel lineIt = m_layer->iteratorPixelSelectionBegin( 0, x, -1, y);

	KisIteratorPixel pixelIt = *lineIt;
	KisIteratorPixel lastPixel = lineIt.end();

	if (difference(m_oldColor, pixelIt) == MIN_SELECTED) {
		return;
	}

	mostRight = floodSegment(x, y, x, &pixelIt, &lastPixel, Right);

	if (lastPixel < pixelIt) mostRight--;

	if (x > 0) {
		mostLeft--;

		KisIteratorLinePixel lineIt2 = m_layer->iteratorPixelSelectionBegin(0, 0, x-1, y);
		KisIteratorPixel lastPixel = lineIt2.begin();
		KisIteratorPixel pixelIt = lineIt2.end();

		mostLeft = floodSegment(x,y, mostLeft, &pixelIt, &lastPixel, Left);

		if (pixelIt < lastPixel)
			mostLeft++;
	}


	// yay for stack overflowing:
	for (int i = mostLeft; i <= mostRight; i++) {
		if (y > 0 && !m_map[(y-1)*m_device -> width() + i])
			floodLine(i, y-1);
		if (y < m_layer->height() - 1 && !m_map[(y+1)*m_device -> width() + i])
			floodLine(i, y+1);
	}
}

int KisFillPainter::floodSegment(int x, int y, int most, KisIteratorPixel* it, KisIteratorPixel* lastPixel, Direction d) {
	bool stop = false;
	QUANTUM diff;

	while( ( ( d == Right && *it <= *lastPixel) || (d == Left && *lastPixel <= *it)) && !stop)
	{
		KisPixelRepresentation data = *it;
		diff = difference(m_oldColor, data);
		if (diff == MAX_SELECTED) {
			m_selection -> setSelected(x, y, diff);
			m_map[y*m_device -> width() + x] = true;
			if (d == Right) {
				it->inc();
				x++; most++;
			} else {
				it->dec();
				x--; most--;
			}
		} else {
			stop = true;
		}
	}
	
	return most;
}

/* RGB-only I fear */
QUANTUM KisFillPainter::difference(QUANTUM* src, KisPixelRepresentation dst)
{
	QUANTUM max = 0, diff = 0;
	for (int i = 0; i < m_device->depth(); i++) {
		// added extra (QUANTUM) casts just to be on the safe side until that is fixed
		diff = QABS((QUANTUM)src[i] - (QUANTUM)dst[i]);
		if (diff > max)
			max = diff;
	}
	return (max < m_threshold) ? MAX_SELECTED : MIN_SELECTED;
}
