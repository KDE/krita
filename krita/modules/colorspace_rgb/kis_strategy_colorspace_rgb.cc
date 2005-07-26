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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
#include "kis_iterators_pixel.h"
#include "kis_color_conversions.h"
#include "kis_integer_maths.h"

#include "composite.h"

#define downscale(quantum)  (quantum) //((unsigned char) ((quantum)/257UL))
#define upscale(value)  (value) // ((QUANTUM) (257UL*(value)))

namespace {
	const Q_INT32 MAX_CHANNEL_RGB = 3;
	const Q_INT32 MAX_CHANNEL_RGBA = 4;
}

KisStrategyColorSpaceRGB::KisStrategyColorSpaceRGB() :
	KisStrategyColorSpace(KisID("RGBA", i18n("RGB/Alpha (8 bits/channel)")), TYPE_BGRA_8, icSigRgbData)
{
	m_channels.push_back(new KisChannelInfo(i18n("Red"), 2, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("Green"), 1, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("Blue"), 0, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("Alpha"), 3, ALPHA));
}

KisStrategyColorSpaceRGB::~KisStrategyColorSpaceRGB()
{
}

void KisStrategyColorSpaceRGB::setPixel(Q_UINT8 *pixel, Q_UINT8 red, Q_UINT8 green, Q_UINT8 blue, Q_UINT8 alpha) const
{
	pixel[PIXEL_RED] = red;
	pixel[PIXEL_GREEN] = green;
	pixel[PIXEL_BLUE] = blue;
	pixel[PIXEL_ALPHA] = alpha;
}

void KisStrategyColorSpaceRGB::getPixel(const Q_UINT8 *pixel, Q_UINT8 *red, Q_UINT8 *green, Q_UINT8 *blue, Q_UINT8 *alpha) const
{
	*red = pixel[PIXEL_RED];
	*green = pixel[PIXEL_GREEN];
	*blue = pixel[PIXEL_BLUE];
	*alpha = pixel[PIXEL_ALPHA];
}

void KisStrategyColorSpaceRGB::nativeColor(const QColor& c, Q_UINT8 *dst, KisProfileSP /*profile*/)
{
 	//kdDebug(DBG_AREA_CMS) << "nativeColor called: " << c.red() << ", " << c.green() << ", " << c.blue() << "\n";
	dst[PIXEL_RED] = upscale(c.red());
	dst[PIXEL_GREEN] = upscale(c.green());
	dst[PIXEL_BLUE] = upscale(c.blue());
}

void KisStrategyColorSpaceRGB::nativeColor(const QColor& c, QUANTUM opacity, Q_UINT8 *dst, KisProfileSP /*profile*/)
{
 	//kdDebug(DBG_AREA_CMS) << "nativeColor called with opacity: " << c.red() << ", " << c.green() << ", " << c.blue() << "\n";
	dst[PIXEL_RED] = upscale(c.red());
	dst[PIXEL_GREEN] = upscale(c.green());
	dst[PIXEL_BLUE] = upscale(c.blue());
	dst[PIXEL_ALPHA] = opacity;
}

void KisStrategyColorSpaceRGB::getAlpha(const Q_UINT8 *pixel, Q_UINT8 *alpha)
{
	*alpha = pixel[PIXEL_ALPHA];
}

void KisStrategyColorSpaceRGB::setAlpha(Q_UINT8 *pixels, Q_UINT8 alpha, Q_INT32 nPixels)
{
	while (nPixels > 0) {
		pixels[PIXEL_ALPHA] = alpha;
		--nPixels;
		pixels += MAX_CHANNEL_RGBA;
	}
}

void KisStrategyColorSpaceRGB::toQColor(const Q_UINT8 *src, QColor *c, KisProfileSP /*profile*/)
{
	c -> setRgb(downscale(src[PIXEL_RED]), downscale(src[PIXEL_GREEN]), downscale(src[PIXEL_BLUE]));
 	//kdDebug(DBG_AREA_CMS) << "RGB::toQColor called: " << c->red() << ", " << c->green() << ", " << c->blue() << "\n";
}

