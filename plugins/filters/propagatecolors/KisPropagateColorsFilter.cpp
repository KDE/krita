/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <cmath>

#include <filter/kis_filter_category_ids.h>
#include <KoUpdater.h>
#include <kis_filter_configuration.h>
#include <KisGlobalResourcesInterface.h>
#include <KisSequentialIteratorProgress.h>
#include <kis_processing_information.h>
#include <KoColorSpaceRegistry.h>

#include "KisPropagateColorsConfigWidget.h"
#include "KisPropagateColorsFilterConfiguration.h"
#include "KisPropagateColorsFilter.h"

KisPropagateColorsFilter::KisPropagateColorsFilter()
    : KisFilter(KoID(KisPropagateColorsFilterConfiguration::defaultId(),
                     KisPropagateColorsFilterConfiguration::defaultName()),
                FiltersCategoryColorId,
                KisPropagateColorsFilterConfiguration::defaultMenuName())
{
    setSupportsPainting(false);
    setSupportsThreading(false);
    setSupportsAdjustmentLayers(false);
    setSupportsLevelOfDetail(false);
}

template <quint32 DiagonalDistance, bool BoundedExpansion, bool ExpandAlpha>
struct GenericExpansionStrategy
{
    constexpr static quint32 infinity {0xFFFFFF};
    constexpr static quint32 orthogonalDistance {256};
    constexpr static quint32 diagonalDistance {DiagonalDistance};

    KisPaintDeviceSP distanceMap;
    KisPaintDeviceSP sourceDevice;
    const KoColorSpace *sourceDeviceColorSpace;
    const qint32 sourceDevicePixelSize;
    const QRect rect;
    const quint32 expansionAmount;

