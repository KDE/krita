/*
 *  SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-only
 */

#include "kis_tonemap_adjustment.h"
#include "KoColorTransferFunctions.h"

#include <KoColorSpace.h>
#include <KoColorModelStandardIds.h>
#include <KoColorTransformation.h>
#include <KoID.h>

#include <KoGamutMappingUtils.h>

#include <kis_debug.h>

namespace {

struct XYZLuminanceModel {
    static ALWAYS_INLINE float luminance (const QVector<float> channels) {
        return channels[1];
    }

    static ALWAYS_INLINE QVector<float> setLuminance(const QVector<float> channels, const float luminance) {
        QVector<float> newChannels = channels;
        float sum = channels[0]+channels[1]+channels[2];
        float x = channels[0]/sum;
        float y = channels[1]/sum;
        float Y = luminance;
        newChannels[0] = (x*Y)/y;
        newChannels[1] = Y;
        newChannels[2] = ((1.0-x-y)*Y)/y;
        return newChannels;
    }
};

struct LABLuminanceModel {
    static ALWAYS_INLINE float luminance (const QVector<float> channels) {
        return powf(channels[0], 3.0);
    }

    static ALWAYS_INLINE QVector<float> setLuminance(const QVector<float> channels, const float luminance) {
        QVector<float> newChannels = channels;
        newChannels[0] = cbrtf(luminance);
        return newChannels;
    }
};

}

template <class LuminanceModel>
class KisReinhardToneMapAdjustment: public KoColorTransformation
{
public:
    KisReinhardToneMapAdjustment(const KoColorSpace *sourceSpace): m_sourceSpace(sourceSpace) {}

    void transform(const quint8 *srcU8, quint8 *dstU8, qint32 nPixels) const override
    {
        if (!m_sourceSpace) {
            qWarning() << "no source space!";
            return;
        }

        while (nPixels > 0) {
            QVector<float> originalChannels(m_sourceSpace->channelCount());
            quint8* tmpOriginal = new quint8(m_sourceSpace->pixelSize());
            m_sourceSpace->normalisedChannelsValue(srcU8, originalChannels);

            float Y = LuminanceModel::luminance(originalChannels);
            Y = KoGamutMappingUtils::reinhardToneMapping(Y, m_maxWhite);
            originalChannels = LuminanceModel::setLuminance(originalChannels, Y);

            m_sourceSpace->fromNormalisedChannelsValue(tmpOriginal, originalChannels);
            memcpy(dstU8, tmpOriginal, m_sourceSpace->pixelSize());

            --nPixels;
            srcU8 += m_sourceSpace->pixelSize();
            dstU8 += m_sourceSpace->pixelSize();
        }
    }

    QList<QString> parameters() const override
    {
        QList<QString> list;
        list << "maxWhite";
        return list;
    }

    int parameterId(const QString& name) const override
    {
        if (name == "maxWhite") {
            return 0;
        }
        return -1;
    }

    void setParameter(int id, const QVariant& parameter) override
    {
        switch(id)
        {
        case 0:
            m_maxWhite = parameter.toFloat();
            break;
        default:
            ;
        }
    }

private:
    const KoColorSpace *m_sourceSpace;
    float m_maxWhite{1.0};

};

KisToneMapAdjustmentFactory::KisToneMapAdjustmentFactory()
    : KoColorTransformationFactory("tonemap_adjustment")
{

}

QList<QPair<KoID, KoID> > KisToneMapAdjustmentFactory::supportedModels() const
{
    QList< QPair< KoID, KoID > > l;
    // TODO: consider 16bit float.
    l.append(QPair< KoID, KoID >(XYZAColorModelID , Float32BitsColorDepthID));
    l.append(QPair< KoID, KoID >(LABAColorModelID , Float32BitsColorDepthID));
    return l;
}

KoColorTransformation *KisToneMapAdjustmentFactory::createTransformation(const KoColorSpace *colorSpace, QHash<QString, QVariant> parameters) const
{
    KoColorTransformation * adj;

    if (colorSpace->colorModelId() == XYZAColorModelID) {
        adj = new KisReinhardToneMapAdjustment<XYZLuminanceModel>(colorSpace);
    } else if (colorSpace->colorModelId() == LABAColorModelID) {
        adj = new KisReinhardToneMapAdjustment<LABLuminanceModel>(colorSpace);
    } else {
        dbgKrita << "Unsupported color space " << colorSpace->id() << " in KisGamutMapClipAdjustmentFactory::createTransformation";
        return nullptr;
    }

    adj->setParameters(parameters);
    return adj;
}
