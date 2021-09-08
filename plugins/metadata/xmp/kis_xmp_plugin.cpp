/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_xmp_plugin.h"

#include <kpluginfactory.h>

#include <kis_meta_data_backend_registry.h>

#include "kis_xmp_io.h"

K_PLUGIN_FACTORY_WITH_JSON(KisIptcIOPluginFactory, "kritaxmp.json", registerPlugin<KisXmpPlugin>();)

KisXmpPlugin::KisXmpPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisMetadataBackendRegistry::instance()->add(new KisXMPIO());
}

KisXmpPlugin::~KisXmpPlugin()
{
}

#include "kis_xmp_plugin.moc"
