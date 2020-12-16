/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2005 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_pixelize_filter_plugin.h"


#include <kpluginfactory.h>

#include <kis_paint_device.h>
#include <kis_global.h>
#include <filter/kis_filter_registry.h>

#include "kis_pixelize_filter.h"

K_PLUGIN_FACTORY_WITH_JSON(KisPixelizeFilterPluginFactory, "kritapixelizefilter.json", registerPlugin<KisPixelizeFilterPlugin>();)

KisPixelizeFilterPlugin::KisPixelizeFilterPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisFilterRegistry::instance()->add(new KisPixelizeFilter());
}

KisPixelizeFilterPlugin::~KisPixelizeFilterPlugin()
{
}

#include "kis_pixelize_filter_plugin.moc"


