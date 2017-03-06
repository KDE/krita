/*
 *  Copyright (c) 2017 Eugene Ingerman
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
