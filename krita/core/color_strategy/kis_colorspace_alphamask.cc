/*
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
#include "kis_colorspace_alphamask.h"
#include "tiles/kispixeldata.h"
#include "kis_iterators_pixel.h"

namespace {
	const Q_INT32 MAX_CHANNEL_ALPHAMASK = 1;
}

ChannelInfo KisColorSpaceAlphaMask::channelInfo[1] = { ChannelInfo("Alpha", 0) };

KisColorSpaceAlphaMask::KisColorSpaceAlphaMask() :
	KisStrategyColorSpace("alpha mask")
{
	m_maskColor = KoColor::black();
}

KisColorSpaceAlphaMask::~KisColorSpaceAlphaMask()
{
}

void KisColorSpaceAlphaMask::nativeColor(const KoColor& /*c*/, QUANTUM *dst)
{
	dst[PIXEL_MASK] = OPACITY_OPAQUE;
}

void KisColorSpaceAlphaMask::nativeColor(const KoColor& /*c*/, QUANTUM opacity, QUANTUM *dst)
{
	dst[PIXEL_MASK] = opacity;
}

void KisColorSpaceAlphaMask::nativeColor(const QColor& /*c*/, QUANTUM *dst)
{
	dst[PIXEL_MASK] = OPACITY_OPAQUE;
}

void KisColorSpaceAlphaMask::nativeColor(const QColor& /*c*/, QUANTUM opacity, QUANTUM *dst)
{
	dst[PIXEL_MASK] = opacity;
}

void KisColorSpaceAlphaMask::nativeColor(QRgb /*rgb*/, QUANTUM *dst)
{
	dst[PIXEL_MASK] = OPACITY_OPAQUE;
}

void KisColorSpaceAlphaMask::nativeColor(QRgb /*rgb*/, QUANTUM opacity, QUANTUM *dst)
{
	dst[PIXEL_MASK] = opacity;
}

void KisColorSpaceAlphaMask::toKoColor(const QUANTUM */*src*/, KoColor *c)
{
	c = &m_maskColor;
}

void KisColorSpaceAlphaMask::toKoColor(const QUANTUM *src, KoColor *c, QUANTUM *opacity)
{
	c = &m_maskColor;
	*opacity = src[PIXEL_MASK];
}

ChannelInfo* KisColorSpaceAlphaMask::channelsInfo() const
{
	return channelInfo;
}
bool KisColorSpaceAlphaMask::alpha() const
{
	return true;
}

Q_INT32 KisColorSpaceAlphaMask::depth() const
{
	return MAX_CHANNEL_ALPHAMASK;
}

QImage KisColorSpaceAlphaMask::convertToImage(const QUANTUM *data, Q_INT32 width, Q_INT32 height, Q_INT32 stride) const 
{
	QImage img(width, height, 32, 0, QImage::LittleEndian);

	Q_INT32 i = 0;
	uchar *j = img.bits();

	while ( i < stride * height ) {
		*( j + PIXEL_MASK ) = *( data + i );
		// XXX: for previews of the mask, it would be handy to
		// make this always black.
		*( j + PIXEL_RED )   = m_maskColor.R();
		*( j + PIXEL_GREEN ) = m_maskColor.G();
		*( j + PIXEL_BLUE )  = m_maskColor.B();
		
		i += MAX_CHANNEL_ALPHAMASK;
		j += 4; // Because we're hard-coded 32 bits deep, 4 bytes
		
	}

	return img;
}

