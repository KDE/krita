/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisColorSmudgeStrategyStamp.h"

#include "kis_fixed_paint_device.h"
#include "kis_image.h"
#include "KisOverlayPaintDeviceWrapper.h"


KisColorSmudgeStrategyStamp::KisColorSmudgeStrategyStamp(KisPainter *painter, KisImageSP image, bool smearAlpha,
                                                         bool useDullingMode, bool useOverlayMode)
        : KisColorSmudgeStrategyWithOverlay(painter, image, smearAlpha, useDullingMode, useOverlayMode)
        , m_origDab(new KisFixedPaintDevice(m_layerOverlayDevice->overlayColorSpace())) // TODO: check compositionSourceColorSpace!
{
}

KisColorSmudgeStrategyBase::DabColoringStrategy &KisColorSmudgeStrategyStamp::coloringStrategy()
{
    return m_coloringStrategy;
}

void KisColorSmudgeStrategyStamp::updateMask(KisDabCache *dabCache, const KisPaintInformation &info,
                                             const KisDabShape &shape, const QPointF &cursorPoint, QRect *dstDabRect, qreal lightnessStrength)
{

    static KoColor color(Qt::black, m_origDab->colorSpace());

    m_origDab = dabCache->fetchDab(m_origDab->colorSpace(),
                                   color,
                                   cursorPoint,
                                   shape,
                                   info,
                                   1.0,
                                   dstDabRect,
                                   lightnessStrength);

    m_coloringStrategy.setStampDab(m_origDab);

    const int numPixels = m_origDab->bounds().width() * m_origDab->bounds().height();

    m_maskDab->setRect(m_origDab->bounds());
    m_maskDab->lazyGrowBufferWithoutInitialization();
    m_origDab->colorSpace()->copyOpacityU8(m_origDab->data(), m_maskDab->data(), numPixels);

    m_shouldPreserveMaskDab = false;
}
