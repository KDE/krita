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
#include "kis_strategy_colorspace_grayscale.h"
#include "kis_strategy_colorspace_rgb.h"
#include "tiles/kispixeldata.h"
#include "kis_iterators_pixel.h"

namespace {
	const Q_INT32 MAX_CHANNEL_GRAYSCALE = 1;
	const Q_INT32 MAX_CHANNEL_GRAYSCALEA = 2;
}

ChannelInfo KisStrategyColorSpaceGrayscale::channelInfo[1] = { ChannelInfo("Gray", 1) };

KisStrategyColorSpaceGrayscale::KisStrategyColorSpaceGrayscale() : 	m_pixmap(RENDER_WIDTH * 2, RENDER_HEIGHT * 2)
{
	kdDebug() << "KisStrategyColorSpaceGrayscale::KisStrategyColorSpaceGrayscale" << endl;
	m_buf = new QUANTUM[RENDER_WIDTH * RENDER_HEIGHT * MAX_CHANNEL_GRAYSCALEA];
}

KisStrategyColorSpaceGrayscale::~KisStrategyColorSpaceGrayscale()
{
	delete[] m_buf;
}

void KisStrategyColorSpaceGrayscale::nativeColor(const KoColor& c, QUANTUM *dst)
{
	dst[PIXEL_GRAY] = upscale((c.R() + c.G() + c.B() )/3);
}

void KisStrategyColorSpaceGrayscale::nativeColor(const KoColor& c, QUANTUM opacity, QUANTUM *dst)
{
	dst[PIXEL_GRAY] = upscale((c.R() + c.G() + c.B() )/3);
	dst[PIXEL_GRAY_ALPHA] = opacity;
}

void KisStrategyColorSpaceGrayscale::nativeColor(const QColor& c, QUANTUM *dst)
{
	KoColor k = KoColor( c );
	dst[PIXEL_GRAY] = upscale((k.R() + k.G() + k.B() )/3);
}

void KisStrategyColorSpaceGrayscale::nativeColor(const QColor& c, QUANTUM opacity, QUANTUM *dst)
{
	KoColor k = KoColor( c );
	dst[PIXEL_GRAY] = upscale((k.R() + k.G() + k.B() )/3);
	dst[PIXEL_GRAY_ALPHA] = opacity;
}

void KisStrategyColorSpaceGrayscale::nativeColor(QRgb rgb, QUANTUM *dst)
{
	dst[PIXEL_GRAY] = upscale((qRed(rgb) + qGreen(rgb) + qBlue(rgb) )/3);
}

void KisStrategyColorSpaceGrayscale::nativeColor(QRgb rgb, QUANTUM opacity, QUANTUM *dst)
{
	dst[PIXEL_GRAY] = upscale((qRed(rgb) + qGreen(rgb) + qBlue(rgb) )/3);
	dst[PIXEL_GRAY_ALPHA] = opacity;
}

ChannelInfo* KisStrategyColorSpaceGrayscale::channelsInfo() const
{
	return channelInfo;
}

