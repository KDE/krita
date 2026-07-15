/*
 *  SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-only
 */

#include "kis_gamutmap_clip_adjustment.h"

#include <kis_debug.h>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>

#include <KoColorConversions.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpaceTraits.h>
#include <KoColorTransformation.h>
#include <KoID.h>

template<typename _channel_type_,typename traits>
class KisGamutMapClipAdjustment : public KoColorTransformation
{
    typedef traits PixelTrait;
    typedef typename PixelTrait::Pixel Pixel;
public:
    KisGamutMapClipAdjustment(const KoColorSpace *sourceSpace): m_sourceSpace(sourceSpace) {}
public:

    static qreal difference(const QVector<float> &a, const QVector<float> &b) {
        //This is E76, but we should also have a E2000 implementation.
        float dL = a[0] - b[0];
        float dA = a[1] - b[1];
        float dB = a[2] - b[2];
        return qSqrt(dL * dL + dA * dA + dB * dB);
    }

    // Algorithm based of https://drafts.csswg.org/css-color-4/#pseudo-binsearch
    void transform(const quint8 *srcU8, quint8 *dstU8, qint32 nPixels) const override
    {
        if (!m_sourceSpace) {
            qWarning() << "no source space!";
            return;
        }
        const KoColorProfile *targetProfile = KoColorSpaceRegistry::instance()->profileByName(m_targetColorProfileName);
        if (!targetProfile) {
            qWarning() << "no target profile found for..." << m_targetColorProfileName;
            return;
        }
        const KoColorSpace *targetSpace = KoColorSpaceRegistry::instance()->colorSpace(targetProfile->colorModelID(), Integer16BitsColorDepthID.id(), targetProfile);
        if (!targetSpace) {
            qWarning() << "no target space!";
            return;
        }

        while (nPixels > 0) {
            // We could make good use of a LUT here.
            QVector<float> originalChannels(m_sourceSpace->channelCount());
            QVector<float> clippedChannels(m_sourceSpace->channelCount());
            quint8* tmpOriginal = new quint8(m_sourceSpace->pixelSize());
            quint8* tmpTarget = new quint8(targetSpace->pixelSize());
            quint8* tmpClipped = new quint8(m_sourceSpace->pixelSize());

            m_sourceSpace->normalisedChannelsValue(srcU8, originalChannels);
            m_sourceSpace->fromNormalisedChannelsValue(tmpOriginal, originalChannels);

            m_sourceSpace->convertPixelsTo(tmpOriginal, tmpTarget, targetSpace, 1, KoColorConversionTransformation::IntentRelativeColorimetric, KoColorConversionTransformation::adjustmentConversionFlags());
            targetSpace->convertPixelsTo(tmpTarget, tmpClipped, m_sourceSpace, 1, KoColorConversionTransformation::IntentRelativeColorimetric, KoColorConversionTransformation::adjustmentConversionFlags());

            m_sourceSpace->normalisedChannelsValue(tmpClipped, clippedChannels);

            QVector<qreal> originalLCH(3);
            QVector<qreal> modifiedChannels(3);
            QVector<qreal> clippedLCH(3);
            if (difference(originalChannels, clippedChannels) > 0.02) {
                LabToLCH(float(originalChannels[0]), float(originalChannels[1]), float(originalChannels[2]), &originalLCH[0], &originalLCH[1], &originalLCH[2]);
                float min = 0;
                float max = originalLCH[1];

                while (max-min > 0.0001) {

                    LabToLCH(float(originalChannels[0]), float(originalChannels[1]), float(originalChannels[2]), &originalLCH[0], &originalLCH[1], &originalLCH[2]);
                    LabToLCH(float(clippedChannels[0]), float(clippedChannels[1]), float(clippedChannels[2]), &clippedLCH[0], &clippedLCH[1], &clippedLCH[2]);

                    qreal newChroma = (max + min) * 0.5;

                    LCHToLab(originalLCH[0], newChroma, originalLCH[2], &modifiedChannels[0], &modifiedChannels[1], &modifiedChannels[2]);
                    originalChannels[0] = modifiedChannels[0];
                    originalChannels[1] = modifiedChannels[1];
                    originalChannels[2] = modifiedChannels[2];

                    m_sourceSpace->fromNormalisedChannelsValue(tmpOriginal, originalChannels);
                    m_sourceSpace->convertPixelsTo(tmpOriginal, tmpTarget, targetSpace, 1, KoColorConversionTransformation::IntentRelativeColorimetric, KoColorConversionTransformation::adjustmentConversionFlags());
                    targetSpace->convertPixelsTo(tmpTarget, tmpClipped, m_sourceSpace, 1, KoColorConversionTransformation::IntentRelativeColorimetric, KoColorConversionTransformation::adjustmentConversionFlags());
                    m_sourceSpace->normalisedChannelsValue(tmpClipped, clippedChannels);

                    if (difference(originalChannels, clippedChannels) < 0.02) {
                        min = newChroma;
                    } else {
                        max = newChroma;
                    }
                }
            }

            memcpy(dstU8, tmpOriginal, m_sourceSpace->pixelSize());

            --nPixels;
            srcU8 += m_sourceSpace->pixelSize();
            dstU8 += m_sourceSpace->pixelSize();
        }
    }

