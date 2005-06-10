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
#include "kis_strategy_colorspace_test.h"
#include "composite.h"
#include "kis_iterators_pixel.h"
#include "kis_color_conversions.h"

struct testcspixel
{
	Q_UINT16 bmg;
	Q_UINT8 g;
	Q_UINT8 r;
	Q_UINT8 alpha;
	Q_UINT8 dummy;
};

KisStrategyColorSpaceTestCS::KisStrategyColorSpaceTestCS() :
	KisStrategyColorSpace(KisID("test", i18n("TestCS/Alpha")), 0, icMaxEnumData)
{
	m_channels.push_back(new KisChannelInfo(i18n("red"), 2, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("green"), 1, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("blue+green"), 0, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("alpha"), 3, ALPHA));
}

KisStrategyColorSpaceTestCS::~KisStrategyColorSpaceTestCS()
{
}

void KisStrategyColorSpaceTestCS::nativeColor(const QColor& c, QUANTUM *dst, KisProfileSP /*profile*/)
{
	testcspixel *pix = (testcspixel *)dst;
	
	pix->r = c.red();
	pix->g = c.green();
	pix->bmg = c.blue()*16 + c.green();
}

void KisStrategyColorSpaceTestCS::nativeColor(const QColor& c, QUANTUM opacity, QUANTUM *dst, KisProfileSP /*profile*/)
{
	testcspixel *pix = (testcspixel *)dst;
	
	pix->r = c.red();
	pix->g = c.green();
	pix->bmg = c.blue()*16 + c.green();
	pix->alpha = opacity;
}

void KisStrategyColorSpaceTestCS::toQColor(const QUANTUM *src, QColor *c, KisProfileSP /*profile*/)
{
	testcspixel *pix = (testcspixel *)src;
	c -> setRgb(pix->r, pix->g, (pix->bmg - pix->g)/16);
}

void KisStrategyColorSpaceTestCS::toQColor(const QUANTUM *src, QColor *c, QUANTUM *opacity, KisProfileSP /*profile*/)
{
	testcspixel *pix = (testcspixel *)src;
	c -> setRgb(pix->r, pix->g, (pix->bmg - pix->g)/16);
	*opacity = pix->alpha;
}

Q_INT8 KisStrategyColorSpaceTestCS::difference(const Q_UINT8 *src1, const Q_UINT8 *src2)
{
	testcspixel *pix1 = (testcspixel *)src1;
	testcspixel *pix2 = (testcspixel *)src2;
	return QMAX(QABS(pix2->r - pix1->r), QMAX(QABS(pix2->g - pix1->g),
	QABS((pix2->bmg - pix2->g - pix1->bmg + pix1->g)/16)));
}

