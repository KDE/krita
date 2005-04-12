/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
#include <qstring.h>
#include <qptrlist.h>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>

#include "kis_types.h"
#include "kis_fileformat_registry.h"
#include "kis_fileformat.h"
#include "kis_paint_device.h"
#include "kis_filter.h"
#include "kis_view.h"

KisFileFormatRegistry *KisFileFormatRegistry::m_singleton = 0;

KisFileFormatRegistry::KisFileFormatRegistry()
{
 	kdDebug() << "Creating KisFileFormatRegistry" << endl;
  	Q_ASSERT(KisFileFormatRegistry::m_singleton == 0);
  	KisFileFormatRegistry::m_singleton = this;
}

KisFileFormatRegistry::~KisFileFormatRegistry()
{
}

KisFileFormatRegistry* KisFileFormatRegistry::instance()
{
  	if(KisFileFormatRegistry::m_singleton == 0)
  	{
  		KisFileFormatRegistry::m_singleton = new KisFileFormatRegistry();
		Q_CHECK_PTR(KisFileFormatRegistry::m_singleton);
  	}
  	return KisFileFormatRegistry::m_singleton;
}