void KisStrategyColorSpaceRGB::toQColor(const Q_UINT8 *src, QColor *c, QUANTUM *opacity, KisProfileSP /*profile*/)
{
 	//kdDebug(DBG_AREA_CMS) << "src: " << downscale(src[PIXEL_RED]) << ", " << downscale(src[PIXEL_GREEN]) << ", " << downscale(src[PIXEL_BLUE]) << "\n";
 	//kdDebug(DBG_AREA_CMS) << "src (not scaled down): " << src[PIXEL_RED] << ", " << src[PIXEL_GREEN] << ", " << src[PIXEL_BLUE] << "\n";
	c -> setRgb(downscale(src[PIXEL_RED]), downscale(src[PIXEL_GREEN]), downscale(src[PIXEL_BLUE]));
	//kdDebug(DBG_AREA_CMS) << "Opacity: " << src[PIXEL_ALPHA] << "\n";
	*opacity = src[PIXEL_ALPHA];
 	//kdDebug(DBG_AREA_CMS) << "RGB::toQColor called: " << c->red() << ", " << c->green() << ", " << c->blue() << ", "  << "\n";
}

Q_INT8 KisStrategyColorSpaceRGB::difference(const Q_UINT8 *src1, const Q_UINT8 *src2)
{
	//return KisStrategyColorSpace::difference(src1, src2);
	return QMAX(QABS(src2[PIXEL_RED] - src1[PIXEL_RED]),
				QMAX(QABS(src2[PIXEL_GREEN] - src1[PIXEL_GREEN]),
	QABS(src2[PIXEL_BLUE] - src1[PIXEL_BLUE])));
}

void KisStrategyColorSpaceRGB::mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const
{
	Q_UINT32 totalRed = 0, totalGreen = 0, totalBlue = 0, newAlpha = 0;
	
	while (nColors--)
	{
		Q_UINT32 alpha = (*colors)[PIXEL_ALPHA];
		Q_UINT32 alphaTimesWeight = UINT8_MULT(alpha, *weights);

		totalRed += (*colors)[PIXEL_RED] * alphaTimesWeight;
		totalGreen += (*colors)[PIXEL_GREEN] * alphaTimesWeight;
		totalBlue += (*colors)[PIXEL_BLUE] * alphaTimesWeight;
		newAlpha += alphaTimesWeight;

		weights++;
		colors++;
	}

	Q_ASSERT(newAlpha <= 255);

	dst[PIXEL_ALPHA] = newAlpha;

	if (newAlpha > 0) {
		totalRed = UINT8_DIVIDE(totalRed, newAlpha);
		totalGreen = UINT8_DIVIDE(totalGreen, newAlpha);
		totalBlue = UINT8_DIVIDE(totalBlue, newAlpha);
	}

	// Divide by 255.
	totalRed += 0x80;
	Q_UINT32 dstRed = ((totalRed >> 8) + totalRed) >> 8;
	Q_ASSERT(dstRed <= 255);
	dst[PIXEL_RED] = dstRed;

	totalGreen += 0x80;
	Q_UINT32 dstGreen = ((totalGreen >> 8) + totalGreen) >> 8;
	Q_ASSERT(dstGreen <= 255);
	dst[PIXEL_GREEN] = dstGreen;

	totalBlue += 0x80;
	Q_UINT32 dstBlue = ((totalBlue >> 8) + totalBlue) >> 8;
	Q_ASSERT(dstBlue <= 255);
	dst[PIXEL_BLUE] = dstBlue;
}

vKisChannelInfoSP KisStrategyColorSpaceRGB::channels() const
{
	return m_channels;
}

bool KisStrategyColorSpaceRGB::hasAlpha() const
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

QImage KisStrategyColorSpaceRGB::convertToQImage(const Q_UINT8 *data, Q_INT32 width, Q_INT32 height,
						 KisProfileSP srcProfile, KisProfileSP dstProfile,
						 Q_INT32 renderingIntent, float /*exposure*/)

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
	QImage img = QImage(const_cast<Q_UINT8 *>(data), width, height, 32, 0, 0, QImage::LittleEndian);
	img.setAlphaBuffer(true);
	// XXX: The previous version of this code used the quantum data directly
	// as an optimisation. We're introducing a copy overhead here which could
	// be factored out again if needed.
	img = img.copy();
