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
#include <klocale.h>

#include <config.h>

#include LCMS_HEADER

#include "kis_colorspace_registry.h"
#include "kis_image.h"
#include "kis_colorspace_alpha.h"
#include "kis_channelinfo.h"
#include "kis_types.h"
#include "kis_id.h"

namespace {
	const PIXELTYPE PIXEL_MASK = 0;
}

KisColorSpaceAlpha::KisColorSpaceAlpha() :
	KisStrategyColorSpace(KisID("ALPHA", i18n("Alpha mask")),  TYPE_GRAY_8, icSigGrayData)
{
// 	kdDebug() << "Alpha mask created\n";
	m_maskColor = Qt::red;
	m_inverted = false;
	m_channels.push_back(new KisChannelInfo(i18n("alpha"), 0, ALPHA));
}

KisColorSpaceAlpha::~KisColorSpaceAlpha()
{
}

void KisColorSpaceAlpha::nativeColor(const QColor& /*c*/, QUANTUM *dst, KisProfileSP /*profile*/)
{
	dst[PIXEL_MASK] = OPACITY_OPAQUE;
}

void KisColorSpaceAlpha::nativeColor(const QColor& /*c*/, QUANTUM opacity, QUANTUM *dst, KisProfileSP /*profile*/)
{
	dst[PIXEL_MASK] = opacity;
}

void KisColorSpaceAlpha::toQColor(const QUANTUM */*src*/, QColor *c, KisProfileSP /*profile*/)
{
	c -> setRgb(m_maskColor.red(), m_maskColor.green(), m_maskColor.blue());
}

void KisColorSpaceAlpha::toQColor(const QUANTUM *src, QColor *c, QUANTUM *opacity, KisProfileSP /*profile*/)
{
	c -> setRgb(m_maskColor.red(), m_maskColor.green(), m_maskColor.blue());
	if (m_inverted) {
		*opacity = OPACITY_OPAQUE - src[PIXEL_MASK];
	}
	else {
		*opacity = src[PIXEL_MASK];
	}
}

Q_INT8 KisColorSpaceAlpha::difference(const QUANTUM* src1, const QUANTUM* src2)
{
	return QABS(src2[PIXEL_MASK] - src1[PIXEL_MASK]);
}

vKisChannelInfoSP KisColorSpaceAlpha::channels() const
{
	return m_channels;
}
bool KisColorSpaceAlpha::alpha() const
{
	return true; // Of course!
}

// XXX: We convert the alpha space to create a mask for display in selection previews
// etc. No need to actually use the profiles here to create a mask image -- they don't
// need to be true color.
QImage KisColorSpaceAlpha::convertToQImage(const QUANTUM *data, Q_INT32 width, Q_INT32 height,
					   KisProfileSP /*srcProfile*/, KisProfileSP /*dstProfile*/,
					   Q_INT32 /*renderingIntent*/)
{

	QImage img(width, height, 32, 0, QImage::LittleEndian);

	Q_INT32 i = 0;
	uchar *j = img.bits();

	while ( i < width * height * pixelSize()) {

		// Temporary copy until I figure out something better

		PIXELTYPE PIXEL_BLUE = 0;
		PIXELTYPE PIXEL_GREEN = 1;
		PIXELTYPE PIXEL_RED = 2;
		PIXELTYPE PIXEL_ALPHA = 3;

		// XXX: for previews of the mask, it is be handy to
		// make this always black.

		*( j + PIXEL_RED )   = *( data + i );
		*( j + PIXEL_GREEN ) = *( data + i );
		*( j + PIXEL_BLUE )  = *( data + i );
		*( j + PIXEL_ALPHA ) = *( data + i );

		i += 1;
		j += 4; // Because we're hard-coded 32 bits deep, 4 bytes

	}
	return img;
}

