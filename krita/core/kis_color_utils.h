/*
 *  kis_color_utils.h - part of Krita aka Krayon aka KImageShop
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

#if !defined KIS_COLOR_UTILS_H_
#define KIS_COLOR_UTILS_H_

#include <qcolor.h>

class KisPixelPacket;

namespace KisColorUtils {
	QRgb bytes2rgb(const uchar* bytes, int bpp);
	void rgb2bytes(const QRgb& rgb, uchar* bytes, int bpp);
	void blendOver(KisPixelPacket *dst, const KisPixelPacket *src);
}

#endif // KIS_COLOR_UTILS_H_

