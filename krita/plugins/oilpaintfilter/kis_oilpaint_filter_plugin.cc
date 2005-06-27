/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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
#include "kis_oilpaint_filter_plugin.h"
#include "kis_oilpaint_filter.h"

typedef KGenericFactory<KisOilPaintFilterPlugin> KisOilPaintFilterPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kritaoilpaintfilter, KisOilPaintFilterPluginFactory( "krita" ) )

KisOilPaintFilterPlugin::KisOilPaintFilterPlugin(QObject *parent, const char *name, const QStringList &) : KParts::Plugin(parent, name)
{
        setInstance(KisOilPaintFilterPluginFactory::instance());

        kdDebug(DBG_AREA_PLUGINS) << "OilPaintFilter plugin. Class: "
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

        KisFilterSP krdf = createFilter<KisOilPaintFilter>(view);
	(void) new KAction(i18n("&Oilpaint..."), 0, 0, krdf, SLOT(slotActivated()), actionCollection(), "oilpaint_filter");
}

KisOilPaintFilterPlugin::~KisOilPaintFilterPlugin()
{
}

