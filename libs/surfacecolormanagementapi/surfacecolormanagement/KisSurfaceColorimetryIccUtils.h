/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSURFACECOLORIMETRYICCUTILS_H
#define KISSURFACECOLORIMETRYICCUTILS_H

#include "KisSurfaceColorimetry.h"
#include <KoColorProfileConstants.h>

namespace KisSurfaceColorimetry
{
    using namespace KisColorimetryUtils;

    KRITASURFACECOLORMANAGEMENTAPI_EXPORT
    ColorPrimaries namedPrimariesToPigmentPrimaries(NamedPrimaries primaries);

    KRITASURFACECOLORMANAGEMENTAPI_EXPORT
    TransferCharacteristics namedTransferFunctionToPigmentTransferFunction(NamedTransferFunction transfer);

    struct PigmentProfileRequest {
        QVector<double> colorants;
        ColorPrimaries colorPrimariesType = PRIMARIES_UNSPECIFIED;
        TransferCharacteristics transferFunction = TRC_UNSPECIFIED;

        inline bool isValid() const {
            return transferFunction != TRC_UNSPECIFIED &&
                (colorPrimariesType != PRIMARIES_UNSPECIFIED || colorants.size() == 8);
        }
    };

    KRITASURFACECOLORMANAGEMENTAPI_EXPORT
    PigmentProfileRequest colorSpaceToRequest(ColorSpace cs);
}

#endif /* KISSURFACECOLORIMETRYICCUTILS_H */