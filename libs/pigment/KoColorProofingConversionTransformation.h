/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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
                                            ConversionFlags conversionFlags);
    virtual ~KoColorProofingConversionTransformation();

public:

    /**
     * @brief proofingSpace
     * @return the space that is used to proof the color transform
     */
    const KoColorSpace *proofingSpace() const;

private:

    const KoColorSpace *m_proofingSpace;
};

#endif
