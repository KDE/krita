/*
 *  kis_pixel_packet.cc - part of KImageShop aka Krayon aka Krita
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
#include <qcolor.h>
#include <koColor.h>
#include "kis_pixel_packet.h"

using namespace Magick;

KisPixelPacket::KisPixelPacket()
{
	red = 0;
	green = 0;
	blue = 0;
	opacity = 0;
}

KisPixelPacket::KisPixelPacket(const QRgb& rgb)
{
	red = Upscale(qRed(rgb));
	green = Upscale(qGreen(rgb));
	blue = Upscale(qBlue(rgb)); 
	opacity = TransparentOpacity - Upscale(qAlpha(rgb));
}

KisPixelPacket::KisPixelPacket(const QColor& clr)
{
	QRgb rgb = clr.rgb();

	red = Upscale(qRed(rgb));
	green = Upscale(qGreen(rgb));
	blue = Upscale(qBlue(rgb)); 
	opacity = TransparentOpacity - Upscale(qAlpha(rgb));
}

KisPixelPacket::KisPixelPacket(const KoColor& clr)
{
	QColor qc = clr.color();
	QRgb rgb = qc.rgb();

	red = Upscale(qRed(rgb));
	green = Upscale(qGreen(rgb));
	blue = Upscale(qBlue(rgb)); 
	opacity = TransparentOpacity - Upscale(qAlpha(rgb));
}

KisPixelPacket::KisPixelPacket(const KisPixelPacket& rhs)
{
	if (&rhs != this)
		memcpy(this, &rhs, sizeof(KisPixelPacket));
}

KisPixelPacket::KisPixelPacket(const PixelPacket& rhs)
{
	if (&rhs != this)
		memcpy(this, &rhs, sizeof(KisPixelPacket));
}

KisPixelPacket::KisPixelPacket(int r, int g, int b, int a)
{
	red = static_cast<Quantum>(r);
	green = static_cast<Quantum>(g);
	blue = static_cast<Quantum>(b);
	opacity = static_cast<Quantum>(a);
}

KisPixelPacket& KisPixelPacket::operator=(const QRgb& rgb)
{
	red = Upscale(qRed(rgb));
	green = Upscale(qGreen(rgb));
	blue = Upscale(qBlue(rgb)); 
	opacity = TransparentOpacity - Upscale(qAlpha(rgb));
	return *this;
}

KisPixelPacket& KisPixelPacket::operator=(const QColor& clr)
{
	QRgb rgb = clr.rgb();

	red = Upscale(qRed(rgb));
	green = Upscale(qGreen(rgb));
	blue = Upscale(qBlue(rgb)); 
	opacity = TransparentOpacity - Upscale(qAlpha(rgb));
	return *this;
}

KisPixelPacket& KisPixelPacket::operator=(const KoColor& clr)
{
	QColor c = clr.color();
	QRgb rgb = c.rgb();

	red = Upscale(qRed(rgb));
	green = Upscale(qGreen(rgb));
	blue = Upscale(qBlue(rgb)); 
	opacity = TransparentOpacity - Upscale(qAlpha(rgb));
	return *this;
}

KisPixelPacket& KisPixelPacket::operator=(const KisPixelPacket& rhs)
{
	if (&rhs != this)
		memcpy(this, &rhs, sizeof(KisPixelPacket));

	return *this;
}

KisPixelPacket& KisPixelPacket::operator=(const PixelPacket& rhs)
{
	if (&rhs != this) {
		red = rhs.red;
		green = rhs.green;
		blue = rhs.blue;
		opacity = rhs.opacity;
	}

	return *this;
}

KisPixelPacket::operator QRgb() const
{
	return qRgba(Downscale(red), Downscale(green), Downscale(blue), Downscale(TransparentOpacity - opacity));
}

KisPixelPacket::operator QColor() const
{
	QRgb rgb = *this;

	return QColor(rgb);
}

KisPixelPacket::operator KoColor() const
{
	QColor c = *this;

	return KoColor(c);
}