    GenericExpansionStrategy(KisPaintDeviceSP sourceDevice,
                             const QRect &applyRect,
                             qreal expansionAmount)
        : distanceMap(new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8()))
        , sourceDevice(sourceDevice)
        , sourceDeviceColorSpace(sourceDevice->colorSpace())
        , sourceDevicePixelSize(sourceDeviceColorSpace->pixelSize())
        , rect(applyRect)
        , expansionAmount(static_cast<quint32>(qRound(expansionAmount * 256.0)))
    {}

    void initializePixel(quint32 *distancePixel, quint8 *devicePixel) const
    {
        const quint8 alpha = sourceDeviceColorSpace->opacityU8(devicePixel);
        *distancePixel = alpha == 0 ? infinity : 0;

        if constexpr (ExpandAlpha) {
            if (alpha > 0) {
                sourceDeviceColorSpace->setOpacity(devicePixel, OPACITY_OPAQUE_U8, 1);
            }
        }
    }

    void setPixel(quint32 *distancePixel, quint8 *devicePixel,
                  quint32 distance, const quint8 *neighborDevicePixel) const
    {
        if (distance >= *distancePixel) {
            return;
        }
        if constexpr (BoundedExpansion) {
            if (distance > expansionAmount) {
                return;
            }
        }

        *distancePixel = distance;
        memcpy(devicePixel, neighborDevicePixel, sourceDevicePixelSize);

        if constexpr (!ExpandAlpha) {
            sourceDeviceColorSpace->setOpacity(devicePixel, OPACITY_TRANSPARENT_U8, 1);
        }
    }

    void updatePixel(quint32 *distancePixel, quint8 *devicePixel,
                     const quint32 *neighborDistancePixel, const quint8 *neighborDevicePixel,
                     quint32 relativeDistance) const
    {
        const quint32 newDistance = *neighborDistancePixel + relativeDistance;
        setPixel(distancePixel, devicePixel, newDistance, neighborDevicePixel);
    }

    void updatePixel(quint32 *distancePixel, quint8 *devicePixel,
                     const quint32 *neighborDistancePixel1, const quint8 *neighborDevicePixel1,
                     const quint32 *neighborDistancePixel2, const quint8 *neighborDevicePixel2,
                     quint32 relativeDistance1, quint32 relativeDistance2) const
    {
        const quint32 newDistance1 = *neighborDistancePixel1 + relativeDistance1;
        const quint32 newDistance2 = *neighborDistancePixel2 + relativeDistance2;

        if (newDistance1 < newDistance2) {
            setPixel(distancePixel, devicePixel, newDistance1, neighborDevicePixel1);
        } else {
            setPixel(distancePixel, devicePixel, newDistance2, neighborDevicePixel2);
        }
    }

    void updatePixel(quint32 *distancePixel, quint8 *devicePixel,
                     const quint32 *neighborDistancePixel1, const quint8 *neighborDevicePixel1,
                     const quint32 *neighborDistancePixel2, const quint8 *neighborDevicePixel2,
                     const quint32 *neighborDistancePixel3, const quint8 *neighborDevicePixel3,
                     quint32 relativeDistance1, quint32 relativeDistance2, quint32 relativeDistance3) const
    {
        const quint32 newDistance1 = *neighborDistancePixel1 + relativeDistance1;
        const quint32 newDistance2 = *neighborDistancePixel2 + relativeDistance2;
        const quint32 newDistance3 = *neighborDistancePixel3 + relativeDistance3;

        if (newDistance1 < newDistance2) {
            if (newDistance1 < newDistance3) {
                setPixel(distancePixel, devicePixel, newDistance1, neighborDevicePixel1);
            } else {
                setPixel(distancePixel, devicePixel, newDistance3, neighborDevicePixel3);
            }
        } else {
            if (newDistance2 < newDistance3) {
                setPixel(distancePixel, devicePixel, newDistance2, neighborDevicePixel2);
            } else {
                setPixel(distancePixel, devicePixel, newDistance3, neighborDevicePixel3);
            }
        }
    }

    void updatePixel(quint32 *distancePixel, quint8 *devicePixel,
                     const quint32 *neighborDistancePixel1, const quint8 *neighborDevicePixel1,
                     const quint32 *neighborDistancePixel2, const quint8 *neighborDevicePixel2,
                     const quint32 *neighborDistancePixel3, const quint8 *neighborDevicePixel3,
                     const quint32 *neighborDistancePixel4, const quint8 *neighborDevicePixel4,
                     quint32 relativeDistance1, quint32 relativeDistance2,
                     quint32 relativeDistance3, quint32 relativeDistance4) const
    {
        const quint32 newDistance1 = *neighborDistancePixel1 + relativeDistance1;
        const quint32 newDistance2 = *neighborDistancePixel2 + relativeDistance2;
        const quint32 newDistance3 = *neighborDistancePixel3 + relativeDistance3;
        const quint32 newDistance4 = *neighborDistancePixel4 + relativeDistance4;

        if (newDistance1 < newDistance2) {
            if (newDistance1 < newDistance3) {
                if (newDistance1 < newDistance4) {
                    setPixel(distancePixel, devicePixel, newDistance1, neighborDevicePixel1);
                } else {
                    setPixel(distancePixel, devicePixel, newDistance4, neighborDevicePixel4);
                }
            } else {
                if (newDistance3 < newDistance4) {
                    setPixel(distancePixel, devicePixel, newDistance3, neighborDevicePixel3);
                } else {
                    setPixel(distancePixel, devicePixel, newDistance4, neighborDevicePixel4);
                }
            }
        } else {
            if (newDistance2 < newDistance3) {
                if (newDistance2 < newDistance4) {
                    setPixel(distancePixel, devicePixel, newDistance2, neighborDevicePixel2);
                } else {
                    setPixel(distancePixel, devicePixel, newDistance4, neighborDevicePixel4);
                }
            } else {
                if (newDistance3 < newDistance4) {
                    setPixel(distancePixel, devicePixel, newDistance3, neighborDevicePixel3);
                } else {
                    setPixel(distancePixel, devicePixel, newDistance4, neighborDevicePixel4);
                }
            }
        }
    }
};

static void swapRowPointers(quint8 **distanceMapRow1,
                            quint8 **distanceMapRow2,
                            quint8 **deviceRow1,
                            quint8 **deviceRow2)
{
    quint8 *temp = *distanceMapRow1;
    *distanceMapRow1 = *distanceMapRow2;
    *distanceMapRow2 = temp;
    temp = *deviceRow1;
    *deviceRow1 = *deviceRow2;
    *deviceRow2 = temp;
}

struct ScaledProgress
{
    constexpr static qint32 maximumProgress {100 << 16};
    constexpr static qint32 scaledHalfUnit {0x7FFF};
    qint32 scaledProgress;
    const qint32 progressStep;
    KoUpdater *progressUpdater;

