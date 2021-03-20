/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _KIS_DYNAMIC_SENSOR_DISTANCE_H_
#define _KIS_DYNAMIC_SENSOR_DISTANCE_H_

#include "kis_dynamic_sensor.h"
//
class KisDynamicSensorDistance : public QObject, public KisDynamicSensor
{
    Q_OBJECT
public:
    using KisSerializableConfiguration::fromXML;
    using KisSerializableConfiguration::toXML;


    KisDynamicSensorDistance();
    ~KisDynamicSensorDistance() override { }
    qreal value(const KisPaintInformation&) override;
    QWidget* createConfigurationWidget(QWidget* parent, QWidget*) override;
public Q_SLOTS:
    virtual void setPeriodic(bool periodic);
    virtual void setLength(int length);

    void toXML(QDomDocument&, QDomElement&) const override;
    void fromXML(const QDomElement&) override;
private:
    bool m_periodic;
};

#endif
