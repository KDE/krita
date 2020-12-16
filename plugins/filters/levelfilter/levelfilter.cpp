/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Frederic Coiffier <fcoiffie@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "levelfilter.h"

#include <kpluginfactory.h>

#include "kis_level_filter.h"
#include "filter/kis_filter_registry.h"

K_PLUGIN_FACTORY_WITH_JSON(LevelFilterFactory, "kritalevelfilter.json", registerPlugin<LevelFilter>();)

LevelFilter::LevelFilter(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisFilterRegistry::instance()->add(new KisLevelFilter());

}

LevelFilter::~LevelFilter()
{
}

#include "levelfilter.moc"
