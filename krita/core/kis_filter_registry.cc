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

#include "kis_filter_registry.h"
#include "kis_paint_device.h"
#include "kis_filter.h"

KisFilterRegistry *KisFilterRegistry::m_singleton = 0;

KisFilterRegistry::KisFilterRegistry()
{
	kdDebug() << " creating a KisFilterRegistry" << endl;
	Q_ASSERT(KisFilterRegistry::m_singleton == 0);
	KisFilterRegistry::m_singleton = this;
}

KisFilterRegistry::~KisFilterRegistry()
{
}

KisFilterRegistry* KisFilterRegistry::singleton()
{
	if(KisFilterRegistry::m_singleton == 0)
	{
		KisFilterRegistry::m_singleton = new KisFilterRegistry();
	}
	return KisFilterRegistry::m_singleton;
}
