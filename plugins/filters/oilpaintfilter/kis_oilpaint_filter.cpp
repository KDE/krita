/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 * SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 * ported from digikam, Copyrighted by Gilles Caulier,
 * Original Oilpaint algorithm copyrighted 2004 by
 * Pieter Z. Voloshyn <pieter_voloshyn at ame.com.br>.
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_oilpaint_filter.h"

#include <stdlib.h>
#include <vector>

#include <QPoint>
#include <QSpinBox>
#include <QDateTime>

#include <klocalizedstring.h>
#include <kis_debug.h>
#include <kpluginfactory.h>

#include <KoUpdater.h>

#include <KisDocument.h>
#include <kis_image.h>
#include <KisSequentialIteratorProgress.h>
#include <kis_layer.h>
#include <filter/kis_filter_registry.h>
#include <kis_global.h>
#include <kis_types.h>
#include <filter/kis_filter_category_ids.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>
#include <kis_paint_device.h>
#include "widgets/kis_multi_integer_filter_widget.h"
#include <KisGlobalResourcesInterface.h>


KisOilPaintFilter::KisOilPaintFilter() : KisFilter(id(), FiltersCategoryArtisticId, i18n("&Oilpaint..."))
{
    setSupportsPainting(true);
    setSupportsThreading(false);
    setSupportsAdjustmentLayers(true);
}

void KisOilPaintFilter::processImpl(KisPaintDeviceSP device,
                                    const QRect& applyRect,
                                    const KisFilterConfigurationSP config,
                                    KoUpdater* progressUpdater
                                    ) const
{
    Q_ASSERT(!device.isNull());

    //read the filter configuration values from the KisFilterConfiguration object
    const quint32 brushSize = config ? config->getInt("brushSize", 1) : 1;
    const quint32 smooth = config ? config->getInt("smooth", 30) : 30;

    OilPaint(device, device, applyRect, brushSize, smooth, progressUpdater);
}

// This method have been ported from Pieter Z. Voloshyn algorithm code.

/* Function to apply the OilPaint effect.
 *
 * data             => The image data in RGBA mode.
 * w                => Width of image.
 * h                => Height of image.
 * BrushSize        => Brush size.
 * Smoothness       => Smooth value.
 *
 * Theory           => Using MostFrequentColor function we take the main color in
 *                     a matrix and simply write at the original position.
 */

void KisOilPaintFilter::OilPaint(const KisPaintDeviceSP src, KisPaintDeviceSP dst, const QRect &applyRect,
                                 int BrushSize, int Smoothness, KoUpdater* progressUpdater) const
{
    KisSequentialConstIteratorProgress it(src, applyRect, progressUpdater);
    KisSequentialIterator dstIt(dst, applyRect);

    while (it.nextPixel() && dstIt.nextPixel()) {
        MostFrequentColor(src, dstIt.rawData(), applyRect, it.x(), it.y(), BrushSize, Smoothness);
    }
}

// This method has been ported from Pieter Z. Voloshyn's algorithm code in Digikam.

/* Function to determine the most frequent color in a matrix
 *
 * Bits             => Bits array
 * Width            => Image width
 * Height           => Image height
 * X                => Position horizontal
 * Y                => Position vertical
 * Radius           => Is the radius of the matrix to be analyzed
 * Intensity        => Intensity to calculate
 *
 * Theory           => This function creates a matrix with the analyzed pixel in
 *                     the center of this matrix and find the most frequenty color
 */

void KisOilPaintFilter::MostFrequentColor(KisPaintDeviceSP src, quint8* dst, const QRect& /*bounds*/, int X, int Y, int Radius, int Intensity) const
{
    uint I;

    double Scale = Intensity / 255.0;

    // Alloc some arrays to be used
    uchar *IntensityCount = new uchar[(Intensity + 1)];

    const KoColorSpace* cs = src->colorSpace();

    QVector<float> channel(cs->channelCount());
    QVector<float>* AverageChannels = new QVector<float>[(Intensity + 1)];

    // Erase the array
    memset(IntensityCount, 0, (Intensity + 1) * sizeof(uchar));

    int startx = X - Radius;
    int starty = Y - Radius;
    int width = (2 * Radius) + 1;
    int height = (2 * Radius) + 1;

    qreal middlePointAlpha = 1;
    {
        // if the current pixel is transparent, the result must be transparent, too.
        KisSequentialConstIterator middlePointIt(src, QRect(X, Y, 1, 1));
        middlePointIt.nextPixel();
        middlePointAlpha = cs->opacityF(middlePointIt.oldRawData());
    }


    KisSequentialConstIterator srcIt(src, QRect(startx, starty, width, height));
    while (middlePointAlpha > 0 && srcIt.nextPixel()) {

        cs->normalisedChannelsValue(srcIt.oldRawData(), channel);

        if (cs->opacityU8(srcIt.oldRawData()) == 0) {
            // if the pixel is transparent, it's not going to provide any useful information
            continue;
        }

        I = (uint)(cs->intensity8(srcIt.oldRawData()) * Scale);

        IntensityCount[I]++;

        if (IntensityCount[I] == 1) {
            AverageChannels[I] = channel;
        } else {
            for (int i = 0; i < channel.size(); i++) {
                AverageChannels[I][i] += channel[i];
            }
        }
    }

    I = 0;
    int MaxInstance = 0;

    for (int i = 0 ; i <= Intensity ; ++i) {
        if (IntensityCount[i] > MaxInstance) {
            I = i;
            MaxInstance = IntensityCount[i];
        }
    }

    if (MaxInstance != 0) {
        channel = AverageChannels[I];
        for (int i = 0; i < channel.size(); i++) {
            channel[i] /= MaxInstance;
        }
        cs->fromNormalisedChannelsValue(dst, channel);
        cs->setOpacity(dst, OPACITY_OPAQUE_U8, middlePointAlpha);
    } else {
        memset(dst, 0, cs->pixelSize());
        cs->setOpacity(dst, OPACITY_OPAQUE_U8, middlePointAlpha);
    }


    delete [] IntensityCount;        // free all the arrays
    delete [] AverageChannels;
}

QRect KisOilPaintFilter::neededRect(const QRect & rect, const KisFilterConfigurationSP _config, int /*lod*/) const
{
    const quint32 brushSize = _config ? _config->getInt("brushSize", 1) : 1;
    return rect.adjusted(-brushSize * 2, -brushSize * 2, brushSize * 2, brushSize * 2);
}

QRect KisOilPaintFilter::changedRect(const QRect & rect, const KisFilterConfigurationSP _config, int /*lod*/) const
{
    const quint32 brushSize = _config ? _config->getInt("brushSize", 1) : 1;

    return rect.adjusted( -brushSize*2, -brushSize*2, brushSize*2, brushSize*2);
}


KisConfigWidget * KisOilPaintFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP, bool) const
{
    vKisIntegerWidgetParam param;
    param.push_back(KisIntegerWidgetParam(1, 5, 1, i18n("Brush size"), "brushSize"));
    param.push_back(KisIntegerWidgetParam(10, 255, 30, i18nc("smooth out the painting strokes the filter creates", "Smooth"), "smooth"));
    KisMultiIntegerFilterWidget * w = new KisMultiIntegerFilterWidget(id().id(),  parent,  id().id(),  param);
    w->setConfiguration(defaultConfiguration(KisGlobalResourcesInterface::instance()));
    return w;
}

KisFilterConfigurationSP KisOilPaintFilter::defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    KisFilterConfigurationSP config = factoryConfiguration(resourcesInterface);
    config->setProperty("brushSize", 1);
    config->setProperty("smooth", 30);

    return config;
}