    ScaledProgress(qint32 steps, KoUpdater *progressUpdater)
        : scaledProgress(0)
        , progressStep(maximumProgress / steps)
        , progressUpdater(progressUpdater)
    {}

    void stepUp()
    {
        scaledProgress += progressStep;
        progressUpdater->setProgress((scaledProgress >> 16) + ((scaledProgress & 0xFFFF) > scaledHalfUnit));
    }

    void complete()
    {
        scaledProgress = maximumProgress;
        progressUpdater->setProgress(100);
    }
};

template <typename ExpansionStrategy>
void expand(ExpansionStrategy &expansionStrategy, KoUpdater *progressUpdater)
{
    ScaledProgress progress(expansionStrategy.rect.height() * 2, progressUpdater);

    QVector<quint8> distanceMapRows(expansionStrategy.rect.width() * 4 * 2);
    QVector<quint8> deviceRows(expansionStrategy.rect.width() * expansionStrategy.sourceDevicePixelSize * 2);
    quint8 *distanceMapRow1, *distanceMapRow2;
    quint8 *deviceRow1, *deviceRow2;

    // Forwards pass
    distanceMapRow1 = distanceMapRows.data();
    distanceMapRow2 = distanceMapRows.data() + expansionStrategy.rect.width() * 4;
    deviceRow1 = deviceRows.data();
    deviceRow2 = deviceRows.data() + expansionStrategy.rect.width() * expansionStrategy.sourceDevicePixelSize;
    // Top row
    expansionStrategy.distanceMap->readBytes(distanceMapRow1, expansionStrategy.rect.left(),
                                             expansionStrategy.rect.top(), expansionStrategy.rect.width(), 1);
    expansionStrategy.sourceDevice->readBytes(deviceRow1, expansionStrategy.rect.left(),
                                              expansionStrategy.rect.top(), expansionStrategy.rect.width(), 1);
    // Left pixel
    expansionStrategy.initializePixel(reinterpret_cast<quint32*>(distanceMapRow1), deviceRow1);
    // Middle pixels
    {
        quint32 *distancePixel = reinterpret_cast<quint32*>(distanceMapRow1) + 1;
        quint8 *devicePixel = deviceRow1 + expansionStrategy.sourceDevicePixelSize;
        for (qint32 x = 1; x < expansionStrategy.rect.width();
                ++x, ++distancePixel, devicePixel += expansionStrategy.sourceDevicePixelSize) {

            expansionStrategy.initializePixel(distancePixel, devicePixel);
            if (*distancePixel == 0) {
                continue;
            }
            expansionStrategy.updatePixel(
                distancePixel, devicePixel,
                distancePixel - 1, devicePixel - expansionStrategy.sourceDevicePixelSize,
                expansionStrategy.orthogonalDistance
            );
        }
    }
    expansionStrategy.distanceMap->writeBytes(distanceMapRow1, expansionStrategy.rect.left(),
                                              expansionStrategy.rect.top(), expansionStrategy.rect.width(), 1);
    expansionStrategy.sourceDevice->writeBytes(deviceRow1, expansionStrategy.rect.left(),
                                               expansionStrategy.rect.top(), expansionStrategy.rect.width(), 1);
    progress.stepUp();
    // Rest of rows
    for (qint32 y = expansionStrategy.rect.top() + 1; y <= expansionStrategy.rect.bottom(); ++y) {
        expansionStrategy.distanceMap->readBytes(distanceMapRow2, expansionStrategy.rect.left(),
                                                 y, expansionStrategy.rect.width(), 1);
        expansionStrategy.sourceDevice->readBytes(deviceRow2, expansionStrategy.rect.left(), y,
                                                  expansionStrategy.rect.width(), 1);

        quint32 *topDistancePixel = reinterpret_cast<quint32*>(distanceMapRow1);
        quint32 *distancePixel = reinterpret_cast<quint32*>(distanceMapRow2);
        quint8 *topDevicePixel = deviceRow1;
        quint8 *devicePixel = deviceRow2;
        // Left pixel
        {
            expansionStrategy.initializePixel(distancePixel, devicePixel);
            if (*distancePixel != 0) {
                if (expansionStrategy.rect.width() > 1) {
                    expansionStrategy.updatePixel(
                        distancePixel, devicePixel,
                        topDistancePixel, topDevicePixel,
                        topDistancePixel + 1, topDevicePixel + expansionStrategy.sourceDevicePixelSize,
                        expansionStrategy.orthogonalDistance, expansionStrategy.diagonalDistance
                    );
                } else {
                    expansionStrategy.updatePixel(
                        distancePixel, devicePixel,
                        topDistancePixel, topDevicePixel,
                        expansionStrategy.orthogonalDistance
                    );
                }
            }
            ++topDistancePixel;
            ++distancePixel;
            topDevicePixel += expansionStrategy.sourceDevicePixelSize;
            devicePixel += expansionStrategy.sourceDevicePixelSize;
        }
        // Middle pixels
        for (qint32 x = 1; x < expansionStrategy.rect.width() - 1;
                ++x, ++topDistancePixel, ++distancePixel,
                topDevicePixel += expansionStrategy.sourceDevicePixelSize,
                devicePixel += expansionStrategy.sourceDevicePixelSize) {

            expansionStrategy.initializePixel(distancePixel, devicePixel);
            if (*distancePixel == 0) {
                continue;
            }
            expansionStrategy.updatePixel(
                distancePixel, devicePixel,
                topDistancePixel - 1,topDevicePixel - expansionStrategy.sourceDevicePixelSize,
                topDistancePixel, topDevicePixel,
                topDistancePixel + 1, topDevicePixel + expansionStrategy.sourceDevicePixelSize,
                distancePixel - 1, devicePixel - expansionStrategy.sourceDevicePixelSize,
                expansionStrategy.diagonalDistance, expansionStrategy.orthogonalDistance,
                expansionStrategy.diagonalDistance, expansionStrategy.orthogonalDistance
            );
        }
        // Right pixel
        expansionStrategy.initializePixel(distancePixel, devicePixel);
        if (expansionStrategy.rect.width() > 1 && *distancePixel != 0) {
            expansionStrategy.updatePixel(
                distancePixel, devicePixel,
                topDistancePixel - 1, topDevicePixel - expansionStrategy.sourceDevicePixelSize,
                distancePixel - 1, devicePixel - expansionStrategy.sourceDevicePixelSize,
                topDistancePixel, topDevicePixel,
                expansionStrategy.diagonalDistance, expansionStrategy.orthogonalDistance,
                expansionStrategy.orthogonalDistance
            );
        }
        // Write new pixels
        expansionStrategy.distanceMap->writeBytes(distanceMapRow2, expansionStrategy.rect.left(),
                                                  y, expansionStrategy.rect.width(), 1);
        expansionStrategy.sourceDevice->writeBytes(deviceRow2, expansionStrategy.rect.left(),
                                                   y, expansionStrategy.rect.width(), 1);
        // Swap pointers
        swapRowPointers(&distanceMapRow1, &distanceMapRow2, &deviceRow1, &deviceRow2);

        progress.stepUp();
    }

    // Backwards pass
    // Bottom row
    // Right pixel (no op)
    // Middle pixels
    {
        quint32 *distancePixel = reinterpret_cast<quint32*>(distanceMapRow1) + expansionStrategy.rect.width() - 2;
        quint8 *devicePixel =
            deviceRow1 + expansionStrategy.sourceDevicePixelSize * (expansionStrategy.rect.width() - 2);
        for (qint32 x = 1; x < expansionStrategy.rect.width();
                ++x, --distancePixel, devicePixel -= expansionStrategy.sourceDevicePixelSize) {

            if (*distancePixel == 0) {
                continue;
            }
            expansionStrategy.updatePixel(
                distancePixel, devicePixel,
                distancePixel + 1, devicePixel + expansionStrategy.sourceDevicePixelSize,
                expansionStrategy.orthogonalDistance
            );
        }
    }
    expansionStrategy.distanceMap->writeBytes(distanceMapRow1, expansionStrategy.rect.left(),
                                              expansionStrategy.rect.bottom(), expansionStrategy.rect.width(), 1);
    expansionStrategy.sourceDevice->writeBytes(deviceRow1, expansionStrategy.rect.left(),
                                               expansionStrategy.rect.bottom(), expansionStrategy.rect.width(), 1);
    progress.stepUp();
    // Rest of rows
    for (qint32 y = expansionStrategy.rect.bottom() - 1; y >= expansionStrategy.rect.top(); --y) {
        expansionStrategy.distanceMap->readBytes(distanceMapRow2, expansionStrategy.rect.left(),
                                                 y, expansionStrategy.rect.width(), 1);
        expansionStrategy.sourceDevice->readBytes(deviceRow2, expansionStrategy.rect.left(),
                                                  y, expansionStrategy.rect.width(), 1);

        quint32 *bottomDistancePixel =
            reinterpret_cast<quint32*>(distanceMapRow1) + expansionStrategy.rect.width() - 1;
        quint32 *distancePixel =
            reinterpret_cast<quint32*>(distanceMapRow2) + expansionStrategy.rect.width() - 1;
        quint8 *bottomDevicePixel =
            deviceRow1 + expansionStrategy.sourceDevicePixelSize * (expansionStrategy.rect.width() - 1);
        quint8 *devicePixel =
            deviceRow2 + expansionStrategy.sourceDevicePixelSize * (expansionStrategy.rect.width() - 1);
        // Right pixel
        {
            if (*distancePixel != 0) {
                if (expansionStrategy.rect.width() > 1) {
                    expansionStrategy.updatePixel(
                        distancePixel, devicePixel,
                        bottomDistancePixel, bottomDevicePixel,
                        bottomDistancePixel - 1, bottomDevicePixel - expansionStrategy.sourceDevicePixelSize,
                        expansionStrategy.orthogonalDistance, expansionStrategy.diagonalDistance
                    );
                } else {
                    expansionStrategy.updatePixel(
                        distancePixel, devicePixel,
                        bottomDistancePixel, bottomDevicePixel,
                        expansionStrategy.orthogonalDistance
                    );
                }
            }
            --bottomDistancePixel;
            --distancePixel;
            bottomDevicePixel -= expansionStrategy.sourceDevicePixelSize;
            devicePixel -= expansionStrategy.sourceDevicePixelSize;
        }
        // Middle pixels
        for (qint32 x = 1; x < expansionStrategy.rect.width() - 1;
                ++x, --bottomDistancePixel, --distancePixel,
                bottomDevicePixel -= expansionStrategy.sourceDevicePixelSize,
                devicePixel -= expansionStrategy.sourceDevicePixelSize) {

            if (*distancePixel == 0) {
                continue;
            }
            expansionStrategy.updatePixel(
                distancePixel, devicePixel,
                bottomDistancePixel + 1, bottomDevicePixel + expansionStrategy.sourceDevicePixelSize,
                bottomDistancePixel, bottomDevicePixel,
                bottomDistancePixel - 1, bottomDevicePixel - expansionStrategy.sourceDevicePixelSize,
                distancePixel + 1, devicePixel + expansionStrategy.sourceDevicePixelSize,
                expansionStrategy.diagonalDistance, expansionStrategy.orthogonalDistance,
                expansionStrategy.diagonalDistance, expansionStrategy.orthogonalDistance
            );
        }
        // Left pixel
        if (expansionStrategy.rect.width() > 1 && *distancePixel != 0) {
            expansionStrategy.updatePixel(
                distancePixel, devicePixel,
                bottomDistancePixel + 1, bottomDevicePixel + expansionStrategy.sourceDevicePixelSize,
                bottomDistancePixel, bottomDevicePixel,
                distancePixel + 1, devicePixel + expansionStrategy.sourceDevicePixelSize,
                expansionStrategy.diagonalDistance, expansionStrategy.orthogonalDistance,
                expansionStrategy.orthogonalDistance
            );
        }
        // Write new pixels
        expansionStrategy.distanceMap->writeBytes(distanceMapRow2, expansionStrategy.rect.left(),
                                                  y, expansionStrategy.rect.width(), 1);
        expansionStrategy.sourceDevice->writeBytes(deviceRow2, expansionStrategy.rect.left(), y,
                                                   expansionStrategy.rect.width(), 1);
        // Swap pointers
        swapRowPointers(&distanceMapRow1, &distanceMapRow2, &deviceRow1, &deviceRow2);

        progress.stepUp();
    }

    progress.complete();
}

