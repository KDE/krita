/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisDisplayConfig.h"

#include <KoColorProfile.h>
#include <kis_config.h>

namespace {
KoColorConversionTransformation::Intent
renderingIntentFromConfig(const KisConfig &cfg)
{
    return (KoColorConversionTransformation::Intent)cfg.monitorRenderIntent();
}


KoColorConversionTransformation::ConversionFlags
conversionFlagsFromConfig(const KisConfig &cfg)
{
    KoColorConversionTransformation::ConversionFlags conversionFlags =
        KoColorConversionTransformation::HighQuality;

    if (cfg.useBlackPointCompensation()) conversionFlags |= KoColorConversionTransformation::BlackpointCompensation;
    if (!cfg.allowLCMSOptimization()) conversionFlags |= KoColorConversionTransformation::NoOptimization;

    return conversionFlags;
}

}

KisDisplayConfig::KisDisplayConfig()
    : profile(nullptr)
    , intent(KoColorConversionTransformation::internalRenderingIntent())
    , conversionFlags(KoColorConversionTransformation::internalConversionFlags())
{
}

KisDisplayConfig::KisDisplayConfig(int screen, const KisConfig &config)
    : profile(config.displayProfile(screen))
    , intent(renderingIntentFromConfig(config))
    , conversionFlags(conversionFlagsFromConfig(config))
{
}

KisDisplayConfig::KisDisplayConfig(const KoColorProfile *_profileOverride, const KisConfig &config)
    : profile(_profileOverride)
    , intent(renderingIntentFromConfig(config))
    , conversionFlags(conversionFlagsFromConfig(config))
{
}

KisDisplayConfig::KisDisplayConfig(const KoColorProfile *_profile,
                                   KoColorConversionTransformation::Intent _intent,
                                   KoColorConversionTransformation::ConversionFlags _conversionFlags)
    : profile(_profile)
    , intent(_intent)
    , conversionFlags(_conversionFlags)
{
}

bool KisDisplayConfig::operator==(const KisDisplayConfig &rhs) const
{
    return profile == rhs.profile &&
            intent == rhs.intent &&
            conversionFlags == rhs.conversionFlags;
}
