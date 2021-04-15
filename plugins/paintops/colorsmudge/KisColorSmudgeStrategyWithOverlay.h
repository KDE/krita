/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KRITA_KISCOLORSMUDGESTRATEGYWITHOVERLAY_H
#define KRITA_KISCOLORSMUDGESTRATEGYWITHOVERLAY_H

#include "KisColorSmudgeStrategyBase.h"
#include "kis_painter.h"


class KisColorSmudgeStrategyWithOverlay : public KisColorSmudgeStrategyBase
{
public:
    KisColorSmudgeStrategyWithOverlay(KisPainter *painter,
                                      KisImageSP image,
                                      bool smearAlpha,
                                      bool useDullingMode,
                                      bool useOverlayMode);

    virtual ~KisColorSmudgeStrategyWithOverlay();

    void initializePainting() override;

    QVector<KisPainter*> finalPainters();

    QVector<QRect> paintDab(const QRect &srcRect, const QRect &dstRect, const KoColor &currentPaintColor, qreal opacity,
                            qreal colorRateValue, qreal smudgeRateValue, qreal maxPossibleSmudgeRateValue,
                            qreal lightnessStrengthValue, qreal smudgeRadiusValue) override;

protected:
    KisFixedPaintDeviceSP m_maskDab;
    bool m_shouldPreserveMaskDab = true;
    QScopedPointer<KisOverlayPaintDeviceWrapper> m_layerOverlayDevice;

private:
    QScopedPointer<KisOverlayPaintDeviceWrapper> m_imageOverlayDevice;
    KisColorSmudgeSourceSP m_sourceWrapperDevice;
    KisPainter m_finalPainter;
    QScopedPointer<KisPainter> m_overlayPainter;
    bool m_smearAlpha = true;
    KisPainter *m_initializationPainter = 0;
};


#endif //KRITA_KISCOLORSMUDGESTRATEGYWITHOVERLAY_H
