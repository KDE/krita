/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
 *  Copyright (c) 2014 Mohit Goyal <mohit.bits2011@gmail.com>
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



#ifndef KIS_DYNAMIC_SENSOR_FUZZY_H
#define KIS_DYNAMIC_SENSOR_FUZZY_H

#include "kis_dynamic_sensor.h"
#include "kis_paint_information.h"

#include <kis_paintop.h>
#include <KoID.h>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QDomElement>

class KisDynamicSensorFuzzy : public QObject, public KisDynamicSensor
{
    Q_OBJECT
public:
    bool dependsOnCanvasRotation() const;
    QWidget* createConfigurationWidget(QWidget* parent, QWidget*);

    using KisSerializableConfiguration::fromXML;
    using KisSerializableConfiguration::toXML;
    void toXML(QDomDocument&, QDomElement&) const;
    void fromXML(const QDomElement&);

    bool rotationModeEnabled() const;

    KisDynamicSensorFuzzy();
    virtual ~KisDynamicSensorFuzzy() {}
    qreal value(const KisPaintInformation &info);

public Q_SLOTS:
    void setRotationModeEnabled(int state);

private:
    bool m_rotationModeEnabled;

};

#endif // KIS_DYNAMIC_SENSOR_FUZZY_H
