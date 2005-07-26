/*
 * tool_filter.cc -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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
#include <stdlib.h>
#include <vector>

#include <qpoint.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <kis_global.h>
#include <kis_types.h>
#include <kis_tool_registry.h>
#include <kis_paintop_registry.h>
#include <kis_factory.h>
#include <kis_factory.h>

#include "tool_filter.h"
#include "kis_filterop.h"
#include "kis_tool_filter.h"


typedef KGenericFactory<ToolFilter> ToolFilterFactory;
K_EXPORT_COMPONENT_FACTORY( kritatoolfilter, ToolFilterFactory( "krita" ) )


ToolFilter::ToolFilter(QObject *parent, const char *name, const QStringList &)
	: KParts::Plugin(parent, name)
{
       	setInstance(ToolFilterFactory::instance());

 	kdDebug(DBG_AREA_PLUGINS) << "Filter tool plugin. Class: "
 		  << className()
 		  << ", Parent: "
 		  << parent -> className()
 		  << "\n";

	if ( parent -> inherits( "KisFactory" ) ) {

		KisPaintOpRegistry * pr = KisPaintOpRegistry::instance();
		pr -> add( new KisFilterOpFactory );

		KisToolRegistry * tr = KisToolRegistry::instance();
		tr -> add( new KisToolFilterFactory( actionCollection() ) );

 	}
}

ToolFilter::~ToolFilter()
{
}

#include "tool_filter.moc"
