/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_SENSORS_TEST_H
#define KIS_SENSORS_TEST_H

#include <QTest>

#include <brushengine/kis_paint_information.h>
#include "kis_dynamic_sensor.h"

class KisSensorsTest : public QObject
{
    Q_OBJECT
public:
    KisSensorsTest();
private Q_SLOTS:

    void testDrawingAngle();
private:
    void testBound(KisDynamicSensorSP sensor);
private:
    QList<KisPaintInformation> paintInformations;
};

#endif
