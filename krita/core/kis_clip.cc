/*
 *  kis_clip.cc - part of Krayon
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#include <qimage.h>

#include "kis_paint_device.h"
#include "kis_types.h"
#include "kis_profile.h"

KisClip::KisClip(QImage & clip, KisProfileSP profile)
{
	new KisPaintDevice(qimg.width(), qimg.height(),
			   KisColorSpaceRegistry::instance()->colorSpace( qimg.hasAlphaBuffer() ? "RGBA" : "RGB" ),
			   "Clipboard Selection");

	m_clipboard -> convertFromImage(qimg);
	m_profile = profile;
}

KisClip::KisClip(KisPaintDeviceSP clip, KisProfileSP profile)
{
	m_clip = clip;
	m_profile = profile;
}

KisClip::~KisClip()
{
}