void KisStrategyColorSpaceTestCS::mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const
{
	Q_UINT32 red=0, green=0, blue=0;
	
	while(nColors--)
	{
		testcspixel *pix = (testcspixel *)colors;
		red += pix->r * *weights;
		green += pix->g * *weights;
		blue += pix->bmg * *weights;
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

vKisChannelInfoSP KisStrategyColorSpaceTestCS::channels() const
{
	return m_channels;
}

bool KisStrategyColorSpaceTestCS::alpha() const
{
	return true;
}

Q_INT32 KisStrategyColorSpaceTestCS::nChannels() const
{
	return 4;
}

Q_INT32 KisStrategyColorSpaceTestCS::nColorChannels() const
{
	return 3;
}

Q_INT32 KisStrategyColorSpaceTestCS::pixelSize() const
{
	return sizeof(struct testcspixel);
}

QImage KisStrategyColorSpaceTestCS::convertToQImage(const QUANTUM *data, Q_INT32 width, Q_INT32 height,
						 KisProfileSP srcProfile, KisProfileSP dstProfile,
						 Q_INT32 renderingIntent)

{
	testcspixel *pix = (testcspixel *)data;

#ifdef __BIG_ENDIAN__
	QImage img = QImage(width, height, 32, 0, QImage::LittleEndian);
	img.setAlphaBuffer(true);
	// Find a way to use convertPixelsTo without needing to code a
	// complete agrb color strategy or something like that.

	Q_INT32 i = 0;
	uchar *j = img.bits();

	while ( i < width * height) {

		// Swap the bytes
		*( j + 0)  = pix->alpha/256;
		*( j + 1 ) = pix->r;
		*( j + 2 ) = pix->g;;
		*( j + 3 ) = (pix->bmg );//- pix->g)/16;

		pix++;
		i++;

		j += 4; // Because we're hard-coded 32 bits deep, 4 bytes
	}

#else
	QImage img = QImage(width, height, 32, 0, QImage::LittleEndian);
	img.setAlphaBuffer(true);

	Q_INT32 i = 0;
	uchar *j = img.bits();

	while ( i < width * height) {

		// Swap the bytes
		*( j + 3) = pix->alpha;
		*( j + 2) = pix->r;
		*( j + 1) = pix->g;
		*( j + 0) = (pix->bmg - pix->g)/16;

		pix++;
		i++;

		j += 4; // Because we're hard-coded 32 bits deep, 4 bytes
	}

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

void KisStrategyColorSpaceTestCS::adjustBrightness(const Q_UINT8 *src, Q_UINT8 *dst, Q_INT8 adjust) const
{
	testcspixel *spix = (testcspixel *)src;
	testcspixel *dpix = (testcspixel *)dst;
	
	Q_INT32 nd = spix->r + adjust;
	dpix->r = QMAX( 0, QMIN( QUANTUM_MAX, nd ) );
	
	nd = spix->g + adjust;
	dpix->g = QMAX( 0, QMIN( QUANTUM_MAX, nd ) );
	
	nd = (spix->bmg - spix->g)/16 + adjust;
	dpix->bmg = QMAX( 0, QMIN( QUANTUM_MAX, nd ) );

	dpix->bmg = dpix->bmg*16 + dpix->g;
}

void KisStrategyColorSpaceTestCS::adjustContrast(const Q_UINT8 *src, Q_UINT8 *dst, Q_INT8 adjust) const
{
	testcspixel *spix = (testcspixel *)src;
	testcspixel *dpix = (testcspixel *)dst;
/*	
	Q_INT32 nd = spix->r + adjust;
	dpix->r = QMAX( 0, QMIN( QUANTUM_MAX, nd ) );
	
	nd = spix->g + adjust;
	dpix->g = QMAX( 0, QMIN( QUANTUM_MAX, nd ) );
	
	nd = (spix->bmg - spix->g)/16 + adjust;
	dpix->bmg = QMAX( 0, QMIN( QUANTUM_MAX, nd ) );

	dpix->bmg = dpix->bmg*16 + dpix->g;
	*/
	//XXX Nothing done yet
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

void KisStrategyColorSpaceTestCS::compositeOver(QUANTUM *dstRowStart, Q_INT32 dstRowStride, const QUANTUM *srcRowStart, Q_INT32 srcRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	while (rows > 0) {

		const QUANTUM *src = srcRowStart;
		QUANTUM *dst = dstRowStart;
		Q_INT32 columns = numColumns;

		while (columns > 0) {
			testcspixel *pix = (testcspixel *)src;
			testcspixel *dstpix = (testcspixel *)dst;
			Q_UINT16 srcAlpha = pix->alpha;

			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = INT_MULT(srcAlpha, opacity);
				}

				if (srcAlpha == OPACITY_OPAQUE) {
					memcpy(dst, src, sizeof(struct testcspixel));
				} else {
					QUANTUM dstAlpha = dstpix->alpha;

					QUANTUM srcBlend;

					if (dstAlpha == OPACITY_OPAQUE) {
						srcBlend = srcAlpha;
					} else {
						QUANTUM newAlpha = dstAlpha + INT_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
						dstpix->alpha = newAlpha;

						if (newAlpha != 0) {
							srcBlend = INT_DIVIDE(srcAlpha, newAlpha);
						} else {
							srcBlend = srcAlpha;
						}
					}

					if (srcBlend == OPACITY_OPAQUE) {
						memcpy(dst, src, sizeof(struct testcspixel));
					} else {
						dstpix->r = INT_BLEND(pix->r, dstpix->r, srcBlend);
						dstpix->bmg = INT_BLEND((pix->bmg - pix->g)/16, (dstpix->bmg -dstpix->g)/16, srcBlend);
						dstpix->g = INT_BLEND(pix->g, dstpix->g, srcBlend);
						dstpix->bmg = dstpix->bmg*16 + dstpix->g;
					}
				}
			}
			
			columns--;
			src += sizeof(struct testcspixel);
			dst += sizeof(struct testcspixel);
			
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
	}
}

void KisStrategyColorSpaceTestCS::compositeErase(QUANTUM *dst, 
		    Q_INT32 dstRowSize,
		    const QUANTUM *src, 
		    Q_INT32 srcRowSize,
		    Q_INT32 rows, 
		    Q_INT32 cols, 
		    QUANTUM /*opacity*/ = OPACITY_OPAQUE)
{
	Q_INT32 i;

	while (rows-- > 0) {
		testcspixel *pix = (testcspixel *)src;
		testcspixel *dstpix = (testcspixel *)dst;

		for (i = cols; i > 0; i--, pix++, dstpix++) {
			if (dstpix->alpha < pix->alpha) {
				continue;
			}
			else {
				dstpix->alpha = pix->alpha;
			}
		}

		dst += dstRowSize;
		src += srcRowSize;
	}
}

void KisStrategyColorSpaceTestCS::bitBlt(Q_INT32 pixelSize,
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
	case COMPOSITE_COPY:
		compositeCopy(pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;
	case COMPOSITE_ERASE:
		compositeErase(dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
		break;

	case COMPOSITE_NO:
		// No composition.
		break;
	default:
		break;
	}
}

KisCompositeOpList KisStrategyColorSpaceTestCS::userVisiblecompositeOps() const
{
	KisCompositeOpList list;

	list.append(KisCompositeOp(COMPOSITE_OVER));

	return list;
}

