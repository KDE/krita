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
 * intent, conversionflags and warning color have default set to them. This
 * wasn't possible for profileSpace.
 */
class KRITAIMAGE_EXPORT KisProofingConfiguration {
public:
    KisProofingConfiguration();
    ~KisProofingConfiguration();
    KoColorConversionTransformation::Intent conversionIntent; ///< This is the intent for the first transform.
    KoColorConversionTransformation::Intent displayIntent; ///< This is the intent for the second transform.
    bool useBlackPointCompensationFirstTransform; ///< Whether to use BCP on the first transform. All other flags are handled by displayFlags;
    KoColorConversionTransformation::ConversionFlags displayFlags; ///< flags for the second transform.
    KoColor warningColor;
    QString proofingProfile;
    QString proofingModel;
    QString proofingDepth;
    double adaptationState;
    bool storeSoftproofingInsideImage;

    enum DisplayTransformState {
        Monitor, ///< Whether to use monitor rendering intent and flags for the second transform.
        Paper,   ///< Whether to use Paper settings (absolute colorimetric, 0% adaptation.)
        Custom   ///< Let artists configure their own.
    };
    DisplayTransformState displayMode;

    KoColorConversionTransformation::Intent determineDisplayIntent(KoColorConversionTransformation::Intent monitorDisplayIntent);
    KoColorConversionTransformation::ConversionFlags determineDisplayFlags(KoColorConversionTransformation::ConversionFlags monitorDisplayFlags);
    double determineAdaptationState();
};

#endif // KISPROOFINGCONFIGURATION_H
