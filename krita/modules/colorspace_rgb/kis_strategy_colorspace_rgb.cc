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

#include <config.h>
#include <limits.h>
#include <stdlib.h>
#include LCMS_HEADER

#include <qimage.h>

#include <kdebug.h>
#include <klocale.h>

#include "kis_image.h"
#include "kis_strategy_colorspace_rgb.h"
#include "composite.h"
#include "kis_iterators_pixel.h"
#include "kis_color_conversions.h"

namespace {
	const Q_INT32 MAX_CHANNEL_RGB = 3;
	const Q_INT32 MAX_CHANNEL_RGBA = 4;
}

KisStrategyColorSpaceRGB::KisStrategyColorSpaceRGB() :
	KisStrategyColorSpace(KisID("RGBA", i18n("RGB/Alpha")), TYPE_BGRA_8, icSigRgbData)
{
	m_channels.push_back(new KisChannelInfo(i18n("red"), 2, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("green"), 1, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("blue"), 0, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("alpha"), 3, ALPHA));
}

KisStrategyColorSpaceRGB::~KisStrategyColorSpaceRGB()
{
}

void KisStrategyColorSpaceRGB::nativeColor(const QColor& c, QUANTUM *dst, KisProfileSP /*profile*/)
{
	dst[PIXEL_RED] = upscale(c.red());
	dst[PIXEL_GREEN] = upscale(c.green());
	dst[PIXEL_BLUE] = upscale(c.blue());
}

void KisStrategyColorSpaceRGB::nativeColor(const QColor& c, QUANTUM opacity, QUANTUM *dst, KisProfileSP /*profile*/)
{
	dst[PIXEL_RED] = upscale(c.red());
	dst[PIXEL_GREEN] = upscale(c.green());
	dst[PIXEL_BLUE] = upscale(c.blue());
	dst[PIXEL_ALPHA] = opacity;
}

void KisStrategyColorSpaceRGB::toQColor(const QUANTUM *src, QColor *c, KisProfileSP /*profile*/)
{
	c -> setRgb(downscale(src[PIXEL_RED]), downscale(src[PIXEL_GREEN]), downscale(src[PIXEL_BLUE]));
}

void KisStrategyColorSpaceRGB::toQColor(const QUANTUM *src, QColor *c, QUANTUM *opacity, KisProfileSP /*profile*/)
{
	c -> setRgb(downscale(src[PIXEL_RED]), downscale(src[PIXEL_GREEN]), downscale(src[PIXEL_BLUE]));
	*opacity = src[PIXEL_ALPHA];
}

Q_INT8 KisStrategyColorSpaceRGB::difference(const QUANTUM* src1, const QUANTUM* src2)
{
	//return KisStrategyColorSpace::difference(src1, src2);
	return QMAX(QABS(src2[PIXEL_RED] - src1[PIXEL_RED]),
				QMAX(QABS(src2[PIXEL_GREEN] - src1[PIXEL_GREEN]),
	QABS(src2[PIXEL_BLUE] - src1[PIXEL_BLUE])));
}

void KisStrategyColorSpaceRGB::mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const
{
	Q_UINT32 red=0, green=0, blue=0;
	
	while(nColors--)
	{
		red += (*colors)[PIXEL_RED] * *weights;
		green += (*colors)[PIXEL_GREEN] * *weights;
		blue += (*colors)[PIXEL_BLUE] * *weights;
		weights++;
		colors++;
	}
	
	// Now downscale to 8 bit
	red += 0x80;
	*dst++ = ((red >> 8) + red) >> 8;
	green += 0x80;
	*dst++ = ((green >> 8) + green) >> 8;
	blue += 0x80;
	*dst++ = ((blue >> 8) + blue) >> 8;
}

vKisChannelInfoSP KisStrategyColorSpaceRGB::channels() const
{
	return m_channels;
}

bool KisStrategyColorSpaceRGB::alpha() const
{
	return true;
}

Q_INT32 KisStrategyColorSpaceRGB::nChannels() const
{
	return MAX_CHANNEL_RGBA;
}

