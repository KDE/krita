/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#ifndef KOLCMSCOLORSPACETRAIT_H
#define KOLCMSCOLORSPACETRAIT_H

#include <QMap>
#include <QColor>
#include <QStringList>
#include <QPair>
//Added by qt3to4:
#include <Q3MemArray>

#include "KoChannelInfo.h"
#include "KoColorProfile.h"
#include "KoID.h"
#include "KoCompositeOp.h"
#include "KoColorSpace.h"

class QPainter;

/**
 * This class provide most of the implementation for lcms based color spaces
 */
class PIGMENT_EXPORT KoLcmsColorSpaceTrait : public virtual KoColorSpace {


public:

    /**
     * @param cmType the lcms type indentification for this colorspace, may be 0
     * @param colorSpaceSignature the icc identification for this colorspace, may be 0
     * @param profile the profile this colorspace uses for transforms
     */
    KoLcmsColorSpaceTrait(DWORD cmType,
                          icColorSpaceSignature colorSpaceSignature,
                          KoColorProfile *profile);

    void init();

    virtual ~KoLcmsColorSpaceTrait();

    virtual bool operator==(const KoLcmsColorSpaceTrait& rhs) const {
        return (id() == rhs.id() && m_profile == rhs.m_profile);
    }


//================== Information about this color strategy ========================//

public:


    //========== Channels =====================================================//

    // Return a vector describing all the channels this color model has.

    virtual quint32 nSubstanceChannels() const { return 0; };

    virtual void getSingleChannelPixel(quint8 *dstPixel, const quint8 *srcPixel, quint32 channelIndex);

    //========== Identification ===============================================//

    void setColorSpaceType(quint32 type) { m_cmType = type; }
    quint32 colorSpaceType() { return m_cmType; }

    virtual icColorSpaceSignature colorSpaceSignature() { return m_colorSpaceSignature; }

    //========== Capabilities =================================================//

    /**
     * Returns true if the colorspace supports channel values outside the
     * (normalised) range 0 to 1.
     */
    virtual bool hasHighDynamicRange() const { return false; }

    //========== Display profiles =============================================//

    virtual KoColorProfile * getProfile() { return m_profile; };


//================= Conversion functions ==================================//


    virtual void fromQColor(const QColor& c, quint8 *dst, KoColorProfile * profile = 0);
    virtual void fromQColor(const QColor& c, quint8 opacity, quint8 *dst, KoColorProfile * profile = 0);

    virtual void toQColor(const quint8 *src, QColor *c, KoColorProfile * profile = 0);
    virtual void toQColor(const quint8 *src, QColor *c, quint8 *opacity, KoColorProfile * profile = 0);

    virtual QImage convertToQImage(const quint8 *data, qint32 width, qint32 height,
                                   KoColorProfile *  dstProfile,
                                   qint32 renderingIntent = INTENT_PERCEPTUAL,
                                   float exposure = 0.0f);

    virtual void toLabA16(const quint8 * src, quint8 * dst, const quint32 nPixels) const;
    virtual void fromLabA16(const quint8 * src, quint8 * dst, const quint32 nPixels) const;

    virtual bool convertPixelsTo(const quint8 * src,
                                 quint8 * dst, KoColorSpace * dstColorSpace,
                                 quint32 numPixels,
                                 qint32 renderingIntent = INTENT_PERCEPTUAL);

//============================== Manipulation fucntions ==========================//


//
// The manipulation functions have default implementations that _convert_ the pixel
// to a QColor and back. Reimplement these methods in your color strategy!
//
    virtual KoColorAdjustment *createBrightnessContrastAdjustment(quint16 *transferValues);

    virtual KoColorAdjustment *createDesaturateAdjustment();

    virtual KoColorAdjustment *createPerChannelAdjustment(quint16 **transferValues);

    virtual void applyAdjustment(const quint8 *src, quint8 *dst, KoColorAdjustment *, qint32 nPixels);

    virtual void invertColor(quint8 * src, qint32 nPixels);

    virtual quint8 difference(const quint8* src1, const quint8* src2);

    virtual void mixColors(const quint8 **colors, const quint8 *weights, quint32 nColors, quint8 *dst) const;

    virtual void convolveColors(quint8** colors, qint32* kernelValues, KoChannelInfo::enumChannelFlags channelFlags, quint8 *dst, qint32 factor, qint32 offset, qint32 nPixels) const;

    virtual void darken(const quint8 * src, quint8 * dst, qint32 shade, bool compensate, double compensation, qint32 nPixels) const;

    virtual quint8 intensity8(const quint8 * src) const;

    virtual KoID mathToolboxID() const;

    virtual void bitBlt(quint8 *dst,
                qint32 dststride,
                KoColorSpace * srcSpace,
                const quint8 *src,
                qint32 srcRowStride,
                const quint8 *srcAlphaMask,
                qint32 maskRowStride,
                quint8 opacity,
                qint32 rows,
                qint32 cols,
                const KoCompositeOp& op);

//========================== END of Public API ========================================//

protected:


    /**
     * Compose two byte arrays containing pixels in the same color
     * model together.
     */
    virtual void bitBlt(quint8 *dst,
                qint32 dstRowSize,
                const quint8 *src,
                qint32 srcRowStride,
                const quint8 *srcAlphaMask,
                qint32 maskRowStride,
                quint8 opacity,
                qint32 rows,
                qint32 cols,
                const KoCompositeOp& op) = 0;

    virtual cmsHTRANSFORM createTransform(KoColorSpace * dstColorSpace,
                          KoColorProfile *  srcProfile,
                          KoColorProfile *  dstProfile,
                          qint32 renderingIntent);

    virtual void compositeCopy(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity);

protected:

    QStringList m_profileFilenames;
    quint8 * m_qcolordata; // A small buffer for conversion from and to qcolor.
    //qint32 m_alphaPos; // The position in _bytes_ of the alpha channel
    //qint32 m_alphaSize; // The width in _bytes_ of the alpha channel

private:

    cmsHTRANSFORM m_defaultToRGB;    // Default transform to 8 bit sRGB
    cmsHTRANSFORM m_defaultFromRGB;  // Default transform from 8 bit sRGB

    cmsHPROFILE   m_lastRGBProfile;  // Last used profile to transform to/from RGB
    cmsHTRANSFORM m_lastToRGB;       // Last used transform to transform to RGB
    cmsHTRANSFORM m_lastFromRGB;     // Last used transform to transform from RGB

    cmsHTRANSFORM m_defaultToLab;
    cmsHTRANSFORM m_defaultFromLab;

    KoColorProfile *  m_profile;
    KoColorSpace *m_lastUsedDstColorSpace;
    cmsHTRANSFORM m_lastUsedTransform;

    DWORD m_cmType;                           // The colorspace type as defined by littlecms
    icColorSpaceSignature m_colorSpaceSignature; // The colorspace signature as defined in icm/icc files

    // cmsHTRANSFORM is a void *, so this should work.
    typedef QMap<KoColorSpace *, cmsHTRANSFORM>  TransformMap;
    TransformMap m_transforms; // Cache for existing transforms

    KoLcmsColorSpaceTrait(const KoLcmsColorSpaceTrait&);
    KoLcmsColorSpaceTrait& operator=(const KoLcmsColorSpaceTrait&);

    Q3MemArray<quint8> m_conversionCache; // XXX: This will be a bad problem when we have threading.
};

#endif // KIS_LCMS_BASE_COLORSPACE_H_
