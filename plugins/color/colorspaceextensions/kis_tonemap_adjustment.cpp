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

#include <kis_debug.h>

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

            float sum = originalChannels[0]+originalChannels[1]+originalChannels[2];
            float x = originalChannels[0]/sum;
            float y = originalChannels[1]/sum;
            float Y = originalChannels[1];
            Y = Y/(Y+1);
            originalChannels[0] = (x*Y)/y;
            originalChannels[1] = Y;
            originalChannels[2] = ((1.0-x-y)*Y)/y;

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
        return list;
    }

    int parameterId(const QString& name) const override
    {
        Q_UNUSED(name)
        return -1;
    }

    void setParameter(int id, const QVariant& parameter) override
    {
        Q_UNUSED(id)
        Q_UNUSED(parameter)
    }

private:
    const KoColorSpace *m_sourceSpace;

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
    return l;
}

KoColorTransformation *KisToneMapAdjustmentFactory::createTransformation(const KoColorSpace *colorSpace, QHash<QString, QVariant> parameters) const
{
    KoColorTransformation * adj;

    if (colorSpace->colorModelId() == XYZAColorModelID) {
        adj = new KisReinhardToneMapAdjustment(colorSpace);
    } else {
        dbgKrita << "Unsupported color space " << colorSpace->id() << " in KisGamutMapClipAdjustmentFactory::createTransformation";
        return nullptr;
    }

    adj->setParameters(parameters);
    return adj;
}