Q_INT32 KisStrategyColorSpaceRGB::nColorChannels() const
{
	return MAX_CHANNEL_RGB;
}

Q_INT32 KisStrategyColorSpaceRGB::pixelSize() const
{
	return MAX_CHANNEL_RGBA;
}

QImage KisStrategyColorSpaceRGB::convertToQImage(const QUANTUM *data, Q_INT32 width, Q_INT32 height,
						 KisProfileSP srcProfile, KisProfileSP dstProfile,
						 Q_INT32 renderingIntent)

{

#ifdef __BIG_ENDIAN__
	QImage img = QImage(width, height, 32, 0, QImage::LittleEndian);
	img.setAlphaBuffer(true);
	// Find a way to use convertPixelsTo without needing to code a
	// complete agrb color strategy or something like that.

	Q_INT32 i = 0;
	uchar *j = img.bits();

	while ( i < width * height * MAX_CHANNEL_RGBA) {

		// Swap the bytes
		*( j + 0)  = *( data + i + PIXEL_ALPHA );
		*( j + 1 ) = *( data + i + PIXEL_RED );
		*( j + 2 ) = *( data + i + PIXEL_GREEN );
		*( j + 3 ) = *( data + i + PIXEL_BLUE );

		i += MAX_CHANNEL_RGBA;

		j += MAX_CHANNEL_RGBA; // Because we're hard-coded 32 bits deep, 4 bytes

	}

#else
	QImage img = QImage(const_cast<QUANTUM *>(data), width, height, 32, 0, 0, QImage::LittleEndian);
	img.setAlphaBuffer(true);
	// XXX: The previous version of this code used the quantum data directly
	// as an optimisation. We're introducing a copy overhead here which could
	// be factored out again if needed.
	img = img.copy();
#endif

//   	kdDebug() << "convertToQImage: (" << width << ", " << height << ")"
//   		  << " srcProfile: " << srcProfile << ", " << "dstProfile: " << dstProfile << "\n";


	if (srcProfile != 0 && dstProfile != 0) {
		convertPixelsTo(img.bits(), srcProfile,
				img.bits(), this, dstProfile,
				width * height, renderingIntent);
	}

	return img;
}

void KisStrategyColorSpaceRGB::adjustBrightness(const Q_UINT8 *src, Q_UINT8 *dst, Q_INT8 adjust) const
{
	for( int i = 0; i < 3; i++)
	{
		// change the brightness
		int nd = src[ i ] + adjust;
		dst[i] = QMAX( 0, QMIN( QUANTUM_MAX, nd ) );
	}
}

void KisStrategyColorSpaceRGB::adjustContrast(const Q_UINT8 *src, Q_UINT8 *dst, Q_INT8 adjust) const
{
	double contrast = (100.0 + adjust) / 100;
	contrast *= contrast;
	
	for( int i = 0; i < 3; i++)
	{
		// change the brightness
		int nd = src[ i ];
		nd = (int)(((nd - QUANTUM_MAX / 2 ) * contrast) + QUANTUM_MAX / 2);
		dst[i] = QMAX( 0, QMIN( QUANTUM_MAX, nd ) );
	}
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

void KisStrategyColorSpaceRGB::compositeOver(QUANTUM *dstRowStart, Q_INT32 dstRowStride, const QUANTUM *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const QUANTUM *src = srcRowStart;
		QUANTUM *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			QUANTUM srcAlpha = src[PIXEL_ALPHA];

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = INT_MULT(src[PIXEL_ALPHA], opacity);
				}

				if (srcAlpha == OPACITY_OPAQUE) {
					memcpy(dst, src, MAX_CHANNEL_RGBA * sizeof(QUANTUM));
				} else {
					QUANTUM dstAlpha = dst[PIXEL_ALPHA];

					QUANTUM srcBlend;

					if (dstAlpha == OPACITY_OPAQUE) {
						srcBlend = srcAlpha;
					} else {
						QUANTUM newAlpha = dstAlpha + INT_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
						dst[PIXEL_ALPHA] = newAlpha;

						if (newAlpha != 0) {
							srcBlend = INT_DIVIDE(srcAlpha, newAlpha);
						} else {
							srcBlend = srcAlpha;
						}
					}

					if (srcBlend == OPACITY_OPAQUE) {
						memcpy(dst, src, MAX_CHANNEL_RGB * sizeof(QUANTUM));
					} else {
						dst[PIXEL_RED] = INT_BLEND(src[PIXEL_RED], dst[PIXEL_RED], srcBlend);
						dst[PIXEL_GREEN] = INT_BLEND(src[PIXEL_GREEN], dst[PIXEL_GREEN], srcBlend);
						dst[PIXEL_BLUE] = INT_BLEND(src[PIXEL_BLUE], dst[PIXEL_BLUE], srcBlend);
					}
				}
			}

			columns--;
			src += MAX_CHANNEL_RGBA;
			dst += MAX_CHANNEL_RGBA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

