/*
 *  Copyright (c) 2007-2008 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
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

#include "IccColorSpaceEngine.h"

#include "KoColorModelStandardIds.h"

#include <klocalizedstring.h>

#include "LcmsColorSpace.h"

// -- KoLcmsColorConversionTransformation --

class KoLcmsColorConversionTransformation : public KoColorConversionTransformation
{
public:
    KoLcmsColorConversionTransformation(const KoColorSpace *srcCs, quint32 srcColorSpaceType, LcmsColorProfileContainer *srcProfile,
                                        const KoColorSpace *dstCs, quint32 dstColorSpaceType, LcmsColorProfileContainer *dstProfile,
                                        Intent renderingIntent,
                                        ConversionFlags conversionFlags)
        : KoColorConversionTransformation(srcCs, dstCs, renderingIntent, conversionFlags)
        , m_transform(0)
    {
        Q_ASSERT(srcCs);
        Q_ASSERT(dstCs);
        Q_ASSERT(renderingIntent < 4);

        if (srcCs->colorDepthId() == Integer8BitsColorDepthID
                || srcCs->colorDepthId() == Integer16BitsColorDepthID) {

            if ((srcProfile->name().contains(QLatin1String("linear"), Qt::CaseInsensitive) ||
                 dstProfile->name().contains(QLatin1String("linear"), Qt::CaseInsensitive)) &&
                    !conversionFlags.testFlag(KoColorConversionTransformation::NoOptimization)) {
                conversionFlags |= KoColorConversionTransformation::NoOptimization;
            }
        }

        m_transform = cmsCreateTransform(srcProfile->lcmsProfile(),
                                         srcColorSpaceType,
                                         dstProfile->lcmsProfile(),
                                         dstColorSpaceType,
                                         renderingIntent,
                                         conversionFlags);

        Q_ASSERT(m_transform);
    }

    ~KoLcmsColorConversionTransformation() override
    {
        cmsDeleteTransform(m_transform);
    }

public:

    void transform(const quint8 *src, quint8 *dst, qint32 numPixels) const override
    {
        Q_ASSERT(m_transform);

        qint32 srcPixelSize = srcColorSpace()->pixelSize();
        qint32 dstPixelSize = dstColorSpace()->pixelSize();

        cmsDoTransform(m_transform, const_cast<quint8 *>(src), dst, numPixels);
        // Lcms does nothing to the destination alpha channel so we must convert that manually.
        while (numPixels > 0) {
            qreal alpha = srcColorSpace()->opacityF(src);
            dstColorSpace()->setOpacity(dst, alpha, 1);

            src += srcPixelSize;
            dst += dstPixelSize;
            numPixels--;
        }

    }
private:
    mutable cmsHTRANSFORM m_transform;
};

class KoLcmsColorProofingConversionTransformation : public KoColorProofingConversionTransformation
{
public:
    KoLcmsColorProofingConversionTransformation(const KoColorSpace *srcCs, quint32 srcColorSpaceType, LcmsColorProfileContainer *srcProfile,
                                                const KoColorSpace *dstCs, quint32 dstColorSpaceType, LcmsColorProfileContainer *dstProfile,
                                                const KoColorSpace *proofingSpace,
                                                Intent renderingIntent,
                                                Intent proofingIntent,
                                                ConversionFlags conversionFlags,
                                                quint8 *gamutWarning,
                                                double adaptationState
                                                )
        : KoColorProofingConversionTransformation(srcCs, dstCs, proofingSpace, renderingIntent, proofingIntent, conversionFlags, gamutWarning, adaptationState)
        , m_transform(0)
    {
        Q_ASSERT(srcCs);
        Q_ASSERT(dstCs);
        Q_ASSERT(renderingIntent < 4);

        if (srcCs->colorDepthId() == Integer8BitsColorDepthID
                || srcCs->colorDepthId() == Integer16BitsColorDepthID) {

            if ((srcProfile->name().contains(QLatin1String("linear"), Qt::CaseInsensitive) ||
                 dstProfile->name().contains(QLatin1String("linear"), Qt::CaseInsensitive)) &&
                    !conversionFlags.testFlag(KoColorConversionTransformation::NoOptimization)) {
                conversionFlags |= KoColorConversionTransformation::NoOptimization;
            }
        }

        quint16 alarm[cmsMAXCHANNELS];//this seems to be bgr???
        alarm[0] = (cmsUInt16Number)gamutWarning[2]*256;
        alarm[1] = (cmsUInt16Number)gamutWarning[1]*256;
        alarm[2] = (cmsUInt16Number)gamutWarning[0]*256;
        cmsSetAlarmCodes(alarm);
        cmsSetAdaptationState(adaptationState);

        m_transform = cmsCreateProofingTransform(srcProfile->lcmsProfile(),
                                                 srcColorSpaceType,
                                                 dstProfile->lcmsProfile(),
                                                 dstColorSpaceType,
                                                 dynamic_cast<const IccColorProfile *>(proofingSpace->profile())->asLcms()->lcmsProfile(),
                                                 renderingIntent,
                                                 proofingIntent,
                                                 conversionFlags);
        cmsSetAdaptationState(1);

        Q_ASSERT(m_transform);
    }

    ~KoLcmsColorProofingConversionTransformation() override
    {
        cmsDeleteTransform(m_transform);
    }

public:

    void transform(const quint8 *src, quint8 *dst, qint32 numPixels) const override
    {
        Q_ASSERT(m_transform);

        qint32 srcPixelSize = srcColorSpace()->pixelSize();
        qint32 dstPixelSize = dstColorSpace()->pixelSize();
        //cmsSetAdaptationState(0);

        cmsDoTransform(m_transform, const_cast<quint8 *>(src), dst, numPixels);
        // Lcms does nothing to the destination alpha channel so we must convert that manually.
        while (numPixels > 0) {
            qreal alpha = srcColorSpace()->opacityF(src);
            dstColorSpace()->setOpacity(dst, alpha, 1);

            src += srcPixelSize;
            dst += dstPixelSize;
            numPixels--;
        }
        //cmsSetAdaptationState(1);

    }
private:
    mutable cmsHTRANSFORM m_transform;
};

struct IccColorSpaceEngine::Private {
};

IccColorSpaceEngine::IccColorSpaceEngine() : KoColorSpaceEngine("icc", i18n("ICC Engine")), d(new Private)
{
}

IccColorSpaceEngine::~IccColorSpaceEngine()
{
    delete d;
}

const KoColorProfile* IccColorSpaceEngine::addProfile(const QString &filename)
{
    KoColorSpaceRegistry *registry = KoColorSpaceRegistry::instance();

    KoColorProfile *profile = new IccColorProfile(filename);
    Q_CHECK_PTR(profile);

    // this our own loading code; sometimes it fails because of an lcms error
    profile->load();

    // and then lcms can read the profile from file itself without problems,
    // quite often, and we can initialize it
    if (!profile->valid()) {
        cmsHPROFILE cmsp = cmsOpenProfileFromFile(filename.toLatin1(), "r");
        profile = LcmsColorProfileContainer::createFromLcmsProfile(cmsp);
    }

    if (profile->valid()) {
        dbgPigment << "Valid profile : " << profile->fileName() << profile->name();
        registry->addProfile(profile);
    } else {
        dbgPigment << "Invalid profile : " << profile->fileName() << profile->name();
        delete profile;
        profile = 0;
    }

    return profile;
}

const KoColorProfile* IccColorSpaceEngine::addProfile(const QByteArray &data)
{
    KoColorSpaceRegistry *registry = KoColorSpaceRegistry::instance();

    KoColorProfile *profile = new IccColorProfile(data);
    Q_CHECK_PTR(profile);

    if (profile->valid()) {
        dbgPigment << "Valid profile : " << profile->fileName() << profile->name();
        registry->addProfile(profile);
    } else {
        dbgPigment << "Invalid profile : " << profile->fileName() << profile->name();
        delete profile;
        profile = 0;
    }

    return profile;
}

void IccColorSpaceEngine::removeProfile(const QString &filename)
{
    KoColorSpaceRegistry *registry = KoColorSpaceRegistry::instance();

    KoColorProfile *profile = new IccColorProfile(filename);
    Q_CHECK_PTR(profile);
    profile->load();

    if (profile->valid() && registry->profileByName(profile->name())) {
        registry->removeProfile(profile);
    }
}

KoColorConversionTransformation *IccColorSpaceEngine::createColorTransformation(const KoColorSpace *srcColorSpace,
                                                                                const KoColorSpace *dstColorSpace,
                                                                                KoColorConversionTransformation::Intent renderingIntent,
                                                                                KoColorConversionTransformation::ConversionFlags conversionFlags) const
{
    Q_ASSERT(srcColorSpace);
    Q_ASSERT(dstColorSpace);

    return new KoLcmsColorConversionTransformation(
                srcColorSpace, computeColorSpaceType(srcColorSpace),
                dynamic_cast<const IccColorProfile *>(srcColorSpace->profile())->asLcms(), dstColorSpace, computeColorSpaceType(dstColorSpace),
                dynamic_cast<const IccColorProfile *>(dstColorSpace->profile())->asLcms(), renderingIntent, conversionFlags);

}
KoColorProofingConversionTransformation *IccColorSpaceEngine::createColorProofingTransformation(const KoColorSpace *srcColorSpace,
                                                                                                const KoColorSpace *dstColorSpace,
                                                                                                const KoColorSpace *proofingSpace,
                                                                                                KoColorConversionTransformation::Intent renderingIntent,
                                                                                                KoColorConversionTransformation::Intent proofingIntent,
                                                                                                KoColorConversionTransformation::ConversionFlags conversionFlags,
                                                                                                quint8 *gamutWarning,
                                                                                                double adaptationState) const
{
    Q_ASSERT(srcColorSpace);
    Q_ASSERT(dstColorSpace);

    return new KoLcmsColorProofingConversionTransformation(
                srcColorSpace, computeColorSpaceType(srcColorSpace),
                dynamic_cast<const IccColorProfile *>(srcColorSpace->profile())->asLcms(), dstColorSpace, computeColorSpaceType(dstColorSpace),
                dynamic_cast<const IccColorProfile *>(dstColorSpace->profile())->asLcms(), proofingSpace, renderingIntent, proofingIntent, conversionFlags, gamutWarning,
                adaptationState
                );
}

quint32 IccColorSpaceEngine::computeColorSpaceType(const KoColorSpace *cs) const
{
    Q_ASSERT(cs);

    if (const KoLcmsInfo *lcmsInfo = dynamic_cast<const KoLcmsInfo *>(cs)) {
        return lcmsInfo->colorSpaceType();
    } else {
        QString modelId = cs->colorModelId().id();
        QString depthId = cs->colorDepthId().id();
        // Compute the depth part of the type
        quint32 depthType;

        if (depthId == Integer8BitsColorDepthID.id()) {
            depthType = BYTES_SH(1);
        } else if (depthId == Integer16BitsColorDepthID.id()) {
            depthType = BYTES_SH(2);
        } else if (depthId == Float16BitsColorDepthID.id()) {
            depthType = BYTES_SH(2);
        } else if (depthId == Float32BitsColorDepthID.id()) {
            depthType = BYTES_SH(4);
        } else if (depthId == Float64BitsColorDepthID.id()) {
            depthType = BYTES_SH(0);
        } else {
            qWarning() << "Unknown bit depth";
            return 0;
        }
        // Compute the model part of the type
        quint32 modelType = 0;

        if (modelId == RGBAColorModelID.id()) {
            if (depthId.startsWith(QLatin1Char('U'))) {
                modelType = (COLORSPACE_SH(PT_RGB) | EXTRA_SH(1) | CHANNELS_SH(3) | DOSWAP_SH(1) | SWAPFIRST_SH(1));
            } else if (depthId.startsWith(QLatin1Char('F'))) {
                modelType = (COLORSPACE_SH(PT_RGB) | EXTRA_SH(1) | CHANNELS_SH(3));
            }
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
            qWarning() << "Cannot convert colorspace to lcms modeltype";
            return 0;
        }
        return depthType | modelType;
    }
}

bool IccColorSpaceEngine::supportsColorSpace(const QString &colorModelId, const QString &colorDepthId, const KoColorProfile *profile) const
{
    Q_UNUSED(colorDepthId);
    return colorModelId != RGBAColorModelID.id() || !profile || profile->name() != "High Dynamic Range UHDTV Wide Color Gamut Display (Rec. 2020) - SMPTE ST 2084 PQ EOTF";
}
