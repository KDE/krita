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
#include "kdebug.h"

#include "kis_types.h"
#include "kis_colorspace_registry.h"
#include "kis_colorspace_alpha.h"

KisColorSpaceRegistry *KisColorSpaceRegistry::m_singleton = 0;

KisColorSpaceRegistry::KisColorSpaceRegistry()
{
	kdDebug() << " creating a KisColorSpaceRegistry" << endl;
	Q_ASSERT(KisColorSpaceRegistry::m_singleton == 0);
	KisColorSpaceRegistry::m_singleton = this;
	// Hack to add the alpha color space which isn't a module or 
	// selections wouldn't work. It adds itself.
	KisStrategyColorSpaceSP alpha = new KisColorSpaceAlpha();
	add(alpha);
}

KisColorSpaceRegistry::~KisColorSpaceRegistry()
{
}

KisColorSpaceRegistry* KisColorSpaceRegistry::singleton()
{
	if(KisColorSpaceRegistry::m_singleton == 0)
	{
		KisColorSpaceRegistry::m_singleton = new KisColorSpaceRegistry();
	}
	return KisColorSpaceRegistry::m_singleton;
}

KisStrategyColorSpaceSP KisColorSpaceRegistry::colorSpace(const QString& name) const
{
	return get(name);
}

QStringList KisColorSpaceRegistry::listColorSpaceNames() const
{
	return listKeys();
}
