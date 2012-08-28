/*
 *  Copyright (c) 2011 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_DYNAMIC_SENSOR_LIST_H_
#define _KIS_DYNAMIC_SENSOR_LIST_H_

#include "kis_vec.h"

#include "kis_dynamic_sensor.h"
//
class KisDynamicSensorList : public KisDynamicSensor
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    using KisSerializableConfiguration::fromXML;
    using KisSerializableConfiguration::toXML;

    KisDynamicSensorList();
    virtual ~KisDynamicSensorList();
    virtual qreal value(const KisPaintInformation&);
    virtual void reset();

    virtual void toXML(QDomDocument&, QDomElement&) const;
    virtual void fromXML(const QDomElement&);
    
    /**
     * Remove the sensor corrseponnding to the id from the list of sensors.
     * And return it. It becomes the responsibility of the caller to delete
     * the sensor.
     */
    KisDynamicSensor* takeSensor(const QString& id);
    
    /**
     * @return the list of sensor ids countained by this @ref KisDynamicSensorList
     */
    QList<QString> sensorIds() const;
    
    bool hasSensor(const QString& id);
    /**
     * Add a new sensor to the list
     */
    void addSensor(KisDynamicSensor* sensor);
    KisDynamicSensor* getSensor(QString arg1);
private:
    QList<KisDynamicSensor*> m_list;
};

#endif
