/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KRITA_KISCOLORSMUDGESTRATEGYMASK_H
#define KRITA_KISCOLORSMUDGESTRATEGYMASK_H

#include "KisColorSmudgeStrategyWithOverlay.h"

class KisColorSmudgeStrategyMask : public KisColorSmudgeStrategyWithOverlay
{
public:
    KisColorSmudgeStrategyMask(KisPainter *painter,
                               KisImageSP image,
                               bool smearAlpha,
                               bool useDullingMode,
                               bool useOverlayMode);

    DabColoringStrategy &coloringStrategy() override;

    void updateMask(KisDabCache *dabCache,
                    const KisPaintInformation& info,
                    const KisDabShape &shape,
                    const QPointF &cursorPoint,
                    QRect *dstDabRect, 
                    qreal lightnessStrength) override;

private:
    DabColoringStrategyMask m_coloringStrategy;
};

#endif //KRITA_KISCOLORSMUDGESTRATEGYMASK_H
