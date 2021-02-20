/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _KO_COLOR_CONVERSION_TRANSFORMATION_ABSTRACT_FACTORY_H_
#define _KO_COLOR_CONVERSION_TRANSFORMATION_ABSTRACT_FACTORY_H_

#include "kritapigment_export.h"

#include <KoColorConversionTransformation.h>
#include <KoColorProofingConversionTransformation.h>

class KRITAPIGMENT_EXPORT KoColorConversionTransformationAbstractFactory
{
public:
    KoColorConversionTransformationAbstractFactory() {}
    virtual ~KoColorConversionTransformationAbstractFactory() {}

    /**
     * Creates a color transformation between the source color space and the destination
     * color space.
     *
     * @param srcColorSpace source color space
     * @param dstColorSpace destination color space
     * @param renderingIntent rendering intent
     * @param conversionFlags conversion flags
     */
    virtual KoColorConversionTransformation* createColorTransformation(const KoColorSpace* srcColorSpace,
                                                                       const KoColorSpace* dstColorSpace,
                                                                       KoColorConversionTransformation::Intent renderingIntent,
                                                                       KoColorConversionTransformation::ConversionFlags conversionFlags) const = 0;

    virtual KoColorProofingConversionTransformation* createColorProofingTransformation(const KoColorSpace* srcColorSpace,
                                                                       const KoColorSpace* dstColorSpace,
                                                                       const KoColorSpace* proofingSpace,
                                                                       KoColorProofingConversionTransformation::Intent renderingIntent,
                                                                       KoColorProofingConversionTransformation::Intent proofingIntent,
                                                                       KoColorProofingConversionTransformation::ConversionFlags conversionFlags,
                                                                       quint8 *gamutWarning,
                                                                       double adaptationState) const
    {
        Q_UNUSED(srcColorSpace);
        Q_UNUSED(dstColorSpace);
        Q_UNUSED(proofingSpace);
        Q_UNUSED(renderingIntent);
        Q_UNUSED(proofingIntent);
        Q_UNUSED(conversionFlags);
        Q_UNUSED(gamutWarning);
        Q_UNUSED(adaptationState);
        qFatal("createColorProofinTransform undefined.");
        return 0;
    }
};

#endif
