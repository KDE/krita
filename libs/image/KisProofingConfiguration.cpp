/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisProofingConfiguration.h"


KisProofingConfiguration::KisProofingConfiguration()
    : intent(KoColorConversionTransformation::IntentAbsoluteColorimetric),
      conversionFlags(KoColorConversionTransformation::BlackpointCompensation),
      warningColor(KoColor()),
      proofingProfile("Chemical proof"),
      proofingModel("CMYKA"),
      proofingDepth("U8"),
      adaptationState(1.0),
      storeSoftproofingInsideImage(false)
{
}

KisProofingConfiguration::~KisProofingConfiguration()
{
}

