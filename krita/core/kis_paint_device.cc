/*
 *  kis_paint_device.cc - part of KImageShop aka Krayon aka Krita
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

#include <Magick++.h>

#include <qcstring.h>
#include <qdatastream.h>
#include <qpoint.h>
#include <qsize.h>

#include <kdebug.h>

#include <koStore.h>

#include "kis_global.h"
#include "kis_image_cmd.h"
#include "kis_pixel_region.h"
#include "kis_util.h"
#include "kis_tile.h"

using namespace Magick;

const int TILE_BYTES = TILE_SIZE * TILE_SIZE * sizeof(int);
    
KisPaintDevice::KisPaintDevice(const QString& name, int width, int height, uchar depth, const QRgb& defaultColor, bool alpha)
{
	Color clr(Upscale(qRed(defaultColor)), Upscale(qGreen(defaultColor)), Upscale(qBlue(defaultColor)), TransparentOpacity - Upscale(qAlpha(defaultColor)));

	m_depth = depth;
	m_name = name;
	m_imgRect = QRect(0, 0, width, height);
	m_tileRect = KisUtil::findTileExtents(m_imgRect);
	m_visible = true;
	m_opacity = CHANNEL_MAX;
	m_tiles = new Image(Geometry(m_tileRect.width(), m_tileRect.height()), clr);
	m_tiles -> matte(alpha);
}

KisPaintDevice::~KisPaintDevice()
{
}

void KisPaintDevice::resize(int width, int height, uchar depth)
{
	m_imgRect.setWidth(width);
      	m_imgRect.setHeight(height);
	m_tileRect = KisUtil::findTileExtents(m_imgRect);
//	m_tiles -> scale(Geometry(width, height));
}

void KisPaintDevice::findTileNumberAndOffset(QPoint pt, int *tileNo, int *offset) const
{
	pt = pt - tileExtents().topLeft();
	*tileNo = (pt.y() / TILE_SIZE) * xTiles() + pt.x() / TILE_SIZE;
	*offset = (pt.y() % TILE_SIZE) * TILE_SIZE + pt.x() % TILE_SIZE;
}

void KisPaintDevice::findTileNumberAndPos(QPoint pt, int *tileNo, int *x, int *y) const
{
	pt = pt - tileExtents().topLeft();
	*tileNo = (pt.y() / TILE_SIZE) * xTiles() + pt.x() / TILE_SIZE;
	*y = pt.y() % TILE_SIZE;
	*x = pt.x() % TILE_SIZE;
}

QRect KisPaintDevice::tileExtents() const
{
	return m_tileRect;
}

bool KisPaintDevice::writeToStore(KoStore *store)
{
#if 0
	for (int ty = 0; ty < yTiles(); ty++) 
		for (int tx = 0; tx < xTiles(); tx++) {
			KisTileSP src = m_tiles.getTile(tx, ty);
			const char *p = reinterpret_cast<const char*>(src -> data());

			if (store -> write(p, TILE_BYTES) != TILE_BYTES)
				return false;
		}

	return true;
#endif
	return false;
}

bool KisPaintDevice::loadFromStore(KoStore *store)
{
#if 0
	int nread;

	for (int ty = 0; ty < yTiles(); ty++) {
		for (int tx = 0; tx < xTiles(); tx++) {
			KisTileSP dst = m_tiles.getTile(tx, ty);
			char *p = reinterpret_cast<char*>(dst -> data());

			nread = store -> read(p, TILE_BYTES);

			if (nread != TILE_BYTES)
				return false;
		}
	}

	return true;
#endif
	return false;
}

QRect KisPaintDevice::imageExtents() const
{
	return m_imgRect;
}

void KisPaintDevice::moveBy(int dx, int dy)
{
	m_imgRect.moveBy(dx, dy);
	m_tileRect.moveBy(dx, dy);
}

void KisPaintDevice::moveTo(int x, int y)
{
	int dx = x - m_imgRect.x();
	int dy = y - m_imgRect.y();

	m_imgRect.moveTopLeft(QPoint(x, y));
	m_tileRect.moveBy(dx,dy);
}

void KisPaintDevice::allocateRect(const QRect& rc, uchar depth)
{
	resize(m_tileRect.width(), m_tileRect.height(), depth);
	m_imgRect = rc;
}

const KisPixelPacket* KisPaintDevice::getConstPixels(int x, int y, int w, int h) const
{
	return 0;
}

KisPixelPacket* KisPaintDevice::getPixels(int x, int y, int w, int h)
{
	return static_cast<KisPixelPacket*>(m_tiles -> getPixels(x, y, w, h));
}

void KisPaintDevice::syncPixels(KisPixelPacket *region)
{
	m_tiles -> syncPixels();
}

int KisPaintDevice::xTiles() const
{
	return m_tileRect.width() / TILE_SIZE;
}

int KisPaintDevice::yTiles() const
{
	return m_tileRect.height() / TILE_SIZE;
}

uchar KisPaintDevice::depth() const
{
	return m_depth;
}

void KisPaintDevice::setName(const QString& name)
{
	m_name = name;
}

QString KisPaintDevice::name() const
{
	return m_name;
}

KisTileSP KisPaintDevice::getTile(unsigned int x, unsigned int y) 
{ 
	// TODO
	return 0;
}

const KisTileSP KisPaintDevice::getTile(unsigned int x, unsigned int y) const
{ 
	// TODO
	return 0;
}

uchar KisPaintDevice::opacity() const 
{ 
	return m_opacity; 
}

void KisPaintDevice::setOpacity(uchar o) 
{ 
	m_opacity = o; 
}

bool KisPaintDevice::visible() const 
{ 
	return m_visible; 
}

void KisPaintDevice::setVisible(bool v) 
{ 
	m_visible = v; 
}

int KisPaintDevice::width() const
{
	return m_imgRect.width();
}

int KisPaintDevice::height() const
{
	return m_imgRect.height();
}

QSize KisPaintDevice::size() const
{
	return QSize(width(), height());
}

