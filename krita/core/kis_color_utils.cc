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

#include "kis_color_utils.h"
#include "kis_global.h"

namespace KisColorUtils {
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
}

