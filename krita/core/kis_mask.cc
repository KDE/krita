/*
 *  kis_mask.cc - part of KImageShop
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
#include "kis_image_cmd.h"
#include "kis_mask.h"

KisMask::KisMask(KisMask::MaskType type, const QString& name, uint width, uint height) : super(name, width, height, 4, 0)
{
	m_type = type;
}

KisMask::~KisMask()
{
}

void KisMask::setPixel(uint x, uint y, const uchar *src, KisImageCmd *cmd)
{
	if (m_type != CUSTOMMASK)
		super::setPixel(x, y, src, cmd);
}

void KisMask::setPixel(uint x, uint y, const QRgb& rgb, KisImageCmd *cmd)
{
	if (m_type != CUSTOMMASK)
		super::setPixel(x, y, rgb, cmd);
}

bool KisMask::pixel(uint x, uint y, uchar **val)
{
	QRgb rgb;

	if (!pixel(x, y, &rgb))
		return false;

	KisColorUtils::rgb2bytes(rgb, *val, bpp());
	return true;
}

bool KisMask::pixel(uint x, uint y, QRgb *rgb)
{
	if (m_type == CUSTOMMASK)
		return super::pixel(x, y, rgb);

	switch (m_type) {
		case REDMASK:
			*rgb = qRgba(0, 255, 255, 255);
			break;
		case GREENMASK:
			*rgb = qRgba(255, 0, 255, 255);
			break;
		case BLUEMASK:
			*rgb = qRgba(255, 255, 0, 255);
			break;
		case GREYMASK:
			break;
	}

	return true;
}

bool KisMask::writeToStore(KoStore * /*store*/)
{
	return true;
}

bool KisMask::loadFromStore(KoStore * /*store*/)
{
	return true;
}

