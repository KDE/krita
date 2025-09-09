/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisProofingConfiguration.h"


KisProofingConfiguration::KisProofingConfiguration()
    : conversionIntent(KoColorConversionTransformation::IntentRelativeColorimetric),
      displayIntent(KoColorConversionTransformation::IntentAbsoluteColorimetric),
      useBlackPointCompensationFirstTransform(true),
      displayFlags(KoColorConversionTransformation::HighQuality),
      warningColor(KoColor(Qt::green, KoColorSpaceRegistry::instance()->rgb8())),
      proofingProfile("Chemical proof"),
      proofingModel("CMYKA"),
      proofingDepth("U8"),
      adaptationState(1.0),
      storeSoftproofingInsideImage(false),
      displayMode(Paper)
{
}

KisProofingConfiguration::~KisProofingConfiguration()
{
}

KoColorConversionTransformation::Intent KisProofingConfiguration::determineDisplayIntent(KoColorConversionTransformation::Intent monitorDisplayIntent)
{
    if (displayMode == Monitor) return monitorDisplayIntent;
    if (displayMode == Paper) return KoColorConversionTransformation::IntentAbsoluteColorimetric;
    return displayIntent;
}

KoColorConversionTransformation::ConversionFlags KisProofingConfiguration::determineDisplayFlags(KoColorConversionTransformation::ConversionFlags monitorDisplayFlags)
{
    KoColorConversionTransformation::ConversionFlags flags;
    if (displayMode == Monitor) {
        flags = monitorDisplayFlags;
    } else if (displayMode == Paper) {
        flags = KoColorConversionTransformation::HighQuality;
    } else {
        flags = displayFlags;
    }
    flags.setFlag(KoColorConversionTransformation::GamutCheck, displayFlags.testFlag(KoColorConversionTransformation::GamutCheck));
    flags.setFlag(KoColorConversionTransformation::SoftProofing, displayFlags.testFlag(KoColorConversionTransformation::SoftProofing));
    return flags;
}

double KisProofingConfiguration::determineAdaptationState()
{
    if (displayMode == Paper) return 0.0;
    return adaptationState;
}

