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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <limits.h>
#include <stdlib.h>

#include <config.h>
#include LCMS_HEADER

#include <qimage.h>

#include <klocale.h>
#include <kdebug.h>

#include "kis_color_conversions.h"
#include "kis_strategy_colorspace.h"
#include "kis_colorspace_registry.h"
#include "kis_image.h"
#include "kis_colorspace_wet_sticky.h"
#include "kis_iterators_pixel.h"
#include "kis_integer_maths.h"

using namespace WetAndSticky;

KisColorSpaceWetSticky::KisColorSpaceWetSticky() :
	KisStrategyColorSpace(KisID("W&S", i18n("Wet & Sticky")), 0, icMaxEnumData)
{

	// Basic representational definition
	m_channels.push_back(new KisChannelInfo(i18n("blue"), 0, COLOR, 1));
	m_channels.push_back(new KisChannelInfo(i18n("green"), 1, COLOR, 1));
	m_channels.push_back(new KisChannelInfo(i18n("red"), 2, COLOR, 1));
	m_channels.push_back(new KisChannelInfo(i18n("alpha"), 3, ALPHA, 1));


	// Paint definition
	m_channels.push_back(new KisChannelInfo(i18n("hue"), 4, COLOR, sizeof(float)));
	m_channels.push_back(new KisChannelInfo(i18n("saturation"), 5, COLOR, sizeof(float)));
	m_channels.push_back(new KisChannelInfo(i18n("lightness"), 6, COLOR, sizeof(float)));
	m_channels.push_back(new KisChannelInfo(i18n("liquid content"), 7, SUBSTANCE, 1));
	m_channels.push_back(new KisChannelInfo(i18n("drying rate"), 8, SUBSTANCE, 1));
	m_channels.push_back(new KisChannelInfo(i18n("miscibility"), 9, SUBSTANCE, 1));


	// Substrate definition
	m_channels.push_back(new KisChannelInfo(i18n("gravitational direction"), 11, SUBSTRATE, sizeof(enumDirection)));
	m_channels.push_back(new KisChannelInfo(i18n("gravitational strength"), 12, SUBSTRATE, 1));
	m_channels.push_back(new KisChannelInfo(i18n("absorbancy"), 13, SUBSTRATE, 1));
	m_channels.push_back(new KisChannelInfo(i18n("paint volume"), 14, SUBSTANCE));

	kdDebug() << "Size of cell: " << sizeof(CELL) << " size of paint: " << sizeof(PAINT) << " size of rgba: " << sizeof(RGBA) << "\n";
}


KisColorSpaceWetSticky::~KisColorSpaceWetSticky()
{
}

void KisColorSpaceWetSticky::nativeColor(const QColor& c, Q_UINT8 *dst, KisProfileSP profile)
{
	CELL_PTR p = (CELL_PTR) dst;
	Q_UINT8 r, g, b;

	r = c.red();
	g = c.green();
	b = c.blue();

	p -> representation.color.red = r;
	p -> representation.color.green = g;
	p -> representation.color.blue = b;
	p -> representation.alpha = OPACITY_OPAQUE;

	rgb_to_hls(r, g, b, &p -> contents.color.hue, &p -> contents.color.lightness, &p -> contents.color.saturation);

	p -> contents.liquid_content = 0;
	p -> contents.drying_rate = 0;
	p -> contents.miscibility = 0;

	p -> gravity.direction = SOUTH;
	p -> gravity.strength = 10;

	p -> absorbancy = 10;
	p -> volume = 0;


	kdDebug() << "qcolor: "
		<< " r: " << c.red() << " b: " << c.blue() << " g: " << c.red()
		<< " native color: (" << p->representation.color.red << ","
		                      << p->representation.color.green << ","
		                      << p->representation.color.blue << ","
		                      << p->representation.alpha << ") "
		<< ", hls: (" << p->contents.color.hue << ","
		              << p->contents.color.lightness << ","
		              << p->contents.color.saturation << ")\n";
}

