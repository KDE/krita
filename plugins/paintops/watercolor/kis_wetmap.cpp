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
#include <KoColorSpaceMaths.h>

using namespace Arithmetic;

KisWetMap::KisWetMap()
{
    m_wetMap = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb16());
}

KisWetMap::~KisWetMap()
{
    delete m_wetMap;
}

// Adding water in circle with the given center and radius
void KisWetMap::addWater(QPoint pos, qreal radius)
{
    QRect rect(pos - QPointF(radius, radius).toPoint(),
               pos + QPointF(radius, radius).toPoint());
    KisSequentialIterator it(m_wetMap, rect);

    do {
        //qDebug() << it.x() << it.y() << it.x() * it.x() + it.y() * it.y() << radius * radius;

        QPoint place(it.x(), it.y());
        QVector2D vec(place - pos);
        if ((vec.x() * vec.x() + vec.y() * vec.y()) <= (radius * radius)) {
            qint16 *mydata = reinterpret_cast<qint16*>(it.rawData());
            vec.normalize();
            mydata[0] = unitValue<qint16>();
            mydata[3] = unitValue<qint16>();
            mydata[1] = unitValue<qint16>() * vec.x();
            mydata[2] = unitValue<qint16>() * vec.y();
        }
    } while (it.nextPixel());
}

// Updating wetmap for simulating drying process
void KisWetMap::update()
{
    if (m_wetMap->exactBounds().size() != QSize(0, 0)) {
        KisSequentialIterator it(m_wetMap, m_wetMap->exactBounds());

        do {
            qint16 *mydata = reinterpret_cast<qint16*>(it.rawData());
            if (mydata[0] > 127) {            // If there some water
                mydata[0] -= 128;           // The "evaporated" part of the water
            } else {                           // If there is no water
                mydata[1] = zeroValue<qint16>();             // Remove speed vector
                mydata[2] = zeroValue<qint16>();
                mydata[3] = zeroValue<qint16>();
                mydata[0] = zeroValue<qint16>();
            }
            if (m_wetMap->exactBounds().size() == QSize(0, 0))
                break;
        } while (it.nextPixel());
    }
}

// Returns water count in point (x, y)
int KisWetMap::getWater(int x, int y)
{
    KisSequentialIterator it(m_wetMap, m_wetMap->exactBounds());

    do {
        if (it.x() == x && it.y() == y)
            break;
    } while (it.nextPixel());

    qint16 *mydata = reinterpret_cast<qint16*>(it.rawData());
    return mydata[0];
}

QPoint KisWetMap::getSpeed(int x, int y)
{
    KisSequentialIterator it(m_wetMap, m_wetMap->exactBounds());

    do {
        if (it.x() == x && it.y() == y)
            break;
    } while (it.nextPixel());

    qint16 *mydata = reinterpret_cast<qint16*>(it.rawData());
    return QPoint(mydata[1], mydata[2]);
}

// Returns paint device for visualizing wet map
KisPaintDeviceSP KisWetMap::getPaintDevice()
{
    return m_wetMap;
}
