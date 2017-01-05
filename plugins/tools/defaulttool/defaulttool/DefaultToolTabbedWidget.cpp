/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#include "DefaultToolTabbedWidget.h"

#include <QLabel>
#include "kis_icon_utils.h"
#include "DefaultToolGeometryWidget.h"
#include "KoStrokeConfigWidget.h"
#include "KoFillConfigWidget.h"
#include <KoInteractionTool.h>


DefaultToolTabbedWidget::DefaultToolTabbedWidget(KoInteractionTool *tool, QWidget *parent)
    : KoTitledTabWidget(parent)
{
    setObjectName("default-tool-tabbed-widget");

    DefaultToolGeometryWidget *geometryWidget = new DefaultToolGeometryWidget(tool, this);
    geometryWidget->setWindowTitle(i18n("Geometry"));
    addTab(geometryWidget, KisIconUtils::loadIcon("krita_tool_polygon"), QString());

    KoStrokeConfigWidget *strokeWidget = new KoStrokeConfigWidget(this);
    strokeWidget->setWindowTitle(i18n("Stroke"));
    strokeWidget->setCanvas(tool->canvas());
    addTab(strokeWidget, KisIconUtils::loadIcon("krita_tool_line"), QString());

    KoFillConfigWidget *fillWidget = new KoFillConfigWidget(this);
    fillWidget->setWindowTitle(i18n("Fill"));
    fillWidget->setCanvas(tool->canvas());
    addTab(fillWidget, KisIconUtils::loadIcon("krita_tool_color_fill"), QString());
}

DefaultToolTabbedWidget::~DefaultToolTabbedWidget()
{
}
