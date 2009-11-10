/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_DYNAMIC_SENSOR_H_
#define _KIS_DYNAMIC_SENSOR_H_

#include <krita_export.h>

#include <QObject>

#include <KoID.h>

#include <klocale.h>

#include "kis_serializable_configuration.h"

class QWidget;
class KisPaintInformation;
class KisSensorSelector;

const KoID FuzzyId("fuzzy", i18n("Fuzzy")); ///< generate a random number
const KoID SpeedId("speed", i18n("Speed")); ///< generate a number depending on the speed of the cursor
const KoID TimeId("time", i18n("Time")); ///< generate a number that increase with time
const KoID DrawingAngleId("drawingangle", i18n("Drawing angle")); ///< number depending on the angle
const KoID PressureId("pressure", i18n("Pressure")); ///< number depending on the pressure
const KoID XTiltId("xtilt", i18n("X-Tilt")); ///< number depending on X-tilt
const KoID YTiltId("ytilt", i18n("Y-Tilt")); ///< number depending on Y-tilt

/**
 * Sensor are used to extract from KisPaintInformation a single
 * double value which can be used to control the parameters of
 * a brush.
 */
class PAINTOP_EXPORT KisDynamicSensor : public KisSerializableConfiguration
{
public:
    enum ParameterSign {
        NegativeParameter = -1,
        UnSignedParameter = 0,
        PositiveParameter = 1
    };
protected:
    KisDynamicSensor(const KoID& id);
public:
    virtual ~KisDynamicSensor();
    /**
     * @return the value of this sensor for the given KisPaintInformation
     */
    virtual double parameter(const KisPaintInformation& info) = 0;
    /**
     * This function is call before beginning a stroke to reset the sensor.
     * Default implementation does nothing.
     */
    virtual void reset();
    virtual QWidget* createConfigurationWidget(QWidget* parent, KisSensorSelector*);
    /**
     * Creates a sensor from its identifiant.
     */
    static KisDynamicSensor* id2Sensor(const KoID&);
    static KisDynamicSensor* id2Sensor(const QString& s) {
        return id2Sensor(KoID(s));
    }
    static KisDynamicSensor* createFromXML(const QDomElement&);
    /**
     * @return the list of sensors
     */
    static QList<KoID> sensorsIds();
    /**
     * @return the identifiant of this sensor
     */
    inline QString id() const {
        return m_id.id();
    }
    inline QString name() const {
        return m_id.name();
    }

    using KisSerializableConfiguration::fromXML;
    using KisSerializableConfiguration::toXML;

    virtual void toXML(QDomDocument&, QDomElement&) const;
    virtual void fromXML(const QDomElement&);
private:
    const KoID& m_id;
};

#endif
