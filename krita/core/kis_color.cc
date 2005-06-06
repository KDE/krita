/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_pixel.h"
#include "kis_color.h"
#include "kis_profile.h"
#include "kis_strategy_colorspace.h"

KisColor::KisColor(Q_UINT8 * data, KisStrategyColorSpaceSP colorStrategy, KisProfileSP profile)
{
	m_data = new Q_UINT8[colorStrategy->pixelSize()];
	memcpy(m_data, data, colorStrategy->pixelSize());
	m_colorStrategy = colorStrategy;
	m_profile = profile;
}


KisColor::KisColor(KisColor &src, KisStrategyColorSpaceSP colorStrategy, KisProfileSP profile)
{
	m_data = new Q_UINT8[colorStrategy->pixelSize()];
	// XXX: We shouldn't use KisPixel as an intermediary.
	// XXX: the position of the alpha channel is wrong, of course, but that doesn't hurt for the
	//      conversion and it's too costly to determine at the moment.
	KisPixel srcPixel = KisPixel(src.data(), src.data(), src.colorStrategy(), src.profile());
	KisPixel dstPixel = KisPixel(m_data, m_data, colorStrategy, profile);
	src.colorStrategy()->convertTo(srcPixel, dstPixel);
	
}
