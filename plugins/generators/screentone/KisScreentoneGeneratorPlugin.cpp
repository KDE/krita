/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <kpluginfactory.h>
#include <generator/kis_generator_registry.h>

#include "KisScreentoneGenerator.h"
#include "KisScreentoneGeneratorPlugin.h"

K_PLUGIN_FACTORY_WITH_JSON(KritaScreentoneGeneratorFactory, "KritaScreentoneGenerator.json", registerPlugin<KisScreentoneGeneratorPlugin>();)

KisScreentoneGeneratorPlugin::KisScreentoneGeneratorPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisGeneratorRegistry::instance()->add(new KisScreentoneGenerator());
}

KisScreentoneGeneratorPlugin::~KisScreentoneGeneratorPlugin()
{
}

#include "KisScreentoneGeneratorPlugin.moc"
