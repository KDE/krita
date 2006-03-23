/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_ABSTRACT_COLORSPACE_H_
#define KIS_ABSTRACT_COLORSPACE_H_

#include <qmap.h>
#include <qcolor.h>
#include <qstringlist.h>
#include <qpair.h>

#include "kis_global.h"
#include "kis_channelinfo.h"
#include "kis_profile.h"
#include "kis_id.h"
#include "kis_composite_op.h"
#include "kis_colorspace.h"

class QPainter;
class KisPixelRO;
class KisColorSpaceFactoryRegistry;


/**
 * A colorspace strategy is the definition of a certain color model
 * in Krita.
 */
class KisAbstractColorSpace : public KisColorSpace {


public:

    /**
     * @param id The unique human and machine readable identifiation of this colorspace
     * @param cmType the lcms type indentification for this colorspace, may be 0
     * @param colorSpaceSignature the icc identification for this colorspace, may be 0
     * @param parent the registry that owns this instance
     * @param profile the profile this colorspace uses for transforms
     */
    KisAbstractColorSpace(const KisID & id,
                          DWORD cmType,
                          icColorSpaceSignature colorSpaceSignature,
                          KisColorSpaceFactoryRegistry * parent,
                          KisProfile *profile);

    void init();

    virtual ~KisAbstractColorSpace();

    virtual bool operator==(const KisAbstractColorSpace& rhs) const {
        return (m_id == rhs.m_id && m_profile == rhs.m_profile);
    }


//================== Information about this color strategy ========================//

public:


    //========== Channels =====================================================//

    // Return a vector describing all the channels this color model has.
    virtual QValueVector<KisChannelInfo *> channels() const = 0;

    virtual Q_UINT32 nChannels() const = 0;

    virtual Q_UINT32 nColorChannels() const = 0;

    virtual Q_UINT32 nSubstanceChannels() const { return 0; };

    virtual Q_UINT32 pixelSize() const = 0;

    virtual QString channelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const = 0;

    virtual QString normalisedChannelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const = 0;

    virtual Q_UINT8 scaleToU8(const Q_UINT8 * srcPixel, Q_INT32 channelPos) = 0;

    virtual Q_UINT16 scaleToU16(const Q_UINT8 * srcPixel, Q_INT32 channelPos) = 0;

    virtual void getSingleChannelPixel(Q_UINT8 *dstPixel, const Q_UINT8 *srcPixel, Q_UINT32 channelIndex);

    //========== Identification ===============================================//

    virtual KisID id() const { return m_id; }

    void setColorSpaceType(Q_UINT32 type) { m_cmType = type; }
    Q_UINT32 colorSpaceType() { return m_cmType; }

    virtual icColorSpaceSignature colorSpaceSignature() { return m_colorSpaceSignature; }

    //========== Capabilities =================================================//

    virtual KisCompositeOpList userVisiblecompositeOps() const = 0;

    /**
     * Returns true if the colorspace supports channel values outside the 
     * (normalised) range 0 to 1.
     */
    virtual bool hasHighDynamicRange() const { return false; }

    //========== Display profiles =============================================//

    virtual KisProfile * getProfile() { return m_profile; };


//================= Conversion functions ==================================//


    virtual void fromQColor(const QColor& c, Q_UINT8 *dst, KisProfile * profile = 0);
    virtual void fromQColor(const QColor& c, Q_UINT8 opacity, Q_UINT8 *dst, KisProfile * profile = 0);

    virtual void toQColor(const Q_UINT8 *src, QColor *c, KisProfile * profile = 0);
    virtual void toQColor(const Q_UINT8 *src, QColor *c, Q_UINT8 *opacity, KisProfile * profile = 0);

    virtual QImage convertToQImage(const Q_UINT8 *data, Q_INT32 width, Q_INT32 height,
                                   KisProfile *  dstProfile,
                                   Q_INT32 renderingIntent = INTENT_PERCEPTUAL,
                                   float exposure = 0.0f);

    virtual bool convertPixelsTo(const Q_UINT8 * src,
                                 Q_UINT8 * dst, KisColorSpace * dstColorSpace,
                                 Q_UINT32 numPixels,
                                 Q_INT32 renderingIntent = INTENT_PERCEPTUAL);

//============================== Manipulation fucntions ==========================//


//
// The manipulation functions have default implementations that _convert_ the pixel
// to a QColor and back. Reimplement these methods in your color strategy!
//
    virtual KisColorAdjustment *createBrightnessContrastAdjustment(Q_UINT16 *transferValues);

