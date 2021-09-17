/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <cmath>

#include <kis_histogram.h>

#include "KisAutoLevels.h"

namespace KisAutoLevels
{

QPair<qreal, qreal> getMeanAndMedian(ChannelHistogram histogram, qreal begin, qreal end)
{
    Q_ASSERT(histogram.histogram);
    Q_ASSERT(histogram.channel >= 0 && histogram.channel < histogram.histogram->producer()->channels().size());

    histogram.histogram->setChannel(histogram.channel);

    const int numberOfBins = histogram.histogram->producer()->numberOfBins();
    const int beginBin = static_cast<int>(begin * static_cast<qreal>(numberOfBins));
    const int endBin = static_cast<int>(end * static_cast<qreal>(numberOfBins));

    qreal numberOfSamples = 0.0;
    qreal meanSum = 0.0;

    for (int i = beginBin; i < endBin; ++i) {
        const qreal current = static_cast<qreal>(histogram.histogram->getValue(i));
        numberOfSamples += current;
        meanSum += current * static_cast<qreal>(i);
    }

    const qreal mean = meanSum / (numberOfSamples * static_cast<qreal>(numberOfBins));
    qreal probabilityAccumulator = 0.0;
    qreal median = 0.0;

    for (int i = beginBin; i < endBin; ++i) {
        const qreal probability = static_cast<qreal>(histogram.histogram->getValue(i)) / numberOfSamples;
        probabilityAccumulator += probability;
        if (probabilityAccumulator >= 0.5) {
            median = static_cast<qreal>(i) / static_cast<qreal>(numberOfBins);
            break;
        }
    }

    return {mean, median};
}

QPair<qreal, qreal> getInputBlackAndWhitePoints(ChannelHistogram histogram,
                                                qreal shadowsClipping,
                                                qreal highlightsClipping)
{
    Q_ASSERT(histogram.histogram);
    Q_ASSERT(histogram.channel >= 0 && histogram.channel < histogram.histogram->producer()->channels().size());

    histogram.histogram->setChannel(histogram.channel);

    int numberOfBins = histogram.histogram->producer()->numberOfBins();

    Q_ASSERT(numberOfBins > 1);

    const qreal totalNumberOfSamples = static_cast<qreal>(histogram.histogram->producer()->count());

    // This basically integrates the probability mass function given by the
    // histogram, from the left and the right, until the thresholds given by the
    // clipping are reached, to obtain the black and white points

    int blackPoint = 0;
    qreal accumulator = 0.0;
    for (int i = 0; i < numberOfBins; ++i) {
        const qreal sampleCountForBin = static_cast<qreal>(histogram.histogram->getValue(i));
        const qreal probability = sampleCountForBin / totalNumberOfSamples;

        accumulator += probability;
        if (accumulator > shadowsClipping) {
            break;
        }
        blackPoint = i;
    }

    int whitePoint = numberOfBins - 1;
    accumulator = 0.0;
    for (int i = numberOfBins - 1; i >= 0; --i) {
        const qreal sampleCountForBin = static_cast<qreal>(histogram.histogram->getValue(i));
        const qreal probability = sampleCountForBin / totalNumberOfSamples;

        accumulator += probability;
        if (accumulator > highlightsClipping) {
            break;
        }
        whitePoint = i;
    }

    if (whitePoint <= blackPoint) {
        if (blackPoint + 1 == numberOfBins) {
            whitePoint = blackPoint;
            --blackPoint;
        } else {
            whitePoint = blackPoint + 1;
        }
    }

    return
        {
            static_cast<qreal>(blackPoint) / static_cast<qreal>(numberOfBins),
            static_cast<qreal>(whitePoint) / static_cast<qreal>(numberOfBins)
        };
}

QPair<KoColor, KoColor> getDarkestAndWhitestColors(const KisPaintDeviceSP device,
                                                   qreal shadowsClipping,
                                                   qreal highlightsClipping);

qreal getGamma(qreal blackPoint, qreal whitePoint, qreal inputIntensity, qreal outputIntensity)
{
    Q_ASSERT(blackPoint < whitePoint);
    Q_ASSERT(inputIntensity > blackPoint && inputIntensity < whitePoint);
    
    if (qFuzzyIsNull(outputIntensity)) {
        return 0.01;
    }
    if (qFuzzyCompare(outputIntensity, 1.0)) {
        return 10.0;
    }

    const qreal inputIntensityAfterLinearMapping =
        (inputIntensity - blackPoint) / (whitePoint - blackPoint);

    return qBound(0.01, log(inputIntensityAfterLinearMapping) / log(outputIntensity), 10.0);
}


QVector<KisLevelsCurve> adjustMonochromaticContrast(ChannelHistogram lightnessHistogram,
                                                   QVector<ChannelHistogram> &channelsHistograms,
                                                   qreal shadowsClipping,
                                                   qreal highlightsClipping,
                                                   qreal maximumInputBlackAndWhiteOffset,
                                                   MidtonesAdjustmentMethod midtonesAdjustmentMethod,
                                                   qreal midtonesAdjustmentAmount,
                                                   const QVector<qreal> &outputBlackPoints,
                                                   const QVector<qreal> &outputWhitePoints,
                                                   const QVector<qreal> &outputMidtones)
{
    Q_ASSERT(lightnessHistogram.histogram);
    Q_ASSERT(lightnessHistogram.channel >= 0);
    Q_ASSERT(lightnessHistogram.channel < lightnessHistogram.histogram->producer()->channels().size());
    Q_ASSERT(outputBlackPoints.size() == channelsHistograms.size());
    Q_ASSERT(outputWhitePoints.size() == channelsHistograms.size());
    Q_ASSERT(outputMidtones.size() == channelsHistograms.size());

    QVector<KisLevelsCurve> levelsCurves;

    const QPair<qreal, qreal> inputBlackAndWhitePoints =
        getInputBlackAndWhitePoints(lightnessHistogram, shadowsClipping, highlightsClipping);
    const qreal inputBlackPoint = qMin(maximumInputBlackAndWhiteOffset, inputBlackAndWhitePoints.first);
    const qreal inputWhitePoint = qMax(1.0 - maximumInputBlackAndWhiteOffset, inputBlackAndWhitePoints.second);
    const qreal linearMappingMidPoint = (inputBlackPoint + inputWhitePoint) / 2.0;

    for (int i = 0; i < channelsHistograms.size(); ++i) {
        ChannelHistogram &channelHistogram = channelsHistograms[i];
            
        qreal gamma = 1.0;
        if (midtonesAdjustmentMethod != MidtonesAdjustmentMethod_None &&
            channelHistogram.histogram &&
            channelHistogram.channel >= 0 &&
            channelHistogram.channel < channelHistogram.histogram->producer()->channels().size()) {

            qreal inputIntensity;
            QPair<qreal, qreal> meanAndMedian = getMeanAndMedian(channelHistogram, inputBlackPoint, inputWhitePoint);

            if (midtonesAdjustmentMethod == MidtonesAdjustmentMethod_UseMean) {
                inputIntensity = meanAndMedian.first;
            } else {
                inputIntensity = meanAndMedian.second;
            }

            inputIntensity = linearMappingMidPoint + (inputIntensity - linearMappingMidPoint) * midtonesAdjustmentAmount;
            gamma = getGamma(inputBlackPoint, inputWhitePoint, inputIntensity, outputMidtones[i]);
        }

        levelsCurves.append(
            KisLevelsCurve(
                inputBlackPoint,
                inputWhitePoint,
                gamma,
                outputBlackPoints[i],
                outputWhitePoints[i]
            )
        );
    }

    return levelsCurves;
}

QVector<KisLevelsCurve> adjustPerChannelContrast(QVector<ChannelHistogram> &channelsHistograms,
                                                qreal shadowsClipping,
                                                qreal highlightsClipping,
                                                qreal maximumInputBlackAndWhiteOffset,
                                                MidtonesAdjustmentMethod midtonesAdjustmentMethod,
                                                qreal midtonesAdjustmentAmount,
                                                const QVector<qreal> &outputBlackPoints,
                                                const QVector<qreal> &outputWhitePoints,
                                                const QVector<qreal> &outputMidtones)
{
    QVector<KisLevelsCurve> levelsCurves;

    for (int i = 0; i < channelsHistograms.size(); ++i) {
        QVector<ChannelHistogram> channelHistogram{channelsHistograms[i]};
        levelsCurves.append(
            adjustMonochromaticContrast(
                channelHistogram[0],
                channelHistogram,
                shadowsClipping,
                highlightsClipping,
                maximumInputBlackAndWhiteOffset,
                midtonesAdjustmentMethod,
                midtonesAdjustmentAmount,
                {outputBlackPoints[i]},
                {outputWhitePoints[i]},
                {outputMidtones[i]}
            )
        );
    }

    return levelsCurves;
}

}