template <quint32 DiagonalDistance, bool BoundedExpansion>
void selectStrategyAndExpand2(KisPaintDeviceSP device,
                              const QRect &applyRect,
                              const KisPropagateColorsFilterConfiguration *filterConfig,
                              KoUpdater *progressUpdater)
{
    const KisPropagateColorsFilterConfiguration::AlphaChannelMode alphaChannelMode = filterConfig->alphaChannelMode();
    
    if (alphaChannelMode == KisPropagateColorsFilterConfiguration::AlphaChannelMode_Preserve) {
        GenericExpansionStrategy<DiagonalDistance, BoundedExpansion, false> expansionStrategy(
            device, applyRect, filterConfig->expansionAmount()
        );
        expand(expansionStrategy, progressUpdater);
    } else {
        GenericExpansionStrategy<DiagonalDistance, BoundedExpansion, true> expansionStrategy(
            device, applyRect, filterConfig->expansionAmount()
        );
        expand(expansionStrategy, progressUpdater);
    }
}

template <quint32 DiagonalDistance>
void selectStrategyAndExpand1(KisPaintDeviceSP device,
                              const QRect &applyRect,
                              const KisPropagateColorsFilterConfiguration *filterConfig,
                              KoUpdater *progressUpdater)
{
    const KisPropagateColorsFilterConfiguration::ExpansionMode expansionMode = filterConfig->expansionMode();
    
    if (expansionMode == KisPropagateColorsFilterConfiguration::ExpansionMode_Bounded) {
        selectStrategyAndExpand2<DiagonalDistance, true>(device, applyRect, filterConfig, progressUpdater);
    } else {
        selectStrategyAndExpand2<DiagonalDistance, false>(device, applyRect, filterConfig, progressUpdater);
    }
}

