/*
 *  Copyright (c) 2003 Boudewijn Rempt (boud@valdyas.org)
 *
 *  This program is free software; you can CYANistribute it and/or modify
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

#include <kdebug.h>
#include <klocale.h>

#include "kis_config.h"
#include "kis_image.h"
#include "kis_colorspace_registry.h"
#include "kis_strategy_colorspace_cmyka.h"
#include "kis_iterators_pixel.h"

#include "kis_resource.h"
#include "kis_resourceserver.h"
#include "kis_resource_mediator.h"
#include "kis_factory.h"
#include "kis_profile.h"

namespace cmyka {
	const Q_INT32 MAX_CHANNEL_CMYK = 4;
	const Q_INT32 MAX_CHANNEL_CMYKA = 5;
}

KisStrategyColorSpaceCMYKA::KisStrategyColorSpaceCMYKA() :
	KisStrategyColorSpace(KisID("CMYKA", i18n("CMYK/Alpha")),
			      (COLORSPACE_SH(PT_CMYK) | CHANNELS_SH(4) | BYTES_SH(1) | EXTRA_SH(1)),
			      icSigCmykData)
{
	m_channels.push_back(new KisChannelInfo(i18n("cyan"), 0, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("magenta"), 1, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("yellow"), 2, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("black"), 3, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("alpha"), 4, ALPHA));
}

KisStrategyColorSpaceCMYKA::~KisStrategyColorSpaceCMYKA()
{
}

void KisStrategyColorSpaceCMYKA::nativeColor(const QColor& color, QUANTUM *dst, KisProfileSP profile)
{
	QUANTUM c = 255 - color.red();
	QUANTUM m = 255 - color.green();
	QUANTUM y = 255 - color.blue();

	QUANTUM k = 255;

	if (c < k) k = c;
	if (m < k) k = m;
	if (y < k) k = y;

	dst[PIXEL_CYAN] = (c - k) / ( 255 - k);
	dst[PIXEL_MAGENTA] = (m  - k) / ( 255 - k);
	dst[PIXEL_YELLOW] = (y - k) / ( 255 - k);
	dst[PIXEL_BLACK] = k;
}

void KisStrategyColorSpaceCMYKA::nativeColor(const QColor& c, QUANTUM opacity, QUANTUM *dst, KisProfileSP profile)
{
	nativeColor(c, dst);
	dst[PIXEL_CMYK_ALPHA] = opacity;
 }

void KisStrategyColorSpaceCMYKA::toQColor(const QUANTUM *src, QColor *c, KisProfileSP profile)
{
// 	c -> setCMYK(downscale(src[PIXEL_CYAN]), downscale(src[PIXEL_MAGENTA]), downscale(src[PIXEL_YELLOW]), downscale(src[PIXEL_BLACK]));
}

void KisStrategyColorSpaceCMYKA::toQColor(const QUANTUM *src, QColor *c, QUANTUM *opacity, KisProfileSP profile)
{
// 	c -> setCMYK(downscale(src[PIXEL_CYAN]), downscale(src[PIXEL_MAGENTA]), downscale(src[PIXEL_YELLOW]), downscale(src[PIXEL_BLACK]));
 	*opacity = src[PIXEL_CMYK_ALPHA];
}

vKisChannelInfoSP KisStrategyColorSpaceCMYKA::channels() const
{
	return m_channels;
}

bool KisStrategyColorSpaceCMYKA::alpha() const
{
	return true;
}

Q_INT32 KisStrategyColorSpaceCMYKA::nChannels() const
{
	return cmyka::MAX_CHANNEL_CMYKA;
}

Q_INT32 KisStrategyColorSpaceCMYKA::nColorChannels() const
{
	return cmyka::MAX_CHANNEL_CMYK;
}

Q_INT32 KisStrategyColorSpaceCMYKA::pixelSize() const
{
	return cmyka::MAX_CHANNEL_CMYKA;
}


QImage KisStrategyColorSpaceCMYKA::convertToQImage(const QUANTUM *data, Q_INT32 width, Q_INT32 height,
						   KisProfileSP srcProfile, KisProfileSP dstProfile,
						   Q_INT32 renderingIntent)
{

	kdDebug() << "convertToQImage: (" << width << ", " << height << ")\n";

	QImage img(width, height, 32, 0, QImage::LittleEndian);

	if (srcProfile == 0 || dstProfile == 0) {

		Q_INT32 i = 0;
		uchar *j = img.bits();

		while ( i < width * height * cmyka::MAX_CHANNEL_CMYKA) {
			QUANTUM k = *( data + i + PIXEL_BLACK );
			QUANTUM c = *( data + i + PIXEL_CYAN );
			QUANTUM m = *( data + i + PIXEL_MAGENTA );
			QUANTUM y = *( data + i + PIXEL_YELLOW );

			c = c * ( QUANTUM_MAX - k) + k;
			m = m * ( QUANTUM_MAX - k) + k;
			y = y * ( QUANTUM_MAX - k) + k;

			// XXX: Temporary copy
			const PIXELTYPE PIXEL_BLUE = 0;
			const PIXELTYPE PIXEL_GREEN = 1;
			const PIXELTYPE PIXEL_RED = 2;
			const PIXELTYPE PIXEL_ALPHA = 3;

			*( j + PIXEL_ALPHA ) = *( data + i + PIXEL_CMYK_ALPHA ) ;
			*( j + PIXEL_RED )   = QUANTUM_MAX - c;
			*( j + PIXEL_GREEN ) = QUANTUM_MAX - m;
			*( j + PIXEL_BLUE )  = QUANTUM_MAX - y;


			i += cmyka::MAX_CHANNEL_CMYKA;
			j += 4; // Because we're hard-coded 32 bits deep, 4 bytes
		}

        }
	else {
		kdDebug() << "Going to transform with profiles\n";

		KisStrategyColorSpaceSP dstCS = KisColorSpaceRegistry::instance() -> get("RGBA");
		convertPixelsTo(const_cast<QUANTUM *>(data), srcProfile,
				img.bits(), dstCS, dstProfile,
				width * height, renderingIntent);

	}

        return img;

}


void KisStrategyColorSpaceCMYKA::bitBlt(Q_INT32 stride,
				       QUANTUM *dst,
				       Q_INT32 dststride,
				       QUANTUM *src,
				       Q_INT32 srcstride,
				       QUANTUM opacity,
				       Q_INT32 rows,
				       Q_INT32 cols,
				       CompositeOp op)
{
	Q_INT32 linesize = stride * sizeof(QUANTUM) * cols;
	QUANTUM *d;
	QUANTUM *s;
	Q_INT32 i;

	if (rows <= 0 || cols <= 0)
		return;
	switch (op) {
	case COMPOSITE_COPY:
		d = dst;
		s = src;

		while (rows-- > 0) {
			memcpy(d, s, linesize);
			d += dststride;
			s += srcstride;
		}
		break;
	case COMPOSITE_CLEAR:
		d = dst;
		s = src;
		while (rows-- > 0) {
			memset(d, 0, linesize);
			d += dststride;
		}
		break;
	case COMPOSITE_OVER:
	default:
		if (opacity == OPACITY_TRANSPARENT)
			return;
		if (opacity != OPACITY_OPAQUE) {
			while (rows -- > 0) {
				d = dst;
				s = src;
				for (i = cols; i > 0; i--, d += stride, s += stride) {
					if (s[PIXEL_CMYK_ALPHA] == OPACITY_TRANSPARENT)
						continue;

					int srcAlpha = (s[PIXEL_CMYK_ALPHA] * opacity + QUANTUM_MAX / 2) / QUANTUM_MAX;
					int dstAlpha = (d[PIXEL_CMYK_ALPHA] * (QUANTUM_MAX - srcAlpha) + QUANTUM_MAX / 2) / QUANTUM_MAX;

					d[PIXEL_CYAN] = (d[PIXEL_CYAN]   * dstAlpha + s[PIXEL_CYAN]   * srcAlpha + QUANTUM_MAX / 2) / QUANTUM_MAX;
					d[PIXEL_MAGENTA] = (d[PIXEL_MAGENTA]   * dstAlpha + s[PIXEL_MAGENTA]   * srcAlpha + QUANTUM_MAX / 2) / QUANTUM_MAX;
					d[PIXEL_YELLOW] = (d[PIXEL_YELLOW]   * dstAlpha + s[PIXEL_YELLOW]   * srcAlpha + QUANTUM_MAX / 2) / QUANTUM_MAX;
					d[PIXEL_BLACK] = (d[PIXEL_BLACK]   * dstAlpha + s[PIXEL_BLACK]   * srcAlpha + QUANTUM_MAX / 2) / QUANTUM_MAX;

					d[PIXEL_CMYK_ALPHA] = (d[PIXEL_CMYK_ALPHA] * (QUANTUM_MAX - srcAlpha) + srcAlpha * QUANTUM_MAX + QUANTUM_MAX / 2) / QUANTUM_MAX;
					if (d[PIXEL_CMYK_ALPHA] != 0) {
						d[PIXEL_CYAN] = (d[PIXEL_CYAN] * QUANTUM_MAX) / d[PIXEL_CMYK_ALPHA];
						d[PIXEL_MAGENTA] = (d[PIXEL_MAGENTA] * QUANTUM_MAX) / d[PIXEL_CMYK_ALPHA];
						d[PIXEL_YELLOW] = (d[PIXEL_YELLOW] * QUANTUM_MAX) / d[PIXEL_CMYK_ALPHA];
						d[PIXEL_BLACK] = (d[PIXEL_BLACK] * QUANTUM_MAX) / d[PIXEL_CMYK_ALPHA];
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
					if (s[PIXEL_CMYK_ALPHA] == OPACITY_TRANSPARENT)
						continue;

					if (d[PIXEL_CMYK_ALPHA] == OPACITY_TRANSPARENT || s[PIXEL_CMYK_ALPHA] == OPACITY_OPAQUE) {
						memcpy(d, s, stride * sizeof(QUANTUM));
						continue;
					}

					int srcAlpha = s[PIXEL_CMYK_ALPHA];
					int dstAlpha = (d[PIXEL_CMYK_ALPHA] * (QUANTUM_MAX - srcAlpha) + QUANTUM_MAX / 2) / QUANTUM_MAX;

					d[PIXEL_CYAN]   = (d[PIXEL_CYAN]   * dstAlpha + s[PIXEL_CYAN]   * srcAlpha + QUANTUM_MAX / 2) / QUANTUM_MAX;
					d[PIXEL_MAGENTA] = (d[PIXEL_MAGENTA] * dstAlpha + s[PIXEL_MAGENTA] * srcAlpha + QUANTUM_MAX / 2) / QUANTUM_MAX;
					d[PIXEL_YELLOW]  = (d[PIXEL_YELLOW]  * dstAlpha + s[PIXEL_YELLOW]  * srcAlpha + QUANTUM_MAX / 2) / QUANTUM_MAX;
					d[PIXEL_BLACK]  = (d[PIXEL_BLACK]  * dstAlpha + s[PIXEL_BLACK]  * srcAlpha + QUANTUM_MAX / 2) / QUANTUM_MAX;
					d[PIXEL_CMYK_ALPHA] = (d[PIXEL_CMYK_ALPHA] * (QUANTUM_MAX - srcAlpha) + srcAlpha * QUANTUM_MAX + QUANTUM_MAX / 2) / QUANTUM_MAX;

					if (d[PIXEL_CMYK_ALPHA] != 0) {
						d[PIXEL_CYAN] = (d[PIXEL_CYAN] * QUANTUM_MAX) / d[PIXEL_CMYK_ALPHA];
						d[PIXEL_MAGENTA] = (d[PIXEL_MAGENTA] * QUANTUM_MAX) / d[PIXEL_CMYK_ALPHA];
						d[PIXEL_YELLOW] = (d[PIXEL_YELLOW] * QUANTUM_MAX) / d[PIXEL_CMYK_ALPHA];
						d[PIXEL_BLACK] = (d[PIXEL_BLACK] * QUANTUM_MAX) / d[PIXEL_CMYK_ALPHA];
					}
				}

				dst += dststride;
				src += srcstride;

			}
		}
	}

}

