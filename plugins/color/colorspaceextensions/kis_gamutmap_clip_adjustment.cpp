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

const QMatrix4x4 xyz50To65 {QMatrix4x4(
    0.955473421488075,    -0.02309845494876471,   0.06325924320057072, 0.0,
    -0.0283697093338637,    1.0099953980813041,    0.021041441191917323, 0.0,
    0.012314014864481998, -0.020507649298898964,  1.330365926242124, 0.0,
    0.0, 0.0, 0.0, 1.0
    )};

const QMatrix4x4 xyz50To65_inv = xyz50To65.inverted();

const QMatrix4x4 XYZtoOkLABLMS {QMatrix4x4(
    0.8190224379967030, 0.3619062600528904, -0.1288737815209879, 0.0,
    0.0329836539323885, 0.9292868615863434,  0.0361446663506424, 0.0,
    0.0481771893596242, 0.2642395317527308,  0.6335478284694309, 0.0,
    0.0, 0.0, 0.0, 1.0
    )};

const QMatrix4x4 XYZtoOkLABLMS_inv = XYZtoOkLABLMS.inverted();

const QMatrix4x4 lmsToOkLab {QMatrix4x4(
    0.2104542683093140,  0.7936177747023054, -0.0040720430116193, 0.0,
    1.9779985324311684, -2.4285922420485799,  0.4505937096174110, 0.0,
    0.0259040424655478,  0.7827717124575296, -0.8086757549230774, 0.0,
    0.0, 0.0, 0.0, 1.0
    )};

const QMatrix4x4 lmsToOkLab_inv = lmsToOkLab.inverted();

struct xyzToOkLabModel {
    static ALWAYS_INLINE QVector<qreal> toLCH (const QVector<float> &channels) {
        QVector4D color (channels[0], channels[1], channels[2], 0.0);

        const QVector4D lms = XYZtoOkLABLMS * (xyz50To65 * color);

        const QVector4D lmsPrime = QVector4D(cbrtf(lms.x()),
                                             cbrtf(lms.y()),
                                             cbrtf(lms.z()), 0.0);

        const QVector4D OkLab = lmsToOkLab * lmsPrime;
        QVector<qreal> lch(3);
        LabToLCH(OkLab.x(), OkLab.y()+0.5, OkLab.z()+0.5, &lch[0], &lch[1], &lch[2]);
        return lch;
    }

    static ALWAYS_INLINE QVector<float> fromLCH (const QVector<qreal> &lch, QVector<float> channels) {
        QVector<qreal> output(3);
        LCHToLab(lch[0], lch[1], lch[2], &output[0], &output[1], &output[2]);


        QVector4D color (output[0], output[1]-0.5, output[2]-0.5, 0.0);

        const QVector4D lmsPrime = lmsToOkLab_inv * color;
        const QVector4D lms = QVector4D(powf(lmsPrime.x(), 3.0),
                                        powf(lmsPrime.y(), 3.0),
                                        powf(lmsPrime.z(), 3.0), 0.0);
        const QVector4D xyz = xyz50To65_inv * (XYZtoOkLABLMS_inv * lms);
        channels[0] = xyz.x();
        channels[1] = xyz.y();
        channels[2] = xyz.z();

        //qDebug() << "lch" << lch << "col" << color << "lmsprime" << lmsPrime << "lms" << lms << "xyz" << channels;
        return channels;
    }
};

const QMatrix4x4 XYZd65toDolbyLMS {QMatrix4x4(
    0.3592832590121217,  0.6976051147779502, -0.0358915932320290, 0.0,
    -0.1920808463704993,  1.1004767970374321,  0.0753748658519118, 0.0,
    0.0070797844607479,  0.0748396662186362,  0.8433265453898765, 0.0,
    0.0, 0.0, 0.0, 1.0
    )};

const QMatrix4x4 XYZd65toDolbyLMS_inv = XYZd65toDolbyLMS.inverted();

const QMatrix4x4 lmsToICtCp {QMatrix4x4(
    2048.0 / 4096.0,   2048.0 / 4096.0,       0.0 , 0.0,
    6610.0 / 4096.0, -13613.0 / 4096.0,  7003.0 / 4096.0 , 0.0,
    17933.0 / 4096.0, -17390.0 / 4096.0,  -543.0 / 4096.0, 0.0,
    0.0, 0.0, 0.0, 1.0
    )};

