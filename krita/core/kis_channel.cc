/*
 *  kis_channel.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Andrew Richards <A.Richards@phys.canterbury.ac.nz>
 *                1999-2000 Matthias ELter  <elter@kde.org>
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

#include <qcolor.h>

#include <kdebug.h>

#include "kis_global.h"
#include "kis_channel.h"

KisChannel::KisChannel(cId id, const QString& name, uint width, uint height, const QRgb& defaultColor) : super(name, width, height, 4, defaultColor)
{
	m_id = id;
	m_imgRect = m_tileRect = QRect(0, 0, width, height);
}

KisChannel::~KisChannel()
{
}

uint KisChannel::lastTileOffsetX()
{
	uint lastTileXOffset = TILE_SIZE - (m_tileRect.right() - m_imgRect.right());

	return ((lastTileXOffset) ? lastTileXOffset :  TILE_SIZE);
}

uint KisChannel::lastTileOffsetY()
{
	uint lastTileYOffset = TILE_SIZE - (m_tileRect.bottom() - m_imgRect.bottom());

	return ((lastTileYOffset) ? lastTileYOffset :  TILE_SIZE);
}

QRect KisChannel::tileRect(int tileNo)
{
	int xTile = tileNo % xTiles();
	int yTile = tileNo / xTiles();

	QRect tr(xTile * TILE_SIZE, yTile * TILE_SIZE, TILE_SIZE, TILE_SIZE);
	tr.moveBy(m_tileRect.x(), m_tileRect.y());

	return(tr);
}

void KisChannel::setPixel(uint x, uint y, const uchar *pixel, KisImageCmd *cmd)
{
	// TODO : Manipulate pixel value before going upstream
	super::setPixel(x, y, pixel, cmd);
}

