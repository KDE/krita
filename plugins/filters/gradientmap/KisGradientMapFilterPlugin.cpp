/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2016 Spencer Brown <sbrown655@gmail.com>
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 * 
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <kpluginfactory.h>
#include <kis_filter_registry.h>

#include "KisGradientMapFilter.h"
#include "KisGradientMapFilterPlugin.h"

K_PLUGIN_FACTORY_WITH_JSON(KritaGradientMapFilterFactory, "KritaGradientMapFilter.json", registerPlugin<KisGradientMapFilterPlugin>();)

KisGradientMapFilterPlugin::KisGradientMapFilterPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisFilterRegistry::instance()->add(new KisGradientMapFilter());
}

KisGradientMapFilterPlugin::~KisGradientMapFilterPlugin()
{}

#include "KisGradientMapFilterPlugin.moc"
