/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <compositeops/KoVcMultiArchBuildSupport.h> //MSVC requires that Vc come first
#include "kis_unsharp_filter.h"
#include <QBitArray>

#include <kis_mask_generator.h>
#include <kis_convolution_kernel.h>
#include <kis_convolution_painter.h>
#include <kis_gaussian_kernel.h>
#include <filter/kis_filter_category_ids.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>
#include <KoConvolutionOp.h>
#include <kis_paint_device.h>
#include "kis_lod_transform.h"

#include "kis_wdg_unsharp.h"
#include "ui_wdgunsharp.h"
#include "KoColorSpaceTraits.h"
#include <KisSequentialIteratorProgress.h>


KisUnsharpFilter::KisUnsharpFilter() : KisFilter(id(), FiltersCategoryEnhanceId, i18n("&Unsharp Mask..."))
{
    setSupportsPainting(true);
    setSupportsAdjustmentLayers(true);
    setSupportsThreading(true);

    /**
     * Officially Unsharp Mask doesn't support LoD, because it
     * generates subtle artifacts when the unsharp radius is smaller
     * than current zoom level. But LoD devices can still appear when
     * the filter is used in Adjustment Layer. So the actual LoD is
     * still counted on.
     */
    setSupportsLevelOfDetail(false);
    setColorSpaceIndependence(FULLY_INDEPENDENT);
}

KisConfigWidget * KisUnsharpFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP, bool) const
{
    return new KisWdgUnsharp(parent);
}

KisFilterConfigurationSP KisUnsharpFilter::defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    KisFilterConfigurationSP config = factoryConfiguration(resourcesInterface);
    config->setProperty("halfSize", 1);
    config->setProperty("amount", 0.5);
    config->setProperty("threshold", 0);
    config->setProperty("lightnessOnly", true);
    return config;
}

void KisUnsharpFilter::processImpl(KisPaintDeviceSP device,
                                   const QRect& applyRect,
                                   const KisFilterConfigurationSP config,
                                   KoUpdater* progressUpdater
                                   ) const
{

    QPointer<KoUpdater> filterUpdater = 0;
    QPointer<KoUpdater> convolutionUpdater = 0;
    QScopedPointer<KoProgressUpdater> updater;

    if (progressUpdater) {
        updater.reset(new KoProgressUpdater(progressUpdater));
        updater->start(100, i18n("Unsharp Mask"));
        // Two sub-sub tasks that each go from 0 to 100.
        convolutionUpdater = updater->startSubtask();
        filterUpdater = updater->startSubtask();
    }

    KIS_SAFE_ASSERT_RECOVER_RETURN(config);

    QVariant value;

    KisLodTransformScalar t(device);

    const qreal halfSize = t.scale(config->getProperty("halfSize", value) ? value.toDouble() : 1.0);
    const qreal amount = (config->getProperty("amount", value)) ? value.toDouble() : 0.5;
    const uint threshold = (config->getProperty("threshold", value)) ? value.toUInt() : 0;
    const uint lightnessOnly = (config->getProperty("lightnessOnly", value)) ? value.toBool() : true;

    QBitArray channelFlags = config->channelFlags();
    KisGaussianKernel::applyGaussian(device, applyRect,
                                     halfSize, halfSize,
                                     channelFlags,
                                     convolutionUpdater);

    qreal weights[2];
    qreal factor = 128;

    weights[0] = factor * (1. + amount);
    weights[1] = -factor * amount;

    if (lightnessOnly) {
        processLightnessOnly(device, applyRect, threshold, weights, factor, channelFlags, filterUpdater);
    } else {
        processRaw(device, applyRect, threshold, weights, factor, channelFlags, filterUpdater);
    }
}

