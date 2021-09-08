/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QHash>

#include <kpluginfactory.h>
#include <kis_filter_registry.h>
#include <filter/kis_filter_category_ids.h>
#include <KoUpdater.h>
#include <kis_filter_configuration.h>
#include <generator/kis_generator.h>
#include <generator/kis_generator_registry.h>
#include <KisGlobalResourcesInterface.h>
#include <KisSequentialIteratorProgress.h>
#include <kis_processing_information.h>
#include <kis_selection.h>
#include <kis_painter.h>
#include <KoCompositeOpRegistry.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>

#include "KisHalftoneFilter.h"
#include "KisHalftoneConfigWidget.h"

K_PLUGIN_FACTORY_WITH_JSON(KritaHalftoneFactory, "KritaHalftone.json", registerPlugin<KritaHalftone>();)

KritaHalftone::KritaHalftone(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisFilterRegistry::instance()->add(new KisHalftoneFilter());
}

KritaHalftone::~KritaHalftone()
{}

KisHalftoneFilter::KisHalftoneFilter()
    : KisFilter(id(), FiltersCategoryArtisticId, i18n("&Halftone..."))
{
    setSupportsPainting(true);
}

void KisHalftoneFilter::processImpl(KisPaintDeviceSP device,
                                    const QRect &applyRect,
                                    const KisFilterConfigurationSP config,
                                    KoUpdater *progressUpdater) const
{
    const KisHalftoneFilterConfiguration *filterConfig =
            dynamic_cast<const KisHalftoneFilterConfiguration*>(config.data());

    Q_ASSERT(device);
    KIS_SAFE_ASSERT_RECOVER_RETURN(filterConfig);
    KIS_SAFE_ASSERT_RECOVER_NOOP(filterConfig->hasLocalResourcesSnapshot());

    const QString mode = filterConfig->mode();
    KIS_SAFE_ASSERT_RECOVER_RETURN(
        mode == KisHalftoneFilterConfiguration::HalftoneMode_Intensity ||
        mode == KisHalftoneFilterConfiguration::HalftoneMode_IndependentChannels ||
        mode == KisHalftoneFilterConfiguration::HalftoneMode_Alpha
    );

    KIS_SAFE_ASSERT_RECOVER_RETURN(
        (device->colorSpace()->colorModelId().id() == AlphaColorModelID.id() &&
         mode == KisHalftoneFilterConfiguration::HalftoneMode_Alpha) ||
        (device->colorSpace()->colorModelId().id() == GrayColorModelID.id() &&
         mode == KisHalftoneFilterConfiguration::HalftoneMode_Intensity) ||
        (device->colorSpace()->colorModelId().id() == GrayAColorModelID.id() &&
         (mode == KisHalftoneFilterConfiguration::HalftoneMode_Intensity ||
          mode == KisHalftoneFilterConfiguration::HalftoneMode_Alpha)) ||
        ((device->colorSpace()->colorModelId().id() == RGBAColorModelID.id() ||
          device->colorSpace()->colorModelId().id() == XYZAColorModelID.id() ||
          device->colorSpace()->colorModelId().id() == LABAColorModelID.id() ||
          device->colorSpace()->colorModelId().id() == CMYKAColorModelID.id() ||
          device->colorSpace()->colorModelId().id() == YCbCrAColorModelID.id()) &&
         (mode == KisHalftoneFilterConfiguration::HalftoneMode_Intensity ||
          mode == KisHalftoneFilterConfiguration::HalftoneMode_IndependentChannels ||
          mode == KisHalftoneFilterConfiguration::HalftoneMode_Alpha))
    );
    
    if (mode == KisHalftoneFilterConfiguration::HalftoneMode_Intensity) {
        KIS_SAFE_ASSERT_RECOVER_RETURN(
            device->colorSpace()->colorModelId().id() != AlphaColorModelID.id()
        );
        processIntensity(device, applyRect, filterConfig, progressUpdater);
    } else if (mode == KisHalftoneFilterConfiguration::HalftoneMode_IndependentChannels) {
        KIS_SAFE_ASSERT_RECOVER_RETURN(
            device->colorSpace()->colorModelId().id() != AlphaColorModelID.id() &&
            device->colorSpace()->colorModelId().id() != GrayColorModelID.id() &&
            device->colorSpace()->colorModelId().id() != GrayAColorModelID.id()
        );
        processChannels(device, applyRect, filterConfig, progressUpdater);
    } else {
        KIS_SAFE_ASSERT_RECOVER_RETURN(
            device->colorSpace()->colorModelId().id() != GrayColorModelID.id()
        );
        if (filterConfig->colorModelId() == AlphaColorModelID.id())
        {
            // This an Alpha layer (mask layer) but the device passed
            // here has the composition color space (GrayA)
            processMask(device, applyRect, filterConfig, progressUpdater);
        } else {
            // process the alpha channel of a multichannel device
            processAlpha(device, applyRect, filterConfig, progressUpdater);
        }
    }
}

