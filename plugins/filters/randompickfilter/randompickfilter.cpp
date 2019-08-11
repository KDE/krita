/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "randompickfilter.h"
#include <stdlib.h>
#include <vector>
#include <math.h>

#include <QPoint>

#include <kis_debug.h>

#include <kpluginfactory.h>
#include <klocalizedstring.h>

#include <KoUpdater.h>

#include <KoMixColorsOp.h>
#include <filter/kis_filter_registry.h>
#include <kis_global.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_paint_device.h>
#include <kis_random_accessor_ng.h>
#include <kis_random_generator.h>
#include <kis_selection.h>
#include <kis_types.h>
#include <filter/kis_filter_category_ids.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>

#include "kis_wdg_random_pick.h"
#include "ui_wdgrandompickoptions.h"
#include <kis_iterator_ng.h>
#include <KisSequentialIteratorProgress.h>

K_PLUGIN_FACTORY_WITH_JSON(KritaRandomPickFilterFactory, "kritarandompickfilter.json", registerPlugin<KritaRandomPickFilter>();)

KritaRandomPickFilter::KritaRandomPickFilter(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisFilterRegistry::instance()->add(new KisFilterRandomPick());
}

KritaRandomPickFilter::~KritaRandomPickFilter()
{
}

KisFilterRandomPick::KisFilterRandomPick() : KisFilter(id(), FiltersCategoryOtherId, i18n("&Random Pick..."))
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setSupportsPainting(true);
}


void KisFilterRandomPick::processImpl(KisPaintDeviceSP device,
                                      const QRect& applyRect,
                                      const KisFilterConfigurationSP config,
                                      KoUpdater* progressUpdater
                                      ) const
{
    Q_UNUSED(config);
    Q_ASSERT(!device.isNull());

    const KoColorSpace * cs = device->colorSpace();

    QVariant value;
    int level = (config && config->getProperty("level", value)) ? value.toInt() : 50;
    int opacity = (config && config->getProperty("opacity", value)) ? value.toInt() : 100;
    double windowsize = (config && config->getProperty("windowsize", value)) ? value.toDouble() : 2.5;

    int seedThreshold = rand();
    int seedH = rand();
    int seedV = rand();

    if (config) {
        seedThreshold = config->getInt("seedThreshold", seedThreshold);
        seedH = config->getInt("seedH", seedH);
        seedV = config->getInt("seedV", seedV);
    }
    KisRandomGenerator randT(seedThreshold);
    KisRandomGenerator randH(seedH);
    KisRandomGenerator randV(seedV);

    KisSequentialIteratorProgress dstIt(device, applyRect, progressUpdater);
    KisRandomConstAccessorSP srcRA = device->createRandomConstAccessorNG(0, 0);

    double threshold = (100 - level) / 100.0;

    qint16 weights[2];
    weights[0] = (255 * opacity) / 100; weights[1] = 255 - weights[0];
    const quint8* pixels[2];
    KoMixColorsOp * mixOp = cs->mixColorsOp();
    while (dstIt.nextPixel()) {
        if (randT.doubleRandomAt(dstIt.x(), dstIt.y()) > threshold) {
            int x = static_cast<int>(dstIt.x() + windowsize * (randH.doubleRandomAt(dstIt.x(), dstIt.y()) - 0.5));
            int y = static_cast<int>(dstIt.y() +  windowsize * (randV.doubleRandomAt(dstIt.x(), dstIt.y()) -0.5));
            srcRA->moveTo(x, y);
            pixels[0] = srcRA->oldRawData();
            pixels[1] = dstIt.oldRawData();
            mixOp->mixColors(pixels, weights, 2, dstIt.rawData());
        }
    }

}

KisConfigWidget * KisFilterRandomPick::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool) const
{
    Q_UNUSED(dev);
    return new KisWdgRandomPick((KisFilter*)this, (QWidget*)parent);
}

KisFilterConfigurationSP KisFilterRandomPick::factoryConfiguration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("randompick", 1);
    config->setProperty("level", 50);
    config->setProperty("windowsize", 2.5);
    config->setProperty("opacity", 100);
    config->setProperty("seedThreshold", rand());
    config->setProperty("seedH", rand());
    config->setProperty("seedV", rand());

    return config;
}

QRect KisFilterRandomPick::neededRect(const QRect& rect, const KisFilterConfigurationSP config, int lod) const
{
    Q_UNUSED(lod);

    QVariant value;
    int windowsize = ceil((config && config->getProperty("windowsize", value)) ? value.toDouble() : 2.5);
    return rect.adjusted(-windowsize, -windowsize, windowsize, windowsize);
}

QRect KisFilterRandomPick::changedRect(const QRect &rect, const KisFilterConfigurationSP config, int lod) const
{
    return neededRect(rect, config, lod);
}

#include "randompickfilter.moc"
