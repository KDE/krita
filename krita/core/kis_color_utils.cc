/*
 *  kis_color_utils.cc - part of Krita aka Krayon aka KImageShop
 *
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <string.h>

#include <Magick++.h>

#include "kis_color_utils.h"
#include "kis_pixel_packet.h"
#include "kis_global.h"

namespace KisColorUtils {
	using namespace Magick;

	QRgb bytes2rgb(const uchar* bytes, int /*bpp*/)
	{
		// XXX
		return qRgba(bytes[PIXEL_RED], bytes[PIXEL_GREEN], bytes[PIXEL_BLUE], bytes[PIXEL_ALPHA]);
	}

	void rgb2bytes(const QRgb& rgb, uchar* bytes, int /*bpp*/)
	{
		// XXX
		bytes[PIXEL_RED] = qRed(rgb);
		bytes[PIXEL_GREEN] = qGreen(rgb);
		bytes[PIXEL_BLUE] = qBlue(rgb);
		bytes[PIXEL_ALPHA] = qAlpha(rgb);
	}

	void blendOver(KisPixelPacket *dst, const KisPixelPacket *src)
	{
		if (!dst || !src)
			return;

		if (src -> opacity != TransparentOpacity) {
			if (src -> opacity == OpaqueOpacity) {
				memcpy(dst, src, sizeof(KisPixelPacket));
			} else if (src -> opacity != TransparentOpacity) {
				Quantum opacity = (MaxRGB - src -> opacity) + src -> opacity * (MaxRGB - dst -> opacity) / MaxRGB;
				Quantum invSrcOpacity = MaxRGB - src -> opacity;
				Quantum invDstOpacity = MaxRGB - dst -> opacity;

				dst -> red = ((invSrcOpacity * src -> red + src -> opacity * invDstOpacity * dst -> red / MaxRGB) / opacity);
				dst -> green = ((invSrcOpacity * src -> green + src -> opacity * invDstOpacity * dst -> green / MaxRGB) / opacity);
				dst -> blue = ((invSrcOpacity * src -> blue + src -> opacity * invDstOpacity * dst -> blue / MaxRGB) / opacity);
				dst -> opacity = MaxRGB - opacity;
			}
		}
	}
}

