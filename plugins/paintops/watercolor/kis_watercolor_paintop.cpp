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
      m_driedPlane(true, 0, painter->device()->colorSpace()),
      m_fixedPlane(&m_driedPlane, painter->device()->colorSpace()),
      m_flowingPlane(false, &m_fixedPlane)
{
    Q_UNUSED(image);
    Q_UNUSED(node);

    m_wetMap = new KisWetMap();
    m_watercolorOption.readOptionSetting(settings);
    m_oldPD = new KisPaintDevice(*painter->device().constData());
}

KisWatercolorPaintOp::~KisWatercolorPaintOp()
{
}

KisSpacingInformation KisWatercolorPaintOp::paintAt(const KisPaintInformation &info)
{
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
        splat->doPaint(painter());
    }

    m_fixedPlane.rewet(m_wetMap, info.pos(), m_watercolorOption.radius, &m_flowingPlane);

    return updateSpacingImpl(info);
}

void KisWatercolorPaintOp::updateSystem()
{
    QRect dirtyRect;

    QElapsedTimer timer;
    timer.start();

    dirtyRect = m_fixedPlane.update(m_wetMap);
    dirtyRect |= m_flowingPlane.update(m_wetMap);

    m_wetMap->update();

    qDebug() << "\n" << ppVar(timer.elapsed());
    qDebug() << ppVar(dirtyRect);

    source()->clear(dirtyRect);
    painter()->bitBlt(dirtyRect.topLeft(),
                      m_oldPD, dirtyRect);

    qDebug() << ppVar(timer.elapsed());
    m_driedPlane.paint(painter(), dirtyRect);
    qDebug() << ppVar(timer.elapsed());
    m_fixedPlane.paint(painter(), dirtyRect);
    qDebug() << ppVar(timer.elapsed());
    m_flowingPlane.paint(painter(), dirtyRect);

    qDebug() << ppVar(timer.elapsed()) << "\n";

}

KisSpacingInformation KisWatercolorPaintOp::updateSpacingImpl(const KisPaintInformation &info) const
{
    Q_UNUSED(info);
    return KisSpacingInformation(m_watercolorOption.radius / 5);
}
