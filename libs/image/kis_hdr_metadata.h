/*
 *  SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_HDR_METADATA_H
#define KIS_HDR_METADATA_H

#include <QPointF>
#include <KoColorimetryUtils.h>
#include <boost/operators.hpp>

#include "kritaimage_export.h"
/**
 * KisRelativeContentLightLevelInformation is a struct that represents
 * HDR10 metadata. It is similar to the same values in MasteringInfo in
 * KisSurfaceColorimetry.h, though typically relative to the diffuse white.
 */

struct KRITAIMAGE_EXPORT KisRelativeContentLightLevelInformation: public boost::equality_comparable<KisRelativeContentLightLevelInformation> {
    /**
     * MaxContentLightLevel or MaxCLL is the brightest pixel in the frame sequence.
     */
    double maxContentLightLevel = 0.0;

    /**
     * @brief maxFrameAverage
     * MaxFrameAverageLightLevel or MaxFALL is the average of average pixel brightnesses in the frame sequence.
     */
    double maxFrameAverageLightLevel = 0.0;

    enum CalculationType {
        XYZLuminance, ///< Calculate luminance by converting to xyz.
        Rec2020Component, ///< Calculate luminance by converting to rec2020
        RGBComponent ///< Calculate luminance on the RGB components if possible.
    };

    CalculationType type = XYZLuminance;

    bool operator==(const KisRelativeContentLightLevelInformation & other) const;;
};

/**
 * @brief The KisColorVolumeInformation class
 * is a struct that represents the 'mastering' display. It's primary purpose
 * is to provide extra information for gamutmapping.
 */
struct KRITAIMAGE_EXPORT KisColorVolumeInformation: public boost::equality_comparable<KisColorVolumeInformation> {
    KoColorimetryUtils::xy white; ///< xyY location of the whitepoint.
    KoColorimetryUtils::xy red; ///< xyY location of the red colorant.
    KoColorimetryUtils::xy green; ///< xyY location of the green colorant.
    KoColorimetryUtils::xy blue; ///< xyY location of the blue colorant.

    double maxLuminance = 0.0; ///< Maximum screen brightness in cd/m²
    double minLuminance = 0.0; ///< Minimum screen brightness in cd/m²

    bool operator==(const KisColorVolumeInformation & other) const;;

    static const KisColorVolumeInformation BT2100PQ;
    static const KisColorVolumeInformation DCIP3D65;
};

Q_DECLARE_METATYPE(KisRelativeContentLightLevelInformation::CalculationType);

#endif // KIS_HDR_METADATA_H
