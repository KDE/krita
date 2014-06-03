/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_DYNAMIC_SENSOR_DRAWING_ANGLE_H
#define __KIS_DYNAMIC_SENSOR_DRAWING_ANGLE_H

#include "kis_dynamic_sensor.h"


class KisDynamicSensorDrawingAngle : public QObject, public KisDynamicSensor
{
    Q_OBJECT
public:
    KisDynamicSensorDrawingAngle();
    qreal value(const KisPaintInformation& info);
    bool dependsOnCanvasRotation() const;

    QWidget* createConfigurationWidget(QWidget* parent, QWidget*);

    using KisSerializableConfiguration::fromXML;
    using KisSerializableConfiguration::toXML;
    void toXML(QDomDocument&, QDomElement&) const;
    void fromXML(const QDomElement&);

    bool fanCornersEnabled() const;
    int fanCornersStep() const;

public slots:
    void setFanCornersEnabled(int state);
    void setFanCornersStep(int angle);

private:
    bool m_fanCornersEnabled;
    int m_fanCornersStep;
};

#endif /* __KIS_DYNAMIC_SENSOR_DRAWING_ANGLE_H */
