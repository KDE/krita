/* This file is part of the KDE project
   Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <kgenericfactory.h>
#include "kis_raindrops_filter_plugin.h"
#include "kis_raindrops_filter.h"

typedef KGenericFactory<KisRainDropsFilterPlugin> KisRainDropsFilterPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kritaraindropsfilter, KisRainDropsFilterPluginFactory( "krita" ) )

KisRainDropsFilterPlugin::KisRainDropsFilterPlugin(QObject *parent, const char *name, const QStringList &) : KParts::Plugin(parent, name)
{
        setInstance(KisRainDropsFilterPluginFactory::instance());

        kdDebug() << "RainDropFilter plugin. Class: " 
                << className() 
                << ", Parent: " 
                << parent -> className()
                << "\n";
        KisView * view;

        if ( !parent->inherits("KisView") )
        {
                return;
        } else {
                view = (KisView*) parent;
        }

        KisFilterSP krdf = createFilter<KisRainDropsFilter>(view);
	(void) new KAction("&Raindrops...", 0, 0, krdf, SLOT(slotActivated()), actionCollection(), "raindrops_filter");
}

KisRainDropsFilterPlugin::~KisRainDropsFilterPlugin()
{
}
