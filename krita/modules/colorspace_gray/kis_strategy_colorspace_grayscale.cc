/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Cyrille Berger
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

#include <kdebug.h>

#include "kis_image.h"
#include "kis_strategy_colorspace_grayscale.h"
#include "tiles/kispixeldata.h"
#include "kis_iterators_pixel.h"

namespace {
	const Q_INT32 MAX_CHANNEL_GRAYSCALE = 1;
	const Q_INT32 MAX_CHANNEL_GRAYSCALEA = 2;
}

ChannelInfo KisStrategyColorSpaceGrayscale::channelInfo[1] = { ChannelInfo("Gray", 0) };

KisStrategyColorSpaceGrayscale::KisStrategyColorSpaceGrayscale() :
	KisStrategyColorSpace("Grayscale + Alpha")
{
}

KisStrategyColorSpaceGrayscale::~KisStrategyColorSpaceGrayscale()
{
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

void KisStrategyColorSpaceGrayscale::toKoColor(const QUANTUM *src, KoColor *c)
{
	c -> setRGB(downscale(src[PIXEL_GRAY]), downscale(src[PIXEL_GRAY]), downscale(src[PIXEL_GRAY]));
}

void KisStrategyColorSpaceGrayscale::toKoColor(const QUANTUM *src, KoColor *c, QUANTUM *opacity)
{
	c -> setRGB(downscale(src[PIXEL_GRAY]), downscale(src[PIXEL_GRAY]), downscale(src[PIXEL_GRAY]));
	*opacity = src[PIXEL_GRAY_ALPHA];
}

ChannelInfo* KisStrategyColorSpaceGrayscale::channelsInfo() const
{
	return channelInfo;
}
bool KisStrategyColorSpaceGrayscale::alpha() const
{
	return true;
}

Q_INT32 KisStrategyColorSpaceGrayscale::depth() const
{
	return MAX_CHANNEL_GRAYSCALEA;
}

QImage KisStrategyColorSpaceGrayscale::convertToImage(const QUANTUM *data, Q_INT32 width, Q_INT32 height, Q_INT32 stride) const 
{
	QImage img(width, height, 32, 0, QImage::LittleEndian);

	Q_INT32 i = 0;
	uchar *j = img.bits();

	while ( i < stride * height ) {
		QUANTUM q = *( data + i + PIXEL_GRAY );

		*( j + PIXEL_ALPHA ) = *( data + i + PIXEL_GRAY_ALPHA );
		*( j + PIXEL_RED )   = q;
		*( j + PIXEL_GREEN ) = q;
		*( j + PIXEL_BLUE )  = q;
		
		i += MAX_CHANNEL_GRAYSCALEA;
		j += 4; // Because we're hard-coded 32 bits deep, 4 bytes
		
	}

	return img;
}


// void KisStrategyColorSpaceGrayscale::bitBlt(Q_INT32 stride,
// 					    QUANTUM *dst,
// 					    Q_INT32 dststride,
// 					    QUANTUM *src,
// 					    Q_INT32 srcstride,
// 					    Q_INT32 rows, 
// 					    Q_INT32 cols, 
// 					    CompositeOp op) const
// {
// 	if (rows <= 0 || cols <= 0)
// 		return;

// 	bitBlt(stride, dst, dststride, src, srcstride, OPACITY_OPAQUE, rows, cols, op);

// }

void KisStrategyColorSpaceGrayscale::bitBlt(Q_INT32 stride,
					    QUANTUM *dst, 
					    Q_INT32 dststride,
					    KisStrategyColorSpaceSP srcSpace,
					    QUANTUM *src, 
					    Q_INT32 srcstride,
					    QUANTUM opacity,
					    Q_INT32 rows, 
					    Q_INT32 cols, 
					    CompositeOp op) const
{
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


