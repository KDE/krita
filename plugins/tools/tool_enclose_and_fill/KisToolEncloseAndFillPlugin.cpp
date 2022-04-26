/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2022 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <kpluginfactory.h>
#include <KoToolRegistry.h>

#include "KisToolEncloseAndFillPlugin.h"
#include "KisToolEncloseAndFillFactory.h"

K_PLUGIN_FACTORY_WITH_JSON(KisToolEncloseAndFillPluginFactory, "kritatoolencloseandfill.json", registerPlugin<KisToolEncloseAndFillPlugin>();)

KisToolEncloseAndFillPlugin::KisToolEncloseAndFillPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KoToolRegistry::instance()->add(new KisToolEncloseAndFillFactory());
}

KisToolEncloseAndFillPlugin::~KisToolEncloseAndFillPlugin()
{
}

#include "KisToolEncloseAndFillPlugin.moc"
