/*
 * defaultpaintops_plugin.cc -- Part of Krita
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
#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <kis_paintop_registry.h>

#include "kis_airbrushop.h"
#include "kis_brushop.h"
#include "kis_duplicateop.h"
#include "kis_eraseop.h"
#include "kis_penop.h"
#include "kis_global.h"
#include "kis_paintop_registry.h"

#include "defaultpaintops_plugin.h"

typedef KGenericFactory<DefaultPaintOpsPlugin> DefaultPaintOpsPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kritadefaultpaintops, DefaultPaintOpsPluginFactory( "kritacore" ) )


DefaultPaintOpsPlugin::DefaultPaintOpsPlugin(QObject *parent, const char *name, const QStringList &)
    : KParts::Plugin(parent, name)
{
     setInstance(DefaultPaintOpsPluginFactory::instance());

    // This is not a gui plugin; only load it when the doc is created.
    if ( parent->inherits("KisPaintOpRegistry") )
    {
        KisPaintOpRegistry * r = dynamic_cast<KisPaintOpRegistry*>(parent);
        // Add hard-coded paint ops. Plugin paintops will add
        // themselves in the plugin initialization code.
        r->add ( new KisAirbrushOpFactory );
        r->add ( new KisBrushOpFactory );
        r->add ( new KisDuplicateOpFactory );
        r->add ( new KisEraseOpFactory );
        r->add ( new KisPenOpFactory );
    }

}

DefaultPaintOpsPlugin::~DefaultPaintOpsPlugin()
{
}

#include "defaultpaintops_plugin.moc"
