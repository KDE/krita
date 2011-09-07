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

#include "selection_tools.h"
#include <klocale.h>
#include <kcomponentdata.h>
#include <kis_debug.h>
#include <kpluginfactory.h>

#include "KoToolRegistry.h"

#include "kis_global.h"
#include "kis_types.h"

#include "kis_tool_select_outline.h"
#include "kis_tool_select_polygonal.h"
#include "kis_tool_select_rectangular.h"
#include "kis_tool_select_contiguous.h"
#include "kis_tool_select_elliptical.h"
#include "kis_tool_select_path.h"
#include "kis_tool_select_similar.h"
#include "kis_tool_select_brush.h"
//#include "kis_tool_select_magnetic.h"

K_PLUGIN_FACTORY(SelectionToolsFactory, registerPlugin<SelectionTools>();)
K_EXPORT_PLUGIN(SelectionToolsFactory("krita"))


SelectionTools::SelectionTools(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KoToolRegistry * r = KoToolRegistry::instance();

    r->add(new KisToolSelectOutlineFactory(QStringList()));
    r->add(new KisToolSelectPolygonalFactory(QStringList()));
    r->add(new KisToolSelectRectangularFactory(QStringList()));
    r->add(new KisToolSelectEllipticalFactory(QStringList()));
    r->add(new KisToolSelectContiguousFactory(QStringList()));
    r->add(new KisToolSelectPathFactory(QStringList()));
    r->add(new KisToolSelectSimilarFactory(QStringList()));
    r->add(new KisToolSelectBrushFactory(QStringList()));
//    r->add(new KisToolSelectMagneticFactory(QStringList()));
}

SelectionTools::~SelectionTools()
{
}

#include "selection_tools.moc"
