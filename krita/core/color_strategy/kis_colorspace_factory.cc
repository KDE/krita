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

#include "kis_colorspace_factory.h"
#include "kis_paint_device.h"

KisColorSpaceFactory *KisColorSpaceFactory::m_singleton = 0;

KisColorSpaceFactory::KisColorSpaceFactory()
{
	Q_ASSERT(KisColorSpaceFactory::m_singleton == 0);
	KisColorSpaceFactory::m_singleton = this;
}

KisColorSpaceFactory::~KisColorSpaceFactory()
{
}

KisColorSpaceFactory *KisColorSpaceFactory::singleton()
{
// 	Q_ASSERT(KisColorSpaceFactoryInterface::m_singleton);
	if(KisColorSpaceFactory::m_singleton == 0)
	{
		KisColorSpaceFactory::m_singleton = new KisColorSpaceFactory();
	}
	return KisColorSpaceFactory::m_singleton;
}


KisStrategyColorSpaceSP KisColorSpaceFactory::create(const KisPaintDeviceSP& device)
{
	KisStrategyColorSpaceSP p;

	if (device != 0)
		p = create(device -> type());

	return p;
}

KisStrategyColorSpaceSP KisColorSpaceFactory::create(enumImgType imgType)
{

	switch (imgType) {
	case IMAGE_TYPE_GREYA:
	case IMAGE_TYPE_GREY:
		return colorSpace("Grayscale + Alpha");
		break;
	case IMAGE_TYPE_RGB:
	case IMAGE_TYPE_RGBA:
		return colorSpace("RGBA");
		break;
	default:
		kdDebug() << "Color space strategy not accessible by create." << endl;
		abort();
		break;
	}
	return 0;
}

void KisColorSpaceFactory::add(enumImgType, KisStrategyColorSpaceSP )
{
	kdDebug() << "KisColorSpaceFactoryFlyweight::add(enumImgType , KisStrategyColorSpaceSP ) is deprecated" << endl;
}

KisStrategyColorSpaceSP KisColorSpaceFactory::colorSpace(const QString& name) const
{
	KisStrategyColorSpaceSP p;
	acFlyweights_cit it = m_flyweights.find(name);

	if (it != m_flyweights.end()) {
		p = it -> second;
		Q_ASSERT(p);
	}

	return p;
}

void KisColorSpaceFactory::add(KisStrategyColorSpaceSP colorspace)
{
	kdDebug() << "add a new colorspace : " << colorspace->name() << endl;
	m_flyweights.insert(acFlyweights::value_type( colorspace->name(),colorspace));
}

