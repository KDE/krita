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

#include <config.h>
#include LCMS_HEADER

#include <qimage.h>

#include <klocale.h>
#include <kdebug.h>

#include "kis_strategy_colorspace.h"
#include "kis_colorspace_registry.h"
#include "kis_image.h"
#include "kis_strategy_colorspace_grayscale.h"
#include "kis_iterators_pixel.h"

namespace {
	const Q_INT32 MAX_CHANNEL_GRAYSCALE = 1;
	const Q_INT32 MAX_CHANNEL_GRAYSCALEA = 2;
}


KisStrategyColorSpaceGrayscale::KisStrategyColorSpaceGrayscale() :
	KisStrategyColorSpace(KisID("GRAYA", i18n("Grayscale/Alpha")), TYPE_GRAYA_8, icSigGrayData)
{
	m_channels.push_back(new KisChannelInfo(i18n("gray"), 0, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("alpha"), 1, ALPHA));
}


KisStrategyColorSpaceGrayscale::~KisStrategyColorSpaceGrayscale()
{
}

void KisStrategyColorSpaceGrayscale::nativeColor(const QColor& c, QUANTUM *dst, KisProfileSP /*profile*/)
{
	// Use qGray for a better rgb -> gray formula: (r*11 + g*16 + b*5)/32.
	dst[PIXEL_GRAY] = upscale(qGray(c.red(), c.green(), c.blue()));
}

void KisStrategyColorSpaceGrayscale::nativeColor(const QColor& c, QUANTUM opacity, QUANTUM *dst, KisProfileSP /*profile*/)
{
	dst[PIXEL_GRAY] = upscale(qGray(c.red(), c.green(), c.blue()));
	dst[PIXEL_GRAY_ALPHA] = opacity;
}

void KisStrategyColorSpaceGrayscale::toQColor(const QUANTUM *src, QColor *c, KisProfileSP /*profile*/)
{
	c -> setRgb(downscale(src[PIXEL_GRAY]), downscale(src[PIXEL_GRAY]), downscale(src[PIXEL_GRAY]));
}

void KisStrategyColorSpaceGrayscale::toQColor(const QUANTUM *src, QColor *c, QUANTUM *opacity, KisProfileSP /*profile*/)
{
	c -> setRgb(downscale(src[PIXEL_GRAY]), downscale(src[PIXEL_GRAY]), downscale(src[PIXEL_GRAY]));
	*opacity = src[PIXEL_GRAY_ALPHA];
}

Q_INT8 KisStrategyColorSpaceGrayscale::difference(const QUANTUM* src1, const QUANTUM* src2)
{
	return QABS(src2[PIXEL_GRAY] - src1[PIXEL_GRAY]);
}

vKisChannelInfoSP KisStrategyColorSpaceGrayscale::channels() const
{
	return m_channels;
}

bool KisStrategyColorSpaceGrayscale::alpha() const
{
	return true;
}

Q_INT32 KisStrategyColorSpaceGrayscale::nChannels() const
{
	return MAX_CHANNEL_GRAYSCALEA;
}

Q_INT32 KisStrategyColorSpaceGrayscale::nColorChannels() const
{
	return MAX_CHANNEL_GRAYSCALE;
}

Q_INT32 KisStrategyColorSpaceGrayscale::pixelSize() const
{
	return MAX_CHANNEL_GRAYSCALEA;
}


