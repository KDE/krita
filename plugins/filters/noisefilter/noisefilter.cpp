/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "noisefilter.h"
#include <vector>

#include <kpluginfactory.h>
#include <klocalizedstring.h>

#include <KoUpdater.h>
#include <KoMixColorsOp.h>
#include <KisRandomGenerator2D.h>
#include <kis_paint_device.h>
#include <kis_types.h>
#include "kis_random_accessor_ng.h"
#include <filter/kis_filter_registry.h>
#include <filter/kis_filter_category_ids.h>
#include <filter/kis_filter_configuration.h>

#include "kis_wdg_noise.h"
#include "ui_wdgnoiseoptions.h"


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

KisFilterConfigurationSP KisFilterNoise::defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    KisFilterConfigurationSP config = factoryConfiguration(resourcesInterface);
    config->setProperty("level", 50);
    config->setProperty("opacity", 100);
    config->setProperty("seedThreshold", rand());
    config->setProperty("seedRed", rand());
    config->setProperty("seedGreen", rand());
    config->setProperty("seedBlue", rand());
    config->setProperty("grayscale", false);
    return config;
}

KisConfigWidget * KisFilterNoise::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool) const
{
    Q_UNUSED(dev);
    return new KisWdgNoise((KisFilter*)this, (QWidget*)parent);
}

void KisFilterNoise::processImpl(KisPaintDeviceSP device,
                                 const QRect& rect,
                                 const KisFilterConfigurationSP config,
                                 KoUpdater* progressUpdater) const
{
    Q_UNUSED(progressUpdater);
    Q_ASSERT(!device.isNull());

    QVariant value;
    const int level = (config && config->getProperty("level", value)) ? value.toInt() : 50;
    const int opacity = (config && config->getProperty("opacity", value)) ? value.toInt() : 100;
    const bool isGrayscale = (config && config->getProperty("grayscale", value)) ? value.toBool() : false;

    const double threshold = (100.0 - level) * 0.01;
    const KoColorSpace* colorSpace = device->colorSpace();
    const quint32 pixelSize = colorSpace->pixelSize();
    const KoMixColorsOp* mixOp = colorSpace->mixColorsOp();

    std::vector<quint8> scratchPixel(pixelSize);

    constexpr quint32 numMixPixels = 2;
    qint16 weights[numMixPixels];
    weights[0] = (255 * opacity) / 100;
    weights[1] = 255 - weights[0];

    const quint8* pixels[numMixPixels];
    pixels[0] = scratchPixel.data();

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

    KisRandomGenerator2D randt(seedThreshold);
    KisRandomGenerator2D randr(seedRed);
    KisRandomGenerator2D randg(seedGreen);
    KisRandomGenerator2D randb(seedBlue);

    KisRandomAccessorSP dstIt = device->createRandomAccessorNG();

    qint32 dstY_ = rect.y();
    qint32 rowsRemaining = rect.height();

    while (rowsRemaining > 0) {
        qint32 dstX_ = rect.x();
        qint32 columnsRemaining = rect.width();
        qint32 numContiguousDstRows = dstIt->numContiguousRows(dstY_);
        qint32 rows = qMin(numContiguousDstRows, rowsRemaining);

        while (columnsRemaining > 0) {
            qint32 numContiguousDstColumns = dstIt->numContiguousColumns(dstX_);
            qint32 columns = qMin(numContiguousDstColumns, columnsRemaining);

            qint32 dstRowStride = dstIt->rowStride(dstX_, dstY_);
            dstIt->moveTo(dstX_, dstY_);

            for (int rowIndex = 0; rowIndex < rows; ++rowIndex) {
                for (int colIndex = 0; colIndex < columns; ++colIndex) {
                    const int px = dstX_ + colIndex;
                    const int py = dstY_ + rowIndex;
                    if (randt.doubleRandomAt(px, py) > threshold) {
                        QColor color;
                        if (isGrayscale) {
                            const int gray = static_cast<int>(randr.doubleRandomAt(px, py) * 255);
                            color = qRgb(gray, gray, gray);
                        } else {
                            color = qRgb(static_cast<int>(randr.doubleRandomAt(px, py) * 255),
                                         static_cast<int>(randg.doubleRandomAt(px, py) * 255),
                                         static_cast<int>(randb.doubleRandomAt(px, py) * 255));
                        }
                        const quint32 dataOffset = colIndex * pixelSize + rowIndex * dstRowStride;
                        if (opacity == 100) {
                            colorSpace->fromQColor(color, dstIt->rawData() + dataOffset);
                        } else {
                            colorSpace->fromQColor(color, scratchPixel.data());
                            pixels[1] = dstIt->oldRawData() + dataOffset;
                            mixOp->mixColors(pixels, weights, numMixPixels, dstIt->rawData() + dataOffset);
                        }
                    }
                }
            }

            dstX_ += columns;
            columnsRemaining -= columns;
        }

        dstY_ += rows;
        rowsRemaining -= rows;
    }
}

#include "noisefilter.moc"

