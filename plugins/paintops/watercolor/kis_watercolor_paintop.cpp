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
    : KisPaintOp(painter),
      m_driedPlane(true),
      m_fixedPlane(&m_driedPlane),
      m_flowingPlane(false, &m_fixedPlane)
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
    QRect dirtyRect;

    // Painting new stroke
    qint16 time = m_timer.elapsed();
    qint16 timeGone = time - m_lastTime;

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

    QList<KisSplat *> newSplats =
            strategy->generate(m_wetMap,
                       info.pos(),
                       m_watercolorOption.radius,
                       painter()->paintColor());

    foreach (KisSplat *splat, newSplats) {
        m_flowingPlane.add(splat);
        dirtyRect |= splat->boundingRect().toAlignedRect();
    }

    // Updating system
    for (int i = 0; i < timeGone / 33; i++) {
//        foreach (KisSplat *splat, m_flowing) {
//            // todo: check if tree should be updated when splat is changed

//            if (splat->update(m_wetMap) == KisSplat::Fixed) {
//                m_fixed.push_back(splat);
//                m_fixedTree.insert(splat->boundingRect(), splat);
//                m_fixedPlane.add(splat);
//                fixedRect |= splat->boundingRect().toRect();

//                m_flowing.removeOne(splat);
//                m_flowingPlane.remove(splat);
//                flowingRect |= splat->boundingRect().toRect();
//            }
//        }

//        fixedRect |= m_fixedPlane.update(wetMap);

//        /*foreach (KisSplat *splat, m_fixed) {
//            if (splat->update(m_wetMap) == KisSplat::Dried) {
//                m_dried.push_back(splat);
//                m_driedPlane.add(splat);
//                driedRect |= splat->boundingRect().toRect();

//                m_fixed.removeOne(splat);
//                m_fixedTree.remove(splat);
//                m_fixedPlane.remove(splat);
//                fixedRect |= splat->boundingRect().toRect();
//            }
//        }*/
        dirtyRect |= m_fixedPlane.update(m_wetMap) |
        m_flowingPlane.update(m_wetMap);

        m_wetMap->update();
    }

    m_lastTime = time - time % 33;
    source()->clear();
    m_driedPlane.paint(painter(), dirtyRect);
    m_fixedPlane.paint(painter(), dirtyRect);
    m_flowingPlane.paint(painter(), dirtyRect);
    return updateSpacingImpl(info);
}

KisSpacingInformation KisWatercolorPaintOp::updateSpacingImpl(const KisPaintInformation &info) const
{
    Q_UNUSED(info);
    return KisSpacingInformation(m_watercolorOption.radius / 5);
}
