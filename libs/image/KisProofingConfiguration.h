#ifndef KISPROOFINGCONFIGURATION_H
#define KISPROOFINGCONFIGURATION_H

#include "KoColor.h"
#include "KoColorSpace.h"
#include "KoColorConversionTransformation.h"

/**
 * @brief The KisProofingConfiguration struct
 * Little struct that stores the proofing configuration for a given file.
 * The actual softproofing and gamutcheck toggles are set in the canvas.
 * intet, conversionflags and warning color have default set to them. This
 * wasn't possible for profileSpace.
 */
struct KisProofingConfiguration {
    KisProofingConfiguration() : intent(KoColorConversionTransformation::IntentAbsoluteColorimetric),
                                 conversionFlags(KoColorConversionTransformation::BlackpointCompensation),
                                 warningColor(KoColor()),
                                 proofingProfile("Chemical proof"),
                                 proofingModel("CMYKA"),
                                 proofingDepth("U8"){}
    KoColorConversionTransformation::Intent intent;
    KoColorConversionTransformation::ConversionFlags conversionFlags;
    KoColor warningColor;
    QString proofingProfile;
    QString proofingModel;
    QString proofingDepth;

};

#endif // KISPROOFINGCONFIGURATION_H