void KisStrategyColorSpaceRGB::compositeMultiply(QUANTUM *dstRowStart, Q_INT32 dstRowStride, const QUANTUM *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const QUANTUM *src = srcRowStart;
		QUANTUM *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			QUANTUM srcAlpha = src[PIXEL_ALPHA];
			QUANTUM dstAlpha = dst[PIXEL_ALPHA];

			srcAlpha = MIN(srcAlpha, dstAlpha);

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = INT_MULT(src[PIXEL_ALPHA], opacity);
				}

				QUANTUM srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					QUANTUM newAlpha = dstAlpha + INT_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = INT_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				QUANTUM srcColor = src[PIXEL_RED];
				QUANTUM dstColor = dst[PIXEL_RED];

				srcColor = INT_MULT(srcColor, dstColor);

				dst[PIXEL_RED] = INT_BLEND(srcColor, dstColor, srcBlend);

				srcColor = src[PIXEL_GREEN];
				dstColor = dst[PIXEL_GREEN];

				srcColor = INT_MULT(srcColor, dstColor);

				dst[PIXEL_GREEN] = INT_BLEND(srcColor, dstColor, srcBlend);

				srcColor = src[PIXEL_BLUE];
				dstColor = dst[PIXEL_BLUE];

				srcColor = INT_MULT(srcColor, dstColor);

				dst[PIXEL_BLUE] = INT_BLEND(srcColor, dstColor, srcBlend);
			}

			columns--;
			src += MAX_CHANNEL_RGBA;
			dst += MAX_CHANNEL_RGBA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

void KisStrategyColorSpaceRGB::compositeDivide(QUANTUM *dstRowStart, Q_INT32 dstRowStride, const QUANTUM *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const QUANTUM *src = srcRowStart;
		QUANTUM *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			QUANTUM srcAlpha = src[PIXEL_ALPHA];
			QUANTUM dstAlpha = dst[PIXEL_ALPHA];

			srcAlpha = MIN(srcAlpha, dstAlpha);

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = INT_MULT(src[PIXEL_ALPHA], opacity);
				}

				QUANTUM srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					QUANTUM newAlpha = dstAlpha + INT_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = INT_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

					QUANTUM srcColor = src[channel];
					QUANTUM dstColor = dst[channel];

					srcColor = MIN((dstColor * (QUANTUM_MAX + 1)) / (1 + srcColor), QUANTUM_MAX);

					QUANTUM newColor = INT_BLEND(srcColor, dstColor, srcBlend);

					dst[channel] = newColor;
				}
			}

			columns--;
			src += MAX_CHANNEL_RGBA;
			dst += MAX_CHANNEL_RGBA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