    QList<QString> parameters() const override
    {
        QList<QString> list;
        list << "targetColorProfileName";
        return list;
    }

    int parameterId(const QString& name) const override
    {
        if (name == "targetColorProfileName") {
            return 0;
        }
        return -1;
    }

    void setParameter(int id, const QVariant& parameter) override
    {
        switch(id)
        {
        case 0:
            m_targetColorProfileName = parameter.toString();
            break;
        default:
            ;
        }
    }

private:

    QString m_targetColorProfileName;
    QString m_targetColorSpaceModel;
    const KoColorSpace *m_sourceSpace;
};

KisGamutMapClipAdjustmentFactory::KisGamutMapClipAdjustmentFactory()
    : KoColorTransformationFactory("gamutmap_clip_adjustment")
{

}

QList<QPair<KoID, KoID> > KisGamutMapClipAdjustmentFactory::supportedModels() const
{
    QList< QPair< KoID, KoID > > l;
    // While the other two work for gamut mapping, they clip HDR values.
    l.append(QPair< KoID, KoID >(LABAColorModelID , Float32BitsColorDepthID));

    // Note: Only ICtCp is really supported.
    l.append(QPair< KoID, KoID >(YCbCrAColorModelID , Float32BitsColorDepthID));
    return l;
}

KoColorTransformation *KisGamutMapClipAdjustmentFactory::createTransformation(const KoColorSpace *colorSpace, QHash<QString, QVariant> parameters) const
{
    KoColorTransformation * adj;

    if (colorSpace->colorModelId() == LABAColorModelID) {
        if (colorSpace->colorDepthId() == Integer8BitsColorDepthID) {
            adj = new KisGamutMapClipAdjustment< quint8, KoLabTraits < quint8 > >(colorSpace);
        } else if (colorSpace->colorDepthId() == Integer16BitsColorDepthID) {
            adj = new KisGamutMapClipAdjustment< quint16, KoLabTraits < quint16 > >(colorSpace);
        } else if (colorSpace->colorDepthId() == Float32BitsColorDepthID) {
            adj = new KisGamutMapClipAdjustment< float, KoLabTraits < float > >(colorSpace);
        } else {
            dbgKrita << "Unsupported color space " << colorSpace->id() << " in KisGamutMapClipAdjustmentFactory::createTransformation";
            return 0;
        }
    } else if (colorSpace->colorModelId() == YCbCrAColorModelID) {
        if (colorSpace->colorDepthId() == Integer8BitsColorDepthID) {
            adj = new KisGamutMapClipAdjustment< quint8, KoYCbCrTraits < quint8 > >(colorSpace);
        } else if (colorSpace->colorDepthId() == Integer16BitsColorDepthID) {
            adj = new KisGamutMapClipAdjustment< quint16, KoYCbCrTraits < quint16 > >(colorSpace);
        } else if (colorSpace->colorDepthId() == Float32BitsColorDepthID) {
            adj = new KisGamutMapClipAdjustment< float, KoYCbCrTraits < float > >(colorSpace);
        } else {
            dbgKrita << "Unsupported color space " << colorSpace->id() << " in KisGamutMapClipAdjustmentFactory::createTransformation";
            return 0;
        }
    } else {
        dbgKrita << "Unsupported color space " << colorSpace->id() << " in KisGamutMapClipAdjustmentFactory::createTransformation";
        return 0;
    }

    adj->setParameters(parameters);
    return adj;
}
