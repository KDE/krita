/*
 *  kis_channel.h - part of KImageShop
 *
 *  Copyright (c) 1999 Andrew Richards <A.Richards@phys.canterbury.ac.nz>
 *                1999-2000 Matthias ELter  <elter@kde.org>
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

#ifndef __kis_channel_h__
#define __kis_channel_h__

#include <qrect.h>
#include <qpoint.h>
#include <koStore.h>

#include "kis_global.h"
#include "kis_paint_device.h"

class KisChannel : public KisPaintDevice {
	typedef KisPaintDevice super;

public:
	KisChannel(cId id, const QString& name, uint width, uint height, uint bpp);
	virtual ~KisChannel();

	cId channelId() const { return m_id; }
	int width() const { return m_imgRect.width(); }
	int height() const { return m_imgRect.height(); }
	QRect tileExtents() const { return m_tileRect; };
	QRect imageExtents() const { return m_imgRect; };
	QPoint offset() const { return m_imgRect.topLeft() - m_tileRect.topLeft(); };


#if 0
    void moveBy(int dx, int dy);
    void moveTo(int x, int y);
#endif
  
	void allocateRect(QRect newRect);
    	QRect tileRect(int tileNo);

	uint lastTileOffsetX();
	uint lastTileOffsetY();

	bool  writeToStore(KoStore *store); 
	bool  loadFromStore(KoStore *store); 

protected:
	cId m_id;
	QRect m_imgRect; 
	QRect m_tileRect;
};

#endif // __kis_channel_h__

