/*
 * wet_sticky_plugin.cc -- Part of Krita
 *
 * Copyright (c) 2005 Boudewijn Rempt (boud@valdyas.org)
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
#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <kis_colorspace_registry.h>
#include "wet_sticky_plugin.h"

#include "kis_wet_sticky_colorspace.h"
#include "kis_ws_engine_filter.h"

typedef KGenericFactory<WetStickyPlugin> WetStickyPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kritawsplugin, WetStickyPluginFactory( "kritacore" ) )


WetStickyPlugin::WetStickyPlugin(QObject *parent, const char *name, const QStringList &)
    : KParts::Plugin(parent, name)
{
           setInstance(WetStickyPluginFactory::instance());

    // This is not a gui plugin; only load it when the doc is created.
    if ( parent->inherits("KisFactory") )
    {
        KoColorSpace * colorSpaceWS = new KisWetStickyColorSpace();
        Q_CHECK_PTR(colorSpaceWS);
        KoColorSpaceRegistry::instance() -> add(colorSpaceWS);
        KisFilterRegistry::instance()->add(new KisWSEngineFilter());
    }

}

WetStickyPlugin::~WetStickyPlugin()
{
}

#include "wet_sticky_plugin.moc"