void KisColorSpaceWetSticky::nativeColor(const QColor& c, QUANTUM opacity, Q_UINT8 *dst, KisProfileSP profile)
{
	CELL_PTR p = (CELL_PTR) dst;
	Q_UINT8 r, g, b;

	r = c.red();
	g = c.green();
	b = c.blue();

	p -> representation.color.red = r;
	p -> representation.color.green = g;
	p -> representation.color.blue = b;
	p -> representation.alpha = opacity;
	rgb_to_hls(r, g, b, &p -> contents.color.hue, &p -> contents.color.lightness, &p -> contents.color.saturation);

	p -> contents.liquid_content = 0;
	p -> contents.drying_rate = 0;
	p -> contents.miscibility = 0;

	p -> gravity.direction = SOUTH;
	p -> gravity.strength = 10;

	p -> absorbancy = 10;
	p -> volume = 0;


	kdDebug() << "qcolor: "
		<< " r: " << c.red() << " b: " << c.blue() << " g: " << c.red()
		<< " native color: (" << p->representation.color.red << ","
		                      << p->representation.color.green << ","
		                      << p->representation.color.blue << ","
		                      << p->representation.alpha << ") "
		<< ", hls: (" << p->contents.color.hue << ","
		              << p->contents.color.lightness << ","
		              << p->contents.color.saturation << ")\n";
	
}

void KisColorSpaceWetSticky::toQColor(const Q_UINT8 *src, QColor *c, KisProfileSP profile)
{
	CELL_PTR p = (CELL_PTR) src;

	c -> setRgb(p -> representation.color.red,
		    p -> representation.color.green,
		    p -> representation.color.blue);

	kdDebug() << "Created qcolor: " << " r: " << c->red() << " b: " << c->blue() << " g: " << c->red() << "\n";
}

void KisColorSpaceWetSticky::toQColor(const Q_UINT8 *src, QColor *c, QUANTUM *opacity, KisProfileSP profile)
{

	CELL_PTR p = (CELL_PTR) src;

	c -> setRgb(p -> representation.color.red,
		    p -> representation.color.green,
		    p -> representation.color.blue);

	*opacity = p -> representation.alpha;
	kdDebug() << "Created qcolor: " << " r: " << c->red() << " b: " << c->blue() << " g: " << c->red() << "\n";
}



KisPixelRO KisColorSpaceWetSticky::toKisPixelRO(const Q_UINT8 *src, KisProfileSP profile)
{
	return KisPixelRO (src, src, this, profile);
}

KisPixel KisColorSpaceWetSticky::toKisPixel(Q_UINT8 *src, KisProfileSP profile)
{
	return KisPixel (src, src, this, profile);
}

void KisColorSpaceWetSticky::mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const
{
}

vKisChannelInfoSP KisColorSpaceWetSticky::channels() const
{
	return m_channels;
}

bool KisColorSpaceWetSticky::alpha() const
{
	return true;
}

Q_INT32 KisColorSpaceWetSticky::nChannels() const
{
	return 14;
}

Q_INT32 KisColorSpaceWetSticky::nColorChannels() const
{
	return 3;
}

Q_INT32 KisColorSpaceWetSticky::nSubstanceChannels() const
{
	return 4;

}

Q_INT32 KisColorSpaceWetSticky::pixelSize() const
{
	return sizeof(CELL);
}


QImage KisColorSpaceWetSticky::convertToQImage(const Q_UINT8 *data, Q_INT32 width, Q_INT32 height,
					       KisProfileSP /*srcProfile*/, KisProfileSP /*dstProfile*/,
					       Q_INT32 /*renderingIntent*/)
{

	QImage img(width, height, 32, 0, QImage::LittleEndian);

	Q_INT32 i = 0;
	uchar *j = img.bits();

	CELL_PTR p = (CELL_PTR) data;

	while ( i < width * height) {

		const Q_UINT8 PIXEL_BLUE = 0;
		const Q_UINT8 PIXEL_GREEN = 1;
		const Q_UINT8 PIXEL_RED = 2;
		const Q_UINT8 PIXEL_ALPHA = 3;

		*( j + PIXEL_ALPHA ) = p -> representation.alpha;
		*( j + PIXEL_RED )   = p -> representation.color.red;
		*( j + PIXEL_GREEN ) = p -> representation.color.green;
		*( j + PIXEL_BLUE )  = p -> representation.color.blue;

		p++;
		i++;
		j += 4; // Because we're hard-coded 32 bits deep, 4 bytes
	}
	return img;
}