void KisPropagateColorsFilter::processImpl(KisPaintDeviceSP device,
                                           const QRect &applyRect,
                                           const KisFilterConfigurationSP config,
                                           KoUpdater *progressUpdater) const
{
    const KisPropagateColorsFilterConfiguration *filterConfig =
            dynamic_cast<const KisPropagateColorsFilterConfiguration*>(config.data());

    Q_ASSERT(device);
    KIS_SAFE_ASSERT_RECOVER_RETURN(filterConfig);

    const KisPropagateColorsFilterConfiguration::DistanceMetric distanceMetric = filterConfig->distanceMetric();

    if (distanceMetric == KisPropagateColorsFilterConfiguration::DistanceMetric_Chessboard) {
        selectStrategyAndExpand1<256>(device, applyRect, filterConfig, progressUpdater);
    } else if (distanceMetric == KisPropagateColorsFilterConfiguration::DistanceMetric_CityBlock) {
        selectStrategyAndExpand1<512>(device, applyRect, filterConfig, progressUpdater);
    } else {
        // 362 is sqrt(2) * 256, rounded
        selectStrategyAndExpand1<362>(device, applyRect, filterConfig, progressUpdater);
    }
}

KisFilterConfigurationSP KisPropagateColorsFilter::factoryConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    return new KisPropagateColorsFilterConfiguration(resourcesInterface);
}

