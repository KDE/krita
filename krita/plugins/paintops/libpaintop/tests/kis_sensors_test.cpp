/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_sensors_test.h"
#include <kis_dynamic_sensor.h>

#include <qtest_kde.h>

KisSensorsTest::KisSensorsTest()
{
    paintInformations.append(KisPaintInformation(QPointF(0,0), 0, 0, 0, 0.0, 0.0, 1.0 ));
    paintInformations.append(KisPaintInformation(QPointF(0,1), 0, 0, 0, 0.0, 0.0, 1.0 ));
    paintInformations.append(KisPaintInformation(QPointF(1,2), 0, 0, 0, 0.0, 0.0, 1.0 ));
    paintInformations.append(KisPaintInformation(QPointF(2,2), 0, 0, 0, 0.0, 0.0, 1.0 ));
    paintInformations.append(KisPaintInformation(QPointF(3,1), 0, 0, 0, 0.0, 0.0, 1.0 ));
    paintInformations.append(KisPaintInformation(QPointF(3,0), 0, 0, 0, 0.0, 0.0, 1.0 ));
    paintInformations.append(KisPaintInformation(QPointF(2,-1), 0, 0, 0, 0.0, 0.0, 1.0 ));
    paintInformations.append(KisPaintInformation(QPointF(1,-1), 0, 0, 0, 0.0, 0.0, 1.0 ));
}

void KisSensorsTest::testDrawingAngle()
{
    KisDynamicSensor* sensor = KisDynamicSensor::id2Sensor(DrawingAngleId);
    testBound(sensor);
}

void KisSensorsTest::testBound(KisDynamicSensor* sensor)
{
    foreach(const KisPaintInformation& pi, paintInformations)
    {
        double v = sensor->parameter(pi);
        QVERIFY(v >= 0.0);
        QVERIFY(v <= 1.0);
    }
}

QTEST_KDEMAIN(KisSensorsTest, GUI)
#include "kis_sensors_test.moc"
