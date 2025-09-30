/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISDISPLAYCONFIG_H
#define KISDISPLAYCONFIG_H

#include <kritaui_export.h>
#include <boost/operators.hpp>
#include <KoColorConversionTransformation.h>

class KoColorProfile;
class KisConfig;
/**
 * @brief KisDisplayConfig
 * This class keeps track of the color management configuration for
 * image to display. This includes the profile used, but also the
 * various flags that need to be set on the final transform.
 */
class KRITAUI_EXPORT KisDisplayConfig : public boost::equality_comparable<KisDisplayConfig>
{
public:
    using Options = std::pair<KoColorConversionTransformation::Intent, KoColorConversionTransformation::ConversionFlags>;

public:
    KisDisplayConfig();
    KisDisplayConfig(const KoColorProfile *_profile,
                     KoColorConversionTransformation::Intent _intent,
                     KoColorConversionTransformation::ConversionFlags _conversionFlags,
                     bool _isHDR = false);
    bool operator==(const KisDisplayConfig &rhs) const;

    Options options() const {
        return std::make_pair(intent, conversionFlags);
    }

    void setOptions(const Options &options) {
        std::tie(intent, conversionFlags) = options;
    }

    static Options optionsFromKisConfig(const KisConfig &cfg);

    const KoColorProfile *profile;
    KoColorConversionTransformation::Intent intent;
    KoColorConversionTransformation::ConversionFlags conversionFlags;
    bool isHDR { false };
};

KRITAUI_EXPORT QDebug operator<<(QDebug debug, const KisDisplayConfig &value);

class KRITAUI_EXPORT KisMultiSurfaceDisplayConfig : public boost::equality_comparable<KisMultiSurfaceDisplayConfig>
{
public:
    using Options = KisDisplayConfig::Options;

public:

    KisMultiSurfaceDisplayConfig() = default;
    bool operator==(const KisMultiSurfaceDisplayConfig &rhs) const;

    KisDisplayConfig uiDisplayConfig() const {
        return KisDisplayConfig(uiProfile, intent, conversionFlags, false);
    }

    KisDisplayConfig canvasDisplayConfig() const {
        return KisDisplayConfig(canvasProfile, intent, conversionFlags, isCanvasHDR);
    }

    Options options() const {
        return std::make_pair(intent, conversionFlags);
    }

    void setOptions(const Options &options) {
        std::tie(intent, conversionFlags) = options;
    }

    const KoColorProfile *uiProfile {nullptr};
    const KoColorProfile *canvasProfile {nullptr};
    KoColorConversionTransformation::Intent intent { KoColorConversionTransformation::internalRenderingIntent() };
    KoColorConversionTransformation::ConversionFlags conversionFlags { KoColorConversionTransformation::internalConversionFlags() };
    bool isCanvasHDR { false };
};

#endif // KISDISPLAYCONFIG_H
