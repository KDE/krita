/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *
 *  this program is free software; you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation; either version 2 of the license, or
 *  (at your option) any later version.
 *
 *  this program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with this program; if not, write to the free software
 *  foundation, inc., 675 mass ave, cambridge, ma 02139, usa.
 */
#include <kdebug.h>
#include <koColor.h>
#include "kis_global.h"
#include "kis_paint_device.h"
#include "kis_image.h"
#include "kis_painter.h"
#include "kis_selection.h"
#include "kistile.h"
#include "kistilemgr.h"
#include "kispixeldata.h"

KisSelection::KisSelection(Q_INT32 width, Q_INT32 height, const enumImgType& imgType, const QString& name) 
	: super(width, height, imgType, name)
{
	visible(false);
	setName(name);
}

KisSelection::KisSelection(KisPaintDeviceSP parent, KisImageSP img, const QString& name, QUANTUM opacity) : super(img, 0, 0, name, opacity)
{
	Q_ASSERT(parent);
	Q_ASSERT(parent -> visible());
	Q_ASSERT(img);
	m_parent = parent;
	m_img = img;
	m_name = name;
	m_firstMove = true;
	connect(m_parent, SIGNAL(visibilityChanged(KisPaintDeviceSP)), SLOT(parentVisibilityChanged(KisPaintDeviceSP)));
}

KisSelection::~KisSelection()
{
}

void KisSelection::commit()
{
#if 1
	if (m_parent) {
		KisPainter gc(m_parent);
		QRect rc = clip();
		Q_INT32 w;
		Q_INT32 h;

		w = width();
		h = height();

		if (!rc.isEmpty()) {
			w = rc.width();
			h = rc.height();
		}

		Q_ASSERT(w <= width());
		Q_ASSERT(h <= height());
		// TODO Go over each tile... if src == dst, then don't do anything.  Just drop the share count.
		gc.bitBlt(m_rc.x() - m_parent -> x(), m_rc.y() - m_parent -> y(), 
				COMPOSITE_COPY, this, opacity(), 
				0, 0,
				m_rc.width(),
				m_rc.height());
	}
#endif
}

bool KisSelection::shouldDrawBorder() const
{
	return true;
}

void KisSelection::move(Q_INT32 x, Q_INT32 y)
{
#if 0
	// Should selections be allowed to move?!?
	QRect rc = clip();

	if (m_firstMove && m_parent) {
		KisPainter gc(m_parent);

		// push_undo_fill
		gc.eraseRect(m_rc);
		m_firstMove = false;
		m_parent -> invalidate(rc);
	}

#endif
	super::move(x, y);
//	rc |= bounds();
//	invalidate(rc);
}

