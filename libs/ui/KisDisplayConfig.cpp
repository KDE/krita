/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisDisplayConfig.h"

#include <KoColorProfile.h>
#include <kis_config.h>
#include <opengl/KisOpenGLModeProber.h>

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
    , isHDR(KisOpenGLModeProber::instance()->useHDRMode())
{
}

KisDisplayConfig::KisDisplayConfig(const KoColorProfile *_profileOverride, const KisConfig &config)
    : profile(_profileOverride)
    , intent(renderingIntentFromConfig(config))
    , conversionFlags(conversionFlagsFromConfig(config))
    , isHDR(KisOpenGLModeProber::instance()->useHDRMode())
{
}

KisDisplayConfig::KisDisplayConfig(const KoColorProfile *_profile,
                                   KoColorConversionTransformation::Intent _intent,
                                   KoColorConversionTransformation::ConversionFlags _conversionFlags,
                                   bool _isHDR)
    : profile(_profile)
    , intent(_intent)
    , conversionFlags(_conversionFlags)
    , isHDR(_isHDR)
{
}

KisDisplayConfig::Options KisDisplayConfig::optionsFromKisConfig(const KisConfig &cfg)
{
    return {renderingIntentFromConfig(cfg),
            conversionFlagsFromConfig(cfg)};
}

bool KisDisplayConfig::operator==(const KisDisplayConfig &rhs) const
{
    return profile == rhs.profile &&
            intent == rhs.intent &&
            conversionFlags == rhs.conversionFlags && 
            isHDR == rhs.isHDR;
}

QDebug operator<<(QDebug debug, const KisDisplayConfig &value) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "KisDisplayConfig(";

    debug.nospace() << "profile: " << value.profile;

    if (value.profile) {
        debug.nospace() << " (" << value.profile->name() << ")";
    }
    debug.nospace() << ", ";
    debug.nospace() << "intent: " << value.intent << ", ";
    debug.nospace() << "conversionFlags: " << value.conversionFlags << ", ";
    debug.nospace() << "isHDR: " << value.isHDR;

    debug.nospace() << ")";
    return debug;
}


bool KisMultiSurfaceDisplayConfig::operator==(const KisMultiSurfaceDisplayConfig &rhs) const
{
    return
        uiProfile == rhs.uiProfile &&
        canvasProfile == rhs.canvasProfile &&
        intent == rhs.intent &&
        conversionFlags == rhs.conversionFlags &&
        isCanvasHDR == rhs.isCanvasHDR;
}