/*
 *  SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-only
 */

#include "kis_gamutmap_clip_adjustment.h"
#include "KoAlwaysInline.h"

#include <kis_debug.h>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>

#include <KoColorConversions.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpaceTraits.h>
#include <KoColorTransformation.h>
#include <KoID.h>

#include <KoGamutMappingUtils.h>
#include <KoColorTransferFunctions.h>

namespace
{


struct xyzToOkLabModel {
    static ALWAYS_INLINE std::array<qreal, 3> toLCH (const float* xyzChannels) {
        const QVector4D OkLab = KoGamutMappingUtils::XYZToOkLab(QVector4D(xyzChannels[0], xyzChannels[1], xyzChannels[2], 0.0));
        std::array<qreal, 3> lch {0.0, 0.0, 0.0};
        LabToLCH(OkLab.x(), OkLab.y()+0.5, OkLab.z()+0.5, &lch[0], &lch[1], &lch[2]);
        return lch;
    }

    static ALWAYS_INLINE void fromLCH (const std::array<qreal, 3> &lch, float *xyzChannels) {
        std::array<qreal, 3> output {0.0, 0.0, 0.0};
        LCHToLab(lch[0], lch[1], lch[2], &output[0], &output[1], &output[2]);

        const QVector4D xyz = KoGamutMappingUtils::OkLabToXYZ(QVector4D(output[0], output[1]-0.5, output[2]-0.5, 0.0));
        xyzChannels[0] = xyz.x();
        xyzChannels[1] = xyz.y();
        xyzChannels[2] = xyz.z();

        //qDebug() << "lch" << lch << "col" << color << "lmsprime" << lmsPrime << "lms" << lms << "xyz" << channels;
    }

    static ALWAYS_INLINE float cuberootLightness(float lightness) {
        return lightness;
    }
};

struct xyzToICtCpModel {
    static ALWAYS_INLINE std::array<qreal, 3> toLCH (const float *xyzChannels) {
        QVector4D color (xyzChannels[0], xyzChannels[1], xyzChannels[2], 1.0);
        const QVector4D ICtCp = KoGamutMappingUtils::XYZToICtCp(color);
        std::array<qreal, 3> lch {0.0, 0.0, 0.0};
        LabToLCH(ICtCp.x(), (ICtCp.y()+0.5), (ICtCp.z()+0.5), &lch[0], &lch[1], &lch[2]);

        return lch;
    }

    static ALWAYS_INLINE void fromLCH (const std::array<qreal, 3> &lch, float *xyzChannels) {
        std::array<qreal, 3> output {0.0, 0.0, 0.0};
        LCHToLab(lch[0], lch[1], lch[2], &output[0], &output[1], &output[2]);


        QVector4D color (output[0], output[1]-0.5, output[2]-0.5, 0.0);
        const QVector4D xyz = KoGamutMappingUtils::ICtCpToXYZ(color);
        xyzChannels[0] = xyz.x();
        xyzChannels[1] = xyz.y();
        xyzChannels[2] = xyz.z();

    }

    static ALWAYS_INLINE float cuberootLightness(float lightness) {
        return cbrtf(removeSmpte2048Curve(lightness));
    }
};

struct labModel {
    static ALWAYS_INLINE std::array<qreal, 3> toLCH (const float *labChannels) {
        std::array<qreal, 3> lch {0.0, 0.0, 0.0};
        LabToLCH(labChannels[0], labChannels[1], labChannels[2], &lch[0], &lch[1], &lch[2]);
        return lch;
    }

    static ALWAYS_INLINE void fromLCH (const std::array<qreal, 3> &lch, float *labChannels) {
        std::array<qreal, 3> output {0.0, 0.0, 0.0};
        LCHToLab(lch[0], lch[1], lch[2], &output[0], &output[1], &output[2]);
        labChannels[0] = output[0];
        labChannels[1] = output[1];
        labChannels[2] = output[2];
    }

    static ALWAYS_INLINE float cuberootLightness(float lightness) {
        return lightness;
    }
};

struct dE76 {
    static ALWAYS_INLINE float difference(const QVector<float> &a, const QVector<float> &b) {
        return KoGamutMappingUtils::deltaE76(a, b);
    }
};
}

template<class ModelType, class DifferenceType>
class KisGamutMapClipAdjustment : public KoColorTransformation
{
public:
    KisGamutMapClipAdjustment(const KoColorSpace *sourceSpace)
        : m_sourceSpace(sourceSpace) {}
public:

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
        const bool greyscale = targetProfile->colorModelID() == GrayAColorModelID.id();
        const KoColorSpace *targetSpace = KoColorSpaceRegistry::instance()->colorSpace(targetProfile->colorModelID(), Integer16BitsColorDepthID.id(), targetProfile);
        if (!targetSpace) {
            qWarning() << "no target space!";
            return;
        }

        QVector<float> originalChannels(m_sourceSpace->channelCount());
        QVector<float> clippedChannels(m_sourceSpace->channelCount());
        std::unique_ptr<quint8[]> tmpOriginal(new quint8[m_sourceSpace->pixelSize()]);
        std::unique_ptr<quint8[]> tmpTarget (new quint8[targetSpace->pixelSize()]);
        std::unique_ptr<quint8[]> tmpClipped (new quint8[m_sourceSpace->pixelSize()]);
        std::array<qreal, 3> LCH {0.0, 0.0, 0.0};