QVector<quint8> KisHalftoneFilter::makeHardnessLut(qreal hardness)
{
    QVector<quint8> hardnessLut(256);
    if (qFuzzyCompare(hardness, 1.0)) {
        for (int i = 0; i < 256; ++i) {
            hardnessLut[i] = i < 128 ? 0 : 255;
        }
    } else {
        qreal m = 1.0 / (1.0 - hardness);
        qreal b = -m * (hardness / 2.0);
        for (int i = 0; i < 256; ++i) {
            hardnessLut[i] = qBound(0, static_cast<int>(qRound((m * (i / 255.0) + b) * 255.0)), 255);
        }
    }
    return hardnessLut;
}

QVector<quint8> KisHalftoneFilter::makeNoiseWeightLut(qreal hardness)
{
    QVector<quint8> noiseWeightLut(256);
    hardness *= 0.99;
    for (int i = 0; i < 256; ++i) {
        qreal iNorm = i / 255.0;
        qreal weight = (2.0 - std::abs(4.0 * iNorm - 2.0)) + hardness;
        noiseWeightLut[i] = qBound(0, static_cast<int>(qRound(weight * 255.0)), 255);
    }
    return noiseWeightLut;
}

KisPaintDeviceSP KisHalftoneFilter::makeGeneratorPaintDevice(KisPaintDeviceSP prototype,
                                                             const QString & prefix,
                                                             const QRect &applyRect,
                                                             const KisHalftoneFilterConfiguration *config,
                                                             KoUpdater *progressUpdater) const
{
    const QString generatorId = config->generatorId(prefix);
    if (generatorId.isEmpty()) {
        return nullptr;
    }

    KisGeneratorSP generator  = KisGeneratorRegistry::instance()->get(generatorId);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(generator, nullptr);

    KisFilterConfigurationSP generatorConfiguration = config->generatorConfiguration(prefix);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(generatorConfiguration, nullptr);

    // Fill the generator device
    KisPaintDeviceSP generatorDevice = m_grayDevicesCache.getDevice(prototype, KoColorSpaceRegistry::instance()->graya8());

    KisProcessingInformation(generatorDevice, applyRect.topLeft(), KisSelectionSP());
    generator->generate(
        KisProcessingInformation(generatorDevice, applyRect.topLeft(),KisSelectionSP()),
        applyRect.size(),
        generatorConfiguration,
        progressUpdater
    );

    return generatorDevice;
}

bool KisHalftoneFilter::checkUpdaterInterruptedAndSetPercent(KoUpdater *progressUpdater, int percent) const
{
    // The updater is null so return false to keep going
    // with the computations
    if (!progressUpdater) {
        return false;
    }

    if (progressUpdater->interrupted()) {
        return true;
    }

    progressUpdater->setProgress(percent);
    return false;
}