void KisStrategyColorSpaceRGB::compositeScreen(QUANTUM *dstRowStart, Q_INT32 dstRowStride, const QUANTUM *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const QUANTUM *src = srcRowStart;
		QUANTUM *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			QUANTUM srcAlpha = src[PIXEL_ALPHA];
			QUANTUM dstAlpha = dst[PIXEL_ALPHA];

			srcAlpha = MIN(srcAlpha, dstAlpha);

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = INT_MULT(src[PIXEL_ALPHA], opacity);
				}

				QUANTUM srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					QUANTUM newAlpha = dstAlpha + INT_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = INT_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

					QUANTUM srcColor = src[channel];
					QUANTUM dstColor = dst[channel];

					srcColor = QUANTUM_MAX - INT_MULT(QUANTUM_MAX - dstColor, QUANTUM_MAX - srcColor);

					QUANTUM newColor = INT_BLEND(srcColor, dstColor, srcBlend);

					dst[channel] = newColor;
				}
			}

			columns--;
			src += MAX_CHANNEL_RGBA;
			dst += MAX_CHANNEL_RGBA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

void KisStrategyColorSpaceRGB::compositeOverlay(QUANTUM *dstRowStart, Q_INT32 dstRowStride, const QUANTUM *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const QUANTUM *src = srcRowStart;
		QUANTUM *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			QUANTUM srcAlpha = src[PIXEL_ALPHA];
			QUANTUM dstAlpha = dst[PIXEL_ALPHA];

			srcAlpha = MIN(srcAlpha, dstAlpha);

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = INT_MULT(src[PIXEL_ALPHA], opacity);
				}

				QUANTUM srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					QUANTUM newAlpha = dstAlpha + INT_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = INT_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

					QUANTUM srcColor = src[channel];
					QUANTUM dstColor = dst[channel];

					srcColor = INT_MULT(dstColor, dstColor + INT_MULT(2 * srcColor, QUANTUM_MAX - dstColor));

					QUANTUM newColor = INT_BLEND(srcColor, dstColor, srcBlend);

					dst[channel] = newColor;
				}
			}

			columns--;
			src += MAX_CHANNEL_RGBA;
			dst += MAX_CHANNEL_RGBA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

void KisStrategyColorSpaceRGB::compositeDodge(QUANTUM *dstRowStart, Q_INT32 dstRowStride, const QUANTUM *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const QUANTUM *src = srcRowStart;
		QUANTUM *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			QUANTUM srcAlpha = src[PIXEL_ALPHA];
			QUANTUM dstAlpha = dst[PIXEL_ALPHA];

			srcAlpha = MIN(srcAlpha, dstAlpha);

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = INT_MULT(src[PIXEL_ALPHA], opacity);
				}

				QUANTUM srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					QUANTUM newAlpha = dstAlpha + INT_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = INT_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

					QUANTUM srcColor = src[channel];
					QUANTUM dstColor = dst[channel];

					srcColor = MIN((dstColor * (QUANTUM_MAX + 1)) / (QUANTUM_MAX + 1 - srcColor), QUANTUM_MAX);

					QUANTUM newColor = INT_BLEND(srcColor, dstColor, srcBlend);

					dst[channel] = newColor;
				}
			}

			columns--;
			src += MAX_CHANNEL_RGBA;
			dst += MAX_CHANNEL_RGBA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

void KisStrategyColorSpaceRGB::compositeBurn(QUANTUM *dstRowStart, Q_INT32 dstRowStride, const QUANTUM *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const QUANTUM *src = srcRowStart;
		QUANTUM *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			QUANTUM srcAlpha = src[PIXEL_ALPHA];
			QUANTUM dstAlpha = dst[PIXEL_ALPHA];

			srcAlpha = MIN(srcAlpha, dstAlpha);

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = INT_MULT(src[PIXEL_ALPHA], opacity);
				}

				QUANTUM srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					QUANTUM newAlpha = dstAlpha + INT_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = INT_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

					QUANTUM srcColor = src[channel];
					QUANTUM dstColor = dst[channel];

					srcColor = MIN(((QUANTUM_MAX - dstColor) * (QUANTUM_MAX + 1)) / (srcColor + 1), QUANTUM_MAX);
					srcColor = CLAMP(QUANTUM_MAX - srcColor, 0, QUANTUM_MAX);

					QUANTUM newColor = INT_BLEND(srcColor, dstColor, srcBlend);

					dst[channel] = newColor;
				}
			}

			columns--;
			src += MAX_CHANNEL_RGBA;
			dst += MAX_CHANNEL_RGBA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

