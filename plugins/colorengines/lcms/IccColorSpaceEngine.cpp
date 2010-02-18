/*
 *  Copyright (c) 2007-2008 Cyrille Berger <cberger@cberger.net>
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

#include "IccColorSpaceEngine.h"

#include "KoColorModelStandardIds.h"

#include <kconfiggroup.h>
#include <klocale.h>

#include "LcmsColorSpace.h"

#include "DebugPigment.h"

// -- KoLcmsColorConversionTransformation --

class KoLcmsColorConversionTransformation : public KoColorConversionTransformation
{
public:
    KoLcmsColorConversionTransformation(const KoColorSpace* srcCs, quint32 srcColorSpaceType, LcmsColorProfileContainer* srcProfile, const KoColorSpace* dstCs, quint32 dstColorSpaceType, LcmsColorProfileContainer* dstProfile, Intent renderingIntent = IntentPerceptual) : KoColorConversionTransformation(srcCs, dstCs, renderingIntent), m_transform(0) {
        m_transform = this->createTransform(
                          srcColorSpaceType,
                          srcProfile,
                          dstColorSpaceType,
                          dstProfile,
                          renderingIntent);
    }
    ~KoLcmsColorConversionTransformation() {
        cmsDeleteTransform(m_transform);
    }
public:
    virtual void transform(const quint8 *src, quint8 *dst, qint32 numPixels) const {
        Q_ASSERT(m_transform);

        qint32 srcPixelSize = srcColorSpace()->pixelSize();
        qint32 dstPixelSize = dstColorSpace()->pixelSize();

        cmsDoTransform(m_transform, const_cast<quint8 *>(src), dst, numPixels);

        // Lcms does nothing to the destination alpha channel so we must convert that manually.
        while (numPixels > 0) {
            quint8 alpha = srcColorSpace()->opacityU8(src);
            dstColorSpace()->setOpacity(dst, alpha, 1);

            src += srcPixelSize;
            dst += dstPixelSize;
            numPixels--;
        }

    }
private:
    cmsHTRANSFORM createTransform(
        quint32 srcColorSpaceType,
        LcmsColorProfileContainer *  srcProfile,
        quint32 dstColorSpaceType,
        LcmsColorProfileContainer *  dstProfile,
        qint32 renderingIntent) const;
private:
    mutable cmsHTRANSFORM m_transform;
};

cmsHTRANSFORM KoLcmsColorConversionTransformation::createTransform(
    quint32 srcColorSpaceType,
    LcmsColorProfileContainer *  srcProfile,
    quint32 dstColorSpaceType,
    LcmsColorProfileContainer *  dstProfile,
    qint32 renderingIntent) const
{
    KConfigGroup cfg = KGlobal::config()->group("");
    bool bpCompensation = cfg.readEntry("useBlackPointCompensation", false);

    int flags = 0;

    if (bpCompensation) {
        flags = cmsFLAGS_BLACKPOINTCOMPENSATION;
    }
    cmsHTRANSFORM tf = cmsCreateTransform(srcProfile->lcmsProfile(),
                                          srcColorSpaceType,
                                          dstProfile->lcmsProfile(),
                                          dstColorSpaceType,
                                          renderingIntent,
                                          flags);

    return tf;
}


struct IccColorSpaceEngine::Private {
};

IccColorSpaceEngine::IccColorSpaceEngine() : KoColorSpaceEngine("icc", i18n("ICC Engine")), d(new Private)
{
}

IccColorSpaceEngine::~IccColorSpaceEngine()
{
    delete d;
}

KoColorConversionTransformation* IccColorSpaceEngine::createColorTransformation(const KoColorSpace* srcColorSpace, const KoColorSpace* dstColorSpace, KoColorConversionTransformation::Intent renderingIntent) const
{
    return new KoLcmsColorConversionTransformation(
               srcColorSpace, computeColorSpaceType(srcColorSpace),
               dynamic_cast<const IccColorProfile*>(srcColorSpace->profile())->asLcms(), dstColorSpace, computeColorSpaceType(dstColorSpace),
               dynamic_cast<const IccColorProfile*>(dstColorSpace->profile())->asLcms(), renderingIntent);

}
quint32 IccColorSpaceEngine::computeColorSpaceType(const KoColorSpace* cs) const
{
    QString modelId = cs->colorModelId().id();
    QString depthId = cs->colorDepthId().id();
    // Compute the depth part of the type
    quint32 depthType;
    if (depthId == Integer8BitsColorDepthID.id()) {
        depthType = BYTES_SH(1);
    } else if (depthId == Integer16BitsColorDepthID.id()) {
        depthType = BYTES_SH(2);
    } else {
        dbgPigmentCS << "Unknow bit depth";
        return 0;
    }
    // Compute the model part of the type
    quint32 modelType;
    if (modelId == RGBAColorModelID.id()) {
        modelType = (COLORSPACE_SH(PT_RGB) | EXTRA_SH(1) | CHANNELS_SH(3) | DOSWAP_SH(1) | SWAPFIRST_SH(1));
    } else if (modelId == XYZAColorModelID.id()) {
        modelType = (COLORSPACE_SH(PT_XYZ) | EXTRA_SH(1) | CHANNELS_SH(3));
    } else if (modelId == LABAColorModelID.id()) {
        modelType = (COLORSPACE_SH(PT_Lab) | EXTRA_SH(1) | CHANNELS_SH(3));
    } else if (modelId == CMYKAColorModelID.id()) {
        modelType = (COLORSPACE_SH(PT_CMYK) | EXTRA_SH(1) | CHANNELS_SH(4));
    } else if (modelId == GrayAColorModelID.id()) {
        modelType = (COLORSPACE_SH(PT_GRAY) | EXTRA_SH(1) | CHANNELS_SH(1));
    } else if (modelId == GrayColorModelID.id()) {
        modelType = (COLORSPACE_SH(PT_GRAY) | CHANNELS_SH(1));
    } else if (modelId == YCbCrAColorModelID.id()) {
        modelType = (COLORSPACE_SH(PT_YCbCr) | EXTRA_SH(1) | CHANNELS_SH(3));
    } else {
        dbgPigmentCS << "Unknow color model";
        return 0;
    }
    return depthType | modelType;
}