QImage KisStrategyColorSpaceGrayscale::convertToQImage(const QUANTUM *data, Q_INT32 width, Q_INT32 height,
						       KisProfileSP srcProfile, KisProfileSP dstProfile,
						       Q_INT32 renderingIntent)
{

	QImage img(width, height, 32, 0, QImage::LittleEndian);

	// No profiles
	if (srcProfile == 0 || dstProfile == 0) {
		Q_INT32 i = 0;
		uchar *j = img.bits();

		while ( i < width * height * MAX_CHANNEL_GRAYSCALEA) {
			QUANTUM q = *( data + i + PIXEL_GRAY );

			// XXX: Temporarily moved here to get rid of these global constants
			const PIXELTYPE PIXEL_BLUE = 0;
			const PIXELTYPE PIXEL_GREEN = 1;
			const PIXELTYPE PIXEL_RED = 2;
			const PIXELTYPE PIXEL_ALPHA = 3;

			*( j + PIXEL_ALPHA ) = *( data + i + PIXEL_GRAY_ALPHA );
			*( j + PIXEL_RED )   = q;
			*( j + PIXEL_GREEN ) = q;
			*( j + PIXEL_BLUE )  = q;

			i += MAX_CHANNEL_GRAYSCALEA;

			j += 4; // Because we're hard-coded 32 bits deep, 4 bytes
		}
		return img;
	}
	else {
		// Do a nice calibrated conversion
		KisStrategyColorSpaceSP dstCS = KisColorSpaceRegistry::instance() -> get("RGBA");
		convertPixelsTo(const_cast<QUANTUM *>(data), srcProfile,
				img.bits(), dstCS, dstProfile,
				width * height, renderingIntent);
	}


	// Create display transform if not present
	return img;
}

