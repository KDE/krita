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

#ifndef KIS_COLORSPACE_REGISTRY_H_
#define KIS_COLORSPACE_REGISTRY_H_

#include "kis_types.h"
#include "kis_generic_registry.h"
#include "koffice_export.h"

class QStringList;


/**
 * This class will contain:
 *  	- a registry of singleton color strategies
 *	- a mapping of ImageMagick color model identifiers to color strategies
 *      - a mapping of lcms color model identifiers to color strategies
 *     -  a mapping of icm color model identifiers to color stratiegs
 */
class KRITACORE_EXPORT KisColorSpaceRegistry : public KisGenericRegistry<KisStrategyColorSpaceSP> {

public:
	virtual ~KisColorSpaceRegistry();

	static KisColorSpaceRegistry* instance();

	KisProfileSP getProfileByName(const QString & name) const;
	
private:
	KisColorSpaceRegistry();
	KisColorSpaceRegistry(const KisColorSpaceRegistry&);
	KisColorSpaceRegistry operator=(const KisColorSpaceRegistry&);

private:
	static KisColorSpaceRegistry *m_singleton;

};

#endif // KIS_COLORSPACE_REGISTRY_H_

