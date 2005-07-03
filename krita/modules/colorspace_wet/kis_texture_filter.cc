/*
 * kis_texture_filter.cc -- Part of Krita
 *
 * Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
 
#include <kdebug.h>
#include <kis_view.h>
#include <kis_image.h>
#include <kis_colorspace_registry.h>

#include "kis_texture_painter.h"
#include "kis_texture_filter.h"

TextureFilter::TextureFilter(KisView* view)
	: m_view(view) {
}

void TextureFilter::slotActivated() {
	kdDebug(DBG_AREA_CMS) << "texture filter activated" << endl;
	if (!m_view -> currentImg())
		return;
	if (!m_view -> currentImg() -> activeDevice())
		return;

	KisPaintDeviceSP device = m_view -> currentImg() -> activeDevice();
	KisStrategyColorSpaceSP cs = device -> colorStrategy();

	if (cs -> id() != KisID("WET","")) {
		kdDebug(DBG_AREA_CMS) << "You set this kind of texture on non-wet layers!.\n";
		return;
	}

	// XXX if params of the painter get configurable, make them here configurable as well?
	KisTexturePainter painter(device);
	painter.createTexture(0, 0, m_view -> currentImg() -> width(), m_view -> currentImg() -> height());
	painter.end();
	m_view -> currentImg() -> notify();
}