#endif

   	kdDebug(DBG_AREA_CMS) << "convertToQImage: (" << width << ", " << height << ")"
   		  << " srcProfile: " << srcProfile << ", " << "dstProfile: " << dstProfile << "\n";


	if (srcProfile != 0 && dstProfile != 0) {
		convertPixelsTo(img.bits(), srcProfile,
				img.bits(), this, dstProfile,
				width * height, renderingIntent);
	}

	return img;
}

void KisStrategyColorSpaceRGB::adjustBrightnessContrast(const Q_UINT8 *src, Q_UINT8 *dst, Q_INT8 brightness, Q_INT8 contrast, Q_INT32 nPixels) const
{
	static cmsHPROFILE profiles[3];
	static cmsHTRANSFORM transform=0;
	static Q_INT8 oldb=0;
	static Q_INT8 oldc=0;
	
	if((oldb != brightness || oldc != contrast) && transform!=0)
	{
		cmsDeleteTransform(transform);
		cmsCloseProfile(profiles[0]);
		cmsCloseProfile(profiles[1]);
		cmsCloseProfile(profiles[2]);
		transform=0;
	}

	if(transform==0)
	{
		double a,b;
		a=contrast/100.0+1.0;
		a *= a;
		b= 50 -50*a + brightness;
		profiles[0] = cmsCreate_sRGBProfile();
		profiles[1] = cmsCreateBCHSWabstractProfile(30, b, a, 0, 0, 6504, 6504);
		profiles[2] = cmsCreate_sRGBProfile();
		transform  = cmsCreateMultiprofileTransform(profiles, 3, TYPE_BGRA_8, TYPE_BGRA_8, INTENT_PERCEPTUAL, 0);
		oldb=brightness;
		oldc=contrast;
	}
	cmsDoTransform(transform, const_cast<Q_UINT8 *>(src), dst, nPixels);
}

void KisStrategyColorSpaceRGB::darken(const Q_UINT8 * src, Q_UINT8 * dst, Q_INT32 shade, bool compensate, double compensation, Q_INT32 nPixels) const
{
	int i = 0;
	
	while (i < nPixels * MAX_CHANNEL_RGBA) {
		if (compensate) {
			dst[i]  = (Q_INT8) QMIN(255,((src[i] * shade) / (compensation * 255)));
			dst[++i]  = (Q_INT8) QMIN(255,((src[i] * shade) / (compensation * 255)));
			dst[++i]  = (Q_INT8) QMIN(255,((src[i] * shade) / (compensation * 255)));
		}
		else {
			dst[i]  = (Q_INT8) QMIN(255, (src[i] * shade / 255));
			dst[++i]  = (Q_INT8) QMIN(255, (src[i] * shade / 255));
			dst[++i]  = (Q_INT8) QMIN(255, (src[i] * shade / 255));
		}

		++i;
		++i;
	}
}

Q_UINT8 KisStrategyColorSpaceRGB::intensity8(const Q_UINT8 * src) const
{
        return (Q_UINT8)(src[PIXEL_RED] * 0.30 + src[PIXEL_GREEN] * 0.59 + src[PIXEL_BLUE] * 0.11) + 0.5;

}



