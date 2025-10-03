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
        flags.setFlag(KoColorConversionTransformation::NoAdaptationAbsoluteIntent, false);
    } else if (displayMode == Paper) {
        flags = KoColorConversionTransformation::HighQuality;
        flags.setFlag(KoColorConversionTransformation::NoAdaptationAbsoluteIntent, false);
    } else {
        flags = displayFlags;
    }
    flags.setFlag(KoColorConversionTransformation::GamutCheck, displayFlags.testFlag(KoColorConversionTransformation::GamutCheck));
    flags.setFlag(KoColorConversionTransformation::SoftProofing, displayFlags.testFlag(KoColorConversionTransformation::SoftProofing));
    return flags;
}

bool KisProofingConfiguration::operator==(const KisProofingConfiguration &other) const {
    return conversionIntent == other.conversionIntent &&
           displayIntent == other.displayIntent &&
           useBlackPointCompensationFirstTransform == other.useBlackPointCompensationFirstTransform &&
           displayFlags == other.displayFlags &&
           warningColor == other.warningColor &&
           proofingProfile == other.proofingProfile &&
           proofingModel == other.proofingModel &&
           proofingDepth == other.proofingDepth &&
           displayMode == other.displayMode;
}

bool KisProofingConfiguration::operator!=(const KisProofingConfiguration &other) const {
    return !(*this == other);
}