void KisHalftoneFilter::processIntensity(KisPaintDeviceSP device,
                                         const QRect &applyRect,
                                         const KisHalftoneFilterConfiguration *config,
                                         KoUpdater *progressUpdater) const
{
    const QString prefix = "intensity_";

    if (checkUpdaterInterruptedAndSetPercent(progressUpdater, 0)) {
        return;
    }
    
    // Make the generator device
    KisPaintDeviceSP generatorDevice = makeGeneratorPaintDevice(device, prefix, applyRect, config, nullptr);
    if (!generatorDevice) {
        return;
    }
    if (checkUpdaterInterruptedAndSetPercent(progressUpdater, 25)) {
        return;
    }

    // Make the hardness and the noise weight LUT
    const qreal hardness = config->hardness(prefix) / 100.0;
    const QVector<quint8> hardnessLut = makeHardnessLut(hardness);
    const QVector<quint8> noiseWeightLut = makeNoiseWeightLut(hardness);

    // Fill the mask device
    KisSelectionSP maskDevice = m_selectionsCache.getSelection();

    {
        const bool invert = config->invert(prefix);

        KisSequentialIterator maskIterator(maskDevice->pixelSelection(), applyRect);
        KisSequentialIterator dstIterator(device, applyRect);
        KisSequentialIterator srcIterator(generatorDevice, applyRect);

        if (!invert) {
            while (maskIterator.nextPixel() && dstIterator.nextPixel() && srcIterator.nextPixel()) {
                int dstGray = device->colorSpace()->intensity8(dstIterator.rawData());
                int srcGray = srcIterator.rawData()[0];
                int srcAlpha = srcIterator.rawData()[1];

                // Combine pixels
                int result = qBound(0, dstGray + (srcGray - 128) * noiseWeightLut[dstGray] * srcAlpha / 0xFE01, 255);

                // Apply hardness
                result = hardnessLut[result];

                *maskIterator.rawData() = 255 - result;
            }
        } else {
            while (maskIterator.nextPixel() && dstIterator.nextPixel() && srcIterator.nextPixel()) {
                int dstGray = device->colorSpace()->intensity8(dstIterator.rawData());
                int srcGray = srcIterator.rawData()[0];
                int srcAlpha = srcIterator.rawData()[1];

                // Combine pixels
                int result = qBound(0, dstGray + (srcGray - 128) * noiseWeightLut[dstGray] * srcAlpha / 0xFE01, 255);

                // Apply hardness
                result = hardnessLut[result];

                *maskIterator.rawData() = result;
            }
        }
        m_grayDevicesCache.putDevice(generatorDevice);
    }
    if (checkUpdaterInterruptedAndSetPercent(progressUpdater, 50)) {
        return;
    }

    // Make the halftone image
    const KoColorSpace *colorSpace;
    if (device->colorSpace()->profile()->isLinear()) {
        colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    } else {
        colorSpace = device->colorSpace();
    }
    KisPaintDeviceSP halftoneDevice = m_genericDevicesCache.getDevice(device, colorSpace);
    
    {
        KisPaintDeviceSP foregroundDevice = m_genericDevicesCache.getDevice(device, colorSpace);
        KoColor foregroundColor = config->foregroundColor(prefix);
        KoColor backgroundColor = config->backgroundColor(prefix);
        const qreal foregroundOpacity = config->foregroundOpacity(prefix) / 100.0;
        const qreal backgroundOpacity = config->backgroundOpacity(prefix) / 100.0;
        foregroundColor.convertTo(colorSpace);
        backgroundColor.convertTo(colorSpace);
        foregroundColor.setOpacity(foregroundOpacity);
        backgroundColor.setOpacity(backgroundOpacity);

        foregroundDevice->fill(applyRect, foregroundColor);
        halftoneDevice->fill(applyRect, backgroundColor);

        KisPainter painter(halftoneDevice, maskDevice);
        painter.setCompositeOp(COMPOSITE_OVER);
        painter.bitBlt(applyRect.topLeft(), foregroundDevice, applyRect);

        m_genericDevicesCache.putDevice(foregroundDevice);
        m_selectionsCache.putSelection(maskDevice);
    }
    if (checkUpdaterInterruptedAndSetPercent(progressUpdater, 75)) {
        return;
    }

    // Make the final image
    {
        KisPainter painter(halftoneDevice);
        painter.setCompositeOp(COMPOSITE_DESTINATION_IN);
        painter.bitBlt(applyRect.topLeft(), device, applyRect);
    }
    {
        KisPainter painter(device);
        painter.setCompositeOp(COMPOSITE_COPY);
        painter.bitBlt(applyRect.topLeft(), halftoneDevice, applyRect);
    }
    m_genericDevicesCache.putDevice(halftoneDevice);

    if (checkUpdaterInterruptedAndSetPercent(progressUpdater, 100)) {
        return;
    }
}

