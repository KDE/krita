/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2005 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_round_corners_filter_plugin.h"

#include <kpluginfactory.h>

#include "kis_round_corners_filter.h"
#include "kis_global.h"
#include "filter/kis_filter_registry.h"

K_PLUGIN_FACTORY_WITH_JSON(KisRoundCornersFilterPluginFactory, "kritaroundcornersfilter.json", registerPlugin<KisRoundCornersFilterPlugin>();)

KisRoundCornersFilterPlugin::KisRoundCornersFilterPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisFilterRegistry::instance()->add(new KisRoundCornersFilter());

}

KisRoundCornersFilterPlugin::~KisRoundCornersFilterPlugin()
{
}

#include "kis_round_corners_filter_plugin.moc"
