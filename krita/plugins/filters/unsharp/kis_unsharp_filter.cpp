/*
 * This file is part of Krita
 *
 * Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#include "kis_unsharp_filter.h"
#include <QBitArray>

#include <kundo2command.h>

#include <kis_mask_generator.h>
#include <kis_convolution_kernel.h>
#include <kis_convolution_painter.h>
#include <kis_gaussian_kernel.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>
#include <KoConvolutionOp.h>
#include <kis_paint_device.h>

#include "kis_wdg_unsharp.h"
#include "ui_wdgunsharp.h"
#include <kis_iterator_ng.h>
#include "KoColorSpaceTraits.h"


KisUnsharpFilter::KisUnsharpFilter() : KisFilter(id(), categoryEnhance(), i18n("&Unsharp Mask..."))
{
    setSupportsPainting(true);
    setSupportsAdjustmentLayers(false);
    setColorSpaceIndependence(FULLY_INDEPENDENT);
}

KisConfigWidget * KisUnsharpFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP) const
{
    return new KisWdgUnsharp(parent);
}

KisFilterConfiguration* KisUnsharpFilter::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfiguration* config = new KisFilterConfiguration(id().id(), 1);
    config->setProperty("halfSize", 1);
    config->setProperty("amount", 50);
    config->setProperty("threshold", 0);
    config->setProperty("lightnessOnly", true);
    return config;
}

void KisUnsharpFilter::processImpl(KisPaintDeviceSP device,
                                   const QRect& applyRect,
                                   const KisFilterConfiguration* config,
                                   KoUpdater* progressUpdater
                                   ) const
{

    QPointer<KoUpdater> filterUpdater = 0;
    QPointer<KoUpdater> convolutionUpdater = 0;
    KoProgressUpdater* updater = 0;

    if (progressUpdater) {
        updater = new KoProgressUpdater(progressUpdater);
        updater->start(100, i18n("Unsharp Mask"));
        // Two sub-sub tasks that each go from 0 to 100.
        convolutionUpdater = updater->startSubtask();
        filterUpdater = updater->startSubtask();
    }

    if (!config) config = new KisFilterConfiguration(id().id(), 1);

    QVariant value;
    const qreal halfSize = (config->getProperty("halfSize", value)) ? value.toDouble() : 1.0;
    const qreal amount = (config->getProperty("amount", value)) ? value.toDouble() : 25;
    const uint threshold = (config->getProperty("threshold", value)) ? value.toUInt() : 0;
    const uint lightnessOnly = (config->getProperty("lightnessOnly", value)) ? value.toBool() : true;

    QBitArray channelFlags = config->channelFlags();
    KisGaussianKernel::applyGaussian(device, applyRect,
                                     halfSize, halfSize,
                                     channelFlags,
                                     progressUpdater);

    if (progressUpdater && progressUpdater->interrupted()) {
        return;
    }

    qreal weights[2];
    qreal factor = 128;

    weights[0] = factor * (1. + amount);
    weights[1] = -factor * amount;

    if (lightnessOnly) {
        processLightnessOnly(device, applyRect, threshold, weights, factor, channelFlags);
    } else {
        processRaw(device, applyRect, threshold, weights, factor, channelFlags);
    }

    delete updater;

    if (progressUpdater) progressUpdater->setProgress(100);
}

void KisUnsharpFilter::processRaw(KisPaintDeviceSP device,
                                  const QRect &rect,
                                  quint8 threshold,
                                  qreal weights[2],
                                  qreal factor,
                                  const QBitArray &channelFlags) const
{
    const KoColorSpace *cs = device->colorSpace();
    const int pixelSize = cs->pixelSize();
    KoConvolutionOp * convolutionOp = cs->convolutionOp();
    KisHLineIteratorSP dstIt = device->createHLineIteratorNG(rect.x(), rect.y(), rect.width());

    quint8 *colors[2];
    colors[0] = new quint8[pixelSize];
    colors[1] = new quint8[pixelSize];

    for (int j = 0; j < rect.height(); j++) {
        do {
            quint8 diff = cs->difference(dstIt->oldRawData(), dstIt->rawDataConst());
            if (diff > threshold) {
                memcpy(colors[0], dstIt->oldRawData(), pixelSize);
                memcpy(colors[1], dstIt->rawDataConst(), pixelSize);
                convolutionOp->convolveColors(colors, weights, dstIt->rawData(), factor, 0, 2, channelFlags);
            } else {
                memcpy(dstIt->rawData(), dstIt->oldRawData(), pixelSize);
            }
        } while (dstIt->nextPixel());
        dstIt->nextRow();
    }

    delete colors[0];
    delete colors[1];
}

void KisUnsharpFilter::processLightnessOnly(KisPaintDeviceSP device,
                                            const QRect &rect,
                                            quint8 threshold,
                                            qreal weights[2],
                                            qreal factor,
                                            const QBitArray & /*channelFlags*/) const
{
    const KoColorSpace *cs = device->colorSpace();
    const int pixelSize = cs->pixelSize();
    KisHLineIteratorSP dstIt = device->createHLineIteratorNG(rect.x(), rect.y(), rect.width());

    quint16 labColorSrc[4];
    quint16 labColorDst[4];

    const int posL = 0;
    const int posAplha = 3;

    const qreal factorInv = 1.0 / factor;

    for (int j = 0; j < rect.height(); j++) {
        do {
            quint8 diff = cs->differenceA(dstIt->oldRawData(), dstIt->rawDataConst());
            if (diff > threshold) {
                cs->toLabA16(dstIt->oldRawData(), (quint8*)labColorSrc, 1);
                cs->toLabA16(dstIt->rawDataConst(), (quint8*)labColorDst, 1);

                qint32 valueL = (labColorSrc[posL] * weights[0] + labColorDst[posL] * weights[1]) * factorInv;
                labColorSrc[posL] = CLAMP(valueL,
                                          KoColorSpaceMathsTraits<quint16>::min,
                                          KoColorSpaceMathsTraits<quint16>::max);

                qint32 valueAlpha = (labColorSrc[posAplha] * weights[0] + labColorDst[posAplha] * weights[1]) * factorInv;
                labColorSrc[posAplha] = CLAMP(valueAlpha,
                                              KoColorSpaceMathsTraits<quint16>::min,
                                              KoColorSpaceMathsTraits<quint16>::max);

                cs->fromLabA16((quint8*)labColorSrc, dstIt->rawData(), 1);
            } else {
                memcpy(dstIt->rawData(), dstIt->oldRawData(), pixelSize);
            }
        } while (dstIt->nextPixel());
        dstIt->nextRow();
    }
}
