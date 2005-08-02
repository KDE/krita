/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
#include "kis_colorspace_template.h"
#include "kis_iterators_pixel.h"
#include "kis_integer_maths.h"

namespace {
	const Q_INT32 MAX_CHANNEL_TEMPLATE = 1;
	const Q_INT32 MAX_CHANNEL_TEMPLATEA = 2;

	const Q_INT32 TYPE_TEMPLATE = 0; // This is the lcms colormodel identifier, 0 if lcms is not appliccable
}

KisColorSpaceTemplate::KisColorSpaceTemplate() :
	KisStrategyColorSpace(KisID("TEMPLATEA", i18n("Templatescale/Alpha")), TYPE_TEMPLATE, icMaxEnumData)
{
	m_channels.push_back(new KisChannelInfo(i18n("Template"), 0, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("Alpha"), 1, ALPHA));
}


KisColorSpaceTemplate::~KisColorSpaceTemplate()
{
}

void KisColorSpaceTemplate::nativeColor(const QColor& c, Q_UINT8 *dst, KisProfileSP /*profile*/)
{
}

void KisColorSpaceTemplate::nativeColor(const QColor& c, QUANTUM opacity, Q_UINT8 *dst, KisProfileSP /*profile*/)
{
}

void KisColorSpaceTemplate::getAlpha(const Q_UINT8 *pixel, Q_UINT8 *alpha)
{
}

void KisColorSpaceTemplate::toQColor(const Q_UINT8 *src, QColor *c, KisProfileSP /*profile*/)
{
}

void KisColorSpaceTemplate::toQColor(const Q_UINT8 *src, QColor *c, QUANTUM *opacity, KisProfileSP /*profile*/)
{
}

Q_INT8 KisColorSpaceTemplate::difference(const Q_UINT8* src1, const Q_UINT8* src2)
{
}

void KisColorSpaceTemplate::mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const
{
}

vKisChannelInfoSP KisColorSpaceTemplate::channels() const
{
	return m_channels;
}

bool KisColorSpaceTemplate::hasAlpha() const
{
	return true;
}

Q_INT32 KisColorSpaceTemplate::nChannels() const
{
	return MAX_CHANNEL_TEMPLATEA;
}

Q_INT32 KisColorSpaceTemplate::nColorChannels() const
{
	return MAX_CHANNEL_TEMPLATE;
}

Q_INT32 KisColorSpaceTemplate::pixelSize() const
{
	return MAX_CHANNEL_TEMPLATEA;
}


QImage KisColorSpaceTemplate::convertToQImage(const Q_UINT8 *data, Q_INT32 width, Q_INT32 height,
						       KisProfileSP srcProfile, KisProfileSP dstProfile,
						       Q_INT32 renderingIntent, float /*exposure*/)
{

	QImage img(width, height, 32, 0, QImage::LittleEndian);

	// No profiles
	if (srcProfile == 0 || dstProfile == 0) {
		Q_INT32 i = 0;
		uchar *j = img.bits();

		while ( i < width * height * MAX_CHANNEL_TEMPLATEA) {
			Q_UINT8 q = *( data + i + PIXEL_TEMPLATE );

			// XXX: Temporarily moved here to get rid of these global constants
			const Q_UINT8 PIXEL_BLUE = 0;
			const Q_UINT8 PIXEL_GREEN = 1;
			const Q_UINT8 PIXEL_RED = 2;
			const Q_UINT8 PIXEL_ALPHA = 3;

			*( j + PIXEL_ALPHA ) = *( data + i + PIXEL_TEMPLATE_ALPHA );
			*( j + PIXEL_RED )   = q;
			*( j + PIXEL_GREEN ) = q;
			*( j + PIXEL_BLUE )  = q;

			i += MAX_CHANNEL_TEMPLATEA;

			j += 4; // Because we're hard-coded 32 bits deep, 4 bytes
		}
		return img;
	}
	else {
		// Do a nice calibrated conversion
		KisStrategyColorSpace * dstCS = KisColorSpaceRegistry::instance() -> get("RGBA");
		convertPixelsTo(const_cast<Q_UINT8 *>(data), srcProfile,
				img.bits(), dstCS, dstProfile,
				width * height, renderingIntent);
	}


	// Create display transform if not present
	return img;
}

