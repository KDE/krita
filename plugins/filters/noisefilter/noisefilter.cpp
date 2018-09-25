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

#include "noisefilter.h"
#include <stdlib.h>
#include <vector>

#include <QPoint>

#include <kis_debug.h>

#include <kpluginfactory.h>
#include <klocalizedstring.h>

#include <KoUpdater.h>

#include <KoMixColorsOp.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_layer.h>
#include <filter/kis_filter_registry.h>
#include <kis_global.h>
#include <kis_random_generator.h>
#include <kis_selection.h>
#include <kis_types.h>
#include <filter/kis_filter_category_ids.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>

#include "kis_wdg_noise.h"
#include "ui_wdgnoiseoptions.h"
#include <KisSequentialIteratorProgress.h>


K_PLUGIN_FACTORY_WITH_JSON(KritaNoiseFilterFactory, "kritanoisefilter.json", registerPlugin<KritaNoiseFilter>();)

KritaNoiseFilter::KritaNoiseFilter(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisFilterRegistry::instance()->add(new KisFilterNoise());

}

KritaNoiseFilter::~KritaNoiseFilter()
{
}

KisFilterNoise::KisFilterNoise() : KisFilter(id(), FiltersCategoryOtherId, i18n("&Random Noise..."))
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setSupportsPainting(true);
}

KisFilterConfigurationSP KisFilterNoise::factoryConfiguration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("noise", 1);
    config->setProperty("level", 50);
    config->setProperty("opacity", 100);
    config->setProperty("seedThreshold", rand());
    config->setProperty("seedRed", rand());
    config->setProperty("seedGreen", rand());
    config->setProperty("seedBlue", rand());
    return config;
}

KisConfigWidget * KisFilterNoise::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev) const
{
    Q_UNUSED(dev);
    return new KisWdgNoise((KisFilter*)this, (QWidget*)parent);
}

void KisFilterNoise::processImpl(KisPaintDeviceSP device,
                                 const QRect& applyRect,
                                 const KisFilterConfigurationSP config,
                                 KoUpdater* progressUpdater
                                 ) const
{
    Q_ASSERT(!device.isNull());

    const KoColorSpace * cs = device->colorSpace();

    QVariant value;
    const int level = (config && config->getProperty("level", value)) ? value.toInt() : 50;
    const int opacity = (config && config->getProperty("opacity", value)) ? value.toInt() : 100;

    quint8* interm = new quint8[cs->pixelSize()];
    const double threshold = (100.0 - level) * 0.01;

    qint16 weights[2];
    weights[0] = (255 * opacity) / 100; weights[1] = 255 - weights[0];

    const quint8* pixels[2];
    pixels[0] = interm;

    KoMixColorsOp * mixOp = cs->mixColorsOp();

    int seedThreshold = rand();
    int seedRed = rand();
    int seedGreen = rand();
    int seedBlue = rand();

    if (config) {
        seedThreshold = config->getInt("seedThreshold", seedThreshold);
        seedRed = config->getInt("seedRed", seedRed);
        seedGreen = config->getInt("seedGreen", seedGreen);
        seedBlue = config->getInt("seedBlue", seedBlue);
    }

    KisRandomGenerator randt(seedThreshold);
    KisRandomGenerator randr(seedRed);
    KisRandomGenerator randg(seedGreen);
    KisRandomGenerator randb(seedBlue);

    KisSequentialIteratorProgress it(device, applyRect, progressUpdater);

    while (it.nextPixel()) {
        if (randt.doubleRandomAt(it.x(), it.y()) > threshold) {
            // XXX: Added static_cast to get rid of warnings
            QColor c = qRgb(static_cast<int>((double)randr.doubleRandomAt(it.x(), it.y()) * 255),
                            static_cast<int>((double)randg.doubleRandomAt(it.x(), it.y()) * 255),
                            static_cast<int>((double)randb.doubleRandomAt(it.x(), it.y()) * 255));
            cs->fromQColor(c, interm, 0);
            pixels[1] = it.oldRawData();
            mixOp->mixColors(pixels, weights, 2, it.rawData());
        }
    }

    delete [] interm;
}

#include "noisefilter.moc"

