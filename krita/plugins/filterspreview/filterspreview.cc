/*
 * This file is part of Krita
 *
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#include "filterspreview.h"

#include <kdebug.h>

#include <kgenericfactory.h>

#include "kisfilterslistview.h"
#include "kis_view.h"
#include "kisdlgfilterspreview.h"

namespace Krita {
namespace Plugins {
namespace FiltersPreview {

typedef KGenericFactory<KritaFiltersPreview> KritaFiltersPreviewFactory;
K_EXPORT_COMPONENT_FACTORY( kritafilterspreview, KritaFiltersPreviewFactory( "krita" ) )

KritaFiltersPreview::KritaFiltersPreview(QObject *parent, const char *name, const QStringList &)
		: KParts::Plugin(parent, name)
{
	setInstance(KritaFiltersPreview::instance());
	
	kdDebug() << "FiltersPreview plugin. Class: "
		  << className()
		  << ", Parent: "
		  << parent -> className()
		  << "\n";

	if ( !parent->inherits("KisView") )
	{
		return;
	} else {
		m_view = (KisView*) parent;
	}

	(void) new KAction(i18n("&Preview filters"), 0, 0, this, SLOT(showFiltersPreviewDialog()), actionCollection(), "krita_filters_preview");
}

KritaFiltersPreview::~KritaFiltersPreview()
{
}

void KritaFiltersPreview::showFiltersPreviewDialog()
{
	KisDlgFiltersPreview dlg(m_view,m_view);
	dlg.exec();
}

};
};
};

