/*
 *  kis_layer.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Andrew Richards <A.Richards@phys.canterbury.ac.nz>
 *                1999-2000 Matthias Elter <elter@kde.org>
 *                2002 Patrick Julien <freak@codepimps.org>
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

#include <assert.h>

#include <kdebug.h>

#include "kis_layer.h"
#include "kis_global.h"

KisLayer::KisLayer(const QString& name, uint width, uint height, uint bpp, cMode cm, const QRgb& defaultColor) : super(name, width, height, bpp, defaultColor)
{
	m_cMode = cm;
	m_visible = true;
	m_linked = false;
	m_current = 0;
	m_opacity= 255;
}

KisLayer::~KisLayer()
{
	kdDebug() << "KisLayer::~KisLayer\n";
}

// XXX Move the firstChannel and nextChannel to KisImage
#if 0
void KisLayer::calcNumChannels()
{
    switch (m_cMode)
    {
	    case cm_Indexed:
	    case cm_Greyscale:
	        m_channels = 1;
	        return;

	    case cm_RGB:
	    case cm_Lab:
	        m_channels = 3;
	        return;

	    case cm_RGBA:
	    case cm_CMYK:
	    case cm_LabA:
	        m_channels = 4;
	        return;

	    case cm_CMYKA:
	        m_channels = 5;
	        return;

	    default:
	        m_channels = 0;
	        return;
    }
}

#endif

QRect KisLayer::tileRect(int tileNo)
{
	int xTile = tileNo % m_tiles.xTiles();
	int yTile = tileNo / m_tiles.xTiles(); // xTiles is used here.  Is this the intent?

	QRect tr(xTile * TILE_SIZE, yTile * TILE_SIZE, TILE_SIZE, TILE_SIZE);

	tr.moveBy(m_tileRect.x(), m_tileRect.y());
	return(tr);
}


int KisLayer::channelLastTileOffsetX() const
{
	Q_ASSERT(false);
	return 0;
//	return m_ch[0]->lastTileOffsetX();
}


int KisLayer::channelLastTileOffsetY() const
{
	Q_ASSERT(false);
	return 0;
//	return m_ch[0]->lastTileOffsetY();
}


bool KisLayer::boundryTileX(int tile) const
{
	return (((tile % xTiles()) + 1) == xTiles());
}


bool KisLayer::boundryTileY(int tile) const
{
	return (((tile/xTiles()) + 1) == yTiles());
}

#include "kis_layer.moc"

