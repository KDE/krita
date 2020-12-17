/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_emboss_filter_plugin.h"

#include <kpluginfactory.h>

#include "kis_emboss_filter.h"
#include "kis_global.h"
#include "filter/kis_filter_registry.h"

K_PLUGIN_FACTORY_WITH_JSON(KisEmbossFilterPluginFactory, "kritaembossfilter.json", registerPlugin<KisEmbossFilterPlugin>();)

KisEmbossFilterPlugin::KisEmbossFilterPlugin(QObject *parent, const QVariantList &) : QObject(parent)
{
    KisFilterRegistry::instance()->add(new KisEmbossFilter());

}

KisEmbossFilterPlugin::~KisEmbossFilterPlugin()
{
}

#include "kis_emboss_filter_plugin.moc"
