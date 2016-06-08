/*
 * default_tools.cc -- Part of Krita
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
#include "kis_tool_colorpicker.h"
#include "kis_tool_line.h"
#include "kis_tool_ellipse.h"
#include "kis_tool_measure.h"
#include "kis_tool_path.h"
#include "kis_tool_move.h"
#include "kis_tool_pencil.h"

K_PLUGIN_FACTORY_WITH_JSON(DefaultToolsFactory, "kritadefaulttools.json", registerPlugin<DefaultTools>();)


DefaultTools::DefaultTools(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KoToolRegistry::instance()->add(new KisToolFillFactory());
    KoToolRegistry::instance()->add(new KisToolGradientFactory());
    KoToolRegistry::instance()->add(new KisToolBrushFactory());
    KoToolRegistry::instance()->add(new KisToolColorPickerFactory());
    KoToolRegistry::instance()->add(new KisToolLineFactory());
    KoToolRegistry::instance()->add(new KisToolEllipseFactory());
    KoToolRegistry::instance()->add(new KisToolRectangleFactory());
    KoToolRegistry::instance()->add(new KisToolMeasureFactory());
    KoToolRegistry::instance()->add(new KisToolPathFactory());
    KoToolRegistry::instance()->add(new KisToolMoveFactory());
    KoToolRegistry::instance()->add(new KisToolMultiBrushFactory());
    KoToolRegistry::instance()->add(new KisToolPencilFactory());
}

DefaultTools::~DefaultTools()
{
}

#include "default_tools.moc"
