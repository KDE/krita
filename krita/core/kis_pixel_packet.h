/*
 *  kis_pixel_packet.h - part of KImageShop aka Krayon aka Krita
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

#if !defined KIS_PIXEL_PACKET_H_
#define KIS_PIXEL_PACKET_H_

#include <Magick++.h>

#include <qcolor.h>

class KoColor;

class KisPixelPacket : public Magick::PixelPacket {
public:
	KisPixelPacket();
	KisPixelPacket(const QRgb& rgb);
	KisPixelPacket(const QColor& clr);
	KisPixelPacket(const KoColor& clr);
	KisPixelPacket(const KisPixelPacket& rhs);
	KisPixelPacket(const Magick::PixelPacket& rhs);
	KisPixelPacket(int r, int g, int b, int a);

	KisPixelPacket& operator=(const QRgb& rgb);
	KisPixelPacket& operator=(const QColor& clr);
	KisPixelPacket& operator=(const KoColor& clr);
	KisPixelPacket& operator=(const KisPixelPacket& rhs);
	KisPixelPacket& operator=(const Magick::PixelPacket& rhs);

	operator QRgb() const;
	operator QColor() const;
	operator KoColor() const;
};

#endif // KIS_PIXEL_PACKET_H_

