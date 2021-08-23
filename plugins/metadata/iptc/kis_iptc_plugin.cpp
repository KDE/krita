/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_iptc_plugin.h"

#include <kpluginfactory.h>

#include <kis_meta_data_backend_registry.h>

#include "kis_iptc_io.h"

K_PLUGIN_FACTORY_WITH_JSON(KisIptcIOPluginFactory, "kritaiptc.json", registerPlugin<KisIptcPlugin>();)

KisIptcPlugin::KisIptcPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisMetadataBackendRegistry::instance()->add(new KisIptcIO());
}

KisIptcPlugin::~KisIptcPlugin()
{
}

#include "kis_iptc_plugin.moc"
