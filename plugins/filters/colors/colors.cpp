/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "colors.h"
#include <kpluginfactory.h>

#include "kis_minmax_filters.h"
#include "kis_color_to_alpha.h"
#include <filter/kis_filter_registry.h>

K_PLUGIN_FACTORY_WITH_JSON(KritaExtensionsColorsFactory, "kritaextensioncolorsfilters.json", registerPlugin<KritaExtensionsColors>();)

KritaExtensionsColors::KritaExtensionsColors(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisFilterRegistry * manager = KisFilterRegistry::instance();
    manager->add(new KisFilterMax());
    manager->add(new KisFilterMin());
    manager->add(new KisFilterColorToAlpha());

}

KritaExtensionsColors::~KritaExtensionsColors()
{
}

#include "colors.moc"
