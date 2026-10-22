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
    static ALWAYS_INLINE float luminance (const float *xyzChannels) {
        return xyzChannels[1];
    }

    static ALWAYS_INLINE void setLuminance(float *xyzChannels, const float luminance) {
        const float sum = xyzChannels[0] + xyzChannels[1] + xyzChannels[2];
        const float x = xyzChannels[0] / sum;
        const float y = xyzChannels[1] / sum;
        const float Y = luminance;
        xyzChannels[0] = (x*Y)/y;
        xyzChannels[1] = Y;
        xyzChannels[2] = ((1.0-x-y)*Y)/y;
    }
};

struct LABLuminanceModel {
    static ALWAYS_INLINE float luminance (const float *labChannels) {
        return powf(labChannels[0], 3.0);
    }

    static ALWAYS_INLINE void setLuminance(float *labChannels, const float luminance) {
        labChannels[0] = cbrtf(luminance);
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

        QVector<float> originalChannels(m_sourceSpace->channelCount());
        std::unique_ptr<quint8[]> tmpOriginal(new quint8[m_sourceSpace->pixelSize()]);
        while (nPixels > 0) {

            m_sourceSpace->normalisedChannelsValue(srcU8, originalChannels);

            float Y = LuminanceModel::luminance(originalChannels.data());
            Y = KoGamutMappingUtils::reinhardToneMapping(Y, m_maxWhite);
            LuminanceModel::setLuminance(originalChannels.data(), Y);

            m_sourceSpace->fromNormalisedChannelsValue(tmpOriginal.get(), originalChannels);
            memcpy(dstU8, tmpOriginal.get(), m_sourceSpace->pixelSize());

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

template <class LuminanceModel>
class KisPiecewiseFilmicToneMapAdjustment: public KoColorTransformation
{
public:
    KisPiecewiseFilmicToneMapAdjustment(const KoColorSpace *sourceSpace): m_sourceSpace(sourceSpace) {}

    void transform(const quint8 *srcU8, quint8 *dstU8, qint32 nPixels) const override
    {
        if (!m_sourceSpace) {
            qWarning() << "no source space!";
            return;
        }

        KoGamutMappingUtils::PiecewiseFilmicCurve curve =
            KoGamutMappingUtils::PiecewiseFilmicCurve::fromUserParams(m_toeStrength, m_toeLength, m_shoulderStrength, m_shoulderLength, m_shoulderAngle);

        QVector<float> originalChannels(m_sourceSpace->channelCount());
        std::unique_ptr<quint8[]> tmpOriginal(new quint8[m_sourceSpace->pixelSize()]);
        while (nPixels > 0) {
            m_sourceSpace->normalisedChannelsValue(srcU8, originalChannels);

            float Y = LuminanceModel::luminance(originalChannels.data());
            Y = curve.evalCurve(Y);
            LuminanceModel::setLuminance(originalChannels.data(), Y);

            m_sourceSpace->fromNormalisedChannelsValue(tmpOriginal.get(), originalChannels);
            memcpy(dstU8, tmpOriginal.get(), m_sourceSpace->pixelSize());

            --nPixels;
            srcU8 += m_sourceSpace->pixelSize();
            dstU8 += m_sourceSpace->pixelSize();
        }
    }

    QList<QString> parameters() const override
    {
        QList<QString> list;
        list << "toeStrength";
        list << "toeLength";
        list << "shoulderStrength";
        list << "shoulderLength";
        list << "shoulderAngle";
        return list;
    }

    int parameterId(const QString& name) const override
    {
        if (name == "toeStrength") {
            return 0;
        } else if (name == "toeLength") {
            return 1;
        } else if (name == "shoulderStrength") {
            return 2;
        } else if (name == "shoulderLength") {
            return 3;
        } else if (name == "shoulderAngle") {
            return 4;
        }
        return -1;
    }

    void setParameter(int id, const QVariant& parameter) override
    {
        switch(id)
        {
        case 0:
            m_toeStrength = parameter.toFloat();
            break;
        case 1:
            m_toeLength = parameter.toFloat();
            break;
        case 2:
            m_shoulderStrength = parameter.toFloat();
            break;
        case 3:
            m_shoulderLength = parameter.toFloat();
            break;
        case 4:
            m_shoulderAngle = parameter.toFloat();
            break;
        default:
            ;
        }
    }

private:
    const KoColorSpace *m_sourceSpace;
    float m_toeStrength{0.5};
    float m_toeLength{0.5};
    float m_shoulderStrength{2.0};
    float m_shoulderLength{0.5};
    float m_shoulderAngle{1.0};

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
        if (parameters["CurveType"].toInt() == KoGamutMappingUtils::Reinhard) {
            adj = new KisReinhardToneMapAdjustment<XYZLuminanceModel>(colorSpace);
        } else {
            adj = new KisPiecewiseFilmicToneMapAdjustment<XYZLuminanceModel>(colorSpace);
        }
    } else if (colorSpace->colorModelId() == LABAColorModelID) {
        if (parameters["CurveType"].toInt() == KoGamutMappingUtils::Reinhard) {
            adj = new KisReinhardToneMapAdjustment<LABLuminanceModel>(colorSpace);
        } else {
            adj = new KisPiecewiseFilmicToneMapAdjustment<LABLuminanceModel>(colorSpace);
        }
    } else {
        dbgKrita << "Unsupported color space " << colorSpace->id() << " in KisGamutMapClipAdjustmentFactory::createTransformation";
        return nullptr;
    }

    adj->setParameters(parameters);
    return adj;
}