        while (nPixels > 0) {
            // We could make good use of a LUT here.
            m_sourceSpace->normalisedChannelsValue(srcU8, originalChannels);
            m_sourceSpace->fromNormalisedChannelsValue(tmpOriginal.get(), originalChannels);

            if (greyscale) {
                LCH = ModelType::toLCH(originalChannels.data());
                LCH[1] = 0;
                ModelType::fromLCH(LCH, originalChannels.data());
                m_sourceSpace->fromNormalisedChannelsValue(tmpOriginal.get(), originalChannels);
            } else if (!m_cuspDescription.isEmpty()) {
                LCH = ModelType::toLCH(originalChannels.data());
                QPointF cusp = KoGamutMappingUtils::cuspForHue(m_cuspDescription, LCH[2]);

                       /// We are treating the cusp triangle as linear inside the SDR standard range.
                       /// Within the standard range, cube root works better than hdr's PQ.
                       /// This is good enough for RGB profiles, CMYK profiles might get better
                       /// results with the MINDE method. Overall though, we just want to move
                       /// the colors into gamut enough so that we don't get hue shifts when converting,
                       /// and this method delivers on that nicely.
                const float L = ModelType::cuberootLightness(LCH[0]);
                const float cuspL = ModelType::cuberootLightness(cusp.y());
                if (LCH[0] > cusp.y()) {
                    const float t = (L - cuspL) / (1.0 - cuspL);
                    const float c = qMax(0.0, cusp.x()* (1.0-t));

                    LCH[1] = qMin(LCH[1], qreal(c));

                } else {
                    const float t = L / (cuspL);
                    const float c = qMax(0.0, cusp.x()*t);
                    LCH[1] = qMin(LCH[1], qreal(c));
                }
                ModelType::fromLCH(LCH, originalChannels.data());
                m_sourceSpace->fromNormalisedChannelsValue(tmpOriginal.get(), originalChannels);
            } else {
                m_sourceSpace->convertPixelsTo(tmpOriginal.get(), tmpTarget.get(), targetSpace, 1, KoColorConversionTransformation::IntentRelativeColorimetric, KoColorConversionTransformation::adjustmentConversionFlags());
                targetSpace->convertPixelsTo(tmpTarget.get(), tmpClipped.get(), m_sourceSpace, 1, KoColorConversionTransformation::IntentRelativeColorimetric, KoColorConversionTransformation::adjustmentConversionFlags());

                m_sourceSpace->normalisedChannelsValue(tmpClipped.get(), clippedChannels);

                if (DifferenceType::difference(originalChannels, clippedChannels) > 0.02) {
                    LCH = ModelType::toLCH(originalChannels.data());

                    float min = 0;
                    float max = LCH[1];

                    while (max-min > 0.0001) {

                        LCH[1] = (max + min) * 0.5;
                        ModelType::fromLCH(LCH, originalChannels.data());


                        m_sourceSpace->fromNormalisedChannelsValue(tmpOriginal.get(), originalChannels);
                        m_sourceSpace->convertPixelsTo(tmpOriginal.get(), tmpTarget.get(), targetSpace, 1, KoColorConversionTransformation::IntentRelativeColorimetric, KoColorConversionTransformation::adjustmentConversionFlags());
                        targetSpace->convertPixelsTo(tmpTarget.get(), tmpClipped.get(), m_sourceSpace, 1, KoColorConversionTransformation::IntentRelativeColorimetric, KoColorConversionTransformation::adjustmentConversionFlags());
                        m_sourceSpace->normalisedChannelsValue(tmpClipped.get(), clippedChannels);

                        if (DifferenceType::difference(originalChannels, clippedChannels) < 0.02) {
                            min = LCH[1];
                        } else {
                            max = LCH[1];
                        }
                    }
                }
            }

            memcpy(dstU8, tmpOriginal.get(), m_sourceSpace->pixelSize());

            --nPixels;
            srcU8 += m_sourceSpace->pixelSize();
            dstU8 += m_sourceSpace->pixelSize();
        }
    }

    QList<QString> parameters() const override
    {
        QList<QString> list;
        list << "targetColorProfileName";
        list << "cuspDescription";
        return list;
    }

    int parameterId(const QString& name) const override
    {
        if (name == "targetColorProfileName") {
            return 0;
        } else if (name == "cuspDescription") {
            return 1;
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
        case 1:

            m_cuspDescription = QMap<float, QPointF>();
            {
                const QVariantMap map = parameter.toMap();
                Q_FOREACH (QString key, map.keys()){
                    m_cuspDescription.insert(key.toFloat(), map.value(key).toPointF());
                }
            }
            break;
        default:
            ;
        }
    }

private:

    QString m_targetColorProfileName;
    QMap<float, QPointF> m_cuspDescription;
    const KoColorSpace *m_sourceSpace;
};

KisGamutMapClipAdjustmentFactory::KisGamutMapClipAdjustmentFactory()
    : KoColorTransformationFactory("gamutmap_clip_adjustment")
{

}

QList<QPair<KoID, KoID> > KisGamutMapClipAdjustmentFactory::supportedModels() const
{
    QList< QPair< KoID, KoID > > l;
    l.append(QPair< KoID, KoID >(XYZAColorModelID , Float32BitsColorDepthID));
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
        adj = new KisGamutMapClipAdjustment< labModel , dE76>(colorSpace);
    } else if (colorSpace->colorModelId() == YCbCrAColorModelID) {
        adj = new KisGamutMapClipAdjustment< labModel , dE76>(colorSpace);
    } else if (colorSpace->colorModelId() == XYZAColorModelID) {
        if (parameters["mappingModel"].toInt() == KoGamutMappingUtils::OkLAB) {
            adj = new KisGamutMapClipAdjustment< xyzToOkLabModel , dE76>(colorSpace);
        } else {
            adj = new KisGamutMapClipAdjustment< xyzToICtCpModel , dE76>(colorSpace);
        }
    } else {
        dbgKrita << "Unsupported color space " << colorSpace->id() << " in KisGamutMapClipAdjustmentFactory::createTransformation";
        return 0;
    }

    adj->setParameters(parameters);
    return adj;
}
