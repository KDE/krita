/*
 *  kis_tiles.cc - part of KImageShop aka Krayon aka Krita
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

#include <qtl.h>

#include <kdebug.h>

#include "kis_global.h"
#include "kis_tiles.h"
#include "kis_tile.h"

KisTiles::KisTiles()
{
}

KisTiles::KisTiles(int width, int height, int depth, const QRgb& defaultColor)
{
	m_defaultColor = defaultColor;
	init(width, height, depth);
}

KisTiles::~KisTiles()
{
	cleanup();
}

KisTileSP KisTiles::setTile(int x, int y, KisTileSP tile)
{
	if (!intersects(x, y))
		return 0;

	int tileNo = getTileNo(x, y);
	KisTileSP oldTile = m_tiles[tileNo];

	m_tiles[tileNo] = tile;
	return oldTile;
}

KisTileSP KisTiles::setTile(const QPoint& pt, KisTileSP tile)
{
	return setTile(pt.x(), pt.y(), tile);
}

KisTileSP KisTiles::getTile(int x, int y)
{
	if (!intersects(x, y))
		return 0;

	int tileNo = getTileNo(x, y);

	if (!m_tiles[tileNo])
		m_tiles[tileNo] = new KisTile(x, y, false, TILE_SIZE, TILE_SIZE, m_depth, m_defaultColor);

	Q_ASSERT(m_tiles[tileNo] != 0);	
	return m_tiles[tileNo];
}

KisTileSP KisTiles::getTile(const QPoint& pt)
{
	return getTile(pt.x(), pt.y());
}

const KisTileSP KisTiles::getTile(int x, int y) const
{
	if (!intersects(x, y))
		return 0;

	int tileNo = getTileNo(x, y);

	if (!m_tiles[tileNo])
		return 0;
	
	return m_tiles[tileNo];
}

const KisTileSP KisTiles::getTile(const QPoint& pt) const
{
	return getTile(pt.x(), pt.y());
}

KisTileSP KisTiles::takeTile(int x, int y)
{
	if (!intersects(x, y))
		return 0;

	int tileNo = getTileNo(x, y);
	KisTileSP tile = m_tiles[tileNo];

	m_tiles[tileNo] = 0;
	return tile;
}

bool KisTiles::swapTile(int x1, int y1, int x2, int y2)
{
	if (!intersects(x1, y1) || !intersects(x2, y2))
		return false;

	int tileNo1 = getTileNo(x1, y1);
	int tileNo2 = getTileNo(x2, y2);

	qSwap(m_tiles[tileNo1], m_tiles[tileNo2]);
	return true;
}

void KisTiles::markDirty(int x, int y)
{
	if (!intersects(x, y))
		return;
	
//	m_tiles[getTileNo(x, y)] -> modifyImage();
}

void KisTiles::resize(int width, int height, int depth)
{
	m_tiles.resize(width * height);

	for (int i = m_yTiles * m_xTiles; i < m_tiles.size(); i++)
		m_tiles[i] = 0;
	
	m_xTiles = width;
	m_yTiles = height;
	m_depth = depth;
}

void KisTiles::init(int width, int height, int depth)
{
	m_xTiles = width;
	m_yTiles = height;
	m_depth = depth;
	m_tiles.resize(m_xTiles * m_yTiles);
	qFill(m_tiles.begin(), m_tiles.end(), static_cast<KisTile*>(0));
}

void KisTiles::cleanup()
{
	m_tiles.clear();
	m_xTiles = 0;
	m_yTiles = 0;
}

int KisTiles::xTiles() const
{ 
	return m_xTiles; 
}

int KisTiles::yTiles() const 
{ 
	return m_yTiles; 
}

int KisTiles::depth() const
{
	return m_depth;
}

int KisTiles::getTileNo(int x, int y) const
{
	return y * xTiles() + x;
}

bool KisTiles::intersects(int x, int y) const
{
	return !(y >= yTiles() || x >= xTiles());
}


