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
	Q_ASSERT(img);
	m_parent = parent;
	m_img = img;
	m_name = name;
	m_firstMove = true;
}

KisSelection::~KisSelection()
{
	// push_undo_bitBlt
	// commit tiles to parent
}

bool KisSelection::shouldDrawBorder() const
{
	return true;
}

void KisSelection::move(Q_INT32 x, Q_INT32 y)
{
	if (m_firstMove) {
		// TODO
		// copy tiles here
		// i.e. create new tiles for parent
		// init tiles to black transparent
		// copy tiles ouvtide the drawoffset to the newly created tiles
		KisPainter gc(m_parent);

		// push_undo_fill
		gc.fillRect(m_rc, KoColor::black(), OPACITY_TRANSPARENT);
		m_firstMove = false;
	}

	super::move(x, y);
	m_parent -> invalidate();
	invalidate();
}

void KisSelection::setBounds(Q_INT32 parentX, Q_INT32 parentY, Q_INT32 width, Q_INT32 height)
{
	KisTileMgrSP tm1 = m_parent -> data();
	KisTileMgrSP tm2;
	KisTileSP tile;
	Q_INT32 tileno;
	Q_INT32 x;
	Q_INT32 y;

	configure(m_img, width, height, m_img -> imgType(), m_name);
	tm2 = data();
	Q_ASSERT(tm2);

	for (y = parentY; y < height; y += TILE_HEIGHT) {
		for (x = parentX; x < width; x += TILE_WIDTH) {
			tileno = tm1 -> tileNum(x, y);

			if (tileno < 0)
				continue;

			tile = tm1 -> tile(tileno, TILEMODE_READ);

			if (tile) {
				tile -> shareRef();
				tm2 -> attach(tile, tileno);
			}
		}
	}

	m_rc.setRect(parentX, parentY, width, height);
	super::move(parentX, parentY);
}

void KisSelection::setBounds(const QRect& rc)
{
	setBounds(rc.x(), rc.y(), rc.width(), rc.height());
}

