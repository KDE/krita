/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisProofingConfiguration.h"


KisProofingConfiguration::KisProofingConfiguration()
    : conversionIntent(KoColorConversionTransformation::IntentRelativeColorimetric),
      displayIntent(KoColorConversionTransformation::IntentAbsoluteColorimetric),
      conversionFlags(KoColorConversionTransformation::BlackpointCompensation),
      displayFlags(KoColorConversionTransformation::HighQuality),
      warningColor(KoColor()),
      proofingProfile("Chemical proof"),
      proofingModel("CMYKA"),
      proofingDepth("U8"),
      adaptationState(1.0),
      storeSoftproofingInsideImage(false),
      useMonitorSettings(false),
      usePaperSettings(false)
{
}

KisProofingConfiguration::~KisProofingConfiguration()
{
}

KoColorConversionTransformation::Intent KisProofingConfiguration::determineDisplayIntent(KoColorConversionTransformation::Intent monitorDisplayIntent)
{
    if (useMonitorSettings) return monitorDisplayIntent;
    if (usePaperSettings) return KoColorConversionTransformation::IntentAbsoluteColorimetric;
    return displayIntent;
}

KoColorConversionTransformation::ConversionFlags KisProofingConfiguration::determineDisplayFlags(KoColorConversionTransformation::ConversionFlags monitorDisplayFlags)
{
    KoColorConversionTransformation::ConversionFlags flags;
    if (useMonitorSettings) {
        flags = monitorDisplayFlags;
    } else if (usePaperSettings) {
        flags = KoColorConversionTransformation::HighQuality;
    } else {
        flags = displayFlags;
    }
    flags.setFlag(KoColorConversionTransformation::GamutCheck, displayFlags.testFlag(KoColorConversionTransformation::GamutCheck));
    return flags;
}

double KisProofingConfiguration::determineAdaptationState()
{
    if (usePaperSettings) return 0.0;
    return adaptationState;
}

