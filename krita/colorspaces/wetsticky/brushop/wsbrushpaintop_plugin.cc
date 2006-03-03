/*
 * wsbrushpaintop_plugin.cc -- Part of Krita
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

#include <kis_paintop_registry.h>
#include <kis_debug_areas.h>
#include "kis_wsbrushop.h"

#include "wsbrushpaintop_plugin.h"

typedef KGenericFactory<WSBrushPaintOpPlugin> WSBrushPaintOpPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kritawsbrushpaintop, WSBrushPaintOpPluginFactory( "kritacore" ) )


WSBrushPaintOpPlugin::WSBrushPaintOpPlugin(QObject *parent, const char *name, const QStringList &)
    : KParts::Plugin(parent, name)
{
    setInstance(WSBrushPaintOpPluginFactory::instance());

    // This is not a gui plugin; only load it when the doc is created.
    if ( parent->inherits("KisFactory") )
    {
        KisPaintOpRegistry::instance() -> add ( new KisWSBrushOpFactory );
    }

}

WSBrushPaintOpPlugin::~WSBrushPaintOpPlugin()
{
}

#include "wsbrushpaintop_plugin.moc"
