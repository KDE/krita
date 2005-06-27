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

void KisStrategyColorSpaceCMYKA::nativeColor(const QColor& color, Q_UINT8 *dst, KisProfileSP profile)
{
	Q_UINT8 c = 255 - color.red();
	Q_UINT8 m = 255 - color.green();
	Q_UINT8 y = 255 - color.blue();

	Q_UINT8 k = 255;

	if (c < k) k = c;
	if (m < k) k = m;
	if (y < k) k = y;

	dst[PIXEL_CYAN] = (c - k) / ( 255 - k);
	dst[PIXEL_MAGENTA] = (m  - k) / ( 255 - k);
	dst[PIXEL_YELLOW] = (y - k) / ( 255 - k);
	dst[PIXEL_BLACK] = k;
}

void KisStrategyColorSpaceCMYKA::nativeColor(const QColor& color, QUANTUM opacity, Q_UINT8 *dst, KisProfileSP profile)
{
	Q_UINT8 c = 255 - color.red();
	Q_UINT8 m = 255 - color.green();
	Q_UINT8 y = 255 - color.blue();

	Q_UINT8 k = 255;

	if (c < k) k = c;
	if (m < k) k = m;
	if (y < k) k = y;

	dst[PIXEL_CYAN] = (c - k) / ( 255 - k);
	dst[PIXEL_MAGENTA] = (m  - k) / ( 255 - k);
	dst[PIXEL_YELLOW] = (y - k) / ( 255 - k);
	dst[PIXEL_BLACK] = k;

	dst[PIXEL_CMYK_ALPHA] = opacity;
}

void KisStrategyColorSpaceCMYKA::toQColor(const Q_UINT8 *src, QColor *c, KisProfileSP profile)
{
// 	c -> setCMYK(downscale(src[PIXEL_CYAN]), downscale(src[PIXEL_MAGENTA]), downscale(src[PIXEL_YELLOW]), downscale(src[PIXEL_BLACK]));
}

void KisStrategyColorSpaceCMYKA::toQColor(const Q_UINT8 *src, QColor *c, QUANTUM *opacity, KisProfileSP profile)
{
// 	c -> setCMYK(downscale(src[PIXEL_CYAN]), downscale(src[PIXEL_MAGENTA]), downscale(src[PIXEL_YELLOW]), downscale(src[PIXEL_BLACK]));
 	*opacity = src[PIXEL_CMYK_ALPHA];
}

Q_INT8 KisStrategyColorSpaceCMYKA::difference(const Q_UINT8* src1, const Q_UINT8* src2)
{
	// XXX This is worng - doesn't take K into account
	//return KisStrategyColorSpace::difference(src1, src2);
	return QMAX(QABS(src2[PIXEL_CYAN] - src1[PIXEL_CYAN]),
				QMAX(QABS(src2[PIXEL_MAGENTA] - src1[PIXEL_MAGENTA]),
	QABS(src2[PIXEL_YELLOW] - src1[PIXEL_YELLOW])));
}