void KisColorSpaceTemplate::bitBlt(Q_INT32 pixelSize,
				      Q_UINT8 *dst,
				      Q_INT32 dstRowStride,
				      const Q_UINT8 *src,
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
		Q_UINT8 *d;
		const Q_UINT8 *s;
		Q_INT32 linesize;

		linesize = pixelSize * sizeof(Q_UINT8) * cols;
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
		Q_UINT8 *d;
		Q_INT32 linesize;

		linesize = pixelSize * sizeof(Q_UINT8) * cols;
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

KisCompositeOpList KisColorSpaceTemplate::userVisiblecompositeOps() const
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

void KisColorSpaceTemplate::compositeOver(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const Q_UINT8 *src = srcRowStart;
		Q_UINT8 *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			Q_UINT8 srcAlpha = src[PIXEL_TEMPLATE_ALPHA];

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = UINT8_MULT(src[PIXEL_TEMPLATE_ALPHA], opacity);
				}

				if (srcAlpha == OPACITY_OPAQUE) {
					memcpy(dst, src, MAX_CHANNEL_TEMPLATEA * sizeof(Q_UINT8));
				} else {
					Q_UINT8 dstAlpha = dst[PIXEL_TEMPLATE_ALPHA];

					Q_UINT8 srcBlend;

					if (dstAlpha == OPACITY_OPAQUE) {
						srcBlend = srcAlpha;
					} else {
						Q_UINT8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
						dst[PIXEL_TEMPLATE_ALPHA] = newAlpha;

						if (newAlpha != 0) {
							srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
						} else {
							srcBlend = srcAlpha;
						}
					}

					if (srcBlend == OPACITY_OPAQUE) {
						memcpy(dst, src, MAX_CHANNEL_TEMPLATE * sizeof(Q_UINT8));
					} else {
						dst[PIXEL_TEMPLATE] = UINT8_BLEND(src[PIXEL_TEMPLATE], dst[PIXEL_TEMPLATE], srcBlend);
					}
				}
			}

			columns--;
			src += MAX_CHANNEL_TEMPLATEA;
			dst += MAX_CHANNEL_TEMPLATEA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

void KisColorSpaceTemplate::compositeMultiply(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const Q_UINT8 *src = srcRowStart;
		Q_UINT8 *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			Q_UINT8 srcAlpha = src[PIXEL_TEMPLATE_ALPHA];
			Q_UINT8 dstAlpha = dst[PIXEL_TEMPLATE_ALPHA];

			srcAlpha = QMIN(srcAlpha, dstAlpha);

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = UINT8_MULT(src[PIXEL_TEMPLATE_ALPHA], opacity);
				}

				Q_UINT8 srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					Q_UINT8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_TEMPLATE_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				Q_UINT8 srcColor = src[PIXEL_TEMPLATE];
				Q_UINT8 dstColor = dst[PIXEL_TEMPLATE];

				srcColor = UINT8_MULT(srcColor, dstColor);

				dst[PIXEL_TEMPLATE] = UINT8_BLEND(srcColor, dstColor, srcBlend);
			}

			columns--;
			src += MAX_CHANNEL_TEMPLATEA;
			dst += MAX_CHANNEL_TEMPLATEA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