template <typename ChannelType>
void KisHalftoneFilter::processChannel(KisPaintDeviceSP device,
                                       KisPaintDeviceSP generatorDevice,
                                       const QRect &applyRect,
                                       const KisHalftoneFilterConfiguration *config,
                                       const QString & prefix,
                                       KoChannelInfo * channelInfo) const
{
    const int channelPos = channelInfo->pos() / sizeof(ChannelType);
    // Make the hardness and the noise weight LUT
    const qreal hardness = config->hardness(prefix) / 100.0;
    const QVector<quint8> hardnessLut = makeHardnessLut(hardness);
    const QVector<quint8> noiseWeightLut = makeNoiseWeightLut(hardness);

    // Fill the device
    const bool invert = config->invert(prefix);
    KisSequentialIterator dstIterator(device, applyRect);
    KisSequentialIterator srcIterator(generatorDevice, applyRect);

    const KoColorSpace *colorSpace = device->colorSpace();

    if (colorSpace->profile()->isLinear()) {
        if (!invert) {
            while (dstIterator.nextPixel() && srcIterator.nextPixel()) {
                int dst = 255 - device->colorSpace()->scaleToU8(dstIterator.rawData(), channelPos);
                int src = srcIterator.rawData()[0];
                int srcAlpha = srcIterator.rawData()[1];
                KoColor c(QColor(src, src, src, srcAlpha), device->colorSpace());
                src = device->colorSpace()->scaleToU8(c.data(), 0);
                srcAlpha = device->colorSpace()->scaleToU8(c.data(), device->colorSpace()->alphaPos());

                // Combine pixels
                int result = qBound(0, dst + (src - 128) * noiseWeightLut[dst] * srcAlpha / 0xFE01, 255);

                // Apply hardness
                result = hardnessLut[result];

                ChannelType *dstPixel = reinterpret_cast<ChannelType*>(dstIterator.rawData());
                ChannelType channelMin = static_cast<ChannelType>(channelInfo->getUIMin());
                ChannelType channelMax = static_cast<ChannelType>(channelInfo->getUIMax());
                dstPixel[channelPos] = static_cast<ChannelType>(mapU8ToRange(255 - result, channelMin, channelMax));
            }
        } else {
            while (dstIterator.nextPixel() && srcIterator.nextPixel()) {
                int dst = device->colorSpace()->scaleToU8(dstIterator.rawData(), channelPos);
                int src = srcIterator.rawData()[0];
                int srcAlpha = srcIterator.rawData()[1];
                KoColor c(QColor(src, src, src, srcAlpha), device->colorSpace());
                src = device->colorSpace()->scaleToU8(c.data(), 0);
                srcAlpha = device->colorSpace()->scaleToU8(c.data(), device->colorSpace()->alphaPos());

                // Combine pixels
                int result = qBound(0, dst + (src - 128) * noiseWeightLut[dst] * srcAlpha / 0xFE01, 255);

                // Apply hardness
                result = hardnessLut[result];

                ChannelType *dstPixel = reinterpret_cast<ChannelType*>(dstIterator.rawData());
                ChannelType channelMin = static_cast<ChannelType>(channelInfo->getUIMin());
                ChannelType channelMax = static_cast<ChannelType>(channelInfo->getUIMax());
                dstPixel[channelPos] = static_cast<ChannelType>(mapU8ToRange(result, channelMin, channelMax));
            }
        }
    } else {
        if (!invert) {
            while (dstIterator.nextPixel() && srcIterator.nextPixel()) {
                int dst = 255 - device->colorSpace()->scaleToU8(dstIterator.rawData(), channelPos);
                int src = srcIterator.rawData()[0];
                int srcAlpha = srcIterator.rawData()[1];

                // Combine pixels
                int result = qBound(0, dst + (src - 128) * noiseWeightLut[dst] * srcAlpha / 0xFE01, 255);

                // Apply hardness
                result = hardnessLut[result];

                ChannelType *dstPixel = reinterpret_cast<ChannelType*>(dstIterator.rawData());
                ChannelType channelMin = static_cast<ChannelType>(channelInfo->getUIMin());
                ChannelType channelMax = static_cast<ChannelType>(channelInfo->getUIMax());
                dstPixel[channelPos] = static_cast<ChannelType>(mapU8ToRange(255 - result, channelMin, channelMax));
            }
        } else {
            while (dstIterator.nextPixel() && srcIterator.nextPixel()) {
                int dst = device->colorSpace()->scaleToU8(dstIterator.rawData(), channelPos);
                int src = srcIterator.rawData()[0];
                int srcAlpha = srcIterator.rawData()[1];

                // Combine pixels
                int result = qBound(0, dst + (src - 128) * noiseWeightLut[dst] * srcAlpha / 0xFE01, 255);

                // Apply hardness
                result = hardnessLut[result];

                ChannelType *dstPixel = reinterpret_cast<ChannelType*>(dstIterator.rawData());
                ChannelType channelMin = static_cast<ChannelType>(channelInfo->getUIMin());
                ChannelType channelMax = static_cast<ChannelType>(channelInfo->getUIMax());
                dstPixel[channelPos] = static_cast<ChannelType>(mapU8ToRange(result, channelMin, channelMax));
            }
        }
    }
}