void KisStrategyColorSpaceRGB::compositeOver(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const Q_UINT8 *src = srcRowStart;
		Q_UINT8 *dst = dstRowStart;
		const Q_UINT8 *mask = maskRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {

			Q_UINT8 srcAlpha = src[PIXEL_ALPHA];

			// apply the alphamask
			if(mask != 0)
			{
				if(*mask != OPACITY_OPAQUE)
					srcAlpha = UINT8_MULT(srcAlpha, *mask);
				mask++;
			}
			
			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
				}

				if (srcAlpha == OPACITY_OPAQUE) {
					memcpy(dst, src, MAX_CHANNEL_RGBA * sizeof(Q_UINT8));
				} else {
					Q_UINT8 dstAlpha = dst[PIXEL_ALPHA];

					Q_UINT8 srcBlend;

					if (dstAlpha == OPACITY_OPAQUE) {
						srcBlend = srcAlpha;
					} else {
						Q_UINT8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
						dst[PIXEL_ALPHA] = newAlpha;

						if (newAlpha != 0) {
							srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
						} else {
							srcBlend = srcAlpha;
						}
					}

					if (srcBlend == OPACITY_OPAQUE) {
						memcpy(dst, src, MAX_CHANNEL_RGB * sizeof(Q_UINT8));
					} else {
						dst[PIXEL_RED] = UINT8_BLEND(src[PIXEL_RED], dst[PIXEL_RED], srcBlend);
						dst[PIXEL_GREEN] = UINT8_BLEND(src[PIXEL_GREEN], dst[PIXEL_GREEN], srcBlend);
						dst[PIXEL_BLUE] = UINT8_BLEND(src[PIXEL_BLUE], dst[PIXEL_BLUE], srcBlend);
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
		if(maskRowStart)
			maskRowStart += maskRowStride;
	}
}

void KisStrategyColorSpaceRGB::compositeMultiply(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const Q_UINT8 *src = srcRowStart;
		Q_UINT8 *dst = dstRowStart;
		Q_INT32 columns = numColumns;
		const Q_UINT8 *mask = maskRowStart;

		while (columns > 0) {

			Q_UINT8 srcAlpha = src[PIXEL_ALPHA];
			Q_UINT8 dstAlpha = dst[PIXEL_ALPHA];

			srcAlpha = QMIN(srcAlpha, dstAlpha);
			
			// apply the alphamask
			if(mask != 0)
			{
				if(*mask != OPACITY_OPAQUE)
					srcAlpha = UINT8_MULT(srcAlpha, *mask);
				mask++;
			}
			

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
				}

				Q_UINT8 srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					Q_UINT8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				Q_UINT8 srcColor = src[PIXEL_RED];
				Q_UINT8 dstColor = dst[PIXEL_RED];

				srcColor = UINT8_MULT(srcColor, dstColor);

				dst[PIXEL_RED] = UINT8_BLEND(srcColor, dstColor, srcBlend);

				srcColor = src[PIXEL_GREEN];
				dstColor = dst[PIXEL_GREEN];

				srcColor = UINT8_MULT(srcColor, dstColor);

				dst[PIXEL_GREEN] = UINT8_BLEND(srcColor, dstColor, srcBlend);

				srcColor = src[PIXEL_BLUE];
				dstColor = dst[PIXEL_BLUE];

				srcColor = UINT8_MULT(srcColor, dstColor);

				dst[PIXEL_BLUE] = UINT8_BLEND(srcColor, dstColor, srcBlend);
			}

			columns--;
			src += MAX_CHANNEL_RGBA;
			dst += MAX_CHANNEL_RGBA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
		if(maskRowStart)
			maskRowStart += maskRowStride;
	}
}

void KisStrategyColorSpaceRGB::compositeDivide(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const Q_UINT8 *src = srcRowStart;
		Q_UINT8 *dst = dstRowStart;
		Q_INT32 columns = numColumns;
		const Q_UINT8 *mask = maskRowStart;

		while (columns > 0) {

			Q_UINT8 srcAlpha = src[PIXEL_ALPHA];
			Q_UINT8 dstAlpha = dst[PIXEL_ALPHA];

			srcAlpha = QMIN(srcAlpha, dstAlpha);

			// apply the alphamask
			if(mask != 0)
			{
				if(*mask != OPACITY_OPAQUE)
					srcAlpha = UINT8_MULT(srcAlpha, *mask);
				mask++;
			}
			
			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
				}

				Q_UINT8 srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					Q_UINT8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

					Q_UINT8 srcColor = src[channel];
					Q_UINT8 dstColor = dst[channel];

					srcColor = QMIN((dstColor * (UINT8_MAX + 1u) + (srcColor / 2u)) / (1u + srcColor), UINT8_MAX);

					Q_UINT8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

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
		if(maskRowStart)
			maskRowStart += maskRowStride;
	}
}