KisConfigWidget *KisPropagateColorsFilter::createConfigurationWidget(QWidget *parent,
                                                                     const KisPaintDeviceSP,
                                                                     bool) const
{
    return new KisPropagateColorsConfigWidget(parent);
}

bool KisPropagateColorsFilter::needsTransparentPixels(const KisFilterConfigurationSP config, const KoColorSpace*) const
{
    const KisPropagateColorsFilterConfiguration *filterConfig =
            dynamic_cast<const KisPropagateColorsFilterConfiguration*>(config.data());

    return filterConfig->expansionMode() == KisPropagateColorsFilterConfiguration::ExpansionMode_Unbounded;
}

QRect KisPropagateColorsFilter::neededRect(const QRect &rect, const KisFilterConfigurationSP config, int) const
{
    const KisPropagateColorsFilterConfiguration *filterConfig =
            dynamic_cast<const KisPropagateColorsFilterConfiguration*>(config.data());

    if (filterConfig->expansionMode() == KisPropagateColorsFilterConfiguration::ExpansionMode_Unbounded) {
        return rect;
    }

    const qint32 expansionAmount = static_cast<qint32>(std::ceil(filterConfig->expansionAmount()));

    return rect.adjusted(-expansionAmount, -expansionAmount, expansionAmount, expansionAmount);
}

QRect KisPropagateColorsFilter::changedRect(const QRect &rect, const KisFilterConfigurationSP config, int lod) const
{
    return neededRect(rect, config, lod);
}
