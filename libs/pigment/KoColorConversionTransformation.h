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

#ifndef _KO_COLOR_CONVERSION_TRANSFORMATION_H_
#define _KO_COLOR_CONVERSION_TRANSFORMATION_H_

#include "KoColorTransformation.h"

#include "kritapigment_export.h"

class KoColorSpace;
class KoColorConversionCache;

/**
 * This is the base class of all color transform that convert the color of a pixel
 */
class KRITAPIGMENT_EXPORT KoColorConversionTransformation : public KoColorTransformation
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

    /**
     * Flags for the color conversion, see lcms2 documentation for more information
     */
    enum ConversionFlag {
        Empty                   = 0x0,
        NoOptimization          = 0x0100,
        GamutCheck              = 0x1000,    // Out of Gamut alarm
        SoftProofing            = 0x4000,    // Do softproofing
        BlackpointCompensation  = 0x2000,
        NoWhiteOnWhiteFixup     = 0x0004,    // Don't fix scum dot
        HighQuality             = 0x0400,    // Use more memory to give better accuracy
        LowQuality              = 0x0800    // Use less memory to minimize resources
    };
    Q_DECLARE_FLAGS(ConversionFlags, ConversionFlag)

    /**
     * We have numerous places where we need to convert color spaces.
     *
     * In several cases the user asks us about the conversion
     * explicitly, e.g. when changing the image type or converting
     * pixel data to the monitor profile. Doing this explicitly the
     * user can choose what rendering intent and conversion flags to
     * use.
     *
     * But there are also cases when we have to do a conversion
     * internally (transparently for the user), for example, when
     * merging heterogeneous images, creating thumbnails, converting
     * data to/from QImage or while doing some adjustments. We cannot
     * ask the user about parameters for every single
     * conversion. That's why in all these non-critical cases the
     * following default values should be used.
     */

    static Intent internalRenderingIntent() { return IntentPerceptual; }
    static ConversionFlags internalConversionFlags() { return BlackpointCompensation; }

    static Intent adjustmentRenderingIntent() { return IntentPerceptual; }
    static ConversionFlags adjustmentConversionFlags() { return ConversionFlags(BlackpointCompensation | NoWhiteOnWhiteFixup); }

public:
    KoColorConversionTransformation(const KoColorSpace* srcCs,
                                    const KoColorSpace* dstCs,
                                    Intent renderingIntent,
                                    ConversionFlags conversionFlags);
    ~KoColorConversionTransformation() override;
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
    Intent renderingIntent() const;

    /**
     * @return the conversion flags
     */
    ConversionFlags conversionFlags() const;

    /**
     * perform the color conversion between two buffers. Make sure that
     * \p src is not the same as \p dst!
     * @param nPixels the number of pixels in the buffers.
     */
    void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const override = 0;

    /**
     * perform the color conversion between two or one buffer. This is a convenience
     * function that allows doing the conversion in-place
     * @param nPixels the number of pixels in the buffers.
     */
    void transformInPlace(const quint8 *src, quint8 *dst, qint32 nPixels) const;

    /**
     * @return false if the  transformation is not valid
     */
    bool isValid() const override { return true; }

private:

    void setSrcColorSpace(const KoColorSpace*) const;
    void setDstColorSpace(const KoColorSpace*) const;

    Private * const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KoColorConversionTransformation::ConversionFlags)

#endif