void KisStrategyColorSpaceRGB::compositeScreen(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const Q_UINT8 *src = srcRowStart;
		Q_UINT8 *dst = dstRowStart;
		Q_INT32 columns = numColumns;
		const Q_UINT8 *mask = maskRowStart;

		while (columns > 0) {

			Q_UINT8 srcAlpha = src[PIXEL_ALPHA];
			Q_UINT8 dstAlpha = dst[PIXEL_ALPHA];

			srcAlpha = QMIN(srcAlpha, dstAlpha);

			// apply the alphamask
			if(mask != 0)
			{
				if(*mask != OPACITY_OPAQUE)
					srcAlpha = UINT8_MULT(srcAlpha, *mask);
				mask++;
			}
			
			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
				}

				Q_UINT8 srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					Q_UINT8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

					Q_UINT8 srcColor = src[channel];
					Q_UINT8 dstColor = dst[channel];

					srcColor = UINT8_MAX - UINT8_MULT(UINT8_MAX - dstColor, UINT8_MAX - srcColor);

					Q_UINT8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

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
		if(maskRowStart)
			maskRowStart += maskRowStride;
	}
}

void KisStrategyColorSpaceRGB::compositeOverlay(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const Q_UINT8 *src = srcRowStart;
		Q_UINT8 *dst = dstRowStart;
		Q_INT32 columns = numColumns;
		const Q_UINT8 *mask = maskRowStart;

		while (columns > 0) {

			Q_UINT8 srcAlpha = src[PIXEL_ALPHA];
			Q_UINT8 dstAlpha = dst[PIXEL_ALPHA];

			srcAlpha = QMIN(srcAlpha, dstAlpha);
			
			// apply the alphamask
			if(mask != 0)
			{
				if(*mask != OPACITY_OPAQUE)
					srcAlpha = UINT8_MULT(srcAlpha, *mask);
				mask++;
			}
			

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
				}

				Q_UINT8 srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					Q_UINT8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

					Q_UINT8 srcColor = src[channel];
					Q_UINT8 dstColor = dst[channel];

					srcColor = UINT8_MULT(dstColor, dstColor + UINT8_MULT(2 * srcColor, UINT8_MAX - dstColor));

					Q_UINT8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

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
		if(maskRowStart)
			maskRowStart += maskRowStride;
	}
}

void KisStrategyColorSpaceRGB::compositeDodge(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const Q_UINT8 *src = srcRowStart;
		Q_UINT8 *dst = dstRowStart;
		Q_INT32 columns = numColumns;
		const Q_UINT8 *mask = maskRowStart;

		while (columns > 0) {

			Q_UINT8 srcAlpha = src[PIXEL_ALPHA];
			Q_UINT8 dstAlpha = dst[PIXEL_ALPHA];

			srcAlpha = QMIN(srcAlpha, dstAlpha);
			
			// apply the alphamask
			if(mask != 0)
			{
				if(*mask != OPACITY_OPAQUE)
					srcAlpha = UINT8_MULT(srcAlpha, *mask);
				mask++;
			}
			

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
				}

				Q_UINT8 srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					Q_UINT8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

					Q_UINT8 srcColor = src[channel];
					Q_UINT8 dstColor = dst[channel];

					srcColor = QMIN((dstColor * (UINT8_MAX + 1)) / (UINT8_MAX + 1 - srcColor), UINT8_MAX);

					Q_UINT8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

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
		if(maskRowStart)
			maskRowStart += maskRowStride;
	}
}

void KisStrategyColorSpaceRGB::compositeBurn(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const Q_UINT8 *src = srcRowStart;
		Q_UINT8 *dst = dstRowStart;
		Q_INT32 columns = numColumns;
		const Q_UINT8 *mask = maskRowStart;

		while (columns > 0) {

			Q_UINT8 srcAlpha = src[PIXEL_ALPHA];
			Q_UINT8 dstAlpha = dst[PIXEL_ALPHA];

			srcAlpha = QMIN(srcAlpha, dstAlpha);

			// apply the alphamask
			if(mask != 0)
			{
				if(*mask != OPACITY_OPAQUE)
					srcAlpha = UINT8_MULT(srcAlpha, *mask);
				mask++;
			}
			
			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
				}

				Q_UINT8 srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					Q_UINT8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

					Q_UINT8 srcColor = src[channel];
					Q_UINT8 dstColor = dst[channel];

					srcColor = QMIN(((UINT8_MAX - dstColor) * (UINT8_MAX + 1)) / (srcColor + 1), UINT8_MAX);
					srcColor = CLAMP(UINT8_MAX - srcColor, 0, UINT8_MAX);

					Q_UINT8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

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
		if(maskRowStart)
			maskRowStart += maskRowStride;
	}
}

