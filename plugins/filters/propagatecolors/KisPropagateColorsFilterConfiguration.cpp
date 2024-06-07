/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisPropagateColorsFilterConfiguration.h"

KisPropagateColorsFilterConfiguration::KisPropagateColorsFilterConfiguration(KisResourcesInterfaceSP resourcesInterface)
    : KisFilterConfiguration(defaultId(), defaultVersion(), resourcesInterface)
{
    setDefaults();
}

KisPropagateColorsFilterConfiguration::KisPropagateColorsFilterConfiguration(
    const KisPropagateColorsFilterConfiguration &rhs
)
    : KisFilterConfiguration(rhs)
{}

KisPropagateColorsFilterConfiguration::~KisPropagateColorsFilterConfiguration()
{}

KisFilterConfigurationSP KisPropagateColorsFilterConfiguration::clone() const
{
    return new KisPropagateColorsFilterConfiguration(*this);
}

KisPropagateColorsFilterConfiguration::DistanceMetric KisPropagateColorsFilterConfiguration::distanceMetric() const
{
    const QString distanceMetricStr = getString("distanceMetric", "");
    if (distanceMetricStr == "chessboard") {
        return DistanceMetric_Chessboard;
    } else if (distanceMetricStr == "cityBlock") {
        return DistanceMetric_CityBlock;
    } else if (distanceMetricStr == "euclidean") {
        return DistanceMetric_Euclidean;
    }
    return defaultDistanceMetric();
}

KisPropagateColorsFilterConfiguration::ExpansionMode KisPropagateColorsFilterConfiguration::expansionMode() const
{
    const QString expansionModeStr = getString("expansionMode", "");
    if (expansionModeStr == "bounded") {
        return ExpansionMode_Bounded;
    } else if (expansionModeStr == "unbounded") {
        return ExpansionMode_Unbounded;
    }
    return defaultExpansionMode();
}

qreal KisPropagateColorsFilterConfiguration::expansionAmount() const
{
    return getDouble("expansionAmount", defaultExpansionAmount());
}

KisPropagateColorsFilterConfiguration::AlphaChannelMode KisPropagateColorsFilterConfiguration::alphaChannelMode() const
{
    const QString alphaChannelModeStr = getString("alphaChannelMode", "expand");
    if (alphaChannelModeStr == "preserve") {
        return AlphaChannelMode_Preserve;
    } else if (alphaChannelModeStr == "expand") {
        return AlphaChannelMode_Expand;
    }
    return defaultAlphaChannelMode();
}

void KisPropagateColorsFilterConfiguration::setDistanceMetric(DistanceMetric newDistanceMetric)
{
    if (newDistanceMetric == DistanceMetric_Chessboard) {
        setProperty("distanceMetric", "chessboard");
    } else if (newDistanceMetric == DistanceMetric_CityBlock) {
        setProperty("distanceMetric", "cityBlock");
    } else {
        setProperty("distanceMetric", "euclidean");
    }
}

void KisPropagateColorsFilterConfiguration::setExpansionMode(ExpansionMode newExpansionMode)
{
    if (newExpansionMode == ExpansionMode_Bounded) {
        setProperty("expansionMode", "bounded");
    } else {
        setProperty("expansionMode", "unbounded");
    }
}

void KisPropagateColorsFilterConfiguration::setExpansionAmount(qreal newExpansionAmount)
{
    setProperty("expansionAmount", newExpansionAmount);
}

void KisPropagateColorsFilterConfiguration::setAlphaChannelMode(AlphaChannelMode newAlphaChannelMode)
{
    if (newAlphaChannelMode == AlphaChannelMode_Preserve) {
        setProperty("alphaChannelMode", "preserve");
    } else {
        setProperty("alphaChannelMode", "expand");
    }
}

void KisPropagateColorsFilterConfiguration::setDefaults()
{
    setDistanceMetric(defaultDistanceMetric());
    setExpansionMode(defaultExpansionMode());
    setExpansionAmount(defaultExpansionAmount());
    setAlphaChannelMode(defaultAlphaChannelMode());
}
