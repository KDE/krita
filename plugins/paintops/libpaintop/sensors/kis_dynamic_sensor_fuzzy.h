/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2014 Mohit Goyal <mohit.bits2011@gmail.com>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */



#ifndef KIS_DYNAMIC_SENSOR_FUZZY_H
#define KIS_DYNAMIC_SENSOR_FUZZY_H

#include "kis_dynamic_sensor.h"
#include <brushengine/kis_paint_information.h>

#include <brushengine/kis_paintop.h>
#include <KoID.h>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QDomElement>

class KisDynamicSensorFuzzy : public QObject, public KisDynamicSensor
{
    Q_OBJECT
public:
    bool dependsOnCanvasRotation() const override;

    bool isAdditive() const override;

    KisDynamicSensorFuzzy(bool fuzzyPerStroke, const QString &parentOptionName);
    ~KisDynamicSensorFuzzy() override {}
    qreal value(const KisPaintInformation &info) override;

    void reset() override;

private:
    const bool m_fuzzyPerStroke;
    QString m_perStrokeRandomSourceKey;
};

#endif // KIS_DYNAMIC_SENSOR_FUZZY_H