void KisStrategyColorSpaceRGB::compositeDarken(QUANTUM *dstRowStart, Q_INT32 dstRowStride, const QUANTUM *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const QUANTUM *src = srcRowStart;
		QUANTUM *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			QUANTUM srcAlpha = src[PIXEL_ALPHA];
			QUANTUM dstAlpha = dst[PIXEL_ALPHA];

			srcAlpha = MIN(srcAlpha, dstAlpha);

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = INT_MULT(src[PIXEL_ALPHA], opacity);
				}

				QUANTUM srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					QUANTUM newAlpha = dstAlpha + INT_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = INT_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

					QUANTUM srcColor = src[channel];
					QUANTUM dstColor = dst[channel];

					srcColor = MIN(srcColor, dstColor);

					QUANTUM newColor = INT_BLEND(srcColor, dstColor, srcBlend);

					dst[channel] = newColor;
				}
			}

			columns--;
			src += MAX_CHANNEL_RGBA;
			dst += MAX_CHANNEL_RGBA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

void KisStrategyColorSpaceRGB::compositeLighten(QUANTUM *dstRowStart, Q_INT32 dstRowStride, const QUANTUM *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const QUANTUM *src = srcRowStart;
		QUANTUM *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			QUANTUM srcAlpha = src[PIXEL_ALPHA];
			QUANTUM dstAlpha = dst[PIXEL_ALPHA];

			srcAlpha = MIN(srcAlpha, dstAlpha);

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = INT_MULT(src[PIXEL_ALPHA], opacity);
				}

				QUANTUM srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					QUANTUM newAlpha = dstAlpha + INT_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = INT_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

					QUANTUM srcColor = src[channel];
					QUANTUM dstColor = dst[channel];

					srcColor = MAX(srcColor, dstColor);

					QUANTUM newColor = INT_BLEND(srcColor, dstColor, srcBlend);

					dst[channel] = newColor;
				}
			}

			columns--;
			src += MAX_CHANNEL_RGBA;
			dst += MAX_CHANNEL_RGBA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

void KisStrategyColorSpaceRGB::compositeHue(QUANTUM *dstRowStart, Q_INT32 dstRowStride, const QUANTUM *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const QUANTUM *src = srcRowStart;
		QUANTUM *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			QUANTUM srcAlpha = src[PIXEL_ALPHA];
			QUANTUM dstAlpha = dst[PIXEL_ALPHA];

			srcAlpha = MIN(srcAlpha, dstAlpha);

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = INT_MULT(src[PIXEL_ALPHA], opacity);
				}

				QUANTUM srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					QUANTUM newAlpha = dstAlpha + INT_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = INT_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				int dstRed = dst[PIXEL_RED];
				int dstGreen = dst[PIXEL_GREEN];
				int dstBlue = dst[PIXEL_BLUE];

				int srcHue;
				int srcSaturation;
				int srcValue;
				int dstHue;
				int dstSaturation;
				int dstValue;

				rgb_to_hsv(src[PIXEL_RED], src[PIXEL_GREEN], src[PIXEL_BLUE], &srcHue, &srcSaturation, &srcValue);
				rgb_to_hsv(dstRed, dstGreen, dstBlue, &dstHue, &dstSaturation, &dstValue);

				int srcRed;
				int srcGreen;
				int srcBlue;

				hsv_to_rgb(srcHue, dstSaturation, dstValue, &srcRed, &srcGreen, &srcBlue);

				dst[PIXEL_RED] = INT_BLEND(srcRed, dstRed, srcBlend);
				dst[PIXEL_GREEN] = INT_BLEND(srcGreen, dstGreen, srcBlend);
				dst[PIXEL_BLUE] = INT_BLEND(srcBlue, dstBlue, srcBlend);
			}

			columns--;
			src += MAX_CHANNEL_RGBA;
			dst += MAX_CHANNEL_RGBA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