void KisHalftoneFilter::processChannels(KisPaintDeviceSP device,
                                        const QRect &applyRect,
                                        const KisHalftoneFilterConfiguration *config,
                                        KoUpdater *progressUpdater) const
{
    const QList<KoChannelInfo *> channels = device->colorSpace()->channels();
    const int progressStep = 100 / (channels.count() * 2);

    if (checkUpdaterInterruptedAndSetPercent(progressUpdater, 0)) {
        return;
    }

    // Make the generator devices
    QVector<KisPaintDeviceSP> generatorDevices(channels.count());
    for (int i = 0; i < channels.count(); ++i) {
        if (channels.at(i)->channelType() == KoChannelInfo::ALPHA) {
            generatorDevices[i] = nullptr;
        } else {
            const QString prefix =
                device->colorSpace()->colorModelId().id() + "_channel" + QString::number(i) + "_";
            KisPaintDeviceSP generatorDevice = makeGeneratorPaintDevice(device, prefix, applyRect, config, nullptr);
            if (generatorDevice) {
                generatorDevices[i] = generatorDevice;
            } else {
                generatorDevices[i] = nullptr;
            }
        }
        if (checkUpdaterInterruptedAndSetPercent(progressUpdater, progressUpdater->progress() + progressStep)) {
            return;
        }
    }

    // process channels
    for (int i = 0; i < channels.count(); ++i) {
        if (!generatorDevices[i]) {
            progressUpdater->setProgress(progressUpdater->progress() + progressStep);
            continue;
        }

        const QString prefix = device->colorSpace()->colorModelId().id() + "_channel" + QString::number(i) + "_";

        switch (channels.at(i)->channelValueType()) {
        case KoChannelInfo::UINT8: {
            processChannel<quint8>(device, generatorDevices[i], applyRect, config, prefix, channels.at(i));
            break;
        } case KoChannelInfo::UINT16: {
            processChannel<quint16>(device, generatorDevices[i], applyRect, config, prefix, channels.at(i));
            break;
        } case KoChannelInfo::UINT32: {
            processChannel<quint32>(device, generatorDevices[i], applyRect, config, prefix, channels.at(i));
            break;
        } case KoChannelInfo::FLOAT16: {
#ifdef HAVE_OPENEXR
            processChannel<half>(device, generatorDevices[i], applyRect, config, prefix, channels.at(i));
#endif
            break;
        } case KoChannelInfo::FLOAT32: {
            processChannel<float>(device, generatorDevices[i], applyRect, config, prefix, channels.at(i));
            break;
        } case KoChannelInfo::FLOAT64: {
            processChannel<double>(device, generatorDevices[i], applyRect, config, prefix, channels.at(i));
            break;
        } case KoChannelInfo::INT8: {
            processChannel<qint8>(device, generatorDevices[i], applyRect, config, prefix, channels.at(i));
            break;
        } case KoChannelInfo::INT16: {
            processChannel<qint16>(device, generatorDevices[i], applyRect, config, prefix, channels.at(i));
            break;
        } default: {
            break;
        }
        }

        m_grayDevicesCache.putDevice(generatorDevices[i]);

        if (checkUpdaterInterruptedAndSetPercent(progressUpdater, progressUpdater->progress() + progressStep)) {
            return;
        }
    }

    if (checkUpdaterInterruptedAndSetPercent(progressUpdater, 100)) {
        return;
    }
}

