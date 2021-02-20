/*
 *  SPDX-FileCopyrightText: 2010-2011 José Luis Vergara <pentalis@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_phong_bumpmap_plugin.h"
#include "kis_phong_bumpmap_filter.h"

#include <kpluginfactory.h>
#include <filter/kis_filter_registry.h>

K_PLUGIN_FACTORY_WITH_JSON(KisPhongBumpmapFactory, "kritaphongbumpmapfilter.json", registerPlugin<KisPhongBumpmapPlugin>();)

KisPhongBumpmapPlugin::KisPhongBumpmapPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisFilterRegistry::instance()->add(new KisFilterPhongBumpmap());
}

KisPhongBumpmapPlugin::~KisPhongBumpmapPlugin()
{
}

#include "kis_phong_bumpmap_plugin.moc"
