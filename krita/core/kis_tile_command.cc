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

#include "kis_types.h"
#include "kis_global.h"
#include "tiles/kistile.h"
#include "tiles/kistilemgr.h"
#include "kis_image.h"
#include "kis_tile_command.h"

KisTileCommand::KisTileCommand(const QString& name, KisPaintDeviceSP device,
		Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height)
{
	m_name = name;
	m_device = device;
	m_rc.setRect(x + m_device -> x(), y + m_device -> y(), width, height);
}

KisTileCommand::KisTileCommand(const QString& name, KisPaintDeviceSP device, const QRect& rc)
{
	m_name = name;
	m_device = device;
	m_rc = rc;
	m_rc.moveBy(m_device -> x(), m_device -> y());
}

KisTileCommand::KisTileCommand(const QString& name, KisPaintDeviceSP device)
{
	m_name = name;
	m_device = device;
	m_rc = device -> bounds();
	m_rc.moveBy(m_device -> x(), m_device -> y());
}

KisTileCommand::~KisTileCommand()
{
	for (TileMap::iterator it = m_tiles.begin(); it != m_tiles.end(); it++)
		it -> second -> shareRelease();
}

void KisTileCommand::execute()
{
	KisTileMgrSP tm = m_device -> data();
	KisImageSP img = m_device -> image();

	for (TileMap::iterator it = m_originals.begin(); it != m_originals.end(); it++) {
		tm -> attach(it -> second, it -> first);
	}

	if (img)
		img -> notify(m_rc);
}

void KisTileCommand::unexecute()
{
	KisTileMgrSP tm = m_device -> data();
	KisImageSP img = m_device -> image();
	KisTileSP tmp;

	if (m_originals.empty()) {
		for (TileMap::iterator it = m_tiles.begin(); it != m_tiles.end(); it++) {
			tmp = tm -> tile(it -> first, TILEMODE_NONE);
			tm -> attach(it -> second, it -> first);
			m_originals[it -> first] = tmp;
		}
	} else {
		for (TileMap::iterator it = m_tiles.begin(); it != m_tiles.end(); it++) {
			tm -> attach(it -> second, it -> first);
		}
	}

	if (img)
		img -> notify(m_rc);
}

QString KisTileCommand::name() const
{
	return m_name;
}

KisTileSP KisTileCommand::tile(Q_INT32 tileNo)
{
	if (m_tiles.count(tileNo) == 0) {
		return  0;
	}
	return m_tiles[tileNo];
}

void KisTileCommand::addTile(Q_INT32 tileNo, KisTileSP tile)
{
	Q_ASSERT(tile);

	if (m_tiles.count(tileNo) == 0) {
		tile -> shareRef();
		m_tiles[tileNo] = tile;
	}
}

