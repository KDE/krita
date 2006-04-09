/*
 * selection_tools.cc -- Part of Krita
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
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <kis_global.h>
#include <kis_types.h>
#include <kis_tool.h>
#include <kis_tool_registry.h>

#include "selection_tools.h"

#include "kis_tool_select_outline.h"
#include "kis_tool_select_polygonal.h"
#include "kis_tool_select_rectangular.h"
#include "kis_tool_select_contiguous.h"
#include "kis_tool_select_elliptical.h"
#include "kis_tool_select_eraser.h"
#include "kis_tool_select_brush.h"

typedef KGenericFactory<SelectionTools> SelectionToolsFactory;
K_EXPORT_COMPONENT_FACTORY( kritaselectiontools, SelectionToolsFactory( "krita" ) )


SelectionTools::SelectionTools(QObject *parent, const char *name, const QStringList &)
    : KParts::Plugin(parent)
{
    setObjectName(name);
    setInstance(SelectionToolsFactory::instance());

    if ( parent->inherits("KisToolRegistry") )
    {
        KisToolRegistry * r = dynamic_cast<KisToolRegistry*>(parent);
        r->add(KisToolFactorySP(new KisToolSelectOutlineFactory()));
        r->add(KisToolFactorySP(new KisToolSelectPolygonalFactory()));
        r->add(KisToolFactorySP(new KisToolSelectRectangularFactory()));
        r->add(KisToolFactorySP(new KisToolSelectBrushFactory()));
        r->add(KisToolFactorySP(new KisToolSelectContiguousFactory()));
        r->add(KisToolFactorySP(new KisToolSelectEllipticalFactory()));
        r->add(KisToolFactorySP(new KisToolSelectEraserFactory()));
    }
}

SelectionTools::~SelectionTools()
{
}

#include "selection_tools.moc"