    virtual KisColorAdjustment *createDesaturateAdjustment();

    virtual KisColorAdjustment *createPerChannelAdjustment(Q_UINT16 **transferValues);

    virtual void applyAdjustment(const Q_UINT8 *src, Q_UINT8 *dst, KisColorAdjustment *, Q_INT32 nPixels);

    virtual void invertColor(Q_UINT8 * src, Q_INT32 nPixels);
    
    virtual Q_UINT8 difference(const Q_UINT8* src1, const Q_UINT8* src2);

    virtual void mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const;

    virtual void convolveColors(Q_UINT8** colors, Q_INT32* kernelValues, KisChannelInfo::enumChannelFlags channelFlags, Q_UINT8 *dst, Q_INT32 factor, Q_INT32 offset, Q_INT32 nPixels) const;

    virtual void darken(const Q_UINT8 * src, Q_UINT8 * dst, Q_INT32 shade, bool compensate, double compensation, Q_INT32 nPixels) const;

    virtual Q_UINT8 intensity8(const Q_UINT8 * src) const;
    
    virtual KisID mathToolboxID() const;

    virtual void bitBlt(Q_UINT8 *dst,
                Q_INT32 dststride,
                KisColorSpace * srcSpace,
                const Q_UINT8 *src,
                Q_INT32 srcRowStride,
                const Q_UINT8 *srcAlphaMask,
                Q_INT32 maskRowStride,
                Q_UINT8 opacity,
                Q_INT32 rows,
                Q_INT32 cols,
                const KisCompositeOp& op);

//========================== END of Public API ========================================//

protected:


    /**
     * Compose two byte arrays containing pixels in the same color
     * model together.
     */
    virtual void bitBlt(Q_UINT8 *dst,
                Q_INT32 dstRowSize,
                const Q_UINT8 *src,
                Q_INT32 srcRowStride,
                const Q_UINT8 *srcAlphaMask,
                Q_INT32 maskRowStride,
                Q_UINT8 opacity,
                Q_INT32 rows,
                Q_INT32 cols,
                const KisCompositeOp& op) = 0;

    virtual cmsHTRANSFORM createTransform(KisColorSpace * dstColorSpace,
                          KisProfile *  srcProfile,
                          KisProfile *  dstProfile,
                          Q_INT32 renderingIntent);

    virtual void compositeCopy(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT8 opacity);

protected:

    QStringList m_profileFilenames;
    Q_UINT8 * m_qcolordata; // A small buffer for conversion from and to qcolor.
    Q_INT32 m_alphaPos; // The position in _bytes_ of the alpha channel
    Q_INT32 m_alphaSize; // The width in _bytes_ of the alpha channel

    QValueVector<KisChannelInfo *> m_channels;

    KisColorSpaceFactoryRegistry * m_parent;

private:

    cmsHTRANSFORM m_defaultToRGB;    // Default transform to 8 bit sRGB
    cmsHTRANSFORM m_defaultFromRGB;  // Default transform from 8 bit sRGB

    cmsHPROFILE   m_lastRGBProfile;  // Last used profile to transform to/from RGB
    cmsHTRANSFORM m_lastToRGB;       // Last used transform to transform to RGB
    cmsHTRANSFORM m_lastFromRGB;     // Last used transform to transform from RGB

    cmsHTRANSFORM m_defaultToLab;
    cmsHTRANSFORM m_defaultFromLab;

    KisProfile *  m_profile;
    KisColorSpace *m_lastUsedDstColorSpace;
    cmsHTRANSFORM m_lastUsedTransform;

    KisID m_id;
    DWORD m_cmType;                           // The colorspace type as defined by littlecms
    icColorSpaceSignature m_colorSpaceSignature; // The colorspace signature as defined in icm/icc files

    // cmsHTRANSFORM is a void *, so this should work.
    typedef QMap<KisColorSpace *, cmsHTRANSFORM>  TransformMap;
    TransformMap m_transforms; // Cache for existing transforms

    KisAbstractColorSpace(const KisAbstractColorSpace&);
    KisAbstractColorSpace& operator=(const KisAbstractColorSpace&);

    QMemArray<Q_UINT8> m_conversionCache; // XXX: This will be a bad problem when we have threading.
};

#endif // KIS_STRATEGY_COLORSPACE_H_