void KisStrategyColorSpaceRGB::compositeDarken(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const Q_UINT8 *src = srcRowStart;
		Q_UINT8 *dst = dstRowStart;
		Q_INT32 columns = numColumns;
		const Q_UINT8 *mask = maskRowStart;

		while (columns > 0) {

			Q_UINT8 srcAlpha = src[PIXEL_ALPHA];
			Q_UINT8 dstAlpha = dst[PIXEL_ALPHA];

			srcAlpha = QMIN(srcAlpha, dstAlpha);

			// apply the alphamask
			if(mask != 0)
			{
				if(*mask != OPACITY_OPAQUE)
					srcAlpha = UINT8_MULT(srcAlpha, *mask);
				mask++;
			}
			
			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
				}

				Q_UINT8 srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					Q_UINT8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

					Q_UINT8 srcColor = src[channel];
					Q_UINT8 dstColor = dst[channel];

					srcColor = QMIN(srcColor, dstColor);

					Q_UINT8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

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
		if(maskRowStart)
			maskRowStart += maskRowStride;
	}
}

void KisStrategyColorSpaceRGB::compositeLighten(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const Q_UINT8 *src = srcRowStart;
		Q_UINT8 *dst = dstRowStart;
		Q_INT32 columns = numColumns;
		const Q_UINT8 *mask = maskRowStart;

		while (columns > 0) {

			Q_UINT8 srcAlpha = src[PIXEL_ALPHA];
			Q_UINT8 dstAlpha = dst[PIXEL_ALPHA];

			srcAlpha = QMIN(srcAlpha, dstAlpha);

			// apply the alphamask
			if(mask != 0)
			{
				if(*mask != OPACITY_OPAQUE)
					srcAlpha = UINT8_MULT(srcAlpha, *mask);
				mask++;
			}
			
			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
				}

				Q_UINT8 srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					Q_UINT8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
					} else {
						srcBlend = srcAlpha;
					}
				}

				for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

					Q_UINT8 srcColor = src[channel];
					Q_UINT8 dstColor = dst[channel];

					srcColor = QMAX(srcColor, dstColor);

					Q_UINT8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

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
		if(maskRowStart)
			maskRowStart += maskRowStride;
	}
}

void KisStrategyColorSpaceRGB::compositeHue(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const Q_UINT8 *src = srcRowStart;
		Q_UINT8 *dst = dstRowStart;
		Q_INT32 columns = numColumns;
		const Q_UINT8 *mask = maskRowStart;

		while (columns > 0) {

			Q_UINT8 srcAlpha = src[PIXEL_ALPHA];
			Q_UINT8 dstAlpha = dst[PIXEL_ALPHA];

			srcAlpha = QMIN(srcAlpha, dstAlpha);

			// apply the alphamask
			if(mask != 0)
			{
				if(*mask != OPACITY_OPAQUE)
					srcAlpha = UINT8_MULT(srcAlpha, *mask);
				mask++;
			}
			
			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
				}

				Q_UINT8 srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					Q_UINT8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
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

				dst[PIXEL_RED] = UINT8_BLEND(srcRed, dstRed, srcBlend);
				dst[PIXEL_GREEN] = UINT8_BLEND(srcGreen, dstGreen, srcBlend);
				dst[PIXEL_BLUE] = UINT8_BLEND(srcBlue, dstBlue, srcBlend);
			}

			columns--;
			src += MAX_CHANNEL_RGBA;
			dst += MAX_CHANNEL_RGBA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
		if(maskRowStart)
			maskRowStart += maskRowStride;
	}
}

