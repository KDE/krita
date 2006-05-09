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

#include <kis_debug_areas.h>
#include <kis_types.h>
#include <kis_tool_registry.h>
#include <kis_paintop_registry.h>

#include "tool_filter.h"
#include "kis_filterop.h"
#include "kis_tool_filter.h"


typedef KGenericFactory<ToolFilter> ToolFilterFactory;
K_EXPORT_COMPONENT_FACTORY( kritatoolfilter, ToolFilterFactory( "krita" ) )


ToolFilter::ToolFilter(QObject *parent, const QStringList &)
    : KParts::Plugin(parent)
{
    setInstance(ToolFilterFactory::instance());

    if ( parent->inherits("KisToolRegistry") )
    {
        KisToolRegistry * r = dynamic_cast<KisToolRegistry*>(parent);
        r->add(KisToolFactorySP(new KisToolFilterFactory()));

        // XXX: Put this in a separate plugin?
        KisPaintOpRegistry * pr = KisPaintOpRegistry::instance();
        pr->add(KisPaintOpFactorySP(new KisFilterOpFactory));

     }
}

ToolFilter::~ToolFilter()
{
}

#include "tool_filter.moc"
