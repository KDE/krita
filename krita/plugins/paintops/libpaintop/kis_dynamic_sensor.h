/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_DYNAMIC_SENSOR_H_
#define _KIS_DYNAMIC_SENSOR_H_

#include <kritapaintop_export.h>

#include <QObject>

#include <KoID.h>

#include <klocale.h>

#include "kis_serializable_configuration.h"
#include "kis_curve_label.h"
#include <kis_cubic_curve.h>
#include <kis_shared_ptr.h>
#include <kis_shared.h>


class QWidget;
class KisPaintInformation;

const KoID FuzzyId("fuzzy", ki18n("Fuzzy")); ///< generate a random number
const KoID SpeedId("speed", ki18n("Speed")); ///< generate a number depending on the speed of the cursor
const KoID FadeId("fade", ki18n("Fade")); ///< generate a number that increase every time you call it (e.g. per dab)
const KoID DistanceId("distance", ki18n("Distance")); ///< generate a number that increase with distance
const KoID TimeId("time", ki18n("Time")); ///< generate a number that increase with time
const KoID DrawingAngleId("drawingangle", ki18n("Drawing angle")); ///< number depending on the angle
const KoID RotationId("rotation", ki18n("Rotation")); ///< rotation coming from the device
const KoID PressureId("pressure", ki18n("Pressure")); ///< number depending on the pressure
const KoID PressureInId("pressurein", ki18n("PressureIn")); ///< number depending on the pressure
const KoID XTiltId("xtilt", ki18n("X-Tilt")); ///< number depending on X-tilt
const KoID YTiltId("ytilt", ki18n("Y-Tilt")); ///< number depending on Y-tilt

/**
 * "TiltDirection" and "TiltElevation" parameters are written to
 * preset files as "ascension" and "declination" to keep backward
 * compatibility with older presets from the days when they were called
 * differently.
 */
const KoID TiltDirectionId("ascension", ki18n("Tilt direction")); /// < number depending on the X and Y tilt, tilt direction is 0 when stylus nib points to you and changes clockwise from -180 to +180.
const KoID TiltElevationId("declination", ki18n("Tilt elevation")); /// < tilt elevation is 90 when stylus is perpendicular to tablet and 0 when it's parallel to tablet

const KoID PerspectiveId("perspective", ki18n("Perspective")); ///< number depending on the distance on the perspective grid
const KoID TangentialPressureId("tangentialpressure", ki18n("Tangential pressure")); ///< the wheel on an airbrush device
const KoID SensorsListId("sensorslist", "SHOULD NOT APPEAR IN THE UI !"); ///< this a non user-visible sensor that can store a list of other sensors, and multiply their output

class KisDynamicSensor;
typedef KisSharedPtr<KisDynamicSensor> KisDynamicSensorSP;

enum DynamicSensorType {
    FUZZY,
    SPEED,
    FADE,
    DISTANCE,
    TIME,
    ANGLE,
    ROTATION,
    PRESSURE,
    XTILT,
    YTILT,
    TILT_DIRECTION,
    TILT_ELEVATATION,
    PERSPECTIVE,
    TANGENTIAL_PRESSURE,
    SENSORS_LIST,
    PRESSURE_IN,
    UNKNOWN = 255
};

/**
 * Sensors are used to extract from KisPaintInformation a single
 * double value which can be used to control the parameters of
 * a brush.
 */
class PAINTOP_EXPORT KisDynamicSensor : public KisSerializableConfiguration, public KisShared
{

public:
    enum ParameterSign {
        NegativeParameter = -1,
        UnSignedParameter = 0,
        PositiveParameter = 1
    };

protected:
    KisDynamicSensor(DynamicSensorType type);

public:

    virtual ~KisDynamicSensor();

    /**
     * @return the value of this sensor for the given KisPaintInformation
     */
    qreal parameter(const KisPaintInformation& info);

    /**
     * This function is call before beginning a stroke to reset the sensor.
     * Default implementation does nothing.
     */
    virtual void reset();

    /**
     * @param selector is a \ref QWidget that countains a signal called "parametersChanged()"
     */
    virtual QWidget* createConfigurationWidget(QWidget* parent, QWidget* selector);

    /**
     * Creates a sensor from its identifiant.
     */
    static KisDynamicSensorSP id2Sensor(const KoID& id);
    static KisDynamicSensorSP id2Sensor(const QString& s) {
        return id2Sensor(KoID(s));
    }

    static DynamicSensorType id2Type(const KoID& id);
    static DynamicSensorType id2Type(const QString& s) {
        return id2Type(KoID(s));
    }

    /**
     * type2Sensor creates a new sensor for the give type
     */
    static KisDynamicSensorSP type2Sensor(DynamicSensorType sensorType);

    static QString minimumLabel(DynamicSensorType sensorType);
    static QString maximumLabel(DynamicSensorType sensorType, int max = -1);

    static KisDynamicSensorSP createFromXML(const QString&);
    static KisDynamicSensorSP createFromXML(const QDomElement&);

    /**
     * @return the list of sensors
     */
    static QList<KoID> sensorsIds();
    static QList<DynamicSensorType> sensorsTypes();

    /**
     * @return the identifiant of this sensor
     */
    static QString id(DynamicSensorType sensorType);

    using KisSerializableConfiguration::fromXML;
    using KisSerializableConfiguration::toXML;

    virtual void toXML(QDomDocument&, QDomElement&) const;
    virtual void fromXML(const QDomElement&);

    void setCurve(const KisCubicCurve& curve);
    const KisCubicCurve& curve() const;
    void removeCurve();
    bool hasCustomCurve() const;

    void setActive(bool active);
    bool isActive() const;

    virtual bool dependsOnCanvasRotation() const;

    inline DynamicSensorType sensorType() const { return m_type; }


    /**
     * @return the currently set length or -1 if not relevant
     */
    int length() { return m_length; }

protected:

    virtual qreal value(const KisPaintInformation& info) = 0;

    int m_length;

private:

    Q_DISABLE_COPY(KisDynamicSensor)

    DynamicSensorType m_type;
    bool m_customCurve;
    KisCubicCurve m_curve;
    bool m_active;

};

#endif
