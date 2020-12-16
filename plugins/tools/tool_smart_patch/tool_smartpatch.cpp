/*
 *  SPDX-FileCopyrightText: 2017 Eugene Ingerman
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tool_smartpatch.h"
#include <QStringList>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include <kis_tool.h>
#include <KoToolRegistry.h>

#include "kis_paint_device.h"
#include "kis_tool_smart_patch.h"


K_PLUGIN_FACTORY_WITH_JSON(DefaultToolsFactory, "kritatoolsmartpatch.json", registerPlugin<ToolSmartPatch>();)


ToolSmartPatch::ToolSmartPatch(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoToolRegistry::instance()->add(new KisToolSmartPatchFactory());
}

ToolSmartPatch::~ToolSmartPatch()
{
}

#include "tool_smartpatch.moc"
