/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "kis_meta_data_backend_registry.h"

#include <QGlobalStatic>

#include <KoPluginLoader.h>

#include <kis_debug.h>

Q_GLOBAL_STATIC(KisMetadataBackendRegistry, s_instance)

KisMetadataBackendRegistry::KisMetadataBackendRegistry()
{
}

KisMetadataBackendRegistry::~KisMetadataBackendRegistry()
{
    Q_FOREACH (const QString &id, keys()) {
        delete get(id);
    }
    dbgRegistry << "Deleting KisMetadataBackendRegistry";
}

void KisMetadataBackendRegistry::init()
{
    KoPluginLoader::instance()->load("Krita/Metadata", "(Type == 'Service') and ([X-Krita-Version] == 28)");
}

KisMetadataBackendRegistry *KisMetadataBackendRegistry::instance()
{
    if (!s_instance.exists()) {
        dbgRegistry << "initializing KisMetadataBackendRegistry";
        s_instance->init();
    }
    return s_instance;
}
