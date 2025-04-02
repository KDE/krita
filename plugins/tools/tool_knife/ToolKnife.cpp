/*
 *  SPDX-FileCopyrightText: 2025 Agata Cacko
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "ToolKnife.h"

#include <kpluginfactory.h>

#include <kis_tool.h>
#include <KoToolRegistry.h>

#include "KisToolKnife.h"


K_PLUGIN_FACTORY_WITH_JSON(DefaultToolsFactory, "kritatoolknife.json", registerPlugin<ToolKnife>();)


ToolKnife::ToolKnife(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoToolRegistry::instance()->add(new KisToolKnifeFactory());
}

ToolKnife::~ToolKnife()
{
}

#include "ToolKnife.moc"