void KisStrategyColorSpaceRGB::compositeSaturation(QUANTUM *dstRowStart, Q_INT32 dstRowStride, const QUANTUM *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const QUANTUM *src = srcRowStart;
		QUANTUM *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			QUANTUM srcAlpha = src[PIXEL_ALPHA];
			QUANTUM dstAlpha = dst[PIXEL_ALPHA];

			srcAlpha = MIN(srcAlpha, dstAlpha);

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = INT_MULT(src[PIXEL_ALPHA], opacity);
				}

				QUANTUM srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					QUANTUM newAlpha = dstAlpha + INT_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = INT_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				int dstRed = dst[PIXEL_RED];
				int dstGreen = dst[PIXEL_GREEN];
				int dstBlue = dst[PIXEL_BLUE];

				int srcHue;
				int srcSaturation;
				int srcValue;
				int dstHue;
				int dstSaturation;
				int dstValue;

				rgb_to_hsv(src[PIXEL_RED], src[PIXEL_GREEN], src[PIXEL_BLUE], &srcHue, &srcSaturation, &srcValue);
				rgb_to_hsv(dstRed, dstGreen, dstBlue, &dstHue, &dstSaturation, &dstValue);

				int srcRed;
				int srcGreen;
				int srcBlue;

				hsv_to_rgb(dstHue, srcSaturation, dstValue, &srcRed, &srcGreen, &srcBlue);

				dst[PIXEL_RED] = INT_BLEND(srcRed, dstRed, srcBlend);
				dst[PIXEL_GREEN] = INT_BLEND(srcGreen, dstGreen, srcBlend);
				dst[PIXEL_BLUE] = INT_BLEND(srcBlue, dstBlue, srcBlend);
			}

			columns--;
			src += MAX_CHANNEL_RGBA;
			dst += MAX_CHANNEL_RGBA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

void KisStrategyColorSpaceRGB::compositeValue(QUANTUM *dstRowStart, Q_INT32 dstRowStride, const QUANTUM *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const QUANTUM *src = srcRowStart;
		QUANTUM *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			QUANTUM srcAlpha = src[PIXEL_ALPHA];
			QUANTUM dstAlpha = dst[PIXEL_ALPHA];

			srcAlpha = MIN(srcAlpha, dstAlpha);

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = INT_MULT(src[PIXEL_ALPHA], opacity);
				}

				QUANTUM srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					QUANTUM newAlpha = dstAlpha + INT_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = INT_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				int dstRed = dst[PIXEL_RED];
				int dstGreen = dst[PIXEL_GREEN];
				int dstBlue = dst[PIXEL_BLUE];

				int srcHue;
				int srcSaturation;
				int srcValue;
				int dstHue;
				int dstSaturation;
				int dstValue;

				rgb_to_hsv(src[PIXEL_RED], src[PIXEL_GREEN], src[PIXEL_BLUE], &srcHue, &srcSaturation, &srcValue);
				rgb_to_hsv(dstRed, dstGreen, dstBlue, &dstHue, &dstSaturation, &dstValue);

				int srcRed;
				int srcGreen;
				int srcBlue;

				hsv_to_rgb(dstHue, dstSaturation, srcValue, &srcRed, &srcGreen, &srcBlue);

				dst[PIXEL_RED] = INT_BLEND(srcRed, dstRed, srcBlend);
				dst[PIXEL_GREEN] = INT_BLEND(srcGreen, dstGreen, srcBlend);
				dst[PIXEL_BLUE] = INT_BLEND(srcBlue, dstBlue, srcBlend);
			}

			columns--;
			src += MAX_CHANNEL_RGBA;
			dst += MAX_CHANNEL_RGBA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

