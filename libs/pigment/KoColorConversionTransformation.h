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

#ifndef _KO_COLOR_CONVERTION_TRANSFORMATION_H_
#define _KO_COLOR_CONVERTION_TRANSFORMATION_H_

#include "KoColorTransformation.h"

#include "pigment_export.h"

class KoColorSpace;
class KoColorConversionCache;

/**
 * This is the base class of all color transform that convert the color of a pixel
 */
class PIGMENTCMS_EXPORT KoColorConversionTransformation : public KoColorTransformation
{
    friend class KoColorConversionCache;
    struct Private;
public:
    /**
     * Possible value for the intent of a color conversion (useful only for ICC
     * transformations)
     */
    enum Intent {
        IntentPerceptual = 0,
        IntentRelativeColorimetric = 1,
        IntentSaturation = 2,
        IntentAbsoluteColorimetric = 3
    };
public:
    KoColorConversionTransformation(const KoColorSpace* srcCs, const KoColorSpace* dstCs, Intent renderingIntent = IntentPerceptual);
    ~KoColorConversionTransformation();
public:
    /**
     * @return the source color space for this transformation.
     */
    const KoColorSpace* srcColorSpace() const;
    /**
     * @return the destination color space for this transformation.
     */
    const KoColorSpace* dstColorSpace() const;
    /**
     * @return the rendering intent of this transformation (this is only useful
     * for ICC transformations)
     */
    Intent renderingIntent();
    /**
     * perform the color conversion between two buffers.
     * @param nPixels the number of pixels in the buffers.
     */
    virtual void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const = 0;
private:
    void setSrcColorSpace(const KoColorSpace*) const;
    void setDstColorSpace(const KoColorSpace*) const;
private:
    Private * const d;
};

#endif
