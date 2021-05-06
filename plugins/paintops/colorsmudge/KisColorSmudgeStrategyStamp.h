/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KRITA_KISCOLORSMUDGESTRATEGYSTAMP_H
#define KRITA_KISCOLORSMUDGESTRATEGYSTAMP_H

#include "KisColorSmudgeStrategyWithOverlay.h"

struct KisColorSmudgeStrategyStamp : public KisColorSmudgeStrategyWithOverlay
{
    KisColorSmudgeStrategyStamp(KisPainter *painter,
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
    KisFixedPaintDeviceSP m_origDab;
    DabColoringStrategyStamp m_coloringStrategy;
};


#endif //KRITA_KISCOLORSMUDGESTRATEGYSTAMP_H
