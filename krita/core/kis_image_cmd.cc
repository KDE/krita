/*
 *  kis_image_cmd.cc - part of Krita AKA Krayon AKA KImageShop
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

#include <qrect.h>
#include <qtl.h>

#include "kis_image.h"
#include "kis_image_cmd.h"
#include "kis_paint_device.h"
#include "kis_tile.h"

KisImageCmd::KisImageCmd(const QString& name, KisImageSP img, KisPaintDeviceSP device)
{
	m_name = name;
	m_img = img;
	m_device = device;
}

KisImageCmd::~KisImageCmd()
{
}

QString KisImageCmd::name() const
{
	return m_name;
}

void KisImageCmd::execute()
{
#if 0
	for (KisTileSPLstIterator it = m_tiles.begin(); it != m_tiles.end(); it++)
		m_device -> swapTile(*it);

	m_img -> markDirty(QRect(0, 0, m_img -> width(), m_img -> height())); // FIXME don't update entire image
#endif
}

void KisImageCmd::unexecute()
{
#if 0
	for (KisTileSPLstIterator it = m_originalTiles.begin(); it != m_originalTiles.end(); it++)
		m_device -> swapTile(*it);

	m_img -> markDirty(QRect(0, 0, m_img -> width(), m_img -> height())); // FIXME don't update entire image
#endif
}

void KisImageCmd::addTile(KisTileSP tile)
{
#if 0
	if (!hasTile(tile)) {
		KisTileSP tileCopy = new KisTile(*tile);

		m_originalTiles.push_back(tileCopy);
		//tile -> setCow();
		m_tiles.push_back(tile);
	}
#endif
}

bool KisImageCmd::hasTile(KisTileSP tile)
{
	return qFind(m_tiles.begin(), m_tiles.end(), tile) != m_tiles.end();
}

