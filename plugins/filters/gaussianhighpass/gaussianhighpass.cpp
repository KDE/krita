/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2019 Miguel Lopez <reptillia39@live.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "gaussianhighpass.h"
#include <kpluginfactory.h>

#include "gaussianhighpass_filter.h"

#include <filter/kis_filter_registry.h>

K_PLUGIN_FACTORY_WITH_JSON(GaussianHighPassPluginFactory, "kritagaussianhighpassfilter.json", registerPlugin<GaussianHighPassPlugin>();)

GaussianHighPassPlugin::GaussianHighPassPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisFilterRegistry::instance()->add(new KisGaussianHighPassFilter());

}

GaussianHighPassPlugin::~GaussianHighPassPlugin()
{
}

#include "gaussianhighpass.moc"
