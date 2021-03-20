/*
 * tool_polygon.cc -- Part of Krita
 *
 * SPDX-FileCopyrightText: 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "tool_polygon.h"

#include <stdlib.h>
#include <vector>

#include <QPoint>

#include <klocalizedstring.h>

#include <kis_debug.h>
#include <kis_paint_device.h>
#include <kpluginfactory.h>

#include <kis_global.h>
#include <kis_types.h>
#include <KoToolRegistry.h>

#include "kis_tool_polygon.h"

K_PLUGIN_FACTORY_WITH_JSON(ToolPolygonFactory, "kritatoolpolygon.json", registerPlugin<ToolPolygon>();)


ToolPolygon::ToolPolygon(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KoToolRegistry::instance()->add(new KisToolPolygonFactory());
}

ToolPolygon::~ToolPolygon()
{
}

#include "tool_polygon.moc"