void KisUnsharpFilter::processRaw(KisPaintDeviceSP device,
                                  const QRect &rect,
                                  quint8 threshold,
                                  qreal weights[2],
                                  qreal factor,
                                  const QBitArray &channelFlags,
                                  KoUpdater *progressUpdater) const
{
    const KoColorSpace *cs = device->colorSpace();
    const int pixelSize = cs->pixelSize();
    KoConvolutionOp * convolutionOp = cs->convolutionOp();

    quint8 *colors[2];
    colors[0] = new quint8[pixelSize];
    colors[1] = new quint8[pixelSize];

    KisSequentialIteratorProgress dstIt(device, rect, progressUpdater);

    while (dstIt.nextPixel()) {
        quint8 diff = 0;
        if (threshold == 1) {
            if (memcmp(dstIt.oldRawData(), dstIt.rawDataConst(), cs->pixelSize()) == 0) {
                diff = 1;
            }
        }
        else {
            diff = cs->difference(dstIt.oldRawData(), dstIt.rawDataConst());
        }

        if (diff >= threshold) {
            memcpy(colors[0], dstIt.oldRawData(), pixelSize);
            memcpy(colors[1], dstIt.rawDataConst(), pixelSize);
            convolutionOp->convolveColors(colors, weights, dstIt.rawData(), factor, 0, 2, channelFlags);
        } else {
            memcpy(dstIt.rawData(), dstIt.oldRawData(), pixelSize);
        }
    }

    delete[] colors[0];
    delete[] colors[1];
}

void KisUnsharpFilter::processLightnessOnly(KisPaintDeviceSP device,
                                            const QRect &rect,
                                            quint8 threshold,
                                            qreal weights[2],
                                            qreal factor,
                                            const QBitArray & /*channelFlags*/,
                                            KoUpdater *progressUpdater) const
{
    const KoColorSpace *cs = device->colorSpace();
    const int pixelSize = cs->pixelSize();

    quint16 labColorSrc[4];
    quint16 labColorDst[4];

    const int posL = 0;
    const int posAplha = 3;

    const qreal factorInv = 1.0 / factor;

    KisSequentialIteratorProgress dstIt(device, rect, progressUpdater);

    while (dstIt.nextPixel()) {
        quint8 diff = cs->differenceA(dstIt.oldRawData(), dstIt.rawDataConst());
        if (diff >= threshold) {
            cs->toLabA16(dstIt.oldRawData(), (quint8*)labColorSrc, 1);
            cs->toLabA16(dstIt.rawDataConst(), (quint8*)labColorDst, 1);

            qint32 valueL = (labColorSrc[posL] * weights[0] + labColorDst[posL] * weights[1]) * factorInv;
            labColorSrc[posL] = CLAMP(valueL,
                                      KoColorSpaceMathsTraits<quint16>::min,
                                      KoColorSpaceMathsTraits<quint16>::max);

            qint32 valueAlpha = (labColorSrc[posAplha] * weights[0] + labColorDst[posAplha] * weights[1]) * factorInv;
            labColorSrc[posAplha] = CLAMP(valueAlpha,
                                          KoColorSpaceMathsTraits<quint16>::min,
                                          KoColorSpaceMathsTraits<quint16>::max);

            cs->fromLabA16((quint8*)labColorSrc, dstIt.rawData(), 1);
        } else {
            memcpy(dstIt.rawData(), dstIt.oldRawData(), pixelSize);
        }
    }
}

QRect KisUnsharpFilter::neededRect(const QRect & rect, const KisFilterConfigurationSP config, int lod) const
{
    KisLodTransformScalar t(lod);

    QVariant value;
    const qreal halfSize = t.scale(config->getProperty("halfSize", value) ? value.toDouble() : 1.0);

    return rect.adjusted(-halfSize * 2, -halfSize * 2, halfSize * 2, halfSize * 2);
}

QRect KisUnsharpFilter::changedRect(const QRect & rect, const KisFilterConfigurationSP config, int lod) const
{
    KisLodTransformScalar t(lod);

    QVariant value;
    const qreal halfSize = t.scale(config->getProperty("halfSize", value) ? value.toDouble() : 1.0);

    return rect.adjusted( -halfSize, -halfSize, halfSize, halfSize);
}
