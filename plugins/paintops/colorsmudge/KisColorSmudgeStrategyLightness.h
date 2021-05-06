/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KRITA_KISCOLORSMUDGESTRATEGYLIGHTNESS_H
#define KRITA_KISCOLORSMUDGESTRATEGYLIGHTNESS_H

#include "KisColorSmudgeStrategyBase.h"
#include "kis_painter.h"

class KisColorSmudgeStrategyLightness : public KisColorSmudgeStrategyBase
{
public:

    KisColorSmudgeStrategyLightness(KisPainter *painter,
                                    bool smearAlpha,
                                    bool useDullingMode);

    void initializePainting() override;

    DabColoringStrategy &coloringStrategy() override;

    void updateMask(KisDabCache *dabCache,
                    const KisPaintInformation& info,
                    const KisDabShape &shape,
                    const QPointF &cursorPoint,
                    QRect *dstDabRect, qreal lightnessStrength) override;

    QVector<QRect> paintDab(const QRect &srcRect, const QRect &dstRect, const KoColor &currentPaintColor, qreal opacity,
                            qreal colorRateValue, qreal smudgeRateValue, qreal maxPossibleSmudgeRateValue,
                            qreal lightnessStrengthValue, qreal smudgeRadiusValue) override;
private:
    KisFixedPaintDeviceSP m_maskDab;
    KisFixedPaintDeviceSP m_origDab;
    KisPaintDeviceSP m_heightmapDevice;
    KisPaintDeviceSP m_colorOnlyDevice;
    KisPaintDeviceSP m_projectionDevice;
    KisOverlayPaintDeviceWrapper *m_layerOverlayDevice;
    KisColorSmudgeSourceSP m_sourceWrapperDevice;
    KisPainter m_finalPainter;
    KisPainter m_heightmapPainter;
    bool m_shouldPreserveOriginalDab = true;
    DabColoringStrategyMask m_coloringStrategy;
    bool m_smearAlpha = true;
    KisPainter *m_initializationPainter = 0;
};


#endif //KRITA_KISCOLORSMUDGESTRATEGYLIGHTNESS_H
