/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_oilpaint_filter_plugin.h"

#include <kpluginfactory.h>

#include "kis_oilpaint_filter.h"
#include "kis_global.h"
#include "filter/kis_filter_registry.h"

K_PLUGIN_FACTORY_WITH_JSON(KisOilPaintFilterPluginFactory, "kritaoilpaintfilter.json", registerPlugin<KisOilPaintFilterPlugin>();)

KisOilPaintFilterPlugin::KisOilPaintFilterPlugin(QObject *parent, const QVariantList &) : QObject(parent)
{
    KisFilterRegistry::instance()->add(new KisOilPaintFilter());

}

KisOilPaintFilterPlugin::~KisOilPaintFilterPlugin()
{
}

#include "kis_oilpaint_filter_plugin.moc"
