/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
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
#include <limits.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>
#include "kis_image.h"
#include "kis_strategy_colorspace_rgb.h"
#include "tiles/kispixeldata.h"

namespace {
	const Q_INT32 MAX_CHANNEL_RGB = 3;
	const Q_INT32 MAX_CHANNEL_RGBA = 4;
}

KisStrategyColorSpaceRGB::KisStrategyColorSpaceRGB()
{
	m_buf = new QUANTUM[RENDER_WIDTH * RENDER_HEIGHT * MAX_CHANNEL_RGBA];
}

KisStrategyColorSpaceRGB::~KisStrategyColorSpaceRGB()
{
	delete[] m_buf;
}

void KisStrategyColorSpaceRGB::nativeColor(const KoColor& c, QUANTUM *dst)
{
	dst[PIXEL_RED] = upscale(c.R());
	dst[PIXEL_GREEN] = upscale(c.G());
	dst[PIXEL_BLUE] = upscale(c.B());
}

void KisStrategyColorSpaceRGB::nativeColor(const KoColor& c, QUANTUM opacity, QUANTUM *dst)
{
	dst[PIXEL_RED] = upscale(c.R());
	dst[PIXEL_GREEN] = upscale(c.G());
	dst[PIXEL_BLUE] = upscale(c.B());
	dst[PIXEL_ALPHA] = opacity;
}

void KisStrategyColorSpaceRGB::nativeColor(const QColor& c, QUANTUM *dst)
{
	dst[PIXEL_RED] = upscale(c.red());
	dst[PIXEL_GREEN] = upscale(c.green());
	dst[PIXEL_BLUE] = upscale(c.blue());
}

void KisStrategyColorSpaceRGB::nativeColor(const QColor& c, QUANTUM opacity, QUANTUM *dst)
{
	dst[PIXEL_RED] = upscale(c.red());
	dst[PIXEL_GREEN] = upscale(c.green());
	dst[PIXEL_BLUE] = upscale(c.blue());
	dst[PIXEL_ALPHA] = opacity;
}

void KisStrategyColorSpaceRGB::nativeColor(QRgb rgb, QUANTUM *dst)
{
	dst[PIXEL_RED] = qRed(rgb);
	dst[PIXEL_GREEN] = qGreen(rgb);
	dst[PIXEL_BLUE] = qBlue(rgb);
}

void KisStrategyColorSpaceRGB::nativeColor(QRgb rgb, QUANTUM opacity, QUANTUM *dst)
{
	dst[PIXEL_RED] = qRed(rgb);
	dst[PIXEL_GREEN] = qGreen(rgb);
	dst[PIXEL_BLUE] = qBlue(rgb);
	dst[PIXEL_ALPHA] = opacity;
}

void KisStrategyColorSpaceRGB::render(KisImageSP projection, QPainter& painter, Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height)
{
	if (projection) {
		KisTileMgrSP tm = projection -> tiles();
		KisPixelDataSP pd = new KisPixelData;
		QImage img;
		QPixmap pixmap(RENDER_WIDTH, RENDER_HEIGHT); // XXX This should be a member, but need to squash a bug in KPixmapIO first.

		pd -> mgr = 0;
		pd -> tile = 0;
		pd -> mode = TILEMODE_READ;
		pd -> x1 = x;
		pd -> x2 = x + width - 1;
		pd -> y1 = y;
		pd -> y2 = y + height - 1;
		pd -> width = pd -> x2 - pd -> x1 + 1;
		pd -> height = pd -> y2 - pd -> y1 + 1;
		pd -> depth = projection -> depth();
		pd -> stride = pd -> depth * pd -> width;
		pd -> owner = false;
		pd -> data = m_buf;
		tm -> readPixelData(pd);
		img = QImage(pd -> data, pd -> width, pd -> height, pd -> depth * CHAR_BIT, 0, 0, QImage::LittleEndian);
		m_pixio.putImage(&pixmap, 0, 0, &img);
		painter.drawPixmap(x, y, pixmap, 0, 0, width, height);
	}
}