bool KisColorSpaceWetSticky::convertPixelsTo(const Q_UINT8 * src, KisProfileSP /*srcProfile*/,
					     Q_UINT8 * dst, KisStrategyColorSpaceSP dstColorStrategy, KisProfileSP dstProfile,
					     Q_UINT32 numPixels,
					     Q_INT32 /*renderingIntent*/)
{
	Q_INT32 dSize = dstColorStrategy -> pixelSize();
	Q_INT32 sSize = pixelSize();

	Q_UINT32 j = 0;
	Q_UINT32 i = 0;
	QColor c;
	CELL_PTR cp;
	while ( i < numPixels ) {
		cp = (CELL_PTR) (src + i);

		c.setRgb(cp -> representation.color.red,
			 cp -> representation.color.green,
			 cp -> representation.color.blue);

		dstColorStrategy -> nativeColor(c, cp -> representation.alpha, (dst + j), dstProfile);

		i += sSize;
		j += dSize;

	}
	return true;

}

void KisColorSpaceWetSticky::adjustBrightness(Q_UINT8 *src1, Q_INT8 adjust) const
{
	//XXX does nothing for now
}


void KisColorSpaceWetSticky::bitBlt(Q_UINT8 *dst,
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
	case COMPOSITE_COPY:
	default:
		compositeCopy(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
		break;
	}

}


void KisColorSpaceWetSticky::compositeOver(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, QUANTUM opacity)
{
	// XXX: This is basically the same as with rgb and used to composite layers for representation. Composition for
	//      painting works differently
	while (rows > 0) {

		const Q_UINT8 *src = srcRowStart;
		Q_UINT8 *dst = dstRowStart;
		const Q_UINT8 *mask = maskRowStart;
		
		CELL_PTR dstCell = (CELL_PTR) dst;
		CELL_PTR srcCell = (CELL_PTR) src;


		Q_INT32 columns = numColumns;

		while (columns > 0) {

			Q_UINT8 srcAlpha = srcCell->representation.alpha;

			// apply the alphamask
			if(mask != 0)
			{
				if(*mask != OPACITY_OPAQUE)
					srcAlpha = UINT8_MULT(srcAlpha, *mask);
				mask++;
			}
			
			if (srcAlpha != OPACITY_TRANSPARENT) {

				if (opacity != OPACITY_OPAQUE) {
					srcAlpha = UINT8_MULT(srcCell->representation.alpha, opacity);
				}

				if (srcAlpha == OPACITY_OPAQUE) {
					memcpy(dst, src, sizeof(CELL));
				} else {
					Q_UINT8 dstAlpha = dstCell->representation.alpha;

					Q_UINT8 srcBlend;

					if (dstAlpha == OPACITY_OPAQUE) {
						srcBlend = srcAlpha;
					} else {
						Q_UINT8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
						dstCell->representation.alpha = newAlpha;

						if (newAlpha != 0) {
							srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
						} else {
							srcBlend = srcAlpha;
						}
					}

					if (srcBlend == OPACITY_OPAQUE) {
						memcpy(dst, src, sizeof(CELL));
					} else {
						dstCell->representation.color.red = UINT8_BLEND(srcCell->representation.color.red, dstCell->representation.color.red, srcBlend);
						dstCell->representation.color.green = UINT8_BLEND(srcCell->representation.color.green, dstCell->representation.color.green, srcBlend);
						dstCell->representation.color.blue = UINT8_BLEND(srcCell->representation.color.blue, dstCell->representation.color.blue, srcBlend);
					}
				}
			}

			columns--;
			src += sizeof(CELL);
			dst += sizeof(CELL);
		}

		rows--;
		srcRowStart += srcRowStride;
		dstRowStart += dstRowStride;
		
		if(maskRowStart)
			maskRowStart += maskRowStride;
	}

}

void KisColorSpaceWetSticky::compositeCopy(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, QUANTUM opacity)
{
	Q_INT32 linesize = sizeof(CELL) * columns;
	Q_UINT8 *d;
	const Q_UINT8 *s;
	d = dst;
	s = src;
	
	while (rows-- > 0) {
		memcpy(d, s, linesize);
		d += dstRowStride;
		s += srcRowStride;
	}

}


KisCompositeOpList KisColorSpaceWetSticky::userVisiblecompositeOps() const
{
	KisCompositeOpList list;

	list.append(KisCompositeOp(COMPOSITE_OVER));

	return list;
}