void KisStrategyColorSpaceRGB::compositeSaturation(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const Q_UINT8 *src = srcRowStart;
		Q_UINT8 *dst = dstRowStart;
		Q_INT32 columns = numColumns;
		const Q_UINT8 *mask = maskRowStart;

		while (columns > 0) {

			Q_UINT8 srcAlpha = src[PIXEL_ALPHA];
			Q_UINT8 dstAlpha = dst[PIXEL_ALPHA];

			srcAlpha = QMIN(srcAlpha, dstAlpha);

			// apply the alphamask
			if(mask != 0)
			{
				if(*mask != OPACITY_OPAQUE)
					srcAlpha = UINT8_MULT(srcAlpha, *mask);
				mask++;
			}
			
			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
				}

				Q_UINT8 srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					Q_UINT8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
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

				dst[PIXEL_RED] = UINT8_BLEND(srcRed, dstRed, srcBlend);
				dst[PIXEL_GREEN] = UINT8_BLEND(srcGreen, dstGreen, srcBlend);
				dst[PIXEL_BLUE] = UINT8_BLEND(srcBlue, dstBlue, srcBlend);
			}

			columns--;
			src += MAX_CHANNEL_RGBA;
			dst += MAX_CHANNEL_RGBA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
		if(maskRowStart)
			maskRowStart += maskRowStride;
	}
}

void KisStrategyColorSpaceRGB::compositeValue(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const Q_UINT8 *src = srcRowStart;
		Q_UINT8 *dst = dstRowStart;
		Q_INT32 columns = numColumns;
		const Q_UINT8 *mask = maskRowStart;

		while (columns > 0) {

			Q_UINT8 srcAlpha = src[PIXEL_ALPHA];
			Q_UINT8 dstAlpha = dst[PIXEL_ALPHA];

			srcAlpha = QMIN(srcAlpha, dstAlpha);

			// apply the alphamask
			if(mask != 0)
			{
				if(*mask != OPACITY_OPAQUE)
					srcAlpha = UINT8_MULT(srcAlpha, *mask);
				mask++;
			}
			
			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
				}

				Q_UINT8 srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					Q_UINT8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
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

				dst[PIXEL_RED] = UINT8_BLEND(srcRed, dstRed, srcBlend);
				dst[PIXEL_GREEN] = UINT8_BLEND(srcGreen, dstGreen, srcBlend);
				dst[PIXEL_BLUE] = UINT8_BLEND(srcBlue, dstBlue, srcBlend);
			}

			columns--;
			src += MAX_CHANNEL_RGBA;
			dst += MAX_CHANNEL_RGBA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
		if(maskRowStart)
			maskRowStart += maskRowStride;
	}
}

void KisStrategyColorSpaceRGB::compositeColor(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const Q_UINT8 *src = srcRowStart;
		Q_UINT8 *dst = dstRowStart;
		Q_INT32 columns = numColumns;
		const Q_UINT8 *mask = maskRowStart;

		while (columns > 0) {

			Q_UINT8 srcAlpha = src[PIXEL_ALPHA];
			Q_UINT8 dstAlpha = dst[PIXEL_ALPHA];

			srcAlpha = QMIN(srcAlpha, dstAlpha);

			// apply the alphamask
			if(mask != 0)
			{
				if(*mask != OPACITY_OPAQUE)
					srcAlpha = UINT8_MULT(srcAlpha, *mask);
				mask++;
			}
			
			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
				}

				Q_UINT8 srcBlend;

				if (dstAlpha == OPACITY_OPAQUE) {
					srcBlend = srcAlpha;
				} else {
					Q_UINT8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
					dst[PIXEL_ALPHA] = newAlpha;

					if (newAlpha != 0) {
						srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
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

				dst[PIXEL_RED] = UINT8_BLEND(srcRed, dstRed, srcBlend);
				dst[PIXEL_GREEN] = UINT8_BLEND(srcGreen, dstGreen, srcBlend);
				dst[PIXEL_BLUE] = UINT8_BLEND(srcBlue, dstBlue, srcBlend);
			}

			columns--;
			src += MAX_CHANNEL_RGBA;
			dst += MAX_CHANNEL_RGBA;
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
		if(maskRowStart)
			maskRowStart += maskRowStride;
	}
}

