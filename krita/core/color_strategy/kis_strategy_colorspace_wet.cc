/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
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

#include <limits.h>
#include <stdlib.h>

#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>

#include <kdebug.h>

#include "kis_image.h"
#include "kis_strategy_colorspace_wet.h"
#include "tiles/kispixeldata.h"

namespace {
	const Q_INT32 MAX_CHANNEL_WET = 8;
}

ChannelInfo KisStrategyColorSpaceWet::channelInfo[MAX_CHANNEL_WET]; // = { ChannelInfo("Red", 3), ChannelInfo("Green", 2), ChannelInfo("Blue", 1) };

KisStrategyColorSpaceWet::KisStrategyColorSpaceWet() : 	m_pixmap(RENDER_WIDTH * 2, RENDER_HEIGHT * 2)
{
	m_buf = new QUANTUM[RENDER_WIDTH * RENDER_HEIGHT * MAX_CHANNEL_WET];
}

KisStrategyColorSpaceWet::~KisStrategyColorSpaceWet()
{
	delete[] m_buf;
}

void KisStrategyColorSpaceWet::nativeColor(const KoColor& c, QUANTUM *dst)
{
}

void KisStrategyColorSpaceWet::nativeColor(const KoColor& c, QUANTUM opacity, QUANTUM *dst)
{}

void KisStrategyColorSpaceWet::nativeColor(const QColor& c, QUANTUM *dst)
{}

void KisStrategyColorSpaceWet::nativeColor(const QColor& c, QUANTUM opacity, QUANTUM *dst)
{}

void KisStrategyColorSpaceWet::nativeColor(QRgb rgb, QUANTUM *dst)
{}

void KisStrategyColorSpaceWet::nativeColor(QRgb rgb, QUANTUM opacity, QUANTUM *dst)
{}

ChannelInfo* KisStrategyColorSpaceWet::channelsInfo() const
{
	return channelInfo;
}


void KisStrategyColorSpaceWet::render(KisImageSP image, QPainter& painter, Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height)
{
	QImage img = convertToImage(image, x, y, width, height);
	if (!img.isNull()) {
#ifdef __BIG_ENDIAN__
		// kpixmapio has a nasty bug on powerpc that shows up as rendering errors
		m_pixmap = m_pixmap.convertFromImage(img);
#else
		m_pixio.putImage(&m_pixmap, 0, 0, &img);
#endif

		painter.drawPixmap(x, y, m_pixmap, 0, 0, width, height);	
	}
}


QImage KisStrategyColorSpaceWet::convertToImage(KisImageSP image, Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height) const 
{
	if (image) {
		KisTileMgrSP tm = image -> tiles();
		KisPixelDataSP pd = new KisPixelData;
		QImage img;

		pd -> mgr = 0;
		pd -> tile = 0;
		pd -> mode = TILEMODE_READ;
		pd -> x1 = x;
		pd -> x2 = x + width - 1;
		pd -> y1 = y;
		pd -> y2 = y + height - 1;
		pd -> width = pd -> x2 - pd -> x1 + 1;
		pd -> height = pd -> y2 - pd -> y1 + 1;
		pd -> depth = image -> depth();
		pd -> stride = pd -> depth * pd -> width;
		pd -> owner = false;
		pd -> data = m_buf;
		tm -> readPixelData(pd);

#ifdef __BIG_ENDIAN__
		img = QImage(pd->width,  pd->height, 32, 0, QImage::LittleEndian);
		Q_INT32 i = 0;
		uchar *j = img.bits();

		while ( i < pd ->stride * pd -> height ) {

			// Swap the bytes
			*( j + 0)  = *( pd->data + i + PIXEL_ALPHA );
			*( j + 1 ) = *( pd->data + i + PIXEL_RED );
			*( j + 2 ) = *( pd->data + i + PIXEL_GREEN );
			*( j + 3 ) = *( pd->data + i + PIXEL_BLUE );

			i += MAX_CHANNEL_RGBA;
			j += MAX_CHANNEL_RGBA; // Because we're hard-coded 32 bits deep, 4 bytes

		}

#else
		img = QImage(pd -> data, pd -> width, pd -> height, pd -> depth * QUANTUM_DEPTH, 0, 0, QImage::LittleEndian);
#endif
		return img;
	}
	else {
		return QImage();
	}
}


void KisStrategyColorSpaceWet::tileBlt(Q_INT32 stride,
				       QUANTUM *dst,
				       Q_INT32 dststride,
				       QUANTUM *src,
				       Q_INT32 srcstride,
				       Q_INT32 rows, 
				       Q_INT32 cols, 
				       CompositeOp op) const
{
	if (rows <= 0 || cols <= 0)
		return;

	tileBlt(stride, dst, dststride, src, srcstride, OPACITY_OPAQUE, rows, cols, op);

}

void KisStrategyColorSpaceWet::tileBlt(Q_INT32 stride,
				       QUANTUM *dst, 
				       Q_INT32 dststride,
				       QUANTUM *src, 
				       Q_INT32 srcstride,
				       QUANTUM opacity,
				       Q_INT32 rows, 
				       Q_INT32 cols, 
				       CompositeOp op) const
{
	if (rows <= 0 || cols <= 0)
		return;
	// There is only one composite op relevant for wet layers
}