void KisStrategyColorSpaceRGB::compositeColor(QUANTUM *dstRowStart, Q_INT32 dstRowStride, const QUANTUM *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const QUANTUM *src = srcRowStart;
		QUANTUM *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			QUANTUM srcAlpha = src[PIXEL_ALPHA];
			QUANTUM dstAlpha = dst[PIXEL_ALPHA];

			srcAlpha = MIN(srcAlpha, dstAlpha);

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = INT_MULT(src[PIXEL_ALPHA], opacity);
				}

				QUANTUM srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					QUANTUM newAlpha = dstAlpha + INT_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = INT_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				int dstRed = dst[PIXEL_RED];
				int dstGreen = dst[PIXEL_GREEN];
				int dstBlue = dst[PIXEL_BLUE];

				int srcHue;
				int srcSaturation;
				int srcLightness;
				int dstHue;
				int dstSaturation;
				int dstLightness;

				rgb_to_hls(src[PIXEL_RED], src[PIXEL_GREEN], src[PIXEL_BLUE], &srcHue, &srcLightness, &srcSaturation);
				rgb_to_hls(dstRed, dstGreen, dstBlue, &dstHue, &dstLightness, &dstSaturation);

				Q_UINT8 srcRed;
				Q_UINT8 srcGreen;
				Q_UINT8 srcBlue;

				hls_to_rgb(srcHue, dstLightness, srcSaturation, &srcRed, &srcGreen, &srcBlue);

				dst[PIXEL_RED] = INT_BLEND(srcRed, dstRed, srcBlend);
				dst[PIXEL_GREEN] = INT_BLEND(srcGreen, dstGreen, srcBlend);
				dst[PIXEL_BLUE] = INT_BLEND(srcBlue, dstBlue, srcBlend);
			}

			columns--;
			src += MAX_CHANNEL_RGBA;
			dst += MAX_CHANNEL_RGBA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

void KisStrategyColorSpaceRGB::bitBlt(Q_INT32 pixelSize,
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
	case COMPOSITE_UNDEF:
		// Undefined == no composition
		break;
	case COMPOSITE_OVER:
		compositeOver(dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_IN:
		compositeIn(pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
	case COMPOSITE_OUT:
		compositeOut(pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_ATOP:
		compositeAtop(pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_XOR:
		compositeXor(pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_PLUS:
		compositePlus(pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_MINUS:
		compositeMinus(pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_ADD:
		compositeAdd(pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_SUBTRACT:
		compositeSubtract(pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_DIFF:
		compositeDiff(pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_MULT:
		compositeMultiply(dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_DIVIDE:
		compositeDivide(dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_BUMPMAP:
		compositeBumpmap(pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_COPY:
		compositeCopy(pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_COPY_RED:
		compositeCopyRed(pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_COPY_GREEN:
		compositeCopyGreen(pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_COPY_BLUE:
		compositeCopyBlue(pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_COPY_OPACITY:
		compositeCopyOpacity(pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_CLEAR:
		compositeClear(pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_DISSOLVE:
		compositeDissolve(pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_DISPLACE:
		compositeDisplace(pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
#if 0
	case COMPOSITE_MODULATE:
		compositeModulate(pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_THRESHOLD:
		compositeThreshold(pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
#endif
	case COMPOSITE_NO:
		// No composition.
		break;
	case COMPOSITE_DARKEN:
		compositeDarken(dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_LIGHTEN:
		compositeLighten(dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_HUE:
		compositeHue(dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_SATURATION:
		compositeSaturation(dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_VALUE:
		compositeValue(dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_COLOR:
		compositeColor(dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_COLORIZE:
		compositeColorize(pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_LUMINIZE:
		compositeLuminize(pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_SCREEN:
		compositeScreen(dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_OVERLAY:
		compositeOverlay(dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_ERASE:
		compositeErase(pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_DODGE:
		compositeDodge(dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_BURN:
		compositeBurn(dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	default:
		break;
	}
}

KisCompositeOpList KisStrategyColorSpaceRGB::userVisiblecompositeOps() const
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
	list.append(KisCompositeOp(COMPOSITE_HUE));
	list.append(KisCompositeOp(COMPOSITE_SATURATION));
	list.append(KisCompositeOp(COMPOSITE_VALUE));
	list.append(KisCompositeOp(COMPOSITE_COLOR));

	return list;
}

