/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "filterop.h"
#include <klocalizedstring.h>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include <KoCompositeOpRegistry.h>

#include <brushengine/kis_paintop_registry.h>
#include "kis_simple_paintop_factory.h"
#include "kis_filterop.h"
#include "kis_filterop_settings.h"
#include "kis_filterop_settings_widget.h"

K_PLUGIN_FACTORY_WITH_JSON(FilterOpFactory, "kritafilterop.json", registerPlugin<FilterOp>();)

FilterOp::FilterOp(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    QStringList whiteList;
    whiteList << COMPOSITE_COPY;

    // This is not a gui plugin; only load it when the doc is created.
    KisPaintOpRegistry *r = KisPaintOpRegistry::instance();
    r->add(new KisSimplePaintOpFactory<KisFilterOp, KisFilterOpSettings, KisFilterOpSettingsWidget>("filter", i18nc("type of a brush engine, shown in the list of brush engines", "Filter"), KisPaintOpFactory::categoryStable(), "krita-filterop.png", QString(), whiteList, 17));

}

FilterOp::~FilterOp()
{
}

#include "filterop.moc"

