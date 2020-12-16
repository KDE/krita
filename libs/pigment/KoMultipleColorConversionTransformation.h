/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _KO_MULTIPLE_COLOR_CONVERSION_TRANSFORMATION_H_
#define _KO_MULTIPLE_COLOR_CONVERSION_TRANSFORMATION_H_

#include <KoColorConversionTransformation.h>

#include "kritapigment_export.h"

/**
 * This color conversion transformation allows to create a chain of color conversion between two color
 * spaces.
 * For instance, imagine we have three color spaces : CSA , CSB and CSC , we only know a transformation
 * from CSA to CSB and from CSB to CSC , and we want to convert from CSA to CSC. Then we can create
 * a KoMultipleColorConversionTransformation, and add to it with
 *  KoMultipleColorConversionTransformation::appendTransfo a transformation from CSA to CSB and a
 * transformation from CSB to CSC.
 */
class KRITAPIGMENT_EXPORT KoMultipleColorConversionTransformation : public KoColorConversionTransformation
{
public:
    /**
     * Create a color transformation from srcCs to dstCs.
     * @param srcCs the first color space in the chain
     * @param dstCs the last color space in the chain
     */
    KoMultipleColorConversionTransformation(const KoColorSpace* srcCs, const KoColorSpace* dstCs, KoColorConversionTransformation::Intent, KoColorConversionTransformation::ConversionFlags conversionFlags);
    ~KoMultipleColorConversionTransformation() override;
    /**
     * Add a transformation to the chain.
     * @param transfo this transformation is then deleted when the
     *                KoMultipleColorConversionTransformation is deleted.
     */
    void appendTransfo(KoColorConversionTransformation* transfo);
    void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const override;
private:
    struct Private;
    Private* const d;
};

#endif
