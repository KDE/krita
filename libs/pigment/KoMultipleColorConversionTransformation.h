/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

#ifndef _KO_MULTIPLE_COLOR_CONVERSION_TRANSFORMATION_H_
#define _KO_MULTIPLE_COLOR_CONVERSION_TRANSFORMATION_H_

#include <KoColorConversionTransformation.h>

#include "pigment_export.h"

/**
 * This color conversion transformation allows to create a chain of color conversion between two color
 * spaces.
 * For instance, imagine we have three color spaces : CSA , CSB and CSC , we only know a transformation
 * from CSA to CSB and from CSB to CSC , and we want to convert from CSA to CSC. Then we can create
 * a KoMultipleColorConversionTransformation, and add to it with
 *  KoMultipleColorConversionTransformation::appendTransfo a transformation from CSA to CSB and a
 * transformation from CSB to CSC.
 */
class PIGMENTCMS_EXPORT KoMultipleColorConversionTransformation : public KoColorConversionTransformation
{
public:
    /**
     * Create a color transformation from srcCs to dstCs.
     * @param srcCs the first color space in the chain
     * @param dstCs the last color space in the chain
     */
    KoMultipleColorConversionTransformation(const KoColorSpace* srcCs, const KoColorSpace* dstCs, Intent renderingIntent = IntentPerceptual);
    ~KoMultipleColorConversionTransformation();
    /**
     * Add a transformation to the chain.
     * @param transfo this transformation is then deleted when the
     *                KoMultipleColorConversionTransformation is deleted.
     */
    void appendTransfo(KoColorConversionTransformation* transfo);
    virtual void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const;
private:
    struct Private;
    Private* const d;
};

#endif