void KisColorSpaceTemplate::compositeDivide(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const Q_UINT8 *src = srcRowStart;
		Q_UINT8 *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			Q_UINT8 srcAlpha = src[PIXEL_TEMPLATE_ALPHA];
			Q_UINT8 dstAlpha = dst[PIXEL_TEMPLATE_ALPHA];

			srcAlpha = QMIN(srcAlpha, dstAlpha);

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = UINT8_MULT(src[PIXEL_TEMPLATE_ALPHA], opacity);
				}

				Q_UINT8 srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					Q_UINT8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_TEMPLATE_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				for (int channel = 0; channel < MAX_CHANNEL_TEMPLATE; channel++) {

					Q_UINT8 srcColor = src[channel];
					Q_UINT8 dstColor = dst[channel];

					srcColor = QMIN((dstColor * (UINT8_MAX + 1)) / (1 + srcColor), UINT8_MAX);

					Q_UINT8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

					dst[channel] = newColor;
				}
			}

			columns--;
			src += MAX_CHANNEL_TEMPLATEA;
			dst += MAX_CHANNEL_TEMPLATEA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

void KisColorSpaceTemplate::compositeScreen(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const Q_UINT8 *src = srcRowStart;
		Q_UINT8 *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			Q_UINT8 srcAlpha = src[PIXEL_TEMPLATE_ALPHA];
			Q_UINT8 dstAlpha = dst[PIXEL_TEMPLATE_ALPHA];

			srcAlpha = QMIN(srcAlpha, dstAlpha);

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = UINT8_MULT(src[PIXEL_TEMPLATE_ALPHA], opacity);
				}

				Q_UINT8 srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					Q_UINT8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_TEMPLATE_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				for (int channel = 0; channel < MAX_CHANNEL_TEMPLATE; channel++) {

					Q_UINT8 srcColor = src[channel];
					Q_UINT8 dstColor = dst[channel];

					srcColor = UINT8_MAX - UINT8_MULT(UINT8_MAX - dstColor, UINT8_MAX - srcColor);

					Q_UINT8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

					dst[channel] = newColor;
				}
			}

			columns--;
			src += MAX_CHANNEL_TEMPLATEA;
			dst += MAX_CHANNEL_TEMPLATEA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

void KisColorSpaceTemplate::compositeOverlay(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const Q_UINT8 *src = srcRowStart;
		Q_UINT8 *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			Q_UINT8 srcAlpha = src[PIXEL_TEMPLATE_ALPHA];
			Q_UINT8 dstAlpha = dst[PIXEL_TEMPLATE_ALPHA];

			srcAlpha = QMIN(srcAlpha, dstAlpha);

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = UINT8_MULT(src[PIXEL_TEMPLATE_ALPHA], opacity);
				}

				Q_UINT8 srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					Q_UINT8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_TEMPLATE_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				for (int channel = 0; channel < MAX_CHANNEL_TEMPLATE; channel++) {

					Q_UINT8 srcColor = src[channel];
					Q_UINT8 dstColor = dst[channel];

					srcColor = UINT8_MULT(dstColor, dstColor + UINT8_MULT(2 * srcColor, UINT8_MAX - dstColor));

					Q_UINT8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

					dst[channel] = newColor;
				}
			}

			columns--;
			src += MAX_CHANNEL_TEMPLATEA;
			dst += MAX_CHANNEL_TEMPLATEA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

void KisColorSpaceTemplate::compositeDodge(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const Q_UINT8 *src = srcRowStart;
		Q_UINT8 *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			Q_UINT8 srcAlpha = src[PIXEL_TEMPLATE_ALPHA];
			Q_UINT8 dstAlpha = dst[PIXEL_TEMPLATE_ALPHA];

			srcAlpha = QMIN(srcAlpha, dstAlpha);

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = UINT8_MULT(src[PIXEL_TEMPLATE_ALPHA], opacity);
				}

				Q_UINT8 srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					Q_UINT8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_TEMPLATE_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				for (int channel = 0; channel < MAX_CHANNEL_TEMPLATE; channel++) {

					Q_UINT8 srcColor = src[channel];
					Q_UINT8 dstColor = dst[channel];

					srcColor = QMIN((dstColor * (UINT8_MAX + 1)) / (UINT8_MAX + 1 - srcColor), UINT8_MAX);

					Q_UINT8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

					dst[channel] = newColor;
				}
			}

			columns--;
			src += MAX_CHANNEL_TEMPLATEA;
			dst += MAX_CHANNEL_TEMPLATEA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

