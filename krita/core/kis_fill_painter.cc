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

#include <qcolor.h>

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
#include "kis_selection.h"
#include "kis_fill_painter.h"
#include "kis_pixel.h"
#include "kis_iterators_pixel.h"
#include "kis_iterators_infinite.h"
#include "kis_iterator.h"

namespace {
}

KisFillPainter::KisFillPainter() 
	: super()
{ 
    m_width = m_height = -1;
}

KisFillPainter::KisFillPainter(KisPaintDeviceSP device) : super(device)
{
    m_width = m_height = -1;
}

// 'regular' filling

void KisFillPainter::fillRect(Q_INT32 x1, Q_INT32 y1, Q_INT32 w, Q_INT32 h, const QColor& c, QUANTUM opacity)
{

	Q_INT32 y;
        Q_UINT8 src[m_device->depth()]; // XXX: Change QColor to KisColor, then use channelsize from color space
	Q_UINT32 depth = m_device->depth();
        m_device->colorStrategy()->nativeColor(c, opacity, src);

	for (y = y1; y < y1 + h; y++)
	{
		KisHLineIterator hiter = m_device->createHLineIterator(x1, w, y, true);
		while( ! hiter.isDone())
		{
			memcpy((Q_UINT8 *)hiter, src, depth);
			hiter++;
		}
	}
}

