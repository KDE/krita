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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <qrect.h>
#include "kis_global.h"
#include "kis_types.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kistile.h"
#include "kistilemgr.h"
#include "kispixeldata.h"

KisPaintDevice::KisPaintDevice(KisImageSP img, Q_INT32 width, Q_INT32 height, const enumImgType& imgType, const QString& name)
{
	init();
	configure(img, width, height, imgType, name);
}

KisPaintDevice::~KisPaintDevice()
{
}

void KisPaintDevice::invalidate(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
	Q_INT32 dx = x + w;
	Q_INT32 dy = y + h;
	Q_INT32 x1;
	Q_INT32 y1;

	m_projectionValid = false;

	for (y1 = y; y1 < dy; y1 += TILE_HEIGHT)
		for (x1 = x; x1 < dx; x1 += TILE_WIDTH)
			m_tiles -> invalidate(x1, y1);
}

void KisPaintDevice::invalidate(const QRect& rc)
{
	invalidate(rc.x(), rc.y(), rc.width(), rc.height());
}

void KisPaintDevice::invalidate()
{
	invalidate(0, 0, width(), height());
}

QPixmap KisPaintDevice::pixmap()
{
	return QPixmap();
}

QPixmap KisPaintDevice::recreatePixmap()
{
	return QPixmap();
}

void KisPaintDevice::configure(KisImageSP image, Q_INT32 width, Q_INT32 height, const enumImgType& imgType, const QString& name)
{
	if (image == 0 || name.isEmpty())
		return;

	m_width = width;
	m_height = height;
	m_imgType = imgType;
	m_depth = image -> depth();
	m_alpha = image -> alpha();
	m_offX = 0;
	m_offY = 0;
	m_tiles = new KisTileMgr(m_depth, width, height);
	m_visible = true;
	m_owner = image;
	m_name = name;
	m_projectionValid = false;
}

void KisPaintDevice::duplicate(KisPaintDevice& , bool )
{

}

void KisPaintDevice::update()
{
	update(0, 0, width(), height());
}

void KisPaintDevice::update(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
	Q_INT32 xoff;
	Q_INT32 yoff;

	if (!m_owner)
		return;
	
	drawOffset(&xoff, &yoff);
	x += xoff;
	y += yoff;
	invalidate(x, y, w, h);
}

QString KisPaintDevice::name()
{
	return m_name;
}

void KisPaintDevice::setName(const QString& name)
{
	if (!name.isEmpty())
		m_name = name;
}

void KisPaintDevice::mergeShadow()
{
	QRect rc;
	KisPixelDataSP shadow;

	if (!m_owner || m_shadow -> empty())
		return;

	maskBounds(&rc);
	shadow = m_shadow -> pixelData(rc.left(), rc.top(), rc.right(), rc.bottom(), TILEMODE_READ);
//	m_owner -> apply(this, shadow, OPACITY_OPAQUE, COMPOSITE_COPY, rc.x(), rc.y());
}

void KisPaintDevice::fill(const KoColor& )
{

}

void KisPaintDevice::maskBounds(Q_INT32 *, Q_INT32 *, Q_INT32 *, Q_INT32 *)
{
}

void KisPaintDevice::maskBounds(QRect *rc)
{
	Q_INT32 x1;
	Q_INT32 y1;
	Q_INT32 x2;
	Q_INT32 y2;

	maskBounds(&x1, &y1, &x2, &y2);
	rc -> setRect(x1, y1, x2 - x1, y2 - y1);
}

bool KisPaintDevice::alpha() const
{
	return m_alpha;
}

enumImgType KisPaintDevice::type() const
{
	switch (m_imgType) {
	case IMAGE_TYPE_INDEXED:
		return IMAGE_TYPE_INDEXEDA;
	case IMAGE_TYPE_GREY:
		return IMAGE_TYPE_GREYA;
	case IMAGE_TYPE_RGB:
		return IMAGE_TYPE_RGBA;
	case IMAGE_TYPE_CMYK:
		return IMAGE_TYPE_CMYKA;
	case IMAGE_TYPE_LAB:
		return IMAGE_TYPE_LABA;
	case IMAGE_TYPE_YUV:
		return IMAGE_TYPE_YUVA;
	default:
		return m_imgType;
	}

	return m_imgType;
}

enumImgType KisPaintDevice::typeWithAlpha() const
{
	switch (m_imgType) {
		case IMAGE_TYPE_INDEXEDA:
			return IMAGE_TYPE_INDEXED;
		case IMAGE_TYPE_GREYA:
			return IMAGE_TYPE_GREY;
		case IMAGE_TYPE_RGBA:
			return IMAGE_TYPE_RGB;
		case IMAGE_TYPE_CMYKA:
			return IMAGE_TYPE_CMYK;
		case IMAGE_TYPE_LABA:
			return IMAGE_TYPE_LAB;
		case IMAGE_TYPE_YUVA:
			return IMAGE_TYPE_YUV;
		default:
			return m_imgType;
	}

	return m_imgType;

}

KisTileMgrSP KisPaintDevice::data()
{
	return m_tiles;
}

const KisTileMgrSP KisPaintDevice::data() const
{
	return m_tiles;
}

KisTileMgrSP KisPaintDevice::shadow()
{
	return m_shadow;
}

const KisTileMgrSP KisPaintDevice::shadow() const
{
	return m_shadow;
}

Q_INT32 KisPaintDevice::quantumSize() const
{
	return 0;
}

Q_INT32 KisPaintDevice::quantumSizeWithAlpha() const
{
	return 0;
}

Q_INT32 KisPaintDevice::x() const
{
	return m_offX;
}

Q_INT32 KisPaintDevice::y() const
{
	return m_offY;
}

Q_INT32 KisPaintDevice::width() const
{
	return m_width;
}

Q_INT32 KisPaintDevice::height() const
{
	return m_height;
}

const bool KisPaintDevice::visible() const
{
	return m_visible;
}

void KisPaintDevice::visible(bool v)
{
	if (m_visible != v) {
		m_visible = v;
		emit visibilityChanged(this);
		update();
	}
}

void KisPaintDevice::drawOffset(Q_INT32 *offx, Q_INT32 *offy)
{
	if (offx && offy) {
		*offx = m_offX;
		*offy = m_offY;
	}
}

bool KisPaintDevice::cmap(KoColorMap& cm)
{
	cm.clear();
	return false;
}

KoColor KisPaintDevice::colorAt()
{
	return KoColor();
}

KisImageSP KisPaintDevice::image()
{
	return m_owner;
}

const KisImageSP KisPaintDevice::image() const
{
	return m_owner;
}

void KisPaintDevice::setImage(KisImageSP image)
{
	m_owner = image;
}

void KisPaintDevice::init()
{
	m_visible = false;
	m_width = 0;
	m_height = 0;
	m_depth = 0;
	m_opacity = 0;
	m_alpha = false;
	m_quantumSize = 0;
	m_offX = 0;
	m_offY = 0;
	m_projectionValid = false;
}

