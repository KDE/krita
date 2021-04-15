/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KRITA_KISCOLORSMUDGESTRATEGYBASE_H
#define KRITA_KISCOLORSMUDGESTRATEGYBASE_H

#include <kis_types.h>

#include "KisColorSmudgeStrategy.h"
#include "KisColorSmudgeSource.h"

class KisPainter;


class KisColorSmudgeStrategyBase : public KisColorSmudgeStrategy
{
public:
    struct DabColoringStrategy
    {
        virtual ~DabColoringStrategy() = default;
        virtual bool supportsFusedDullingBlending() const = 0;
        virtual void blendInColorRate(const KoColor &paintColor, const KoCompositeOp *colorRateOp, quint8 colorRateOpacity,
                                      KisFixedPaintDeviceSP dstDevice, const QRect &dstRect) const = 0;
        virtual void blendInFusedBackgroundAndColorRateWithDulling(KisFixedPaintDeviceSP dst, KisColorSmudgeSourceSP src,
                                                                   const QRect &dstRect,
                                                                   const KoColor &preparedDullingColor,
                                                                   const KoCompositeOp *smearOp,
                                                                   const quint8 smudgeRateOpacity,
                                                                   const KoColor &paintColor,
                                                                   const KoCompositeOp *colorRateOp,
                                                                   const quint8 colorRateOpacity) const = 0;
    };

    struct DabColoringStrategyMask : public DabColoringStrategy
    {
        bool supportsFusedDullingBlending() const override;

        void blendInColorRate(const KoColor &paintColor, const KoCompositeOp *colorRateOp, quint8 colorRateOpacity,
                              KisFixedPaintDeviceSP dstDevice, const QRect &dstRect) const override;

        void blendInFusedBackgroundAndColorRateWithDulling(KisFixedPaintDeviceSP dst,
                                                           KisColorSmudgeSourceSP src,
                                                           const QRect &dstRect,
                                                           const KoColor &preparedDullingColor,
                                                           const KoCompositeOp *smearOp,
                                                           const quint8 smudgeRateOpacity,
                                                           const KoColor &paintColor,
                                                           const KoCompositeOp *colorRateOp,
                                                           const quint8 colorRateOpacity) const override;
    };

    struct DabColoringStrategyStamp : public DabColoringStrategy
    {
        void setStampDab(KisFixedPaintDeviceSP device);

        void blendInColorRate(const KoColor &paintColor, const KoCompositeOp *colorRateOp, quint8 colorRateOpacity,
                              KisFixedPaintDeviceSP dstDevice, const QRect &dstRect) const override;

        bool supportsFusedDullingBlending() const override;

        void blendInFusedBackgroundAndColorRateWithDulling(KisFixedPaintDeviceSP dst,
                                                           KisColorSmudgeSourceSP src,
                                                           const QRect &dstRect,
                                                           const KoColor &preparedDullingColor,
                                                           const KoCompositeOp *smearOp,
                                                           const quint8 smudgeRateOpacity,
                                                           const KoColor &paintColor,
                                                           const KoCompositeOp *colorRateOp,
                                                           const quint8 colorRateOpacity) const override;

    private:
        KisFixedPaintDeviceSP m_origDab;
    };

public:

    KisColorSmudgeStrategyBase(bool useDullingMode);

    void initializePaintingImpl(const KoColorSpace *dstColorSpace,
                                bool smearAlpha,
                                const QString &colorRateCompositeOpId);

    virtual DabColoringStrategy& coloringStrategy() = 0;

    const KoColorSpace* preciseColorSpace() const override;

    virtual QString smearCompositeOp(bool smearAlpha) const;

    virtual QString finalCompositeOp(bool smearAlpha) const;

    virtual quint8 finalPainterOpacity(qreal opacity, qreal smudgeRateValue);

    virtual quint8 colorRateOpacity(qreal opacity, qreal smudgeRateValue, qreal colorRateValue, qreal maxPossibleSmudgeRateValue);

    virtual quint8 dullingRateOpacity(qreal opacity, qreal smudgeRateValue);

    virtual quint8 smearRateOpacity(qreal opacity, qreal smudgeRateValue);

    virtual void sampleDullingColor(const QRect &srcRect, qreal sampleRadiusValue, KisColorSmudgeSourceSP sourceDevice,
                                    KisFixedPaintDeviceSP tempFixedDevice, KisFixedPaintDeviceSP maskDab,
                                    KoColor *resultColor);

    void blendBrush(const QVector<KisPainter *> dstPainters, KisColorSmudgeSourceSP srcSampleDevice,
                    KisFixedPaintDeviceSP maskDab, bool preserveMaskDab, const QRect &srcRect, const QRect &dstRect,
                    const KoColor &currentPaintColor, qreal opacity, qreal smudgeRateValue,
                    qreal maxPossibleSmudgeRateValue, qreal colorRateValue, qreal smudgeRadiusValue);

    void blendInBackgroundWithSmearing(KisFixedPaintDeviceSP dst, KisColorSmudgeSourceSP src, const QRect &srcRect,
                                       const QRect &dstRect, const quint8 smudgeRateOpacity);

    void blendInBackgroundWithDulling(KisFixedPaintDeviceSP dst, KisColorSmudgeSourceSP src, const QRect &dstRect,
                                      const KoColor &preparedDullingColor, const quint8 smudgeRateOpacity);

protected:
    const KoCompositeOp * m_colorRateOp;
    KoColor m_preparedDullingColor;
    const KoCompositeOp * m_smearOp;
private:
    KisFixedPaintDeviceSP m_blendDevice;
    bool m_useDullingMode = true;
};


#endif //KRITA_KISCOLORSMUDGESTRATEGYBASE_H
