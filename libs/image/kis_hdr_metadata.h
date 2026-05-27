/*
 *  SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_HDR_METADATA_H
#define KIS_HDR_METADATA_H

#include <QPointF>
#include <boost/operators.hpp>
/**
 * KisContentLightLevelInformation is a struct that represents
 * HDR10 metadata. It is similar to the same values in MasteringInfo in
 * KisSurfaceColorimetry.h
 */

struct KisContentLightLevelInformation: public boost::equality_comparable<KisContentLightLevelInformation> {
    /**
     * MaxContentLightLevel or MaxCLL is the brightest pixel in the frame sequence.
     */
    double maxContentLightLevel = 0.0;

    /**
     * @brief maxFrameAverage
     * MaxFrameAverageLightLevel or MaxFALL is the average of average pixel brightnesses in the frame sequence.
     */
    double maxFrameAverageLightLevel = 0.0;

    bool operator==(const KisContentLightLevelInformation & other) const {
        return qFuzzyCompare(maxContentLightLevel, other.maxContentLightLevel)
            && qFuzzyCompare(maxFrameAverageLightLevel, other.maxFrameAverageLightLevel);
    };

    enum CalculationType {
        XYZLuminance, ///< Calculate luminance by converting to xyz.
        Rec2020Component, ///< Calculate luminance by converting to rec2020
        RGBComponent ///< Calculate luminance on the RGB components if possible.
    };
};

/**
 * @brief The KisColorVolumeInformation class
 * is a struct that represents the 'mastering' display. It's primary purpose
 * is to provide extra information for gamutmapping.
 */
struct KisColorVolumeInformation: public boost::equality_comparable<KisColorVolumeInformation> {
    QPointF white; ///< xyY location of the whitepoint.
    QPointF red; ///< xyY location of the red colorant.
    QPointF green; ///< xyY location of the green colorant.
    QPointF blue; ///< xyY location of the blue colorant.

    double maxLuminance = 0.0; ///< Maximum screen brightness in cd/m²
    double minLuminance = 0.0; ///< Minimum screen brightness in cd/m²

    bool operator==(const KisColorVolumeInformation & other) const {
        return qFuzzyCompare(maxLuminance, other.maxLuminance)
        && qFuzzyCompare(minLuminance, other.minLuminance)
            && white == other.white && red == other.red
            && green == other.green && blue == other.blue;
    };
};

#endif // KIS_HDR_METADATA_H
