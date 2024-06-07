/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <kpluginfactory.h>

#include <filter/kis_filter_registry.h>

#include "KisPropagateColorsFilter.h"
#include "KisPropagateColorsFilterPlugin.h"

K_PLUGIN_FACTORY_WITH_JSON(PropagateColorsFilterFactory, "kritapropagatecolorsfilter.json", registerPlugin<KisPropagateColorsFilterPlugin>();)

KisPropagateColorsFilterPlugin::KisPropagateColorsFilterPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisFilterRegistry::instance()->add(new KisPropagateColorsFilter());
}

KisPropagateColorsFilterPlugin::~KisPropagateColorsFilterPlugin()
{}

#include "KisPropagateColorsFilterPlugin.moc"
