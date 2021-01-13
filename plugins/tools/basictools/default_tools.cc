/*
 * default_tools.cc -- Part of Krita
 *
 * SPDX-FileCopyrightText: 2004 Boudewijn Rempt (boud@valdyas.org)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "default_tools.h"
#include <QStringList>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include <kis_tool.h>
#include <KoToolRegistry.h>

#include "kis_paint_device.h"
#include "kis_tool_fill.h"
#include "kis_tool_brush.h"
#include "kis_tool_multihand.h"
#include "kis_tool_freehand.h"
#include "kis_tool_gradient.h"
#include "kis_tool_rectangle.h"
#include "kis_tool_colorsampler.h"
#include "kis_tool_line.h"
#include "kis_tool_ellipse.h"
#include "kis_tool_measure.h"
#include "kis_tool_path.h"
#include "kis_tool_move.h"
#include "kis_tool_pencil.h"
#include "kis_tool_pan.h"

K_PLUGIN_FACTORY_WITH_JSON(DefaultToolsFactory, "kritadefaulttools.json", registerPlugin<DefaultTools>();)


DefaultTools::DefaultTools(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KoToolRegistry::instance()->add(new KisToolFillFactory());
    KoToolRegistry::instance()->add(new KisToolGradientFactory());
    KoToolRegistry::instance()->add(new KisToolBrushFactory());
    KoToolRegistry::instance()->add(new KisToolColorSamplerFactory());
    KoToolRegistry::instance()->add(new KisToolLineFactory());
    KoToolRegistry::instance()->add(new KisToolEllipseFactory());
    KoToolRegistry::instance()->add(new KisToolRectangleFactory());
    KoToolRegistry::instance()->add(new KisToolMeasureFactory());
    KoToolRegistry::instance()->add(new KisToolPathFactory());
    KoToolRegistry::instance()->add(new KisToolMoveFactory());
    KoToolRegistry::instance()->add(new KisToolMultiBrushFactory());
    KoToolRegistry::instance()->add(new KisToolPencilFactory());
    KoToolRegistry::instance()->add(new KisToolPanFactory());
}

DefaultTools::~DefaultTools()
{
}

#include "default_tools.moc"
