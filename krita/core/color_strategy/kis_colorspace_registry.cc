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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "kdebug.h"

#include "kis_types.h"
#include "kis_colorspace_registry.h"
#include "kis_colorspace_alpha.h"

KisColorSpaceRegistry *KisColorSpaceRegistry::m_singleton = 0;

KisColorSpaceRegistry::KisColorSpaceRegistry()
{
	Q_ASSERT(KisColorSpaceRegistry::m_singleton == 0);
	KisColorSpaceRegistry::m_singleton = this;
}

KisColorSpaceRegistry::~KisColorSpaceRegistry()
{
}

KisColorSpaceRegistry* KisColorSpaceRegistry::instance()
{
	if(KisColorSpaceRegistry::m_singleton == 0)
	{
		KisColorSpaceRegistry::m_singleton = new KisColorSpaceRegistry();
		Q_CHECK_PTR(KisColorSpaceRegistry::m_singleton);
	}
	return KisColorSpaceRegistry::m_singleton;
}

KisProfileSP KisColorSpaceRegistry::getProfileByName(const QString & name) const
{
	KisProfileSP profile = 0;
	KisIDList keys = listKeys();
	for ( KisIDList::Iterator it = keys.begin(); it != keys.end(); ++it ) {
		profile = get(*it) -> getProfileByName(name);
		if (profile != 0) return profile;
	}
	return profile;
}
