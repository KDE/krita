/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef KISPROOFINGCONFIGURATION_H
#define KISPROOFINGCONFIGURATION_H

#include "KoColor.h"
#include "KoColorSpace.h"
#include "KoColorConversionTransformation.h"
#include "kritaimage_export.h"

/**
 * @brief The KisProofingConfiguration struct
 * Little struct that stores the proofing configuration for a given file.
 * The actual softproofing and gamutcheck toggles are set in the canvas.
 * intet, conversionflags and warning color have default set to them. This
 * wasn't possible for profileSpace.
 */
class KRITAIMAGE_EXPORT KisProofingConfiguration {
public:
    KisProofingConfiguration();
    ~KisProofingConfiguration();
    KoColorConversionTransformation::Intent intent;
    KoColorConversionTransformation::ConversionFlags conversionFlags;
    KoColor warningColor;
    QString proofingProfile;
    QString proofingModel;
    QString proofingDepth;
    double adaptationState;
    bool storeSoftproofingInsideImage;

};

#endif // KISPROOFINGCONFIGURATION_H
