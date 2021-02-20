/*
 * selection_tools.cc -- Part of Krita
 *
 * SPDX-FileCopyrightText: 2004 Boudewijn Rempt (boud@valdyas.org)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "selection_tools.h"
#include <klocalizedstring.h>

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
#include "KisToolSelectMagnetic.h"

K_PLUGIN_FACTORY_WITH_JSON(SelectionToolsFactory, "kritaselectiontools.json", registerPlugin<SelectionTools>();)


SelectionTools::SelectionTools(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KoToolRegistry::instance()->add(new KisToolSelectOutlineFactory());
    KoToolRegistry::instance()->add(new KisToolSelectPolygonalFactory());
    KoToolRegistry::instance()->add(new KisToolSelectRectangularFactory());
    KoToolRegistry::instance()->add(new KisToolSelectEllipticalFactory());
    KoToolRegistry::instance()->add(new KisToolSelectContiguousFactory());
    KoToolRegistry::instance()->add(new KisToolSelectPathFactory());
    KoToolRegistry::instance()->add(new KisToolSelectSimilarFactory());
    KoToolRegistry::instance()->add(new KisToolSelectMagneticFactory());
}

SelectionTools::~SelectionTools()
{
}

#include "selection_tools.moc"
