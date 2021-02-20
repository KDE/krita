/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tool_lazybrush.h"
#include <QStringList>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include <kis_tool.h>
#include <KoToolRegistry.h>

#include "kis_paint_device.h"
#include "kis_tool_lazy_brush.h"


K_PLUGIN_FACTORY_WITH_JSON(DefaultToolsFactory, "kritatoollazybrush.json", registerPlugin<ToolLazyBrush>();)


ToolLazyBrush::ToolLazyBrush(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KoToolRegistry::instance()->add(new KisToolLazyBrushFactory());
}

ToolLazyBrush::~ToolLazyBrush()
{
}

#include "tool_lazybrush.moc"
