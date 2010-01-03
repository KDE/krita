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
#include <kgenericfactory.h>

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

typedef KGenericFactory<SelectionTools> SelectionToolsFactory;
K_EXPORT_COMPONENT_FACTORY(kritaselectiontools, SelectionToolsFactory("krita"))


SelectionTools::SelectionTools(QObject *parent, const QStringList &)
        : QObject(parent)
{
    //setComponentData(SelectionToolsFactory::componentData());

    KoToolRegistry * r = KoToolRegistry::instance();

    r->add(new KisToolSelectOutlineFactory(r, QStringList()));
    r->add(new KisToolSelectPolygonalFactory(r, QStringList()));
    r->add(new KisToolSelectRectangularFactory(r, QStringList()));
    r->add(new KisToolSelectEllipticalFactory(r, QStringList()));
    r->add(new KisToolSelectContiguousFactory(r, QStringList()));
    r->add(new KisToolSelectPathFactory(r, QStringList()));
    r->add(new KisToolSelectSimilarFactory(r, QStringList()));
}

SelectionTools::~SelectionTools()
{
}

#include "selection_tools.moc"
