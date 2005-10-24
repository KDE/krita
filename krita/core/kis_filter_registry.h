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

#ifndef KIS_FILTER_REGISTRY_H_
#define KIS_FILTER_REGISTRY_H_

#include "qptrlist.h"

#include "ksharedptr.h"
#include "kis_types.h"
#include "kis_generic_registry.h"
#include "kaction.h"

class QString;
class KisView;
class QStringList;

class KisFilterRegistry : public KisGenericRegistry<KisFilterSP>,  public KShared
{

public:

	KisFilterRegistry();
	virtual ~KisFilterRegistry();

//  	static KisFilterRegistry* instance();


private:

 	KisFilterRegistry(const KisFilterRegistry&);
 	KisFilterRegistry operator=(const KisFilterRegistry&);

// private:
//  	static KisFilterRegistry *m_singleton;
};

#endif // KIS_FILTERSPACE_REGISTRY_H_

