/*
 * tool_transform.cc -- Part of Krita
 *
 * SPDX-FileCopyrightText: 2004 Boudewijn Rempt (boud@valdyas.org)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tool_transform.h"

#include <klocalizedstring.h>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include <kis_global.h>
#include <kis_types.h>
#include <KoToolRegistry.h>


#include "kis_tool_transform.h"

K_PLUGIN_FACTORY_WITH_JSON(ToolTransformFactory, "kritatooltransform.json", registerPlugin<ToolTransform>();)


ToolTransform::ToolTransform(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KoToolRegistry::instance()->add(new KisToolTransformFactory());
}

ToolTransform::~ToolTransform()
{
}

#include "tool_transform.moc"
