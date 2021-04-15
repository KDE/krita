/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef _KO_ICC_COLOR_SPACE_ENGINE_H_
#define _KO_ICC_COLOR_SPACE_ENGINE_H_

#include <KoColorSpaceEngine.h>

class IccColorSpaceEngine : public KoColorSpaceEngine
{
public:
    IccColorSpaceEngine();
    ~IccColorSpaceEngine() override;
    const KoColorProfile *addProfile(const QString &filename) override;
    const KoColorProfile *addProfile(const QByteArray &data) override;
    const KoColorProfile * getProfile(const QVector<double> &colorants, ColorPrimaries colorPrimaries, TransferCharacteristics transferFunction) override;
    void removeProfile(const QString &filename) override;
    KoColorConversionTransformation *createColorTransformation(const KoColorSpace *srcColorSpace,
            const KoColorSpace *dstColorSpace,
            KoColorConversionTransformation::Intent renderingIntent,
            KoColorConversionTransformation::ConversionFlags conversionFlags) const override;
    KoColorProofingConversionTransformation *createColorProofingTransformation(const KoColorSpace *srcColorSpace,
            const KoColorSpace *dstColorSpace,
            const KoColorSpace *proofingSpace,
            KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::Intent proofingIntent,
            KoColorConversionTransformation::ConversionFlags conversionFlags, quint8 *gamutWarning, double adaptationState) const override;
    quint32 computeColorSpaceType(const KoColorSpace *cs) const;

    bool supportsColorSpace(const QString& colorModelId, const QString& colorDepthId, const KoColorProfile *profile) const override;
private:
    struct Private;
    Private *const d;
};

#endif
