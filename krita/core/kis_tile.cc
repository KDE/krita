/*
 *  kis_tile.cc - part of KImageShop aka Krayon aka Krita
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

#include <exception>

#include <Magick++.h>

#include <qtl.h>

#include <kdebug.h>

#include "kis_global.h"
#include "kis_pixel_packet.h"
#include "kis_magick.h"
#include "kis_tile.h"

using namespace Magick;

KisTile::KisTile(int x, int y, bool alpha, int width, int height, int depth, const QRgb& defaultColor) : m_data(0)
{
	m_data = 0;
	m_alpha = alpha;
	m_depth = depth;
	m_defaultColor = defaultColor;
	setGeometry(x, y, width, height);

	if (qAlpha(defaultColor))
		initTile();
}

KisTile::KisTile(const KisTile& tile) : KShared(tile)
{
	if (&tile != this)
		copy(tile);
}

KisTile& KisTile::operator=(const KisTile& tile)
{
	if (&tile != this)
		copy(tile);

	return *this;
}

KisTile::~KisTile()
{
	delete m_data;
}

const KisPixelPacket* KisTile::getConstPixels(int x, int y, int width, int height) const
{
	if (!m_data)
		return 0;

	QRect geometry(x, y, width, height);

	if (geometry.width() > m_geometry.width() || geometry.height() > m_geometry.height())
		return 0;

	return static_cast<const KisPixelPacket*>(m_data -> getConstPixels(x, y, width, height));
}

const KisPixelPacket* KisTile::getConstPixels() const
{
	return getConstPixels(0, 0, m_geometry.width(), m_geometry.height());
}

KisPixelPacket* KisTile::getPixels(int x, int y, int width, int height)
{
	if (!m_data)
		initTile();
	
	QRect geometry(x, y, width, height);

	if (geometry.width() > m_geometry.width() || geometry.height() > m_geometry.height())
		return 0;

	return static_cast<KisPixelPacket*>(m_data -> getPixels(x, y, width, height));
}

KisPixelPacket* KisTile::getPixels()
{
	return getPixels(0, 0, m_geometry.width(), m_geometry.height());
}

void KisTile::syncPixels()
{
	if (m_data)
		m_data -> syncPixels();
}

void KisTile::modifyImage()
{
//	if (m_data)
//		m_data -> modifyImage();
}

void KisTile::copy(const KisTile& tile)
{
	m_depth = tile.m_depth;
	m_data = 0;
	m_defaultColor = tile.m_defaultColor;
	m_geometry = tile.m_geometry;

	if (tile.m_data) {
		m_data = new Image();
		*m_data = *tile.m_data;
	}
}

void KisTile::clear()
{
	m_depth = 0;
	delete m_data;
	m_data = 0;
	m_defaultColor = 0;
	setGeometry(0, 0, 0, 0);
}

void KisTile::move(int x, int y)
{
	m_geometry.moveBy(x, y);
}

void KisTile::move(const QPoint& parentPos)
{
	m_geometry.moveBy(parentPos.x(), parentPos.y());
}

QPoint KisTile::topLeft() const
{
	return m_geometry.topLeft();
}

QPoint KisTile::bottomRight() const
{
	return m_geometry.bottomRight();
}

const QRect& KisTile::geometry() const
{
	return m_geometry;
}

void KisTile::setGeometry(int x, int y, int w, int h)
{
	m_geometry.setRect(x, y, w, h);
}

void KisTile::setGeometry(const QRect& rc)
{
	m_geometry = rc;
}

QImage KisTile::convertTileToImage()
{
	if (!m_data)
		return QImage();

	return convertFromMagickImage(*m_data);
}

void KisTile::convertTileFromImage(const QImage& img)
{
	if (img.isNull())
		return;

	m_depth = img.depth();
	m_defaultColor = 0;
	setGeometry(0, 0, img.width(), img.height());
	initTile();
	*m_data = convertToMagickImage(img);
}

void KisTile::initTile()
{
	Color c(Upscale(qRed(m_defaultColor)), 
			Upscale(qGreen(m_defaultColor)), 
			Upscale(qBlue(m_defaultColor)), 
			Upscale(TransparentOpacity - qAlpha(m_defaultColor)));

	try {
		delete m_data;
		m_data = new Image(Geometry(m_geometry.width(), m_geometry.height()), "red"); //c);
		m_data -> matte(m_alpha);
	} catch (std::exception& e) {
		kdDebug() << "KisTile::initTile " << e.what() << endl;
		throw;
	}
}

Image* KisTile::getImage()
{
	if (!m_data)
		initTile();

	return m_data;
}

const Image* KisTile::getImage() const
{
	return m_data;
}

int KisTile::depth() const
{
	return m_depth;
}

QSize KisTile::size() const
{
	return m_geometry.size();
}

int KisTile::width() const
{
	return m_geometry.width();
}

int KisTile::height() const
{
	return m_geometry.height();
}

void KisTile::composite(const KisTileSP src, const TileCompositeOperator& op)
{
	if (m_data && src -> m_data) {
		m_data -> composite(*src -> m_data, 0, 0, op);
		m_data -> syncPixels();
	}
}

