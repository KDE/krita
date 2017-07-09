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

#include "kis_watercolor_base_items.h"

#include "kis_splat_generator_strategy.h"

#include <KoCompositeOps.h>

void KisWatercolorBaseItems::paint(QPointF pos, qreal radius, int brushType)
{
    KisSplatGeneratorStrategy *strategy;
    switch (brushType) {
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
        break;
    }

    strategy->generate(&m_flowing, m_wetMap, pos, radius);
}

void KisWatercolorBaseItems::repaint(KisPainter *painter)
{

    foreach (KisSplat *splat, m_flowing.values()) {
        splat->doPaint(painter);
    }

    foreach (KisSplat *splat, m_fixed.values()) {
        splat->doPaint(painter);
    }

    foreach (KisSplat *splat, m_dried.values()) {
        splat->doPaint(painter);
    }
}

void KisWatercolorBaseItems::update()
{
    QList<KisSplat *> list = m_flowing.values();

    foreach (KisSplat *splat, list) {
        if (splat->update(m_wetMap) == KisSplat::Fixed) {
            m_fixed.insert(splat->boundingRect(), splat);
            m_flowing.remove(splat);
        }
    }
    list = m_fixed.values();
    foreach (KisSplat *splat, list) {
        if (splat->update(m_wetMap) == KisSplat::Dried) {
            m_dried.insert(splat->boundingRect(), splat);
            m_fixed.remove(splat);
        }
    }

    m_wetMap->update();
}

KisWatercolorBaseItems::KisWatercolorBaseItems() : m_flowing(4, 2), m_fixed(4, 2), m_dried(4, 2)
{
    m_wetMap = new KisWetMap();
}
