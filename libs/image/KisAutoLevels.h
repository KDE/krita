/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_AUTO_LEVELS_H
#define KIS_AUTO_LEVELS_H

#include <QVector>

#include <KoColor.h>
#include <kis_paint_device.h>
#include <KisLevelsCurve.h>
#include <kritaimage_export.h>

class KisHistogram;

/**
 * @brief This namespace contains functions to  compute the levels adjustment
 * parameters automatically from a histogram
 */
namespace KisAutoLevels
{

/**
 * @brief The different methods to enhance the contrast
 */
enum KRITAIMAGE_EXPORT ShadowsAndHighlightsAdjustmentMethod
{
    ShadowsAndHighlightsAdjustmentMethod_MonochromaticContrast,
    ShadowsAndHighlightsAdjustmentMethod_PerChannelContrast
};

/**
 * @brief The different methods to enhance the mid tones
 */
enum KRITAIMAGE_EXPORT MidtonesAdjustmentMethod
{
    MidtonesAdjustmentMethod_None,
    MidtonesAdjustmentMethod_UseMedian,
    MidtonesAdjustmentMethod_UseMean
};

/**
 * @brief Convenience class that associates a KisHistogram and a channel index.
 * This is useful because setting a channel on the histogram mutates it. This
 * way the functions can change the histogram channel by looking at the
 * "channel" field
 * 
 */
struct KRITAIMAGE_EXPORT ChannelHistogram
{
    KisHistogram *histogram{nullptr};
    int channel{0};
};

/**
 * @brief Takes a reference histogram (luma, lightness) and computes the black
 *        and white points to maximize the contrast having into account
 *        the clipping.
 * @return A pair containing the black and white points indices
 */
QPair<qreal, qreal> KRITAIMAGE_EXPORT getInputBlackAndWhitePoints(ChannelHistogram histogram,
                                                                  qreal shadowsClipping,
                                                                  qreal highlightsClipping);

/**
 * @brief Finds the darkest and whitest colors in the device having into account
 *        the clipping
 * @return A pair containing the "darkest" and "lighter" colors 
 */
QPair<KoColor, KoColor> KRITAIMAGE_EXPORT getDarkestAndWhitestColors(const KisPaintDeviceSP device,
                                                                     qreal shadowsClipping,
                                                                     qreal highlightsClipping);

/**
 * @brief Computes a gamma value that "moves" the input midpoint towards
 *        the output midpoint
 * @param blackPoint If this gamma value will be part of a more complex levels
 *                   adjustment, set its black point here. Set it to 0
 *                   otherwhise. Since the gamma correction is applied after
 *                   the linear mapping given by the black and white points in a
 *                   levels adjustment, you have to provide those here to get
 *                   the correct gamma
 * @param whitePoint same as with blackPoint but for the white point
 * @param inputIntensity This is the intensity value that we want to map to the
 *                       output value. Sensible values are the mean and the median
 * @param outputIntensity This is the intensity value to which the input value
 *                        will be mapped to after the gamma correction. Use 0.5
 *                        to neutralize the midtones
 * @return the gamma value that, when applied after the lineat mapping given by
 *         the black and white points, will map the input intensity to the
 *         output intensity
 */
qreal KRITAIMAGE_EXPORT getGamma(qreal blackPoint,
                                 qreal whitePoint,
                                 qreal inputIntensity,
                                 qreal outputIntensity);

/**
 * @brief Creates a KisLevelsCurve for every channel in "channelsHistograms".
 *        Computes the input black and white points from the intensity histogram.
 *        Computes the gamma from each channels histogram and "outputMidtones"
 *        if the method is not "None". The output black and white points are
 *        computed from "outputBlackPoints" and "outputWhitePoints"
 * @param lightnessHistogram histogram to compute the input black and white
 *                           points from
 * @param channelsHistograms list of histograms to compute the gammas from. This
 *                           is also used to know the number of output levels infos
 * @param shadowsClipping A normalized perentage that is used to know how many
 *                        samples should be clipped on the shadows side of the
 *                        histogram by the input black point
 * @param highlightsClipping A normalized perentage that is used to know how
 *                           many samples should be clipped on the highlights
 *                           side of the histogram by the input white point
 * @param maximumInputBlackAndWhiteOffset A maximum value for the input black
 *                                        and white points. The black point won't
 *                                        be greater than this, and the white
 *                                        point won't be lesser than 1 - this
 * @param midtonesAdjustmentMethod The method used to get the gamma
 * @param midtonesAdjustmentAmount The strength of the midtone adjustment
 * @param outputBlackPoints The output black points used in each levels info
 * @param outputWhitePoints The output white points used in each levels info
 * @param outputMidtones The desired output midtone values for the gamma adjustment
 * @return A list of levels infos containing parameters for the levels adjustment
 */
QVector<KisLevelsCurve> KRITAIMAGE_EXPORT adjustMonochromaticContrast(ChannelHistogram lightnessHistogram,
                                                                      QVector<ChannelHistogram> &channelsHistograms,
                                                                      qreal shadowsClipping,
                                                                      qreal highlightsClipping,
                                                                      qreal maximumInputBlackAndWhiteOffset,
                                                                      MidtonesAdjustmentMethod midtonesAdjustmentMethod,
                                                                      qreal midtonesAdjustmentAmount,
                                                                      const QVector<qreal> &outputBlackPoints,
                                                                      const QVector<qreal> &outputWhitePoints,
                                                                      const QVector<qreal> &outputMidtones);

/**
 * @brief Creates a KisLevelsCurve for every channel in "channelsHistograms".
 *        Computes the input black and white points from each channels
 *        histogram separately. Computes the gamma from each channels histogram
 *        and "outputMidtones" if the method is not "None". The
 *        output black and white points are computed from "outputBlackPoints"
 *        and "outputWhitePoints"
 * @param channelsHistograms list of histograms to compute the gammas from. This
 *                           is also used to know the number of output levels infos
 * @param shadowsClipping A normalized perentage that is used to know how many
 *                        samples should be clipped on the shadows side of the
 *                        histogram by the input black point
 * @param highlightsClipping A normalized perentage that is used to know how
 *                           many samples should be clipped on the highlights
 *                           side of the histogram by the input white point
 * @param maximumInputBlackAndWhiteOffset A maximum value for the input black
 *                                        and white points. The black point won't
 *                                        be greater than this, and the white
 *                                        point won't be lesser than 1 - this
 * @param midtonesAdjustmentMethod The method used to get the gamma
 * @param midtonesAdjustmentAmount The strength of the midtone adjustment
 * @param outputBlackPoints The output black points used in each levels info
 * @param outputWhitePoints The output white points used in each levels info
 * @param outputMidtones The desired output midtone values for the gamma adjustment
 * @return A list of levels infos containing parameters for the levels adjustment
 */
QVector<KisLevelsCurve> KRITAIMAGE_EXPORT adjustPerChannelContrast(QVector<ChannelHistogram> &channelsHistograms,
                                                                   qreal shadowsClipping,
                                                                   qreal highlightsClipping,
                                                                   qreal maximumInputBlackAndWhiteOffset,
                                                                   MidtonesAdjustmentMethod midtonesAdjustmentMethod,
                                                                   qreal midtonesAdjustmentAmount,
                                                                   const QVector<qreal> &outputBlackPoints,
                                                                   const QVector<qreal> &outputWhitePoints,
                                                                   const QVector<qreal> &outputMidtones);

}

#endif