void KisStrategyColorSpaceCMYKA::mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const
{
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


QImage KisStrategyColorSpaceCMYKA::convertToQImage(const Q_UINT8 *data, Q_INT32 width, Q_INT32 height,
						   KisProfileSP srcProfile, KisProfileSP dstProfile,
						   Q_INT32 renderingIntent)
{

	kdDebug(DBG_AREA_CMS) << "convertToQImage: (" << width << ", " << height << ")\n";

	QImage img(width, height, 32, 0, QImage::LittleEndian);

	if (srcProfile == 0 || dstProfile == 0) {

		Q_INT32 i = 0;
		uchar *j = img.bits();

		while ( i < width * height * cmyka::MAX_CHANNEL_CMYKA) {
			Q_UINT8 k = *( data + i + PIXEL_BLACK );
			Q_UINT8 c = *( data + i + PIXEL_CYAN );
			Q_UINT8 m = *( data + i + PIXEL_MAGENTA );
			Q_UINT8 y = *( data + i + PIXEL_YELLOW );

			c = c * ( UINT8_MAX - k) + k;
			m = m * ( UINT8_MAX - k) + k;
			y = y * ( UINT8_MAX - k) + k;

			// XXX: Temporary copy
			const Q_UINT8 PIXEL_BLUE = 0;
			const Q_UINT8 PIXEL_GREEN = 1;
			const Q_UINT8 PIXEL_RED = 2;
			const Q_UINT8 PIXEL_ALPHA = 3;

			*( j + PIXEL_ALPHA ) = *( data + i + PIXEL_CMYK_ALPHA ) ;
			*( j + PIXEL_RED )   = UINT8_MAX - c;
			*( j + PIXEL_GREEN ) = UINT8_MAX - m;
			*( j + PIXEL_BLUE )  = UINT8_MAX - y;


			i += cmyka::MAX_CHANNEL_CMYKA;
			j += 4; // Because we're hard-coded 32 bits deep, 4 bytes
		}

        }
	else {
		kdDebug(DBG_AREA_CMS) << "Going to transform with profiles\n";

		KisStrategyColorSpaceSP dstCS = KisColorSpaceRegistry::instance() -> get("RGBA");
		convertPixelsTo(const_cast<Q_UINT8 *>(data), srcProfile,
				img.bits(), dstCS, dstProfile,
				width * height, renderingIntent);

	}

        return img;

}

void KisStrategyColorSpaceCMYKA::adjustBrightness(Q_UINT8 *src1, Q_INT8 adjust) const
{
	//XXX does nothing for now
}


void KisStrategyColorSpaceCMYKA::bitBlt(Q_INT32 stride,
				       Q_UINT8 *dst,
				       Q_INT32 dststride,
				       const Q_UINT8 *src,
				       Q_INT32 srcstride,
				       QUANTUM opacity,
				       Q_INT32 rows,
				       Q_INT32 cols,
				       const KisCompositeOp& op)
{
	Q_INT32 linesize = stride * sizeof(Q_UINT8) * cols;
	Q_UINT8 *d;
	const Q_UINT8 *s;
	Q_INT32 i;

	if (rows <= 0 || cols <= 0)
		return;
	switch (op.op()) {
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

					int srcAlpha = (s[PIXEL_CMYK_ALPHA] * opacity + UINT8_MAX / 2) / UINT8_MAX;
					int dstAlpha = (d[PIXEL_CMYK_ALPHA] * (UINT8_MAX - srcAlpha) + UINT8_MAX / 2) / UINT8_MAX;

					d[PIXEL_CYAN] = (d[PIXEL_CYAN]   * dstAlpha + s[PIXEL_CYAN]   * srcAlpha + UINT8_MAX / 2) / UINT8_MAX;
					d[PIXEL_MAGENTA] = (d[PIXEL_MAGENTA]   * dstAlpha + s[PIXEL_MAGENTA]   * srcAlpha + UINT8_MAX / 2) / UINT8_MAX;
					d[PIXEL_YELLOW] = (d[PIXEL_YELLOW]   * dstAlpha + s[PIXEL_YELLOW]   * srcAlpha + UINT8_MAX / 2) / UINT8_MAX;
					d[PIXEL_BLACK] = (d[PIXEL_BLACK]   * dstAlpha + s[PIXEL_BLACK]   * srcAlpha + UINT8_MAX / 2) / UINT8_MAX;

					d[PIXEL_CMYK_ALPHA] = (d[PIXEL_CMYK_ALPHA] * (UINT8_MAX - srcAlpha) + srcAlpha * UINT8_MAX + UINT8_MAX / 2) / UINT8_MAX;
					if (d[PIXEL_CMYK_ALPHA] != 0) {
						d[PIXEL_CYAN] = (d[PIXEL_CYAN] * UINT8_MAX) / d[PIXEL_CMYK_ALPHA];
						d[PIXEL_MAGENTA] = (d[PIXEL_MAGENTA] * UINT8_MAX) / d[PIXEL_CMYK_ALPHA];
						d[PIXEL_YELLOW] = (d[PIXEL_YELLOW] * UINT8_MAX) / d[PIXEL_CMYK_ALPHA];
						d[PIXEL_BLACK] = (d[PIXEL_BLACK] * UINT8_MAX) / d[PIXEL_CMYK_ALPHA];
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
						memcpy(d, s, stride * sizeof(Q_UINT8));
						continue;
					}

					int srcAlpha = s[PIXEL_CMYK_ALPHA];
					int dstAlpha = (d[PIXEL_CMYK_ALPHA] * (UINT8_MAX - srcAlpha) + UINT8_MAX / 2) / UINT8_MAX;

					d[PIXEL_CYAN]   = (d[PIXEL_CYAN]   * dstAlpha + s[PIXEL_CYAN]   * srcAlpha + UINT8_MAX / 2) / UINT8_MAX;
					d[PIXEL_MAGENTA] = (d[PIXEL_MAGENTA] * dstAlpha + s[PIXEL_MAGENTA] * srcAlpha + UINT8_MAX / 2) / UINT8_MAX;
					d[PIXEL_YELLOW]  = (d[PIXEL_YELLOW]  * dstAlpha + s[PIXEL_YELLOW]  * srcAlpha + UINT8_MAX / 2) / UINT8_MAX;
					d[PIXEL_BLACK]  = (d[PIXEL_BLACK]  * dstAlpha + s[PIXEL_BLACK]  * srcAlpha + UINT8_MAX / 2) / UINT8_MAX;
					d[PIXEL_CMYK_ALPHA] = (d[PIXEL_CMYK_ALPHA] * (UINT8_MAX - srcAlpha) + srcAlpha * UINT8_MAX + UINT8_MAX / 2) / UINT8_MAX;

					if (d[PIXEL_CMYK_ALPHA] != 0) {
						d[PIXEL_CYAN] = (d[PIXEL_CYAN] * UINT8_MAX) / d[PIXEL_CMYK_ALPHA];
						d[PIXEL_MAGENTA] = (d[PIXEL_MAGENTA] * UINT8_MAX) / d[PIXEL_CMYK_ALPHA];
						d[PIXEL_YELLOW] = (d[PIXEL_YELLOW] * UINT8_MAX) / d[PIXEL_CMYK_ALPHA];
						d[PIXEL_BLACK] = (d[PIXEL_BLACK] * UINT8_MAX) / d[PIXEL_CMYK_ALPHA];
					}
				}

				dst += dststride;
				src += srcstride;

			}
		}
	}

}

KisCompositeOpList KisStrategyColorSpaceCMYKA::userVisiblecompositeOps() const
{
	KisCompositeOpList list;

	list.append(KisCompositeOp(COMPOSITE_OVER));

	return list;
}

