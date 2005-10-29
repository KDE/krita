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
class KisPixel;
class KisPixelRO;
class KisColorSpaceFactoryRegistry;


// XXX: This is not enough to identify all the transforms we need: we
// need something that combines the two profiles + the rendering
// intent + the lcms colortype -- a certain profile can be used with
// many varieties of, e.g. RGB(a).
typedef QPair<KisProfile * , KisProfile * > KisProfilePair;

// This would be better, I guess.
struct transform {
    KisProfile srcProfile;
    Q_UINT32 srcCmType;
    KisProfile dstProfile;
    Q_UINT32 dstType;
    Q_UINT32 renderIntent;
};


/**
 * A colorspace strategy is the definition of a certain color model
 * in Krita.
 */
class KisAbstractColorSpace : public KisColorSpace {


public:

    KisAbstractColorSpace(const KisID & id,
                          DWORD cmType,
                          icColorSpaceSignature colorSpaceSignature,
                          KisColorSpaceFactoryRegistry * parent,
                          KisProfile *p);

    void init();

    virtual ~KisAbstractColorSpace();


//================== Information about this color strategy ========================//

public:


    //========== Channels =====================================================//

    // Return a vector describing all the channels this color model has.
    virtual QValueVector<KisChannelInfo *> channels() const = 0;

    virtual Q_INT32 nChannels() const = 0;

    virtual Q_INT32 nColorChannels() const = 0;

    virtual Q_INT32 nSubstanceChannels() const { return 0; };

    virtual Q_INT32 pixelSize() const = 0;

    virtual bool hasAlpha() const = 0;

    virtual Q_INT32 alphaPos() { return m_alphaPos; }

    virtual Q_INT32 alphaSize() { return m_alphaSize; }

    virtual QString channelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const = 0;

    virtual QString normalisedChannelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const = 0;

    //========== Identification ===============================================//

    virtual KisID id() const { return m_id; }

    void setColorSpaceType(Q_UINT32 type) { m_cmType = type; }
    Q_UINT32 colorSpaceType() { return m_cmType; }

    virtual icColorSpaceSignature colorSpaceSignature() { return m_colorSpaceSignature; }

    //========== Capabilities =================================================//

    virtual KisCompositeOpList userVisiblecompositeOps() const = 0;

    virtual bool valid() { return true; }

    /**
     * Returns true if the colorspace supports channel values outside the 
     * (normalised) range 0 to 1.
     */
    virtual bool hasHighDynamicRange() const { return false; }

    //========== Display profiles =============================================//

    virtual KisProfile * getProfile() { return m_profile; };


//================= Conversion functions ==================================//


    virtual void fromQColor(const QColor& c, Q_UINT8 *dst) = 0;
    virtual void fromQColor(const QColor& c, Q_UINT8 opacity, Q_UINT8 *dst) = 0;

    virtual void toQColor(const Q_UINT8 *src, QColor *c) = 0;
    virtual void toQColor(const Q_UINT8 *src, QColor *c, Q_UINT8 *opacity) = 0;

    virtual KisPixelRO toKisPixelRO(const Q_UINT8 *src) = 0;
    
    virtual KisPixel toKisPixel(Q_UINT8 *src) = 0;

    virtual bool convertTo(KisPixel& src, KisPixel& dst, Q_INT32 renderingIntent = INTENT_PERCEPTUAL);

    virtual QImage convertToQImage(const Q_UINT8 *data, Q_INT32 width, Q_INT32 height,
                                   KisProfile *  dstProfile,
                                   Q_INT32 renderingIntent = INTENT_PERCEPTUAL,
                                   float exposure = 0.0f);

    virtual bool convertPixelsTo(const Q_UINT8 * src,
                                 Q_UINT8 * dst, KisColorSpace * dstColorSpace,
                                 Q_UINT32 numPixels,
                                 Q_INT32 renderingIntent = INTENT_PERCEPTUAL);

    virtual Q_UINT8 scaleToU8(const Q_UINT8 * srcPixel, Q_INT32 channelPos) = 0;

     virtual Q_UINT16 scaleToU16(const Q_UINT8 * srcPixel, Q_INT32 channelPos) = 0;

//============================== Manipulation fucntions ==========================//


//
// The manipulation functions have default implementations that _convert_ the pixel
// to a QColor and back. Reimplement these methods in your color strategy!
//
    virtual KisColorAdjustment *createBrightnessContrastAdjustment(Q_UINT16 *transferValues);

    virtual KisColorAdjustment *createDesaturateAdjustment();

    virtual void applyAdjustment(const Q_UINT8 *src, Q_UINT8 *dst, KisColorAdjustment *, Q_INT32 nPixels);

    virtual void invertColor(Q_UINT8 * src, Q_INT32 nPixels);
    
    virtual Q_INT8 difference(const Q_UINT8* src1, const Q_UINT8* src2);

    virtual void mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const;

    virtual void convolveColors(Q_UINT8** colors, Q_INT32* kernelValues, enumChannelFlags channelFlags, Q_UINT8 *dst, Q_INT32 factor, Q_INT32 offset, Q_INT32 nPixels) const;

    virtual void darken(const Q_UINT8 * src, Q_UINT8 * dst, Q_INT32 shade, bool compensate, double compensation, Q_INT32 nPixels) const;

    virtual Q_UINT8 intensity8(const Q_UINT8 * src) const;

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

    cmsHTRANSFORM m_defaultToRGB;
    cmsHTRANSFORM m_defaultFromRGB;
    cmsHTRANSFORM m_defaultToXYZ;
    cmsHTRANSFORM m_defaultFromXYZ;

    KisProfile *  m_profile;
    KisProfile *  m_lastUsedDstProfile;
    cmsHTRANSFORM m_lastUsedTransform;

    QValueVector<KisChannelInfo *> m_channels;

    KisColorSpaceFactoryRegistry * m_parent;

private:

    KisID m_id;
    DWORD m_cmType;                           // The colorspace type as defined by littlecms
    icColorSpaceSignature m_colorSpaceSignature; // The colorspace signature as defined in icm/icc files

    typedef QMap<KisProfilePair, cmsHTRANSFORM>  TransformMap;
    TransformMap m_transforms; // Cache for existing transforms

    KisAbstractColorSpace(const KisAbstractColorSpace&);
    KisAbstractColorSpace& operator=(const KisAbstractColorSpace&);

    QMemArray<Q_UINT8> m_conversionCache; // XXX: :This will be a bad problem when we have threading.
};

#endif // KIS_STRATEGY_COLORSPACE_H_
