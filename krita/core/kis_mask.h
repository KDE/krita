/*
 *  kis_mask.h - part of KImageShop
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

#if !defined KIS_MASK_H_
#define KIS_MASK_H_

#include "kis_paint_device.h"

class KisMask : public KisPaintDevice {
	typedef KisPaintDevice super;

public:
	enum MaskType {CUSTOMMASK, REDMASK, GREENMASK, BLUEMASK, GREYMASK};

	KisMask(MaskType type, const QString& name, uint width, uint height);
	virtual ~KisMask();

	virtual void setPixel(uint x, uint y, const uchar *src, KisImageCmd *cmd);
	virtual void setPixel(uint x, uint y, const QRgb& rgb, KisImageCmd *cmd);
	virtual bool pixel(uint x, uint y, uchar **val);
	virtual bool pixel(uint x, uint y, QRgb *rgb);
	virtual bool writeToStore(KoStore *store);
	virtual bool loadFromStore(KoStore *store);

private:
	MaskType m_type;
};

#endif // KIS_MASK_H_

