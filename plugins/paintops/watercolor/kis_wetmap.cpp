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
#include <kis_random_accessor_ng.h>

#include "kis_pixel_selection.h"

using namespace Arithmetic;

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
        QPoint place(it.x(), it.y());
        QVector2D vec(place - pos);
        if ((vec.x() * vec.x() + vec.y() * vec.y()) <= (radius * radius)) {
            qint16 *mydata = reinterpret_cast<qint16*>(it.rawData());
            vec.normalize();
            mydata[0] = unitValue<qint16>();                // Water value
            mydata[3] = unitValue<qint16>();                // Alpha ch
            mydata[1] = unitValue<qint16>() * vec.x();      // X of speed
            mydata[2] = unitValue<qint16>() * vec.y();      // Y of speed
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
            if (mydata[0] > 127) {                  // If there some water
                mydata[0] -= 128;                   // The "evaporated" part of the water
            } else {                                // If there is no water
                mydata[1] = zeroValue<qint16>();    // Remove speed vector
                mydata[2] = zeroValue<qint16>();
                mydata[3] = zeroValue<qint16>();
                mydata[0] = zeroValue<qint16>();
            }
            if (m_wetMap->exactBounds().size() == QSize(0, 0))
                break;
        } while (it.nextPixel());
    }
}

// Returns water count in points
QVector<int> KisWetMap::getWater(QVector<QPointF> points)
{
    QVector<int> ret;
    KisRandomConstAccessorSP accesser = m_wetMap->createRandomConstAccessorNG(points.first().toPoint().x(),
                                                                         points.first().toPoint().y());
    points.pop_front();
    const qint16 *data = reinterpret_cast<const qint16 *>(accesser->rawDataConst());
    ret.push_back(data[0]);

    foreach (QPointF point, points) {
        accesser->moveTo(point.toPoint().x(),
                         point.toPoint().y());
        data = reinterpret_cast<const qint16 *>(accesser->rawDataConst());
        ret.push_back(data[0]);
    }

    return ret;
}

// TODO: KisRandomAccessorSP
QVector<QPoint> KisWetMap::getSpeed(QVector<QPointF> points)
{
    QVector<QPoint> ret;
    KisRandomConstAccessorSP accesser = m_wetMap->createRandomConstAccessorNG(points.first().toPoint().x(),
                                                                         points.first().toPoint().y());
    points.pop_front();
    const qint16 *data = reinterpret_cast<const qint16 *>(accesser->rawDataConst());
    ret.push_back(QPoint(data[1], data[2]));

    foreach (QPointF point, points) {
        accesser->moveTo(point.toPoint().x(),
                         point.toPoint().y());
        data = reinterpret_cast<const qint16 *>(accesser->rawDataConst());
        ret.push_back(QPoint(data[1], data[2]));
    }

    return ret;
}

// Returns paint device for visualizing wet map
KisPaintDeviceSP KisWetMap::getPaintDevice()
{
    KisSequentialConstIterator it(m_wetMap, m_wetMap->exactBounds());
    KisPaintDeviceSP ret = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    KisSequentialIterator itRet(ret, m_wetMap->exactBounds());
    do {
        const qint16 *mydata = reinterpret_cast<const qint16*>(it.rawDataConst());
        quint8 *seldata = itRet.rawData();
        seldata[3] = scale<quint8>(mydata[0]);
        seldata[1] = zeroValue<quint8>();
        seldata[2] = zeroValue<quint8>();
        seldata[0] = unitValue<quint8>();

        itRet.nextPixel();
    } while (it.nextPixel());

    return ret;
}
