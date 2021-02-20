/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <kpluginfactory.h>
#include <generator/kis_generator_registry.h>

#include "KisGradientGenerator.h"
#include "KisGradientGeneratorPlugin.h"

K_PLUGIN_FACTORY_WITH_JSON(KritaGradientGeneratorFactory, "KritaGradientGenerator.json", registerPlugin<KisGradientGeneratorPlugin>();)

KisGradientGeneratorPlugin::KisGradientGeneratorPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisGeneratorRegistry::instance()->add(new KisGradientGenerator());
}

KisGradientGeneratorPlugin::~KisGradientGeneratorPlugin()
{
}

#include "KisGradientGeneratorPlugin.moc"
