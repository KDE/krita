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

#include "kis_splat_generator.h"
#include "kis_painter.h"

SplatGenerator::SplatGenerator(int width, KoColor &clr,
                               KisPaintDeviceSP dev) : m_width(width),
                                                       m_color(clr),
                                                       m_device(dev)
{
    m_wetMap = new KisWetMap();

    m_flowing = new QVector<KisSplat*>();
    m_fixed = new QVector<KisSplat*>();
    m_dried = new QVector<KisSplat*>();
}

void SplatGenerator::generateFromPoints(QVector<QPointF> &points, int msec)
{
    foreach (QPointF pnt, points) {
        m_flowing->push_back(new KisSplat(pnt, m_width, m_color));
        m_wetMap->addWater(pnt.toPoint(), m_width);
    }

    KisPainter painter(m_device);
    painter.setPaintColor(m_color);

    for (int i = 0; i <= msec; i +=33) {
        foreach (KisSplat *splat, *m_flowing) {
            painter.fillPainterPath(splat->shape());

            if (splat->update(m_wetMap) == KisSplat::Fixed) {
                m_fixed->push_back(splat);
                m_flowing->removeOne(splat);
            }
        }

        foreach (KisSplat *splat, *m_fixed) {
            if (splat->update(m_wetMap) == KisSplat::Dried) {
                m_dried->push_back(splat);
                m_fixed->removeOne(splat);
            }
        }

        m_wetMap->update();
    }
}
