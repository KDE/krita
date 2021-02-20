/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "unsharp.h"
#include <kpluginfactory.h>

#include "kis_unsharp_filter.h"

#include <filter/kis_filter_registry.h>

K_PLUGIN_FACTORY_WITH_JSON(UnsharpPluginFactory, "kritaunsharpfilter.json", registerPlugin<UnsharpPlugin>();)

UnsharpPlugin::UnsharpPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisFilterRegistry::instance()->add(new KisUnsharpFilter());

}

UnsharpPlugin::~UnsharpPlugin()
{
}

#include "unsharp.moc"
