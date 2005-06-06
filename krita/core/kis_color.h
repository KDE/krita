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
#ifndef _KIS_COLOR_H_
#define _KIS_COLOR_H_

#include "ksharedptr.h"

#include "kis_global.h"
#include "kis_types.h"
#include "kis_profile.h"
#include "kis_strategy_colorspace.h"

class QColor;

/**
 * A KisColor describes a color in a certain colorspace.
 *
 */
class KisColor {

public:


	// Create a KisColor using a native color strategy. The data is copied.
	KisColor(Q_UINT8 * data, KisStrategyColorSpaceSP colorStrategy = 0, KisProfileSP profile = 0);

	// Create a KisColor by converting src into another colorspace
	KisColor(KisColor &src, KisStrategyColorSpaceSP colorStrategy = 0, KisProfileSP profile = 0);

	virtual ~KisColor() { delete [] m_data; }

	Q_UINT8 * data() { return m_data; }
	KisStrategyColorSpaceSP colorStrategy() { return m_colorStrategy; }
	KisProfileSP profile() { return m_profile; }

private:

	Q_UINT8 * m_data;
	
	KisStrategyColorSpaceSP m_colorStrategy;
	KisProfileSP m_profile;
};

#endif
