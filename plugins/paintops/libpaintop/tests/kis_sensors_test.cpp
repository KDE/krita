/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_sensors_test.h"
#include <kis_dynamic_sensor.h>

#include <QTest>

KisSensorsTest::KisSensorsTest()
{
    paintInformations.append(KisPaintInformation(QPointF(0, 0)));
    paintInformations.append(KisPaintInformation(QPointF(0, 1)));
    paintInformations.append(KisPaintInformation(QPointF(1, 2)));
    paintInformations.append(KisPaintInformation(QPointF(2, 2)));
    paintInformations.append(KisPaintInformation(QPointF(3, 1)));
    paintInformations.append(KisPaintInformation(QPointF(3, 0)));
    paintInformations.append(KisPaintInformation(QPointF(2, -1)));
    paintInformations.append(KisPaintInformation(QPointF(1, -1)));
}

void KisSensorsTest::testDrawingAngle()
{
    KisDynamicSensorSP sensor = KisDynamicSensor::id2Sensor(DrawingAngleId, "testname");
    testBound(sensor);
}

void KisSensorsTest::testBound(KisDynamicSensorSP sensor)
{
    Q_FOREACH (const KisPaintInformation & pi, paintInformations) {
        double v = sensor->parameter(pi);
        QVERIFY(v >= 0.0);
        QVERIFY(v <= 1.0);
    }
}

QTEST_MAIN(KisSensorsTest)
