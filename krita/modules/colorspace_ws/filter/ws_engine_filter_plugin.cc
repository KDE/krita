/*
 *  Copyright (c) 2005 Boudewijn Rempt (boud@valdyas.org)
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

#include <kgenericfactory.h>

#include "kis_factory.h"
#include "kis_filter_registry.h"
#include "kis_filter.h"
#include "ws_engine_filter_plugin.h"
#include "kis_ws_engine_filter.h"

typedef KGenericFactory<WSEngineFilterPlugin> WSEngineFilterPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kritawsenginefilter, WSEngineFilterPluginFactory( "krita" ) )

WSEngineFilterPlugin::WSEngineFilterPlugin(QObject *parent, const char *name, const QStringList &) : KParts::Plugin(parent, name)
{
        setInstance(WSEngineFilterPluginFactory::instance());

        kdDebug() << "WS engine filter plugin. Class: "
                << className()
                << ", Parent: "
                << parent -> className()
                << "\n";

        if ( parent->inherits("KisView") )
        {
		KisView * view = dynamic_cast<KisView*>( parent );
		KisFilterSP kef = createFilter<KisWSEngineFilter>(view);
		(void) new KAction("&Wet & Sticky paint engine...", 0, 0, kef, SLOT(slotActivated()), actionCollection(), "wsengine_filter");
	}
}

WSEngineFilterPlugin::~WSEngineFilterPlugin()
{
}
