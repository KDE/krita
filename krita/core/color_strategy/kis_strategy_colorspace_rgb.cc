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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <limits.h>
#include <stdlib.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <kdebug.h>
#include "kis_image.h"
#include "kis_strategy_colorspace_rgb.h"
#include "tiles/kispixeldata.h"

namespace {
	const Q_INT32 MAX_CHANNEL_RGB = 3;
	const Q_INT32 MAX_CHANNEL_RGBA = 4;
}

KisStrategyColorSpaceRGB::KisStrategyColorSpaceRGB() : 	m_pixmap(RENDER_WIDTH * 2, RENDER_HEIGHT * 2)
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
		// kpixmapio has a nasty bug on powerpc that shows up as rendering errors
		m_pixmap = QPixmap(img);

#else
		img = QImage(pd -> data, pd -> width, pd -> height, pd -> depth * CHAR_BIT, 0, 0, QImage::LittleEndian);
		m_pixio.putImage(&m_pixmap, 0, 0, &img);

#endif

		painter.drawPixmap(x, y, m_pixmap, 0, 0, width, height);
	}
}

void KisStrategyColorSpaceRGB::tileBlt(Q_INT32 stride,
	QUANTUM *dst,
	Q_INT32 dststride,
	QUANTUM *src,
	Q_INT32 srcstride,
	Q_INT32 rows, 
	Q_INT32 cols, 
	CompositeOp op) const
{
	Q_INT32 linesize = stride * sizeof(QUANTUM) * cols;
	QUANTUM *d;
	QUANTUM *s;
	QUANTUM alpha;
	Q_INT32 i;

	if (rows <= 0 || cols <= 0)
		return;

	switch (op) {
		case COMPOSITE_COPY:
			d = dst;
			s = src;

			while (rows-- > 0) {
				memcpy(d, s, linesize);
				d += dststride;
				s += srcstride;
			}
			break;
		case COMPOSITE_CLEAR:
			d = dst;
			s = src;

			while (rows-- > 0) {
				memset(d, 0, linesize);
				d += dststride;
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
		case COMPOSITE_NORMAL:
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
// 					alpha = (d[PIXEL_ALPHA] * (QUANTUM_MAX - s[PIXEL_ALPHA]) + s[PIXEL_ALPHA]) / QUANTUM_MAX;
// 					d[PIXEL_ALPHA] = (d[PIXEL_ALPHA] * (QUANTUM_MAX - alpha) + s[PIXEL_ALPHA]) / QUANTUM_MAX;
				}

				dst += dststride;
				src += srcstride;
			}

			break;
		default:
			kdDebug() << "Not Implemented.\n";
			abort();
			return;
	}
}

void KisStrategyColorSpaceRGB::tileBlt(Q_INT32 stride,
			QUANTUM *dst, 
			Q_INT32 dststride,
			QUANTUM *src, 
			Q_INT32 srcstride,
			QUANTUM opacity,
			Q_INT32 rows, 
			Q_INT32 cols, 
			CompositeOp op) const
{
	if (opacity == OPACITY_OPAQUE)
		return tileBlt(stride, dst, dststride, src, srcstride, rows, cols, op);

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

