/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2005-2006 Casper Boemann <cbr@boemann.dk>
 *  Copyright (c) 2004,2006-2007 Cyrille Berger <cberger@cberger.net>
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

#include "KoLcmsColorSpace.h"
#include "KoColorConversionTransformationFactory.h"
#include "KoColorModelStandardIds.h"

#include "kconfiggroup.h"
#include "kdebug.h"


// -- KoLcmsColorConversionTransformationFactory --

class KoLcmsColorConversionTransformationFactory : public KoColorConversionTransformationFactory {
    public:
        KoLcmsColorConversionTransformationFactory(QString _srcModelId, QString _srcDepthId, QString _dstModelId, QString _dstDepthId);
        virtual KoColorConversionTransformation* createColorTransformation(const KoColorSpace* srcColorSpace, const KoColorSpace* dstColorSpace, KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual);
        virtual bool conserveColorInformation() const;
        virtual bool conserveDynamicRange() const;
    private:
        quint32 computeColorSpaceType(QString _modelId, QString _depthId);
    private:
        quint32 m_srcColorSpaceType, m_dstColorSpaceType;
        bool m_conserveColorInformation;
};

KoLcmsColorConversionTransformationFactory::KoLcmsColorConversionTransformationFactory(QString _srcModelId, QString _srcDepthId, QString _dstModelId, QString _dstDepthId) : KoColorConversionTransformationFactory(_srcModelId, _srcDepthId, _dstModelId, _dstDepthId)
{
    m_srcColorSpaceType = computeColorSpaceType( _srcModelId, _srcDepthId);
    Q_ASSERT(m_srcColorSpaceType);
    m_dstColorSpaceType = computeColorSpaceType( _dstModelId, _dstDepthId);
    Q_ASSERT(m_dstColorSpaceType);
    m_conserveColorInformation = not (_dstModelId == GrayAColorModelID.id() or _dstModelId == GrayColorModelID.id()); // color information is lost when converting to Grayscale
}

KoColorConversionTransformation* KoLcmsColorConversionTransformationFactory::createColorTransformation(const KoColorSpace* srcColorSpace, const KoColorSpace* dstColorSpace, KoColorConversionTransformation::Intent renderingIntent )
{
    return new KoLcmsColorConversionTransformation(srcColorSpace, m_srcColorSpaceType, dynamic_cast<KoIccColorProfile*>(srcColorSpace->profile())->asLcms(), dstColorSpace, m_dstColorSpaceType, dynamic_cast<KoIccColorProfile*>(dstColorSpace->profile())->asLcms(), renderingIntent);
}

bool KoLcmsColorConversionTransformationFactory::conserveColorInformation() const
{
    return m_conserveColorInformation;
}

bool KoLcmsColorConversionTransformationFactory::conserveDynamicRange() const
{
    return false; // LCMS color transformation allways lose dynamic range
}

quint32 KoLcmsColorConversionTransformationFactory::computeColorSpaceType(QString _modelId, QString _depthId)
{
    // Compute the depth part of the type
    quint32 depthType;
    if(_depthId == Integer8BitsColorDepthID.id())
    {
        depthType = BYTES_SH(1);
    } else if(_depthId == Integer16BitsColorDepthID.id()) {
        depthType = BYTES_SH(2);
    } else {
        kDebug(31000) << "Unknow bit depth";
        return 0;
    }
    // Compute the model part of the type
    quint32 modelType;
    if(_modelId == RGBAColorModelID.id())
    {
        modelType = (COLORSPACE_SH(PT_RGB)|EXTRA_SH(1)|CHANNELS_SH(3)|DOSWAP_SH(1)|SWAPFIRST_SH(1));
    } else if(_modelId == XYZAColorModelID.id())
    {
        modelType = (COLORSPACE_SH(PT_XYZ)|EXTRA_SH(1)|CHANNELS_SH(3));
    } else if(_modelId == LABAColorModelID.id())
    {
        modelType = (COLORSPACE_SH(PT_Lab)|EXTRA_SH(1)|CHANNELS_SH(3));
    } else if(_modelId == CMYKAColorModelID.id())
    {
        modelType = (COLORSPACE_SH(PT_CMYK)|EXTRA_SH(1)|CHANNELS_SH(4));
    } else if(_modelId == GrayAColorModelID.id())
    {
        modelType = (COLORSPACE_SH(PT_GRAY)|EXTRA_SH(1)|CHANNELS_SH(1));
    } else if(_modelId == GrayColorModelID.id())
    {
        modelType = (COLORSPACE_SH(PT_GRAY)|CHANNELS_SH(1));
    } else if(_modelId == YCbCrAColorModelID.id())
    {
        modelType = (COLORSPACE_SH(PT_YCbCr)|EXTRA_SH(1)|CHANNELS_SH(3));
    } else {
        kDebug(31000) << "Unknow color model";
        return 0;
    }
    return depthType|modelType;
}

// -- KoLcmsColorConversionTransformation --

cmsHTRANSFORM KoLcmsColorConversionTransformation::createTransform(
        quint32 srcColorSpaceType,
        KoLcmsColorProfileContainer *  srcProfile,
        quint32 dstColorSpaceType,
        KoLcmsColorProfileContainer *  dstProfile,
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

// -- KoLcmsColorSpaceFactory --
QList<KoColorConversionTransformationFactory*> KoLcmsColorSpaceFactory::colorConversionLinks() const
{
    return QList<KoColorConversionTransformationFactory*>();
}

KoColorConversionTransformationFactory* KoLcmsColorSpaceFactory::createICCColorConversionTransformationFactory(QString _colorModelId, QString _colorDepthId) const
{
    return new KoLcmsColorConversionTransformationFactory( colorModelId().id(), colorDepthId().id(), _colorModelId, _colorDepthId);
}