void KisSelection::setBounds(Q_INT32 parentX, Q_INT32 parentY, Q_INT32 width, Q_INT32 height)
{
#if 1
	if (m_img) {
		configure(m_img, width, height, m_img -> imgType(), m_name);

		KisPainter gc(this);

		kdDebug(DBG_AREA_CORE) << "selection -> (parentX = " << parentX << ", parentY = " << parentY 
			<< ", width = " << width << ", height = " << height << ")\n";
		gc.bitBlt(0, 0, COMPOSITE_COPY, m_parent, parentX - m_parent -> x(), parentY - m_parent -> y(), width, height);
		m_rc.setRect(parentX, parentY, width, height);
		super::move(parentX, parentY);
	}
#endif

#if 0
	KisTileMgrSP tm1 = m_parent -> data();
	KisTileMgrSP tm2;
	KisTileSP tile;
	Q_INT32 tileno;
	Q_INT32 x;
	Q_INT32 y;
	Q_INT32 offset;
	Q_INT32 clipX;
	Q_INT32 clipY;
	Q_INT32 k;

	configure(m_img, width, height, m_img -> imgType(), m_name);
	tm2 = data();
	Q_ASSERT(tm2);
	kdDebug(DBG_AREA_CORE) << "selection -> (parentX = " << parentX << ", parentY = " << parentY << ", width = " << width << ", height = " << height << ")\n";

	for (y = parentY, k = 0; y < parentY + height; y += TILE_HEIGHT - (y % TILE_HEIGHT)) {
		offset = tm1 -> tileNum(parentX, y);

		for (x = parentX; x < width + parentX; x += TILE_WIDTH - (x % TILE_WIDTH), k++) {
			tileno = tm1 -> tileNum(x, y);

			if (tileno < 0)
				continue;

			tile = tm1 -> tile(tileno, TILEMODE_READ);
			Q_ASSERT(tile);

			if (tile)
				tm2 -> attach(tile, k);
		}
	}

	m_rc.setRect(parentX, parentY, width, height);
	clipX = parentX - parentX / TILE_WIDTH * TILE_WIDTH + 1;
	clipY = parentY - parentY / TILE_HEIGHT * TILE_HEIGHT + 1;
	width -= clipX - parentX;
	height -= clipY - parentY;
	setClip(clipX, clipY, width, height);
	kdDebug(DBG_AREA_CORE) << "selection clip -> (x = " << clipX << ", y = " << clipY << ", width = " << width << ", height = " << height << ")\n";
	
#if 0
	{
		// tile -> shareRef() above sets "Copy on write" flag.
		// however, it doesn't seem to be working very well righ now, so
		// just use a Painter to transfer the data.
		KisPainter gc(this);
		KisPainter g2(m_parent);

		gc.bitBlt(0, 0, COMPOSITE_COPY, m_parent, parentX, parentY, width, height);
//		gc.fillRect(clipX, clipY, width, height, KoColor::red(), OPACITY_OPAQUE);
//		g2.fillRect(m_rc.x(), m_rc.y(), m_rc.width(), m_rc.height(), KoColor::green(), OPACITY_OPAQUE);
	}
#endif

	parentX = parentX / TILE_WIDTH * TILE_WIDTH;
	parentY = parentY / TILE_HEIGHT * TILE_HEIGHT;
	super::move(parentX, parentY);
#endif
}

void KisSelection::fromImage(const QImage& img)
{
	KoColor c;
	QRgb rgb;

	if (!img.isNull()) {
		for (Q_INT32 y = 0; y < height(); y++) {
			for (Q_INT32 x = 0; x < width(); x++) {
				rgb = img.pixel(x, y);

				// TODO pixel opacity
				c.setRGB(upscale(qRed(rgb)), upscale(qGreen(rgb)), upscale(qBlue(rgb)));
				pixel(x, y, c, OPACITY_OPAQUE);
			}
		}
	}
}

QImage KisSelection::toImage()
{
	KisTileMgrSP tm = data();
	KisPixelDataSP raw;
	Q_INT32 stride;
	QUANTUM *src;

	if (tm) {
		if (tm -> width() == 0 || tm -> height() == 0)
			return QImage();

		raw = tm -> pixelData(0, 0, tm -> width() - 1, tm -> height() - 1, TILEMODE_READ);

		if (raw == 0)
			return QImage();

		if (m_clipImg.width() != tm -> width() || m_clipImg.height() != tm -> height())
			m_clipImg.create(tm -> width(), tm -> height(), 32);

		stride = tm -> depth();
		src = raw -> data;

		for (Q_INT32 y = 0; y < tm -> height(); y++) {
			for (Q_INT32 x = 0; x < tm -> width(); x++) {
				// TODO Different img formats
				m_clipImg.setPixel(x, y, qRgb(downscale(src[PIXEL_RED]), downscale(src[PIXEL_GREEN]), downscale(src[PIXEL_BLUE])));
				src += stride;
			}
		}
	}

	return m_clipImg;
}

void KisSelection::setBounds(const QRect& rc)
{
	setBounds(rc.x(), rc.y(), rc.width(), rc.height());
}

void KisSelection::parentVisibilityChanged(KisPaintDeviceSP parent)
{
	visible(parent -> visible());
}

void KisSelection::setParent(KisPaintDeviceSP parent)
{
	m_parent = parent;
}

KisPaintDeviceSP KisSelection::parent() const
{
	return m_parent;
}

#include "kis_selection.moc"