void KisStrategyColorSpaceGrayscale::render(KisImageSP image, QPainter& painter, Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height)
{
	kdDebug() << "KisStrategyColorSpaceGrayscale::render 1" << endl;

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


QImage KisStrategyColorSpaceGrayscale::convertToImage(KisImageSP image, Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height) const 
{
	kdDebug() << "KisStrategyColorSpaceGrayscale::convertToImage 1" << endl;
	if (!image) return QImage();

	return convertToImage(image -> tiles(), image -> depth(), x, y, width, height);
}

QImage KisStrategyColorSpaceGrayscale::convertToImage(KisTileMgrSP tm, Q_UINT32 depth, Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height) const 
{
	kdDebug() << "KisStrategyColorSpaceGrayscale::convertToImage 2" << endl;
	if (!tm) return QImage();

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
	pd -> depth = depth;
	pd -> stride = pd -> depth * pd -> width;
	pd -> owner = false;
	pd -> data = m_buf;
	tm -> readPixelData(pd);
		img = QImage(pd->width,  pd->height, 32, 0, QImage::LittleEndian);
	Q_INT32 i = 0;
	
	uchar *j = img.bits();
	QString s;
	while ( i < pd ->stride * pd -> height ) {
		QUANTUM data = *( pd->data + i + PIXEL_GRAY );

		*( j + PIXEL_ALPHA ) = *( pd->data + i + PIXEL_GRAY_ALPHA );
		*( j + PIXEL_RED )   = data;
		*( j + PIXEL_GREEN ) =  data;
		*( j + PIXEL_BLUE )  =  data;
		
		i += MAX_CHANNEL_GRAYSCALEA;
		j += 4; // Because we're hard-coded 32 bits deep, 4 bytes
		
	}
	return img;

}


void KisStrategyColorSpaceGrayscale::tileBlt(Q_INT32 stride,
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

void KisStrategyColorSpaceGrayscale::tileBlt(Q_INT32 stride,
				       QUANTUM *dst, 
				       Q_INT32 dststride,
				       QUANTUM *src, 
				       Q_INT32 srcstride,
				       QUANTUM opacity,
				       Q_INT32 rows, 
				       Q_INT32 cols, 
				       CompositeOp op) const
{
	kdDebug() << "KisStrategyColorSpaceGrayscale Compositing with: " << op <<" " << stride << " dst=" << dst << " " << dststride << " " << " src=" << src << " " << srcstride << " " << opacity << " " << rows << " " << cols << endl;
	QUANTUM *d;
	QUANTUM *s;
	Q_INT32 i;
	Q_INT32 linesize;

	if (rows <= 0 || cols <= 0)
		return;
	switch (op) {
		case COMPOSITE_COPY:
			linesize = stride * sizeof(QUANTUM) * cols;
			d = dst;
			s = src;
			while (rows-- > 0) {
				memcpy(d, s, linesize);
				d += dststride;
				s += srcstride;
			}
			return;
		case COMPOSITE_CLEAR:
			linesize = stride * sizeof(QUANTUM) * cols;
			d = dst;
			while (rows-- > 0) {
				memset(d, 0, linesize);
				d += dststride;
			}
			return;
		case COMPOSITE_OVER:
		default:
			if (opacity == OPACITY_TRANSPARENT) 
				return;
			if (opacity != OPACITY_OPAQUE) {
				while (rows-- > 0) {
					d = dst;
					s = src;
					for (i = cols; i > 0; i--, d += stride, s += stride) {
						if (s[PIXEL_GRAY_ALPHA] == OPACITY_TRANSPARENT)
							continue;
						int srcAlpha = (s[PIXEL_GRAY_ALPHA] * opacity + QUANTUM_MAX / 2) / QUANTUM_MAX;
						int dstAlpha = (d[PIXEL_GRAY_ALPHA] * (QUANTUM_MAX - srcAlpha) + QUANTUM_MAX / 2) / QUANTUM_MAX;
						d[PIXEL_GRAY]   = (d[PIXEL_GRAY]   * dstAlpha + s[PIXEL_GRAY]   * srcAlpha + QUANTUM_MAX / 2) / QUANTUM_MAX;
						d[PIXEL_GRAY_ALPHA] = (d[PIXEL_ALPHA] * (QUANTUM_MAX - srcAlpha) + srcAlpha * QUANTUM_MAX + QUANTUM_MAX / 2) / QUANTUM_MAX;
						if (d[PIXEL_GRAY_ALPHA] != 0) {
							d[PIXEL_GRAY] = (d[PIXEL_GRAY] * QUANTUM_MAX) / d[PIXEL_GRAY_ALPHA];
						}
					}
					dst += dststride;
					src += srcstride;
				}
			}
			else {
				while (rows-- > 0) {
					d = dst;
					s = src;
					for (i = cols; i > 0; i--, d += stride, s += stride) {
						if (s[PIXEL_GRAY_ALPHA] == OPACITY_TRANSPARENT)
							continue;
						if (d[PIXEL_GRAY_ALPHA] == OPACITY_TRANSPARENT || s[PIXEL_GRAY_ALPHA] == OPACITY_OPAQUE) {
							memcpy(d, s, stride * sizeof(QUANTUM));
							continue;
						}
						int srcAlpha = s[PIXEL_GRAY_ALPHA];
						int dstAlpha = (d[PIXEL_GRAY_ALPHA] * (QUANTUM_MAX - srcAlpha) + QUANTUM_MAX / 2) / QUANTUM_MAX;
						d[PIXEL_GRAY]   = (d[PIXEL_GRAY]   * dstAlpha + s[PIXEL_GRAY]   * srcAlpha + QUANTUM_MAX / 2) / QUANTUM_MAX;
						d[PIXEL_GRAY_ALPHA] = (d[PIXEL_ALPHA] * (QUANTUM_MAX - srcAlpha) + srcAlpha * QUANTUM_MAX + QUANTUM_MAX / 2) / QUANTUM_MAX;
						if (d[PIXEL_GRAY_ALPHA] != 0) {
							d[PIXEL_GRAY] = (d[PIXEL_GRAY] * QUANTUM_MAX) / d[PIXEL_GRAY_ALPHA];
						}
					}
					dst += dststride;
					src += srcstride;
				}
			}

	}
}

void KisStrategyColorSpaceGrayscale::computeDuplicatePixel(KisIteratorPixel* dst, KisIteratorPixel* dab, KisIteratorPixel* src)
{
	KisPixelRepresentationGrayscale dstPR(*dst);
	KisPixelRepresentationGrayscale dabPR(*dab);
	KisPixelRepresentationGrayscale srcPR(*src);
	dstPR.gray() = ( (QUANTUM_MAX - dabPR.gray()) * (srcPR.gray()) ) / QUANTUM_MAX;
	dstPR.alpha() =( dabPR.alpha() * (srcPR.alpha()) ) / QUANTUM_MAX;
}

void KisStrategyColorSpaceGrayscale::convertToRGBA(KisPixelRepresentation& src, KisPixelRepresentationRGB& dst)
{
	KisPixelRepresentationGrayscale prg(src);
	dst.red() = prg.gray();
	dst.green() = prg.gray();
	dst.blue() = prg.gray();
	dst.alpha() = prg.alpha();
}

void KisStrategyColorSpaceGrayscale::convertFromRGBA(KisPixelRepresentationRGB& src, KisPixelRepresentation& dst)
{
	KisPixelRepresentationGrayscale prg(dst);
	prg.gray() = (src.red() + src.green() + src.blue() ) / 3;
	prg.alpha() = src.alpha();
}


