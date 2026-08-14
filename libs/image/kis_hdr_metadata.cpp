/*
 *  SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <kis_hdr_metadata.h>
#include <KisStaticInitializer.h>

KIS_DECLARE_STATIC_INITIALIZER {
    qRegisterMetaType<KisRelativeContentLightLevelInformation::CalculationType>();
}

bool KisRelativeContentLightLevelInformation::operator==(const KisRelativeContentLightLevelInformation &other) const {
    return qFuzzyCompare(maxContentLightLevel, other.maxContentLightLevel)
    && qFuzzyCompare(maxFrameAverageLightLevel, other.maxFrameAverageLightLevel)
        && (type == other.type);
}

bool KisColorVolumeInformation::operator==(const KisColorVolumeInformation &other) const {
    return qFuzzyCompare(maxLuminance, other.maxLuminance)
    && qFuzzyCompare(minLuminance, other.minLuminance)
        && white == other.white && red == other.red
                                             && green == other.green && blue == other.blue;
}

const KisColorVolumeInformation KisColorVolumeInformation::BT2100PQ = {
    {}, // for boost::equality_comparable :(
    KoColorimetryUtils::Colorimetry::BT2020.white().toxy(),
    KoColorimetryUtils::Colorimetry::BT2020.red().toxy(),
    KoColorimetryUtils::Colorimetry::BT2020.green().toxy(),
    KoColorimetryUtils::Colorimetry::BT2020.blue().toxy(),
    1000.0,
    0.005
};
const KisColorVolumeInformation KisColorVolumeInformation::DCIP3D65 = {
    {}, // for boost::equality_comparable :(
    KoColorimetryUtils::Colorimetry::DCIP3.white().toxy(),
    KoColorimetryUtils::Colorimetry::DCIP3.red().toxy(),
    KoColorimetryUtils::Colorimetry::DCIP3.green().toxy(),
    KoColorimetryUtils::Colorimetry::DCIP3.blue().toxy(),
    1000.0,
    0.005
};