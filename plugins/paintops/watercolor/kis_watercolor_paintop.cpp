/* This file is part of the KDE project
 *
 * Copyright (C) 2017 Grigory Tantsevov <tantsevov@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_watercolor_paintop.h"
#include "kis_watercolor_base_items.h"

#include <krita_utils.h>
#include <kis_paintop_settings.h>

#include "KoCompositeOps.h"

#include "kis_splat_generator_strategy.h"

KisWatercolorPaintOp::KisWatercolorPaintOp(const KisPaintOpSettingsSP settings, KisPainter *painter, KisNodeSP node, KisImageSP image)
    : KisPaintOp(painter), m_fixed(4, 2)
{
    Q_UNUSED(image);
    Q_UNUSED(node);

    m_wetMap = new KisWetMap();
    m_watercolorOption.readOptionSetting(settings);
    m_lastTime = 0;
    m_timer.start();
}

KisWatercolorPaintOp::~KisWatercolorPaintOp()
{
//    KisWatercolorBaseItems::instance()->drySystem();
}

KisSpacingInformation KisWatercolorPaintOp::paintAt(const KisPaintInformation &info)
{
    // Painting new stroke
    qint16 time = m_timer.elapsed();
    qint16 timeGone = time - m_lastTime;
//    KisWatercolorBaseItems::instance()->paint(info.pos(), m_watercolorOption.radius,
//                                              m_watercolorOption.type, painter()->paintColor());
    KisSplatGeneratorStrategy *strategy;
    switch (m_watercolorOption.type) {
    case 0:
        strategy = new KisSimpleBrushGenerator();
        break;
    case 1:
        strategy = new KisWetOnDryGenerator();
        break;
    case 2:
        strategy = new KisWetOnWetGenerator();
        break;
    case 3:
        strategy = new KisBlobbyGenerator();
        break;
    case 4:
        strategy = new KisCrunchyGenerator();
        break;
    default:
        strategy = new KisSimpleBrushGenerator();
        break;
    }

    strategy->generate(&m_flowing,
                       m_wetMap,
                       info.pos(),
                       m_watercolorOption.radius,
                       painter()->paintColor());

    KisPaintDeviceSP driedPD = new KisPaintDevice(source()->colorSpace());
    KisPainter *driedPainter = new KisPainter(driedPD);
    // Updating system
    for (int i = 0; i < timeGone / 33; i++) {
//        KisWatercolorBaseItems::instance()->update();
        foreach (KisSplat *splat, m_flowing) {
            if (splat->update(m_wetMap) == KisSplat::Fixed) {
                m_fixed.insert(splat->boundingRect(), splat);
                m_flowing.removeOne(splat);
            }
        }
        foreach (KisSplat *splat, m_fixed.values()) {
            if (splat->update(m_wetMap) == KisSplat::Dried) {
                m_dried.push_back(splat);
                m_fixed.remove(splat);
            }
        }

        m_wetMap->update();
    }

    m_lastTime = time - time % 33;
    source()->clear();
    KisWatercolorBaseItems::instance()->repaint(painter());
    foreach (KisSplat *splat, m_dried) {
        splat->doPaint(driedPainter);
    }

    foreach (KisSplat *splat, m_fixed.values()) {
        splat->doPaint(driedPainter);
    }

    QRect rect = driedPD->extent();
    painter()->bitBlt(rect.topLeft(), driedPD, rect);

    foreach (KisSplat *splat, m_flowing) {
        splat->doPaint(painter());
    }

    return updateSpacingImpl(info);
}

KisSpacingInformation KisWatercolorPaintOp::updateSpacingImpl(const KisPaintInformation &info) const
{
    Q_UNUSED(info);
    return KisSpacingInformation(m_watercolorOption.radius / 5);
}
