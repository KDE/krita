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

#include <qpoint.h>

#include <kis_debug.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <kpluginfactory.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kstandarddirs.h>

#include <KoProgressUpdater.h>
#include <KoUpdater.h>

#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <filter/kis_filter_registry.h>
#include <kis_global.h>
#include <kis_random_generator.h>
#include <kis_selection.h>
#include <kis_types.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>

#include "kis_wdg_noise.h"
#include "ui_wdgnoiseoptions.h"

K_PLUGIN_FACTORY(KritaNoiseFilterFactory, registerPlugin<KritaNoiseFilter>();)
K_EXPORT_PLUGIN(KritaNoiseFilterFactory("krita"))

KritaNoiseFilter::KritaNoiseFilter(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    //setComponentData(KritaNoiseFilterFactory::componentData());
    KisFilterRegistry::instance()->add(new KisFilterNoise());

}

KritaNoiseFilter::~KritaNoiseFilter()
{
}

KisFilterNoise::KisFilterNoise() : KisFilter(id(), categoryOther(), i18n("&Random Noise..."))
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setSupportsPainting(true);
    setSupportsPreview(true);
    setSupportsIncrementalPainting(false);
}

KisFilterConfiguration* KisFilterNoise::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfiguration* config = new KisFilterConfiguration("noise", 1);
    config->setProperty("level", 50);
    config->setProperty("opacity", 100);
    config->setProperty("seedThreshold", rand());
    config->setProperty("seedRed", rand());
    config->setProperty("seedGreen", rand());
    config->setProperty("seedBlue", rand());
    return config;
}

KisConfigWidget * KisFilterNoise::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, const KisImageWSP image) const
{
    Q_UNUSED(dev);
    Q_UNUSED(image);
    return new KisWdgNoise((KisFilter*)this, (QWidget*)parent);
}

void KisFilterNoise::process(KisConstProcessingInformation srcInfo,
                             KisProcessingInformation dstInfo,
                             const QSize& size,
                             const KisFilterConfiguration* config,
                             KoUpdater* progressUpdater
                            ) const
{
    const KisPaintDeviceSP src = srcInfo.paintDevice();
    KisPaintDeviceSP dst = dstInfo.paintDevice();
    QPoint dstTopLeft = dstInfo.topLeft();
    QPoint srcTopLeft = srcInfo.topLeft();
    Q_ASSERT(!src.isNull());
    Q_ASSERT(!dst.isNull());

    if (progressUpdater) {
        progressUpdater->setRange(0, size.width() * size.height());
    }
    int count = 0;

    const KoColorSpace * cs = src->colorSpace();

    QVariant value;
    int level = (config && config->getProperty("level", value)) ? value.toInt() : 50;
    int opacity = (config && config->getProperty("opacity", value)) ? value.toInt() : 100;

    KisHLineConstIteratorPixel srcIt = src->createHLineConstIterator(srcTopLeft.x(), srcTopLeft.y(), size.width(), srcInfo.selection());
    KisHLineIteratorPixel dstIt = dst->createHLineIterator(dstTopLeft.x(), dstTopLeft.y(), size.width(), dstInfo.selection());

    quint8* interm = new quint8[ cs->pixelSize()];
    double threshold = (100.0 - level) * 0.01;

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

    for (int row = 0; row < size.height() && !(progressUpdater && progressUpdater->interrupted()); ++row) {
        while (!srcIt.isDone() && !(progressUpdater && progressUpdater->interrupted())) {
            if (randt.doubleRandomAt(srcIt.x(), srcIt.y()) > threshold) {
                // XXX: Added static_cast to get rid of warnings
                QColor c = qRgb(static_cast<int>((double)randr.doubleRandomAt(srcIt.x(), srcIt.y()) * 255),
                                static_cast<int>((double)randg.doubleRandomAt(srcIt.x(), srcIt.y()) * 255),
                                static_cast<int>((double)randb.doubleRandomAt(srcIt.x(), srcIt.y()) * 255));
                cs->fromQColor(c, interm, 0);
                pixels[1] = srcIt.oldRawData();
                mixOp->mixColors(pixels, weights, 2, dstIt.rawData());
            }
            ++srcIt;
            ++dstIt;
            if (progressUpdater) progressUpdater->setValue(++count);
        }
        srcIt.nextRow();
        dstIt.nextRow();
    }

    delete [] interm;
}

