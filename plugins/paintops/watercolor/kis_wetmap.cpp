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

#include "kis_wetmap.h"
#include "KoColorSpaceRegistry.h"
#include "kis_sequential_iterator.h"
#include <QVector2D>

KisWetMap::KisWetMap()
{
    m_wetMap = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb16());
}

// Adding water in circle with the given center and radius
void KisWetMap::addWater(QPoint pos, qreal radius)
{
    QRect rect(pos - QPointF(radius, radius).toPoint(),
               pos + QPointF(radius, radius).toPoint());
    KisSequentialIterator it(m_wetMap, rect);

    do {
        qint16 *mydata = reinterpret_cast<qint16*>(it.rawData());
        mydata[0] = 255;

        QPoint place(it.x(), it.y());
        QVector2D vec(place - pos);

        vec.normalize();

        mydata[1] = 255 * vec.length();
        mydata[2] = 255 * vec.length();
    } while (it.nextPixel());
}

// Updating wetmap for simulating drying process
void KisWetMap::update()
{
    KisSequentialIterator it(m_wetMap, m_wetMap->exactBounds());

    do {
        qint16 *mydata = reinterpret_cast<qint16*>(it.rawData());
        if (mydata[0] > 0) {            // If there some water
            mydata[0]--;                // The "evaporated" part of the water
        } else {                        // If there is no water
            mydata[1] = 0;              // Remove speed vector
            mydata[2] = 0;
        }
    } while (it.nextPixel());
}

// Returns water count in point (x, y)
int KisWetMap::getWater(int x, int y)
{
    KisSequentialIterator it(m_wetMap, m_wetMap->exactBounds());
    while (it.x() != x && it.y() != y)
        it.nextPixel();

    qint16 *mydata = reinterpret_cast<qint16*>(it.rawData());

    return mydata[0];
}

// Returns paint device for visualizing wet map
KisPaintDeviceSP KisWetMap::getPaintDevice()
{
    return m_wetMap;
}