#if 0 //AUTOLAYER
void KisFillPainter::fillRect(const QRect& rc, KisIteratorInfiniteLinePixel src) {
#if 0
	KisIteratorLinePixel lineIt = m_device->iteratorPixelBegin( 0, rc.x(),
			rc.x() + rc.width() - 1, rc.y());
	KisIteratorLinePixel stopLine = m_device->iteratorPixelBegin( 0, 0,
			-1, rc.y() + rc.height() - 1);

	Q_INT32 depth = m_device -> depth();

	while (lineIt < stopLine) {
		KisIteratorPixel it = lineIt.begin();
		// We know src is an InfiniteLine
		KisIteratorPixel* srcLine = (KisIteratorPixel*)src;
		KisIteratorPixel stop = lineIt.end();
		while (it <= stop) {
			KisPixel data = it;
			KisPixel source = *srcLine;
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
#endif
}
#endif //AUTOLAYER

// flood filling

void KisFillPainter::fillColor(int startX, int startY) {
	genericFillStart(startX, startY);

	// Now create a layer and fill it
	// XXX: size of selection, not parent layer
	KisLayerSP filled = new KisLayer(m_layer->colorStrategy(), "Fill Temporary Layer");
	KisFillPainter painter(filled.data());
	painter.fillRect(0, 0, m_width, m_height, m_paintColor);
	painter.end();

	genericFillEnd(filled);
}

void KisFillPainter::fillPattern(int startX, int startY) {
#if 0
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
#endif
}

void KisFillPainter::genericFillStart(int startX, int startY) {
	KisPaintDeviceSP lay = m_device.data();
	Q_ASSERT(lay); // XXX: isn't this too agressive? maybe just a return; ?

	m_layer = lay;

	if (m_width < 0 || m_height < 0) {
		if (lay->image()) {
			m_width = lay->image()->width();
			m_height = lay->image()->height();
		} else {
			kdDebug() << "KisFillPainter::genericFillStart: no size set, assuming 500x500"
				  << endl;
			m_width = m_height = 500;
		}
	}
    
	m_size = m_width * m_height;

	if (lay -> hasSelection()) {
		m_selection = lay -> selection();
	} else {
		// Create a selection from the surrounding area
		m_selection = new KisSelection(lay, "Fill Temporary Selection");
		m_selection -> clear(QRect(0, 0, m_width, m_height));
		m_oldColor = new QUANTUM[m_device->depth()];

		KisHLineIterator pixelIt = m_layer->createHLineIterator(startX, startX+1, startY, false);
		KisPixel pixel((QUANTUM*)(pixelIt));
		
		for (int i = 0; i < lay -> depth(); i++) {
			m_oldColor[i] = pixel[i];
		}

		m_cancelRequested = false;
		m_currentPercent = 0;
		m_pixelsDone = 0;
		m_map = new bool[m_size];
		for (int i = 0; i < m_size; i++)
			m_map[i] = false;
		m_size*=2;
		emit notifyProgressStage(this, i18n("Making fill outline..."), 0);
		floodLine(startX, startY);
		delete[] m_map;

		delete m_oldColor;
	}
}

void KisFillPainter::genericFillEnd(KisLayerSP filled) {
    if (m_cancelRequested) {
        m_width = m_height = -1;
		return;
    }
	// use the selection as mask over our fill        
    for (int y = 0; y < m_height; y++) {
	    KisHLineIterator line = filled->createHLineIterator(0, m_width, y, true);
	    KisHLineIterator selectionIt = m_selection->createHLineIterator(0, m_width, y, true); 
	    
	    QUANTUM selectionOpacity;
	    QColor notUsed;

	    while(! line.isDone()) {
		    QColor c;
		    QUANTUM opacity;
		    filled -> colorStrategy() -> toQColor((QUANTUM*) line, &c, &opacity);
		    m_selection -> colorStrategy() -> toQColor((QUANTUM*) selectionIt,
							       &notUsed, &selectionOpacity);
		    opacity = ((OPACITY_OPAQUE - selectionOpacity) * opacity)
			    / QUANTUM_MAX;
		    filled -> colorStrategy() -> nativeColor(c, opacity, (QUANTUM*) line);
		    ++m_pixelsDone;
		    line++;
		    selectionIt++;
	    }
	    int progressPercent = (m_pixelsDone * 100) / m_size;
	    if (progressPercent > m_currentPercent) {
		    emit notifyProgress(this, progressPercent);
		    m_currentPercent = progressPercent;
		    
		    if (m_cancelRequested) {
			    m_width = m_height = -1;
			    return;
		    }
	    }
    }
    
    bitBlt(0, 0, m_compositeOp, filled.data(), m_opacity, 0, 0,
	   m_width, m_height);
    
    emit notifyProgressDone(this);
    
    m_width = m_height = 0;
}

void KisFillPainter::floodLine(int x, int y) {
	int mostRight, mostLeft = x;
	
	KisHLineIterator pixelIt = m_layer->createHLineIterator(x, m_width, y, false);

	int lastPixel = m_width;

	if (difference(m_oldColor, ((QUANTUM*)pixelIt)) == MIN_SELECTED) {
		return;
	}

	mostRight = floodSegment(x, y, x, pixelIt, lastPixel, Right);

	if (lastPixel < pixelIt.x()) mostRight--;

	if (x > 0) {
		mostLeft--;

		KisHLineIterator pixelIt = m_layer->createHLineIterator(x - 1, m_width - 1, y, false);
		int lastPixel = 0;

		mostLeft = floodSegment(x,y, mostLeft, pixelIt, lastPixel, Left);

		if (pixelIt.x() < lastPixel)
			mostLeft++;
	}

	int progressPercent = (m_pixelsDone * 100) / m_size;
	if (progressPercent > m_currentPercent) {
		emit notifyProgress(this, progressPercent);
		m_currentPercent = progressPercent;

		if (m_cancelRequested) {
			return;
		}
	}

	// yay for stack overflowing:
	for (int i = mostLeft; i <= mostRight; i++) {
		if (y > 0 && !m_map[(y-1)*m_width + i])
			floodLine(i, y-1);
		if (y < m_height - 1 && !m_map[(y+1)*m_width + i])
			floodLine(i, y+1);
	}
}

int KisFillPainter::floodSegment(int x, int y, int most, KisHLineIterator& it, int lastPixel, Direction d) {
	bool stop = false;
	QUANTUM diff;
	KisHLineIterator selection = m_selection -> createHLineIterator(x, m_width - x, y, true);
	QColor selectionColor = Qt::white; // This is the standard selection colour
	KisStrategyColorSpaceSP colorStrategy = m_selection -> colorStrategy();

	while( ( ( d == Right && it.x() <= lastPixel) || (d == Left && lastPixel <= it.x())) && !stop)
	{
		if (m_map[y*m_width + x])
			break;
		m_map[y*m_width + x] = true;
		++m_pixelsDone;
		KisPixel data = KisPixel((QUANTUM*)(it));
		diff = difference(m_oldColor, data);
		if (diff == MAX_SELECTED) {
			// m_selection -> setSelected(x, y, diff);
			colorStrategy -> nativeColor(selectionColor, diff, (QUANTUM*) selection);
			if (d == Right) {
				it++; selection++;
				x++; most++;
			} else {
				it--; selection--;
				x--; most--;
			}
		} else {
			stop = true;
		}
	}
	
	return most;
}

/* RGB-only I fear */
QUANTUM KisFillPainter::difference(QUANTUM* src, KisPixel dst)
{
	QUANTUM max = 0, diff = 0;
	int depth = m_device->depth();

	for (int i = 0; i < depth; i++) {
		// added extra (QUANTUM) casts just to be on the safe side until that is fixed
		diff = QABS((QUANTUM)src[i] - (QUANTUM)dst[i]);
		if (diff > max)
			max = diff;
	}
	return (max < m_threshold) ? MAX_SELECTED : MIN_SELECTED;
}