void KisStrategyColorSpaceGrayscale::bitBlt(Q_INT32 pixelSize,
				      QUANTUM *dst,
				      Q_INT32 dstRowStride,
				      const QUANTUM *src,
				      Q_INT32 srcRowStride,
				      QUANTUM opacity,
				      Q_INT32 rows,
				      Q_INT32 cols,
				      const KisCompositeOp& op)
{
	switch (op.op()) {
	case COMPOSITE_OVER:
		compositeOver(dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_MULT:
		compositeMultiply(dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_DIVIDE:
		compositeDivide(dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_DARKEN:
		compositeDarken(dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_LIGHTEN:
		compositeLighten(dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_SCREEN:
		compositeScreen(dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_OVERLAY:
		compositeOverlay(dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_DODGE:
		compositeDodge(dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_BURN:
		compositeBurn(dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_COPY: {
		QUANTUM *d;
		const QUANTUM *s;
		Q_INT32 linesize;

		linesize = pixelSize * sizeof(QUANTUM) * cols;
		d = dst;
		s = src;
		while (rows-- > 0) {
			memcpy(d, s, linesize);
			d += dstRowStride;
			s += srcRowStride;
		}
	}
		break;
	case COMPOSITE_CLEAR: {
		QUANTUM *d;
		Q_INT32 linesize;

		linesize = pixelSize * sizeof(QUANTUM) * cols;
		d = dst;
		while (rows-- > 0) {
			memset(d, 0, linesize);
			d += dstRowStride;
		}
	}
		break;
	default:
		break;
	}
}

KisCompositeOpList KisStrategyColorSpaceGrayscale::userVisiblecompositeOps() const
{
	KisCompositeOpList list;

	list.append(KisCompositeOp(COMPOSITE_OVER));
	list.append(KisCompositeOp(COMPOSITE_MULT));
	list.append(KisCompositeOp(COMPOSITE_BURN));
	list.append(KisCompositeOp(COMPOSITE_DODGE));
	list.append(KisCompositeOp(COMPOSITE_DIVIDE));
	list.append(KisCompositeOp(COMPOSITE_SCREEN));
	list.append(KisCompositeOp(COMPOSITE_OVERLAY));
	list.append(KisCompositeOp(COMPOSITE_DARKEN));
	list.append(KisCompositeOp(COMPOSITE_LIGHTEN));

	return list;
}

inline int INT_MULT(int a, int b)
{
	int c = a * b + 0x80;
	return ((c >> 8) + c) >> 8;
}

inline int INT_DIVIDE(int a, int b)
{
	int c = (a * QUANTUM_MAX + (b / 2)) / b;
	return c;
}

inline int INT_BLEND(int a, int b, int alpha)
{
	return INT_MULT(a - b, alpha) + b;
}

inline int MIN(int a, int b)
{
	return a < b ? a : b;
}

inline int MAX(int a, int b)
{
	return a > b ? a : b;
}

void KisStrategyColorSpaceGrayscale::compositeOver(QUANTUM *dstRowStart, Q_INT32 dstRowStride, const QUANTUM *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const QUANTUM *src = srcRowStart;
		QUANTUM *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			QUANTUM srcAlpha = src[PIXEL_GRAY_ALPHA];

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = INT_MULT(src[PIXEL_GRAY_ALPHA], opacity);
				}

				if (srcAlpha == OPACITY_OPAQUE) {
					memcpy(dst, src, MAX_CHANNEL_GRAYSCALEA * sizeof(QUANTUM));
				} else {
					QUANTUM dstAlpha = dst[PIXEL_GRAY_ALPHA];

					QUANTUM srcBlend;

					if (dstAlpha == OPACITY_OPAQUE) {
						srcBlend = srcAlpha;
					} else {
						QUANTUM newAlpha = dstAlpha + INT_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
						dst[PIXEL_GRAY_ALPHA] = newAlpha;

						if (newAlpha != 0) {
							srcBlend = INT_DIVIDE(srcAlpha, newAlpha);
						} else {
							srcBlend = srcAlpha;
						}
					}

					if (srcBlend == OPACITY_OPAQUE) {
						memcpy(dst, src, MAX_CHANNEL_GRAYSCALE * sizeof(QUANTUM));
					} else {
						dst[PIXEL_GRAY] = INT_BLEND(src[PIXEL_GRAY], dst[PIXEL_GRAY], srcBlend);
					}
				}
			}

			columns--;
			src += MAX_CHANNEL_GRAYSCALEA;
			dst += MAX_CHANNEL_GRAYSCALEA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

void KisStrategyColorSpaceGrayscale::compositeMultiply(QUANTUM *dstRowStart, Q_INT32 dstRowStride, const QUANTUM *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const QUANTUM *src = srcRowStart;
		QUANTUM *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			QUANTUM srcAlpha = src[PIXEL_GRAY_ALPHA];
			QUANTUM dstAlpha = dst[PIXEL_GRAY_ALPHA];

			srcAlpha = MIN(srcAlpha, dstAlpha);

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = INT_MULT(src[PIXEL_GRAY_ALPHA], opacity);
				}

				QUANTUM srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					QUANTUM newAlpha = dstAlpha + INT_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_GRAY_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = INT_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				QUANTUM srcColor = src[PIXEL_GRAY];
				QUANTUM dstColor = dst[PIXEL_GRAY];

				srcColor = INT_MULT(srcColor, dstColor);

				dst[PIXEL_GRAY] = INT_BLEND(srcColor, dstColor, srcBlend);
			}

			columns--;
			src += MAX_CHANNEL_GRAYSCALEA;
			dst += MAX_CHANNEL_GRAYSCALEA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

void KisStrategyColorSpaceGrayscale::compositeDivide(QUANTUM *dstRowStart, Q_INT32 dstRowStride, const QUANTUM *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const QUANTUM *src = srcRowStart;
		QUANTUM *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			QUANTUM srcAlpha = src[PIXEL_GRAY_ALPHA];
			QUANTUM dstAlpha = dst[PIXEL_GRAY_ALPHA];

			srcAlpha = MIN(srcAlpha, dstAlpha);

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = INT_MULT(src[PIXEL_GRAY_ALPHA], opacity);
				}

				QUANTUM srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					QUANTUM newAlpha = dstAlpha + INT_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_GRAY_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = INT_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				for (int channel = 0; channel < MAX_CHANNEL_GRAYSCALE; channel++) {

					QUANTUM srcColor = src[channel];
					QUANTUM dstColor = dst[channel];

					srcColor = MIN((dstColor * (QUANTUM_MAX + 1)) / (1 + srcColor), QUANTUM_MAX);

					QUANTUM newColor = INT_BLEND(srcColor, dstColor, srcBlend);

					dst[channel] = newColor;
				}
			}

			columns--;
			src += MAX_CHANNEL_GRAYSCALEA;
			dst += MAX_CHANNEL_GRAYSCALEA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

void KisStrategyColorSpaceGrayscale::compositeScreen(QUANTUM *dstRowStart, Q_INT32 dstRowStride, const QUANTUM *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const QUANTUM *src = srcRowStart;
		QUANTUM *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			QUANTUM srcAlpha = src[PIXEL_GRAY_ALPHA];
			QUANTUM dstAlpha = dst[PIXEL_GRAY_ALPHA];

			srcAlpha = MIN(srcAlpha, dstAlpha);

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = INT_MULT(src[PIXEL_GRAY_ALPHA], opacity);
				}

				QUANTUM srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					QUANTUM newAlpha = dstAlpha + INT_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_GRAY_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = INT_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				for (int channel = 0; channel < MAX_CHANNEL_GRAYSCALE; channel++) {

					QUANTUM srcColor = src[channel];
					QUANTUM dstColor = dst[channel];

					srcColor = QUANTUM_MAX - INT_MULT(QUANTUM_MAX - dstColor, QUANTUM_MAX - srcColor);

					QUANTUM newColor = INT_BLEND(srcColor, dstColor, srcBlend);

					dst[channel] = newColor;
				}
			}

			columns--;
			src += MAX_CHANNEL_GRAYSCALEA;
			dst += MAX_CHANNEL_GRAYSCALEA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

void KisStrategyColorSpaceGrayscale::compositeOverlay(QUANTUM *dstRowStart, Q_INT32 dstRowStride, const QUANTUM *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const QUANTUM *src = srcRowStart;
		QUANTUM *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			QUANTUM srcAlpha = src[PIXEL_GRAY_ALPHA];
			QUANTUM dstAlpha = dst[PIXEL_GRAY_ALPHA];

			srcAlpha = MIN(srcAlpha, dstAlpha);

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = INT_MULT(src[PIXEL_GRAY_ALPHA], opacity);
				}

				QUANTUM srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					QUANTUM newAlpha = dstAlpha + INT_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_GRAY_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = INT_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				for (int channel = 0; channel < MAX_CHANNEL_GRAYSCALE; channel++) {

					QUANTUM srcColor = src[channel];
					QUANTUM dstColor = dst[channel];

					srcColor = INT_MULT(dstColor, dstColor + INT_MULT(2 * srcColor, QUANTUM_MAX - dstColor));

					QUANTUM newColor = INT_BLEND(srcColor, dstColor, srcBlend);

					dst[channel] = newColor;
				}
			}

			columns--;
			src += MAX_CHANNEL_GRAYSCALEA;
			dst += MAX_CHANNEL_GRAYSCALEA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

void KisStrategyColorSpaceGrayscale::compositeDodge(QUANTUM *dstRowStart, Q_INT32 dstRowStride, const QUANTUM *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const QUANTUM *src = srcRowStart;
		QUANTUM *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			QUANTUM srcAlpha = src[PIXEL_GRAY_ALPHA];
			QUANTUM dstAlpha = dst[PIXEL_GRAY_ALPHA];

			srcAlpha = MIN(srcAlpha, dstAlpha);

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = INT_MULT(src[PIXEL_GRAY_ALPHA], opacity);
				}

				QUANTUM srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					QUANTUM newAlpha = dstAlpha + INT_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_GRAY_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = INT_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				for (int channel = 0; channel < MAX_CHANNEL_GRAYSCALE; channel++) {

					QUANTUM srcColor = src[channel];
					QUANTUM dstColor = dst[channel];

					srcColor = MIN((dstColor * (QUANTUM_MAX + 1)) / (QUANTUM_MAX + 1 - srcColor), QUANTUM_MAX);

					QUANTUM newColor = INT_BLEND(srcColor, dstColor, srcBlend);

					dst[channel] = newColor;
				}
			}

			columns--;
			src += MAX_CHANNEL_GRAYSCALEA;
			dst += MAX_CHANNEL_GRAYSCALEA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

void KisStrategyColorSpaceGrayscale::compositeBurn(QUANTUM *dstRowStart, Q_INT32 dstRowStride, const QUANTUM *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const QUANTUM *src = srcRowStart;
		QUANTUM *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			QUANTUM srcAlpha = src[PIXEL_GRAY_ALPHA];
			QUANTUM dstAlpha = dst[PIXEL_GRAY_ALPHA];

			srcAlpha = MIN(srcAlpha, dstAlpha);

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = INT_MULT(src[PIXEL_GRAY_ALPHA], opacity);
				}

				QUANTUM srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					QUANTUM newAlpha = dstAlpha + INT_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_GRAY_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = INT_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				for (int channel = 0; channel < MAX_CHANNEL_GRAYSCALE; channel++) {

					QUANTUM srcColor = src[channel];
					QUANTUM dstColor = dst[channel];

					srcColor = MIN(((QUANTUM_MAX - dstColor) * (QUANTUM_MAX + 1)) / (srcColor + 1), QUANTUM_MAX);
					srcColor = CLAMP(QUANTUM_MAX - srcColor, 0, QUANTUM_MAX);

					QUANTUM newColor = INT_BLEND(srcColor, dstColor, srcBlend);

					dst[channel] = newColor;
				}
			}

			columns--;
			src += MAX_CHANNEL_GRAYSCALEA;
			dst += MAX_CHANNEL_GRAYSCALEA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

void KisStrategyColorSpaceGrayscale::compositeDarken(QUANTUM *dstRowStart, Q_INT32 dstRowStride, const QUANTUM *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const QUANTUM *src = srcRowStart;
		QUANTUM *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			QUANTUM srcAlpha = src[PIXEL_GRAY_ALPHA];
			QUANTUM dstAlpha = dst[PIXEL_GRAY_ALPHA];

			srcAlpha = MIN(srcAlpha, dstAlpha);

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = INT_MULT(src[PIXEL_GRAY_ALPHA], opacity);
				}

				QUANTUM srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					QUANTUM newAlpha = dstAlpha + INT_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_GRAY_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = INT_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				for (int channel = 0; channel < MAX_CHANNEL_GRAYSCALE; channel++) {

					QUANTUM srcColor = src[channel];
					QUANTUM dstColor = dst[channel];

					srcColor = MIN(srcColor, dstColor);

					QUANTUM newColor = INT_BLEND(srcColor, dstColor, srcBlend);

					dst[channel] = newColor;
				}
			}

			columns--;
			src += MAX_CHANNEL_GRAYSCALEA;
			dst += MAX_CHANNEL_GRAYSCALEA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

void KisStrategyColorSpaceGrayscale::compositeLighten(QUANTUM *dstRowStart, Q_INT32 dstRowStride, const QUANTUM *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const QUANTUM *src = srcRowStart;
		QUANTUM *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			QUANTUM srcAlpha = src[PIXEL_GRAY_ALPHA];
			QUANTUM dstAlpha = dst[PIXEL_GRAY_ALPHA];

			srcAlpha = MIN(srcAlpha, dstAlpha);

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = INT_MULT(src[PIXEL_GRAY_ALPHA], opacity);
				}

				QUANTUM srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					QUANTUM newAlpha = dstAlpha + INT_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_GRAY_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = INT_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				for (int channel = 0; channel < MAX_CHANNEL_GRAYSCALE; channel++) {

					QUANTUM srcColor = src[channel];
					QUANTUM dstColor = dst[channel];

					srcColor = MAX(srcColor, dstColor);

					QUANTUM newColor = INT_BLEND(srcColor, dstColor, srcBlend);

					dst[channel] = newColor;
				}
			}

			columns--;
			src += MAX_CHANNEL_GRAYSCALEA;
			dst += MAX_CHANNEL_GRAYSCALEA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

