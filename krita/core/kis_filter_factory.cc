/*
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#include "kis_filter_factory.h"
#include "kis_paint_device.h"
#include "kis_filter.h"

KisFilterFactory *KisFilterFactory::m_singleton = 0;

KisFilterFactory::KisFilterFactory()
{
	kdDebug() << " creating a KisFilterFactory" << endl;
	Q_ASSERT(KisFilterFactory::m_singleton == 0);
	KisFilterFactory::m_singleton = this;
}

KisFilterFactory::~KisFilterFactory()
{
}

KisFilterFactory* KisFilterFactory::singleton()
{
	if(KisFilterFactory::m_singleton == 0)
	{
		KisFilterFactory::m_singleton = new KisFilterFactory();
	}
	return KisFilterFactory::m_singleton;
}