void KisHalftoneFilter::processAlpha(KisPaintDeviceSP device,
                                     const QRect& applyRect,
                                     const KisHalftoneFilterConfiguration *config,
                                     KoUpdater *progressUpdater) const
{
    const QString prefix = "alpha_";
    
    if (checkUpdaterInterruptedAndSetPercent(progressUpdater, 0)) {
        return;
    }

    // Make the generator device
    KisPaintDeviceSP generatorDevice = makeGeneratorPaintDevice(device, prefix, applyRect, config, nullptr);
    if (!generatorDevice) {
        return;
    }
    if (checkUpdaterInterruptedAndSetPercent(progressUpdater, 50)) {
        return;
    }

    // Make the hardness and the noise weight LUT
    const qreal hardness = config->hardness(prefix) / 100.0;
    const QVector<quint8> hardnessLut = makeHardnessLut(hardness);
    const QVector<quint8> noiseWeightLut = makeNoiseWeightLut(hardness);

    // Fill the device
    const bool invert = config->invert(prefix);
    KisSequentialIterator dstIterator(device, applyRect);
    KisSequentialIterator srcIterator(generatorDevice, applyRect);

    if (!invert) {
        while (dstIterator.nextPixel() && srcIterator.nextPixel()) {
            int dst = 255 - device->colorSpace()->opacityU8(dstIterator.rawData());
            int src = srcIterator.rawData()[0];
            int srcAlpha = srcIterator.rawData()[1];

            // Combine pixels
            int result = qBound(0, dst + (src - 128) * noiseWeightLut[dst] * srcAlpha / 0xFE01, 255);

            // Apply hardness
            result = hardnessLut[result];

            device->colorSpace()->setOpacity(dstIterator.rawData(), static_cast<quint8>(255 - result), 1);
        }
    } else {
        while (dstIterator.nextPixel() && srcIterator.nextPixel()) {
            int dst = device->colorSpace()->opacityU8(dstIterator.rawData());
            int src = srcIterator.rawData()[0];
            int srcAlpha = srcIterator.rawData()[1];

            // Combine pixels
            int result = qBound(0, dst + (src - 128) * noiseWeightLut[dst] * srcAlpha / 0xFE01, 255);

            // Apply hardness
            result = hardnessLut[result];

            device->colorSpace()->setOpacity(dstIterator.rawData(), static_cast<quint8>(result), 1);
        }
    }
    m_grayDevicesCache.putDevice(generatorDevice);

    if (checkUpdaterInterruptedAndSetPercent(progressUpdater, 100)) {
        return;
    }
}

void KisHalftoneFilter::processMask(KisPaintDeviceSP device,
                                    const QRect& applyRect,
                                    const KisHalftoneFilterConfiguration *config,
                                    KoUpdater *progressUpdater) const
{
    const QString prefix = "alpha_";
    
    if (checkUpdaterInterruptedAndSetPercent(progressUpdater, 0)) {
        return;
    }

    // Make the generator device
    KisPaintDeviceSP generatorDevice = makeGeneratorPaintDevice(device, prefix, applyRect, config, nullptr);
    if (!generatorDevice) {
        return;
    }
    if (checkUpdaterInterruptedAndSetPercent(progressUpdater, 50)) {
        return;
    }

    // Make the hardness and the noise weight LUT
    const qreal hardness = config->hardness(prefix) / 100.0;
    const QVector<quint8> hardnessLut = makeHardnessLut(hardness);
    const QVector<quint8> noiseWeightLut = makeNoiseWeightLut(hardness);

    // Fill the device
    const bool invert = config->invert(prefix);
    KisSequentialIterator dstIterator(device, applyRect);
    KisSequentialIterator srcIterator(generatorDevice, applyRect);

    if (!invert) {
        while (dstIterator.nextPixel() && srcIterator.nextPixel()) {
            int dst = 255 - *dstIterator.rawData();
            int src = *srcIterator.rawData();

            // Combine pixels
            int result = qBound(0, dst + (src - 128) * noiseWeightLut[dst] / 0xFF, 255);

            // Apply hardness
            result = hardnessLut[result];

            *dstIterator.rawData() = static_cast<quint8>(255 - result);
        }
    } else {
        while (dstIterator.nextPixel() && srcIterator.nextPixel()) {
            int dst = *dstIterator.rawData();
            int src = *srcIterator.rawData();

            // Combine pixels
            int result = qBound(0, dst + (src - 128) * noiseWeightLut[dst] / 0xFF, 255);

            // Apply hardness
            result = hardnessLut[result];

            *dstIterator.rawData() =  static_cast<quint8>(result);
        }
    }
    m_grayDevicesCache.putDevice(generatorDevice);

    if (checkUpdaterInterruptedAndSetPercent(progressUpdater, 100)) {
        return;
    }
}

