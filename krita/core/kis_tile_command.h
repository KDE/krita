/*
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#if !defined KIS_TILE_COMMAND_H_
#define KIS_TILE_COMMAND_H_

#include <map>
#include <qglobal.h>
#include <qstring.h>
#include <kcommand.h>
#include "kis_paint_device.h"

class QRect;

class KisTileCommand : public KCommand {
	typedef std::map<Q_INT32, KisTileSP> TileMap;

public:
	KisTileCommand(const QString& name, KisPaintDeviceSP device, Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height);
	KisTileCommand(const QString& name, KisPaintDeviceSP device, const QRect& rc);
	KisTileCommand(const QString& name, KisPaintDeviceSP device);
	virtual ~KisTileCommand();

public:
	virtual void execute();
	virtual void unexecute();
	virtual QString name() const;

public:
	void addTile(Q_INT32 tileNo, KisTileSP tile);
	KisTileSP tile(Q_INT32 tileNo);

private:
	TileMap m_tiles;
	TileMap m_originals;
	QString m_name;
	KisPaintDeviceSP m_device;
	QRect m_rc;
};

#endif // KIS_TILE_COMMAND_H_

