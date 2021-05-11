/*
 *  SPDX-FileCopyrightText: 2021 Peter Schatz <voronwe13@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KRITA_KISCOLORSMUDGESTRATEGYSMEARLIGHTNESS_H
#define KRITA_KISCOLORSMUDGESTRATEGYSMEARLIGHTNESS_H

#include "KisColorSmudgeStrategyBase.h"
#include "kis_painter.h"
#include "kis_pressure_paint_thickness_option.h"

class KisColorSmudgeStrategySmearLightness : public KisColorSmudgeStrategyBase
{
public:

    KisColorSmudgeStrategySmearLightness(KisPainter* painter,
        bool smearAlpha,
        bool useDullingMode);

    void initializePainting() override;

    DabColoringStrategy& coloringStrategy() override;

    void updateMask(KisDabCache* dabCache,
        const KisPaintInformation& info,
        const KisDabShape& shape,
        const QPointF& cursorPoint,
        QRect* dstDabRect, qreal lightnessStrength) override;

    QVector<QRect> paintDab(const QRect& srcRect, const QRect& dstRect, const KoColor& currentPaintColor, qreal opacity,
        qreal colorRateValue, qreal smudgeRateValue, qreal maxPossibleSmudgeRateValue,
        qreal lightnessStrengthValue, qreal smudgeRadiusValue) override;
private:
    KisFixedPaintDeviceSP m_maskDab;
    KisFixedPaintDeviceSP m_origDab;
    KisPaintDeviceSP m_projectionDevice;
    QScopedPointer<KisOverlayPaintDeviceWrapper> m_layerOverlayDevice;
    KisColorSmudgeSourceSP m_sourceWrapperDevice;
    KisPainter m_finalPainter;
    bool m_shouldPreserveOriginalDab = true;
    DabColoringStrategyMask m_coloringStrategy;
    bool m_smearAlpha = true;
    KisPainter* m_initializationPainter = 0;
    KisPressurePaintThicknessOption::ThicknessMode m_thicknessMode;
};

#endif //KRITA_KISCOLORSMUDGESTRATEGYSMEARLIGHTNESS_H