/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _KIS_DYNAMIC_SENSOR_FADE_H_
#define _KIS_DYNAMIC_SENSOR_FADE_H_

#include "kis_dynamic_sensor.h"

class KisDynamicSensorFade : public QObject, public KisDynamicSensor
{
    Q_OBJECT
public:
    using KisSerializableConfiguration::fromXML;
    using KisSerializableConfiguration::toXML;


    KisDynamicSensorFade();
    ~KisDynamicSensorFade() override { }
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