bool KisColorSpaceAlpha::convertPixelsTo(const QUANTUM * src, KisProfileSP /*srcProfile*/,
					 QUANTUM * dst, KisStrategyColorSpaceSP dstColorStrategy, KisProfileSP dstProfile,
					 Q_UINT32 numPixels,
					 Q_INT32 /*renderingIntent*/)
{
	// No lcms trickery here, we are a QColor + opacity channel
	Q_INT32 size = dstColorStrategy -> pixelSize();

	Q_UINT32 j = 0;
	Q_UINT32 i = 0;

	while ( i < numPixels ) {

		dstColorStrategy -> nativeColor(m_maskColor, OPACITY_OPAQUE - *(src + i), (dst + j), dstProfile);

		i += 1;
		j += size;

	}
	return true;

}


void KisColorSpaceAlpha::bitBlt(Q_INT32 stride,
				QUANTUM *dst,
				Q_INT32 dststride,
				const QUANTUM *src,
				Q_INT32 srcstride,
				QUANTUM opacity,
				Q_INT32 rows,
				Q_INT32 cols,
				const KisCompositeOp& op)
{
//  	kdDebug() << "KisColorSpaceAlpha::bitBlt. stride: " << stride
//  		  << ", dststride: " << dststride
//  		  << ", opacity: " << (Q_UINT8) opacity
//  		  << ", rows: " << rows
//  		  << ", cols: " << cols
//  		  << ", op: " << op << "\n";

	QUANTUM *d;
	const QUANTUM *s;
 	Q_INT32 i;
	Q_INT32 linesize;

	if (rows <= 0 || cols <= 0)
		return;
	switch (op.op()) {
	case COMPOSITE_COPY:
		linesize = stride * sizeof(QUANTUM) * cols;
		d = dst;
		s = src;
		while (rows-- > 0) {
			memcpy(d, s, linesize);
			d += dststride;
			s += srcstride;
		}
		return;
	case COMPOSITE_CLEAR:
		linesize = stride * sizeof(QUANTUM) * cols;
		d = dst;
		while (rows-- > 0) {
			memset(d, OPACITY_TRANSPARENT, linesize);
			d += dststride;
		}
		return;
	case COMPOSITE_ERASE:
		while (rows-- > 0) {
			d = dst;
			s = src;

			for (i = cols; i > 0; i--, d += stride, s += stride) {
				if (d[PIXEL_MASK] < s[PIXEL_MASK]) {
					continue;
				}
				else {
					d[PIXEL_MASK] = s[PIXEL_MASK];
				}

			}

			dst += dststride;
			src += srcstride;
		}
		return;
	case COMPOSITE_OVER:
	default:
		if (opacity == OPACITY_TRANSPARENT)
			return;
		if (opacity != OPACITY_OPAQUE) {
			while (rows-- > 0) {
				d = dst;
				s = src;
				for (i = cols; i > 0; i--, d += stride, s += stride) {
					if (s[PIXEL_MASK] == OPACITY_TRANSPARENT)
						continue;
					int srcAlpha = (s[PIXEL_MASK] * opacity + QUANTUM_MAX / 2) / QUANTUM_MAX;
					d[PIXEL_MASK] = (d[PIXEL_MASK] * (QUANTUM_MAX - srcAlpha) + srcAlpha * QUANTUM_MAX + QUANTUM_MAX / 2) / QUANTUM_MAX;
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
					if (s[PIXEL_MASK] == OPACITY_TRANSPARENT)
						continue;
					if (d[PIXEL_MASK] == OPACITY_TRANSPARENT || s[PIXEL_MASK] == OPACITY_OPAQUE) {
						memcpy(d, s, stride * sizeof(QUANTUM));
						continue;
					}
					int srcAlpha = s[PIXEL_MASK];
					d[PIXEL_MASK] = (d[PIXEL_MASK] * (QUANTUM_MAX - srcAlpha) + srcAlpha * QUANTUM_MAX + QUANTUM_MAX / 2) / QUANTUM_MAX;
				}
				dst += dststride;
				src += srcstride;
			}
		}

	}
}

KisCompositeOpList KisColorSpaceAlpha::userVisiblecompositeOps() const
{
	KisCompositeOpList list;

	list.append(KisCompositeOp(COMPOSITE_OVER));

	return list;
}