void KisStrategyColorSpaceRGB::compositeErase(Q_UINT8 *dst, 
		    Q_INT32 dstRowSize,
		    const Q_UINT8 *src, 
		    Q_INT32 srcRowSize,
		    const Q_UINT8 *srcAlphaMask,
		    Q_INT32 maskRowStride,
		    Q_INT32 rows, 
		    Q_INT32 cols, 
		    QUANTUM /*opacity*/)
{
	Q_INT32 i;
	Q_UINT8 srcAlpha;
	
	while (rows-- > 0)
	{
		const Q_UINT8 *s = src;
		Q_UINT8 *d = dst;
		const Q_UINT8 *mask = srcAlphaMask;

		for (i = cols; i > 0; i--, s+=MAX_CHANNEL_RGBA, d+=MAX_CHANNEL_RGBA)
		{
			srcAlpha = s[PIXEL_ALPHA];
			// apply the alphamask
			if(mask != 0)
			{
				if(*mask != OPACITY_OPAQUE)
					srcAlpha = UINT8_BLEND(srcAlpha, OPACITY_OPAQUE, *mask);
				mask++;
			}
			d[PIXEL_ALPHA] = UINT8_MULT(srcAlpha, d[PIXEL_ALPHA]);
		}

		dst += dstRowSize;
		if(srcAlphaMask)
			srcAlphaMask += maskRowStride;
		src += srcRowSize;
	}
}


void KisStrategyColorSpaceRGB::bitBlt(Q_UINT8 *dst,
				      Q_INT32 dstRowStride,
				      const Q_UINT8 *src,
				      Q_INT32 srcRowStride,
				      const Q_UINT8 *mask,
				      Q_INT32 maskRowStride,
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
		compositeOver(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_IN:
		compositeIn(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
	case COMPOSITE_OUT:
		compositeOut(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_ATOP:
		compositeAtop(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_XOR:
		compositeXor(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_PLUS:
		compositePlus(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_MINUS:
		compositeMinus(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_ADD:
		compositeAdd(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_SUBTRACT:
		compositeSubtract(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_DIFF:
		compositeDiff(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_MULT:
		compositeMultiply(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_DIVIDE:
		compositeDivide(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_BUMPMAP:
		compositeBumpmap(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_COPY:
		compositeCopy(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_COPY_RED:
		compositeCopyRed(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_COPY_GREEN:
		compositeCopyGreen(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_COPY_BLUE:
		compositeCopyBlue(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_COPY_OPACITY:
		compositeCopyOpacity(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_CLEAR:
		compositeClear(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_DISSOLVE:
		compositeDissolve(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_DISPLACE:
		compositeDisplace(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
#if 0
	case COMPOSITE_MODULATE:
		compositeModulate(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_THRESHOLD:
		compositeThreshold(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
#endif
	case COMPOSITE_NO:
		// No composition.
		break;
	case COMPOSITE_DARKEN:
		compositeDarken(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_LIGHTEN:
		compositeLighten(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_HUE:
		compositeHue(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_SATURATION:
		compositeSaturation(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_VALUE:
		compositeValue(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_COLOR:
		compositeColor(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_COLORIZE:
		compositeColorize(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_LUMINIZE:
		compositeLuminize(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_SCREEN:
		compositeScreen(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_OVERLAY:
		compositeOverlay(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_ERASE:
		compositeErase(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_DODGE:
		compositeDodge(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_BURN:
		compositeBurn(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
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

QString KisStrategyColorSpaceRGB::channelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const
{
	Q_ASSERT(channelIndex < nChannels());
	Q_UINT32 channelPosition = m_channels[channelIndex] -> pos();

	return QString().setNum(pixel[channelPosition]);
}

QString KisStrategyColorSpaceRGB::normalisedChannelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const
{
	Q_ASSERT(channelIndex < nChannels());
	Q_UINT32 channelPosition = m_channels[channelIndex] -> pos();

	return QString().setNum(static_cast<float>(pixel[channelPosition]) / UINT8_MAX);
}