void KisColorSpaceAlphaMask::bitBlt(Q_INT32 stride,
				    QUANTUM *dst, 
				    Q_INT32 dststride,
				    QUANTUM *src, 
				    Q_INT32 srcstride,
				    QUANTUM opacity,
				    Q_INT32 rows, 
				    Q_INT32 cols, 
				    CompositeOp op)
{
// 	QUANTUM *d;
// 	QUANTUM *s;
// 	Q_INT32 i;
// 	Q_INT32 linesize;

// 	if (rows <= 0 || cols <= 0)
// 		return;
// 	switch (op) {
// 	case COMPOSITE_COPY:
// 		linesize = stride * sizeof(QUANTUM) * cols;
// 		d = dst;
// 		s = src;
// 		while (rows-- > 0) {
// 			memcpy(d, s, linesize);
// 			d += dststride;
// 			s += srcstride;
// 		}
// 		return;
// 	case COMPOSITE_CLEAR:
// 		linesize = stride * sizeof(QUANTUM) * cols;
// 		d = dst;
// 		while (rows-- > 0) {
// 			memset(d, 0, linesize);
// 			d += dststride;
// 		}
// 		return;
// 	case COMPOSITE_OVER:
// 	default:
// 		if (opacity == OPACITY_TRANSPARENT) 
// 			return;
// 		if (opacity != OPACITY_OPAQUE) {
// 			while (rows-- > 0) {
// 				d = dst;
// 				s = src;
// 				for (i = cols; i > 0; i--, d += stride, s += stride) {
// 					if (s[PIXEL_GRAY_ALPHA] == OPACITY_TRANSPARENT)
// 						continue;
// 					int srcAlpha = (s[PIXEL_GRAY_ALPHA] * opacity + QUANTUM_MAX / 2) / QUANTUM_MAX;
// 					int dstAlpha = (d[PIXEL_GRAY_ALPHA] * (QUANTUM_MAX - srcAlpha) + QUANTUM_MAX / 2) / QUANTUM_MAX;
// 					d[PIXEL_GRAY]   = (d[PIXEL_GRAY]   * dstAlpha + s[PIXEL_GRAY]   * srcAlpha + QUANTUM_MAX / 2) / QUANTUM_MAX;
// 					d[PIXEL_GRAY_ALPHA] = (d[PIXEL_MASK] * (QUANTUM_MAX - srcAlpha) + srcAlpha * QUANTUM_MAX + QUANTUM_MAX / 2) / QUANTUM_MAX;
// 					if (d[PIXEL_GRAY_ALPHA] != 0) {
// 						d[PIXEL_GRAY] = (d[PIXEL_GRAY] * QUANTUM_MAX) / d[PIXEL_GRAY_ALPHA];
// 					}
// 				}
// 				dst += dststride;
// 				src += srcstride;
// 			}
// 		}
// 		else {
// 			while (rows-- > 0) {
// 				d = dst;
// 				s = src;
// 				for (i = cols; i > 0; i--, d += stride, s += stride) {
// 					if (s[PIXEL_GRAY_ALPHA] == OPACITY_TRANSPARENT)
// 						continue;
// 					if (d[PIXEL_GRAY_ALPHA] == OPACITY_TRANSPARENT || s[PIXEL_GRAY_ALPHA] == OPACITY_OPAQUE) {
// 						memcpy(d, s, stride * sizeof(QUANTUM));
// 						continue;
// 					}
// 					int srcAlpha = s[PIXEL_GRAY_ALPHA];
// 					int dstAlpha = (d[PIXEL_GRAY_ALPHA] * (QUANTUM_MAX - srcAlpha) + QUANTUM_MAX / 2) / QUANTUM_MAX;
// 					d[PIXEL_GRAY]   = (d[PIXEL_GRAY]   * dstAlpha + s[PIXEL_GRAY]   * srcAlpha + QUANTUM_MAX / 2) / QUANTUM_MAX;
// 					d[PIXEL_GRAY_ALPHA] = (d[PIXEL_MASK] * (QUANTUM_MAX - srcAlpha) + srcAlpha * QUANTUM_MAX + QUANTUM_MAX / 2) / QUANTUM_MAX;
// 					if (d[PIXEL_GRAY_ALPHA] != 0) {
// 						d[PIXEL_GRAY] = (d[PIXEL_GRAY] * QUANTUM_MAX) / d[PIXEL_GRAY_ALPHA];
// 					}
// 				}
// 				dst += dststride;
// 				src += srcstride;
// 			}
// 		}

// 	}
}

void KisColorSpaceAlphaMask::computeDuplicatePixel(KisIteratorPixel* , KisIteratorPixel* , KisIteratorPixel* )
{
// 	KisPixelRepresentationGrayscale dstPR(*dst);
// 	KisPixelRepresentationGrayscale dabPR(*dab);
// 	KisPixelRepresentationGrayscale srcPR(*src);
// 	dstPR.gray() = ( (QUANTUM_MAX - dabPR.gray()) * (srcPR.gray()) ) / QUANTUM_MAX;
// 	dstPR.alpha() =( dabPR.alpha() * (srcPR.alpha()) ) / QUANTUM_MAX;
}

void KisColorSpaceAlphaMask::convertToRGBA(KisPixelRepresentation& , KisPixelRepresentationRGB& )
{
// 	XXX
}

void KisColorSpaceAlphaMask::convertFromRGBA(KisPixelRepresentationRGB& , KisPixelRepresentation& )
{
//     XXXX
}