void KisColorSpaceTemplate::compositeBurn(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const Q_UINT8 *src = srcRowStart;
		Q_UINT8 *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			Q_UINT8 srcAlpha = src[PIXEL_TEMPLATE_ALPHA];
			Q_UINT8 dstAlpha = dst[PIXEL_TEMPLATE_ALPHA];

			srcAlpha = QMIN(srcAlpha, dstAlpha);

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = UINT8_MULT(src[PIXEL_TEMPLATE_ALPHA], opacity);
				}

				Q_UINT8 srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					Q_UINT8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_TEMPLATE_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				for (int channel = 0; channel < MAX_CHANNEL_TEMPLATE; channel++) {

					Q_UINT8 srcColor = src[channel];
					Q_UINT8 dstColor = dst[channel];

					srcColor = QMIN(((UINT8_MAX - dstColor) * (UINT8_MAX + 1)) / (srcColor + 1), UINT8_MAX);
					srcColor = CLAMP(UINT8_MAX - srcColor, 0, UINT8_MAX);

					Q_UINT8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

					dst[channel] = newColor;
				}
			}

			columns--;
			src += MAX_CHANNEL_TEMPLATEA;
			dst += MAX_CHANNEL_TEMPLATEA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

void KisColorSpaceTemplate::compositeDarken(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const Q_UINT8 *src = srcRowStart;
		Q_UINT8 *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			Q_UINT8 srcAlpha = src[PIXEL_TEMPLATE_ALPHA];
			Q_UINT8 dstAlpha = dst[PIXEL_TEMPLATE_ALPHA];

			srcAlpha = QMIN(srcAlpha, dstAlpha);

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = UINT8_MULT(src[PIXEL_TEMPLATE_ALPHA], opacity);
				}

				Q_UINT8 srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					Q_UINT8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_TEMPLATE_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				for (int channel = 0; channel < MAX_CHANNEL_TEMPLATE; channel++) {

					Q_UINT8 srcColor = src[channel];
					Q_UINT8 dstColor = dst[channel];

					srcColor = QMIN(srcColor, dstColor);

					Q_UINT8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

					dst[channel] = newColor;
				}
			}

			columns--;
			src += MAX_CHANNEL_TEMPLATEA;
			dst += MAX_CHANNEL_TEMPLATEA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

void KisColorSpaceTemplate::compositeLighten(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const Q_UINT8 *src = srcRowStart;
		Q_UINT8 *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			Q_UINT8 srcAlpha = src[PIXEL_TEMPLATE_ALPHA];
			Q_UINT8 dstAlpha = dst[PIXEL_TEMPLATE_ALPHA];

			srcAlpha = QMIN(srcAlpha, dstAlpha);

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = UINT8_MULT(src[PIXEL_TEMPLATE_ALPHA], opacity);
				}

				Q_UINT8 srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					Q_UINT8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_TEMPLATE_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				for (int channel = 0; channel < MAX_CHANNEL_TEMPLATE; channel++) {

					Q_UINT8 srcColor = src[channel];
					Q_UINT8 dstColor = dst[channel];

					srcColor = MAX(srcColor, dstColor);

					Q_UINT8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

					dst[channel] = newColor;
				}
			}

			columns--;
			src += MAX_CHANNEL_TEMPLATEA;
			dst += MAX_CHANNEL_TEMPLATEA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

QString KisStrategyColorSpaceTemplate::channelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const
{
	Q_ASSERT(channelIndex < nChannels());
	Q_UINT32 channelPosition = m_channels[channelIndex] -> pos() / sizeof(Q_UINT8);

	return QString().setNum(pixel[channelPosition]);
}

QString KisStrategyColorSpaceTemplate::normalisedChannelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const
{
	Q_ASSERT(channelIndex < nChannels());
	Q_UINT32 channelPosition = m_channels[channelIndex] -> pos() / sizeof(Q_UINT8);

	return QString().setNum(static_cast<float>(pixel[channelPosition]) / UINT8_MAX);
}

