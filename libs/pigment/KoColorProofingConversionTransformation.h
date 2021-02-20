/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _KO_COLOR_PROOFING_CONVERSION_TRANSFORMATION_H_
#define _KO_COLOR_PROOFING_CONVERSION_TRANSFORMATION_H_

#include "KoColorConversionTransformation.h"

#include "kritapigment_export.h"

class KoColorSpace;
class KoColorConversionCache;

/**
 * This is the base class of all color transform that convert the color of a pixel
 */
class KRITAPIGMENT_EXPORT KoColorProofingConversionTransformation : public KoColorConversionTransformation
{

public:
    KoColorProofingConversionTransformation(const KoColorSpace *srcCs,
                                            const KoColorSpace *dstCs,
                                            const KoColorSpace *proofingSpace,
                                            Intent renderingIntent,
                                            Intent proofingIntent,
                                            ConversionFlags conversionFlags,
                                            quint8 *gamutWarning,
                                            double adaptationState);
    ~KoColorProofingConversionTransformation() override;

public:

    /**
     * @brief proofingSpace
     * @return the space that is used to proof the color transform
     */
    const KoColorSpace *proofingSpace() const;

private:

    Intent m_proofingIntent;
    quint8 *m_gamutWarning;
    double m_adaptationState;
    const KoColorSpace *m_proofingSpace;
};

#endif
