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
#include "kis_tool_freehand.h"
#include "kis_tool_gradient.h"
#include "kis_tool_rectangle.h"
#include "kis_tool_colorpicker.h"
#include "kis_tool_line.h"
#include "kis_tool_ellipse.h"
#include "kis_tool_measure.h"
#include "kis_tool_path.h"
#include "kis_tool_move.h"

K_PLUGIN_FACTORY(DefaultToolsFactory, registerPlugin<DefaultTools>();)
K_EXPORT_PLUGIN(DefaultToolsFactory("krita"))


DefaultTools::DefaultTools(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KoToolRegistry * r = KoToolRegistry::instance();

    r->add(new KisToolFillFactory(r, QStringList()));
    r->add(new KisToolGradientFactory(r, QStringList()));
    r->add(new KisToolBrushFactory(r, QStringList()));
    r->add(new KisToolColorPickerFactory(r, QStringList()));
    r->add(new KisToolLineFactory(r, QStringList()));
    r->add(new KisToolEllipseFactory(r, QStringList()));
    r->add(new KisToolRectangleFactory(r, QStringList()));
    r->add(new KisToolMeasureFactory(r, QStringList()));
    r->add(new KisToolPathFactory(r, QStringList()));
    r->add(new KisToolMoveFactory(r, QStringList()));
}

DefaultTools::~DefaultTools()
{
}

#include "default_tools.moc"
