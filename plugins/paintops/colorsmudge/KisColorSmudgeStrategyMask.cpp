/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisColorSmudgeStrategyMask.h"

#include "kis_image.h"
#include "kis_fixed_paint_device.h"

KisColorSmudgeStrategyMask::KisColorSmudgeStrategyMask(KisPainter *painter, KisImageSP image, bool smearAlpha,
                                                       bool useDullingMode, bool useOverlayMode)
        : KisColorSmudgeStrategyWithOverlay(painter, image, smearAlpha, useDullingMode, useOverlayMode)
{
}

KisColorSmudgeStrategyBase::DabColoringStrategy &KisColorSmudgeStrategyMask::coloringStrategy()
{
    return m_coloringStrategy;
}

void KisColorSmudgeStrategyMask::updateMask(KisDabCache *dabCache, const KisPaintInformation &info, const KisDabShape &shape,
                                       const QPointF &cursorPoint, QRect *dstDabRect)
{
    static const KoColorSpace* cs = KoColorSpaceRegistry::instance()->alpha8();
    static KoColor color(Qt::black, cs);

    m_maskDab = dabCache->fetchDab(cs,
                                   color,
                                   cursorPoint,
                                   shape,
                                   info,
                                   1.0,
                                   dstDabRect);

    m_shouldPreserveMaskDab = !dabCache->needSeparateOriginal();
}
