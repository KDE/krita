/*
 *  kis_image_cmd.h - part of Krita AKA Krayon AKA KImageShop
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

#if !defined(KIS_IMAGE_CMD_H_)
#define KIS_IMAGE_CMD_H_

#include <qstring.h>

#include <kcommand.h>

#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_tile.h"

class KisImageCmd : public KCommand {
	typedef KCommand super;

public:
	KisImageCmd(const QString& name, KisImageSP img, KisPaintDeviceSP device);
	virtual ~KisImageCmd();

	virtual QString name() const;
	virtual void execute();
	virtual void unexecute();

	void addTile(KisTileSP tile);
	bool hasTile(KisTileSP tile);

private:
	KisImageSP m_img;
	KisPaintDeviceSP m_device;
	QString m_name;
	KisTileSPLst m_tiles;
	KisTileSPLst m_originalTiles;
};

#endif // KIS_IMAGE_CMD_H_

