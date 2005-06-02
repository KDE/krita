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

using namespace WetAndSticky;


KisColorSpaceWetSticky::KisColorSpaceWetSticky() :
	KisStrategyColorSpace(KisID("W&S", i18n("Wet & Sticky")), 0, icMaxEnumData)
{

	// Basic representational definition
	m_channels.push_back(new KisChannelInfo(i18n("red"), 0, COLOR, 1));
	m_channels.push_back(new KisChannelInfo(i18n("green"), 1, COLOR, 1));
	m_channels.push_back(new KisChannelInfo(i18n("blue"), 2, COLOR, 1));
	m_channels.push_back(new KisChannelInfo(i18n("alpha"), 3, COLOR, 1));


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

}


KisColorSpaceWetSticky::~KisColorSpaceWetSticky()
{
}

void KisColorSpaceWetSticky::nativeColor(const QColor& c, QUANTUM *dst, KisProfileSP profile)
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

}

void KisColorSpaceWetSticky::nativeColor(const QColor& c, QUANTUM opacity, QUANTUM *dst, KisProfileSP profile)
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

}

void KisColorSpaceWetSticky::toQColor(const QUANTUM *src, QColor *c, KisProfileSP profile)
{
	CELL_PTR p = (CELL_PTR) src;

	c -> setRgb(p -> representation.color.red,
		    p -> representation.color.green,
		    p -> representation.color.blue);

}

void KisColorSpaceWetSticky::toQColor(const QUANTUM *src, QColor *c, QUANTUM *opacity, KisProfileSP profile)
{

	CELL_PTR p = (CELL_PTR) src;

	c -> setRgb(p -> representation.color.red,
		    p -> representation.color.green,
		    p -> representation.color.blue);

	*opacity = p -> representation.alpha;
}



KisPixelRO KisColorSpaceWetSticky::toKisPixelRO(const QUANTUM *src, KisProfileSP profile)
{
	return KisPixelRO (src, src, this, profile);
}

KisPixel KisColorSpaceWetSticky::toKisPixel(QUANTUM *src, KisProfileSP profile)
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


