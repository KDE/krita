/*
 * tool_crop.cc -- Part of Krita
 *
 * SPDX-FileCopyrightText: 2004 Boudewijn Rempt (boud@valdyas.org)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tool_crop.h"
#include "kis_tool_crop.h"

#include <stdlib.h>
#include <vector>

#include <QPoint>

#include <klocalizedstring.h>
#include <ksharedconfig.h>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include <kis_global.h>
#include <kis_types.h>
#include <KoToolRegistry.h>

K_PLUGIN_FACTORY_WITH_JSON(ToolCropFactory, "kritatoolcrop.json", registerPlugin<ToolCrop>();)


ToolCrop::ToolCrop(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoToolRegistry::instance()->add(new KisToolCropFactory());
}

ToolCrop::~ToolCrop()
{
}

#include "tool_crop.moc"