KisFilterConfigurationSP KisHalftoneFilter::defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    KisHalftoneFilterConfigurationSP filterConfig =
        dynamic_cast<KisHalftoneFilterConfiguration*>(factoryConfiguration(resourcesInterface).data());
        
    filterConfig->setMode(KisHalftoneFilterConfiguration::defaultMode());

    QString defaultGeneratorId = KisHalftoneFilterConfiguration::defaultGeneratorId();
    KisGeneratorSP defaultGenerator = KisGeneratorRegistry::instance()->get(defaultGeneratorId);

    // intensity
    filterConfig->setGeneratorId("intensity_", defaultGeneratorId);
    if (defaultGenerator) {
        KisFilterConfigurationSP defaultGeneratorConfiguration =
            defaultGenerator->defaultConfiguration(resourcesInterface);
        if (defaultGeneratorId == "screentone") {
            defaultGeneratorConfiguration->setProperty("rotation", 45.0);
            defaultGeneratorConfiguration->setProperty("contrast", 50.0);
        }
        filterConfig->setGeneratorConfiguration("intensity_", defaultGeneratorConfiguration);
    }
    filterConfig->setHardness("intensity_", KisHalftoneFilterConfiguration::defaultHardness());
    filterConfig->setInvert("intensity_", KisHalftoneFilterConfiguration::defaultInvert());
    filterConfig->setForegroundColor("intensity_", KisHalftoneFilterConfiguration::defaultForegroundColor());
    filterConfig->setForegroundOpacity("intensity_", KisHalftoneFilterConfiguration::defaultForegroundOpacity());
    filterConfig->setBackgroundColor("intensity_", KisHalftoneFilterConfiguration::defaultBackgroundColor());
    filterConfig->setBackgroundOpacity("intensity_", KisHalftoneFilterConfiguration::defaultBackgroundOpacity());
    
    // Alpha
    filterConfig->setGeneratorId("alpha_", "");
    filterConfig->setHardness("alpha_", KisHalftoneFilterConfiguration::defaultHardness());
    filterConfig->setInvert("alpha_", KisHalftoneFilterConfiguration::defaultInvert());

    // The channels only use default generator if it is screentone
    // because there are predefined ways of presenting the patterns (screen angles)

    // Map channel prefixes to rotation angle
    QHash<QString, qreal> channelDict;
    channelDict.insert("RGBA_channel0_", 15.0);
    channelDict.insert("RGBA_channel1_", 45.0);
    channelDict.insert("RGBA_channel2_", 75.0);
    channelDict.insert("CMYKA_channel0_", 15.0);
    channelDict.insert("CMYKA_channel1_", 75.0);
    channelDict.insert("CMYKA_channel2_", 0.0);
    channelDict.insert("CMYKA_channel3_", 45.0);

    for (auto i = channelDict.constBegin(); i != channelDict.constEnd(); ++i) {
        if (defaultGenerator && defaultGeneratorId == "screentone") {
            filterConfig->setGeneratorId(i.key(), "screentone");
            KisFilterConfigurationSP defaultGeneratorConfiguration =
                defaultGenerator->defaultConfiguration(resourcesInterface);
            defaultGeneratorConfiguration->setProperty("rotation", i.value());
            defaultGeneratorConfiguration->setProperty("contrast", 50.0);
            filterConfig->setGeneratorConfiguration(i.key(), defaultGeneratorConfiguration);
        } else {
            filterConfig->setGeneratorId(i.key(), "");
        }
        filterConfig->setHardness(i.key(), KisHalftoneFilterConfiguration::defaultHardness());
        filterConfig->setInvert(i.key(), KisHalftoneFilterConfiguration::defaultInvert());
    }
    
    return filterConfig;
}

KisFilterConfigurationSP KisHalftoneFilter::factoryConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    return new KisHalftoneFilterConfiguration("halftone", 1, resourcesInterface);
}

KisConfigWidget *KisHalftoneFilter::createConfigurationWidget(QWidget *parent, const KisPaintDeviceSP dev, bool useForMasks) const
{
    Q_UNUSED(useForMasks);
    return new KisHalftoneConfigWidget(parent, dev);
}

#include "KisHalftoneFilter.moc"