const QMatrix4x4 lmsToICtCp_inv = lmsToICtCp.inverted();

struct xyzToICtCpModel {
    static ALWAYS_INLINE QVector<qreal> toLCH (const QVector<float> &channels) {
        QVector4D color (channels[0], channels[1], channels[2], 0.0);

        const QVector4D lms =  XYZd65toDolbyLMS * (xyz50To65 * color);

        const QVector4D lmsPrime = QVector4D(applySmpte2048Curve(lms.x()),
                                             applySmpte2048Curve(lms.y()),
                                             applySmpte2048Curve(lms.z()), 0.0);

        const QVector4D ICtCp = lmsToICtCp * lmsPrime;
        QVector<qreal> lch(3);
        LabToLCH(ICtCp.x(), (ICtCp.y()+0.5), (ICtCp.z()+0.5), &lch[0], &lch[1], &lch[2]);

        return lch;
    }

    static ALWAYS_INLINE QVector<float> fromLCH (const QVector<qreal> &lch, QVector<float> channels) {
        QVector<qreal> output(3);
        LCHToLab(lch[0], lch[1], lch[2], &output[0], &output[1], &output[2]);


        QVector4D color (output[0], output[1]-0.5, output[2]-0.5, 0.0);

        const QVector4D lmsPrime = lmsToICtCp_inv * color;
        const QVector4D lms = QVector4D(removeSmpte2048Curve(lmsPrime.x()),
                                        removeSmpte2048Curve(lmsPrime.y()),
                                        removeSmpte2048Curve(lmsPrime.z()), 0.0);
        const QVector4D xyz = xyz50To65_inv * (XYZd65toDolbyLMS_inv * lms);
        channels[0] = xyz.x();
        channels[1] = xyz.y();
        channels[2] = xyz.z();

        return channels;
    }
};

struct labModel {
    static ALWAYS_INLINE QVector<qreal> toLCH (const QVector<float> &channels) {
        QVector<qreal> lch(3);
        LabToLCH(channels[0], channels[1], channels[2], &lch[0], &lch[1], &lch[2]);
        return lch;
    }

    static ALWAYS_INLINE QVector<float> fromLCH (const QVector<qreal> &lch, QVector<float> channels) {
        QVector<qreal> output(3);
        LCHToLab(lch[0], lch[1], lch[2], &output[0], &output[1], &output[2]);
        channels[0] = output[0];
        channels[1] = output[1];
        channels[2] = output[2];
        return channels;
    }
};

struct dE76 {
    static ALWAYS_INLINE float difference(const QVector<float> &a, const QVector<float> &b) {
        return KoGamutMappingUtils::deltaE76(a, b);
    }
};

struct dE2000 {
    static ALWAYS_INLINE float difference(const QVector<float> &a, const QVector<float> &b) {
        return KoGamutMappingUtils::deltaE2000(a, b);
    }
};
}

template<class ModelType, class DifferenceType>
class KisGamutMapClipAdjustment : public KoColorTransformation
{
public:
    KisGamutMapClipAdjustment(const KoColorSpace *sourceSpace): m_sourceSpace(sourceSpace) {}
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

            QVector<qreal> LCH(3);
            if (DifferenceType::difference(originalChannels, clippedChannels) > 0.02) {
                LCH = ModelType::toLCH(originalChannels);
                float min = 0;
                float max = LCH[1];


                while (max-min > 0.0001) {

                    LCH[1] = (max + min) * 0.5;
                    originalChannels = ModelType::fromLCH(LCH, originalChannels);


                    m_sourceSpace->fromNormalisedChannelsValue(tmpOriginal, originalChannels);
                    m_sourceSpace->convertPixelsTo(tmpOriginal, tmpTarget, targetSpace, 1, KoColorConversionTransformation::IntentRelativeColorimetric, KoColorConversionTransformation::adjustmentConversionFlags());
                    targetSpace->convertPixelsTo(tmpTarget, tmpClipped, m_sourceSpace, 1, KoColorConversionTransformation::IntentRelativeColorimetric, KoColorConversionTransformation::adjustmentConversionFlags());
                    m_sourceSpace->normalisedChannelsValue(tmpClipped, clippedChannels);

                    if (DifferenceType::difference(originalChannels, clippedChannels) < 0.02) {
                        min = LCH[1];
                    } else {
                        max = LCH[1];
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
