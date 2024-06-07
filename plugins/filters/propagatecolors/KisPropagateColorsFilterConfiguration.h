/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PROPAGATE_COLORS_FILTER_CONFIGURATION_H
#define KIS_PROPAGATE_COLORS_FILTER_CONFIGURATION_H

#include <QString>

#include <kis_filter_configuration.h>
#include <KoColorSpaceRegistry.h>

class KisPropagateColorsFilterConfiguration : public KisFilterConfiguration
{
public:
    enum DistanceMetric
    {
        DistanceMetric_Chessboard,
        DistanceMetric_CityBlock,
        DistanceMetric_Euclidean
    };

    enum ExpansionMode
    {
        ExpansionMode_Bounded,
        ExpansionMode_Unbounded
    };

    enum AlphaChannelMode
    {
        AlphaChannelMode_Preserve,
        AlphaChannelMode_Expand
    };

    KisPropagateColorsFilterConfiguration(KisResourcesInterfaceSP resourcesInterface);
    KisPropagateColorsFilterConfiguration(const KisPropagateColorsFilterConfiguration &rhs);

    ~KisPropagateColorsFilterConfiguration() override;

    KisFilterConfigurationSP clone() const override;

    static inline QString defaultId() { return "propagatecolors"; }
    static inline QString defaultName() { return i18n("Propagate Colors"); }
    static inline QString defaultMenuName() { return i18n("&Propagate Colors..."); }
    static constexpr qint32 defaultVersion() { return 1; }
    static constexpr DistanceMetric defaultDistanceMetric() { return DistanceMetric_Euclidean; }
    static constexpr ExpansionMode defaultExpansionMode() { return ExpansionMode_Unbounded; }
    static constexpr qreal defaultExpansionAmount() { return 10.0; }
    static constexpr AlphaChannelMode defaultAlphaChannelMode() { return AlphaChannelMode_Expand; }

    DistanceMetric distanceMetric() const;
    ExpansionMode expansionMode() const;
    qreal expansionAmount() const;
    AlphaChannelMode alphaChannelMode() const;

    void setDistanceMetric(DistanceMetric newDistanceMetric);
    void setExpansionMode(ExpansionMode newExpansionMode);
    void setExpansionAmount(qreal newExpansionAmount);
    void setAlphaChannelMode(AlphaChannelMode newAlphaChannelMode);

    void setDefaults();
};

using KisPropagateColorsFilterConfigurationSP = KisPinnedSharedPtr<KisPropagateColorsFilterConfiguration>;

#endif