QImage KisColorSpaceWetSticky::convertToQImage(const QUANTUM *data, Q_INT32 width, Q_INT32 height,
					       KisProfileSP /*srcProfile*/, KisProfileSP /*dstProfile*/,
					       Q_INT32 /*renderingIntent*/)
{

	QImage img(width, height, 32, 0, QImage::LittleEndian);

	Q_INT32 i = 0;
	uchar *j = img.bits();

	CELL_PTR p = (CELL_PTR) data;

	while ( i < width * height) {

		const PIXELTYPE PIXEL_BLUE = 0;
		const PIXELTYPE PIXEL_GREEN = 1;
		const PIXELTYPE PIXEL_RED = 2;
		const PIXELTYPE PIXEL_ALPHA = 3;

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

bool KisColorSpaceWetSticky::convertPixelsTo(const QUANTUM * src, KisProfileSP /*srcProfile*/,
					     QUANTUM * dst, KisStrategyColorSpaceSP dstColorStrategy, KisProfileSP dstProfile,
					     Q_UINT32 numPixels,
					     Q_INT32 /*renderingIntent*/)
{
	kdDebug() << "KisColorSpaceWetSticky::convertPixelsTo\n";
	// No lcms trickery here. we're representationally basically an 8-bit rgba image.

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


void KisColorSpaceWetSticky::bitBlt(Q_INT32 stride,
				    QUANTUM *dst,
				    Q_INT32 dststride,
				    const QUANTUM *src,
				    Q_INT32 srcstride,
				    QUANTUM opacity,
				    Q_INT32 rows,
				    Q_INT32 cols,
				    const KisCompositeOp& op)
{
	Q_INT32 i;
	Q_INT32 linesize;

	QUANTUM *dq;
	const QUANTUM *sq;

	if (rows <= 0 || cols <= 0)
		return;
	switch (op.op()) {
	case COMPOSITE_COPY:

		linesize = stride * sizeof(QUANTUM) * cols;
		dq = dst;
		sq = src;
		while (rows-- > 0) {
			memcpy(dq, sq, linesize);
			dq += dststride;
			sq += srcstride;
		}
		return;
	case COMPOSITE_CLEAR:

		linesize = stride * sizeof(QUANTUM) * cols;
		dq = dst;
		while (rows-- > 0) {
			memset(dq, 0, linesize);
			dq += dststride;
		}
		return;
	case COMPOSITE_OVER:
	default:
		if (opacity == OPACITY_TRANSPARENT)
			return;

		if (opacity != OPACITY_OPAQUE) {
			while (rows-- > 0) {
				CELL_PTR d = (CELL_PTR) dst;
				CELL_PTR s = (CELL_PTR) src;

				for (i = cols; i > 0; i--, d += stride, s += stride) {

					if (s -> representation.alpha == OPACITY_TRANSPARENT)
						continue;

					int srcAlpha = (s -> representation.alpha * opacity + QUANTUM_MAX / 2) / QUANTUM_MAX;
					int dstAlpha = (s -> representation.alpha * (QUANTUM_MAX - srcAlpha) + QUANTUM_MAX / 2) / QUANTUM_MAX;

					d -> representation.color.red   = (d -> representation.color.red * dstAlpha
									   + s -> representation.color.red * srcAlpha + QUANTUM_MAX / 2) / QUANTUM_MAX;

					d -> representation.color.green = (d -> representation.color.green * dstAlpha +
									   s -> representation.color.green * srcAlpha + QUANTUM_MAX / 2) / QUANTUM_MAX;
					d -> representation.color.blue  = (d -> representation.color.blue  * dstAlpha +
									   s -> representation.color.blue  * srcAlpha + QUANTUM_MAX / 2) / QUANTUM_MAX;

					d -> representation.alpha = (d -> representation.alpha * (QUANTUM_MAX - srcAlpha) +
								      srcAlpha * QUANTUM_MAX + QUANTUM_MAX / 2) / QUANTUM_MAX;

					if (d -> representation.alpha != 0) {
						d -> representation.color.red = (d -> representation.color.red * QUANTUM_MAX) / d -> representation.alpha;
						d -> representation.color.green = (d -> representation.color.green * QUANTUM_MAX) / d -> representation.alpha;
						d -> representation.color.blue = (d -> representation.color.blue * QUANTUM_MAX) / d -> representation.alpha;
					}
				}

				dst += dststride;
				src += srcstride;
			}
		}
		else {
			while (rows-- > 0) {
				CELL_PTR d = (CELL_PTR) dst;
				CELL_PTR s = (CELL_PTR) src;

				for (i = cols; i > 0; i--, d += stride, s += stride) {
					if (s -> representation.alpha == OPACITY_TRANSPARENT)
continue;

					if (d -> representation.alpha == OPACITY_TRANSPARENT || s -> representation.alpha == OPACITY_OPAQUE) {
						memcpy(d, s, stride * sizeof(QUANTUM));
						continue;
					}

					int srcAlpha = s -> representation.alpha;
					int dstAlpha = (d -> representation.alpha * (QUANTUM_MAX - srcAlpha) + QUANTUM_MAX / 2) / QUANTUM_MAX;

					d -> representation.color.red   = (d -> representation.color.red   * dstAlpha +
									   s -> representation.color.red   * srcAlpha + QUANTUM_MAX / 2) / QUANTUM_MAX;
					d -> representation.color.green = (d -> representation.color.green * dstAlpha +
									   s -> representation.color.green * srcAlpha + QUANTUM_MAX / 2) / QUANTUM_MAX;
					d -> representation.color.blue  = (d -> representation.color.blue  * dstAlpha +
									   s -> representation.color.blue  * srcAlpha + QUANTUM_MAX / 2) / QUANTUM_MAX;

					d -> representation.alpha = (d -> representation.alpha * (QUANTUM_MAX - srcAlpha) +
								      srcAlpha * QUANTUM_MAX + QUANTUM_MAX / 2) / QUANTUM_MAX;

					if (d -> representation.alpha != 0) {
						d -> representation.color.red = (d -> representation.color.red * QUANTUM_MAX) / d -> representation.alpha;
						d -> representation.color.green = (d -> representation.color.green * QUANTUM_MAX) / d -> representation.alpha;
						d -> representation.color.blue = (d -> representation.color.blue * QUANTUM_MAX) / d -> representation.alpha;
					}
				}

				dst += dststride;
				src += srcstride;
			}
		}

	}
}

KisCompositeOpList KisColorSpaceWetSticky::userVisiblecompositeOps() const
{
	KisCompositeOpList list;

	list.append(KisCompositeOp(COMPOSITE_OVER));

	return list;
}

