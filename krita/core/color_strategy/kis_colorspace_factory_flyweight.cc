/*
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
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

#include <stdlib.h>
#include <kdebug.h>
#include "kis_colorspace_factory_flyweight.h"
#include "kis_paint_device.h"
#include "kis_strategy_colorspace_cmyk.h"
#include "kis_strategy_colorspace_rgb.h"
#include "kis_strategy_colorspace_grayscale.h"

namespace {
	KisColorSpaceFactoryFlyweight moveMe; // XXX Where should we create singletons in Krita?!?
}

KisColorSpaceFactoryFlyweight::KisColorSpaceFactoryFlyweight()
{
}

KisColorSpaceFactoryFlyweight::~KisColorSpaceFactoryFlyweight()
{
}

KisStrategyColorSpaceSP KisColorSpaceFactoryFlyweight::create(const KisPaintDeviceSP& device)
{
	KisStrategyColorSpaceSP p;

	if (device != 0)
		p = create(device -> type());

	return p;
}

KisStrategyColorSpaceSP KisColorSpaceFactoryFlyweight::create(enumImgType imgType)
{
	KisStrategyColorSpaceSP p = find(imgType);

	if (p)
		return p;

	switch (imgType) {
	case IMAGE_TYPE_GREYA:
	case IMAGE_TYPE_GREY:
		p = new KisStrategyColorSpaceGrayscale;
		break;
	case IMAGE_TYPE_RGB:
	case IMAGE_TYPE_RGBA:
		p = new KisStrategyColorSpaceRGB;
		break;
	case IMAGE_TYPE_CMYK:
	case IMAGE_TYPE_CMYKA:
		p = new KisStrategyColorSpaceCMYK;
		break;
	default:
		kdDebug() << "Color space strategy not implemented." << endl;
		abort();
		break;
	}

	m_flyweights[imgType] = p;
	return p;
}

KisStrategyColorSpaceSP KisColorSpaceFactoryFlyweight::find(enumImgType imgType) const
{
	KisStrategyColorSpaceSP p;
	acFlyweights_cit it = m_flyweights.find(imgType);

	if (it != m_flyweights.end()) {
		p = it -> second;
		Q_ASSERT(p);
	}

	return p;
}

