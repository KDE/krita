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

#ifndef KIS_FILEFORMAT_REGISTRY_H_
#define KIS_FILEFORMAT_REGISTRY_H_

#include "qptrlist.h"

#include "ksharedptr.h"
#include "kis_types.h"
#include "kis_generic_registry.h"
#include "kaction.h"
#include "kis_fileformat.h"

class QString;
class KisView;
class QStringList;

/**
 * Store a registry of all fileformat handlers Krita can use to create KisImages
 * and KisPaintDevices.
 */
class KisFileFormatRegistry : public KisGenericRegistry<KisFileFormatSP>,  public KShared
{

public:

	KisFileFormatRegistry();
	virtual ~KisFileFormatRegistry();

  	static KisFileFormatRegistry* instance();


private:

 	KisFileFormatRegistry(const KisFileFormatRegistry&);
 	KisFileFormatRegistry operator=(const KisFileFormatRegistry&);

private:
  	static KisFileFormatRegistry *m_singleton;
};

#endif // KIS_FILEFORMAT_REGISTRY_H_

