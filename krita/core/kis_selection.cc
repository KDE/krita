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
#include "kis_image.h"
#include "kis_painter.h"
#include "kis_selection.h"
#include "kistile.h"
#include "kistilemgr.h"

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
//	gc.fillRect(m_rc, KoColor::red(), OPACITY_OPAQUE);
	// TODO Go over each tile... if src == dst, then don't do anything.  Just drop the share count.
//	gc.bitBlt(m_rc.x(), m_rc.y(), COMPOSITE_COPY, this, opacity(), rc.x(), rc.y(), rc.width(), rc.height());
}

bool KisSelection::shouldDrawBorder() const
{
	return true;
}

void KisSelection::move(Q_INT32 x, Q_INT32 y)
{
	QRect rc = clip();

	if (m_firstMove) {
		KisPainter gc(m_parent);

		// push_undo_fill
		gc.fillRect(m_rc, KoColor::black(), OPACITY_TRANSPARENT);
		m_firstMove = false;
		m_parent -> invalidate(rc);
	}

	super::move(x, y);
	rc |= bounds();
	invalidate(rc);
}

void KisSelection::setBounds(Q_INT32 parentX, Q_INT32 parentY, Q_INT32 width, Q_INT32 height)
{
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

	configure(m_img, parentX + width + 1, parentY + height + 1, m_img -> imgType(), m_name);
	tm2 = data();
	Q_ASSERT(tm2);
	kdDebug(DBG_AREA_CORE) << "selection -> (parentX = " << parentX << ", parentY = " << parentY << ", width = " << width << ", height = " << height << ")\n";

	for (y = parentY, k = 0; y < height; y += TILE_HEIGHT - (y % TILE_HEIGHT)) {
		offset = tm1 -> tileNum(parentX, y);

		for (x = parentX; x < width; x += TILE_WIDTH - (x % TILE_WIDTH), k++) {
			tileno = tm1 -> tileNum(x, y);

			if (tileno < 0)
				continue;

			tile = new KisTile(4, 0, 0);
//			tile = tm1 -> tile(tileno, TILEMODE_READ);
			Q_ASSERT(tile);

#if 0
			if (tile) {
				tile -> shareRef();
				tm2 -> attach(tile, k);
			}
#else
			tm2 -> attach(tile, k);
#endif

		}
	}

	m_rc.setRect(parentX, parentY, width, height);
	clipX = parentX - parentX / TILE_WIDTH * TILE_WIDTH + 1;
	clipY = parentY - parentY / TILE_HEIGHT * TILE_HEIGHT + 1;
	width -= clipX - parentX;
	height -= clipY - parentY;
	setClip(clipX, clipY, width, height);
	kdDebug(DBG_AREA_CORE) << "selection clip -> (x = " << clipX << ", y = " << clipY << ", width = " << width << ", height = " << height << ")\n";
	
#if 1
	{
		// tile -> shareRef() above sets "Copy on write" flag.
		// however, it doesn't seem to be working very well righ now, so
		// just use a Painter to transfer the data.
		KisPainter gc(this);
		KisPainter g2(m_parent);

//		gc.bitBlt(0, 0, COMPOSITE_COPY, m_parent, parentX, parentY, width, height);
//		gc.fillRect(clipX, clipY, width, height, KoColor::red(), OPACITY_OPAQUE);
		g2.fillRect(m_rc.x(), m_rc.y(), m_rc.width(), m_rc.height(), KoColor::green(), OPACITY_OPAQUE);
	}
#endif

	parentX = parentX / TILE_WIDTH * TILE_WIDTH;
	parentY = parentY / TILE_HEIGHT * TILE_HEIGHT;
	super::move(parentX, parentY);
}

void KisSelection::setBounds(const QRect& rc)
{
	setBounds(rc.x(), rc.y(), rc.width(), rc.height());
}

void KisSelection::parentVisibilityChanged(KisPaintDeviceSP parent)
{
	visible(parent -> visible());
}

KisPaintDeviceSP KisSelection::parent() const
{
	return m_parent;
}

#include "kis_selection.moc"

