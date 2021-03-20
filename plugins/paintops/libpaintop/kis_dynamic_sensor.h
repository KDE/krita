/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _KIS_DYNAMIC_SENSOR_H_
#define _KIS_DYNAMIC_SENSOR_H_

#include <kritapaintop_export.h>

#include <QObject>

#include <KoID.h>

#include <klocalizedstring.h>

#include "kis_serializable_configuration.h"
#include "kis_curve_label.h"
#include <kis_cubic_curve.h>
#include <kis_shared_ptr.h>
#include <kis_shared.h>


class QWidget;
class KisPaintInformation;

const KoID FuzzyPerDabId("fuzzy", ki18nc("Context: dynamic sensors", "Fuzzy Dab")); ///< generate a random number
const KoID FuzzyPerStrokeId("fuzzystroke", ki18nc("Context: dynamic sensors", "Fuzzy Stroke")); ///< generate a random number
const KoID SpeedId("speed", ki18nc("Context: dynamic sensors", "Speed")); ///< generate a number depending on the speed of the cursor
const KoID FadeId("fade", ki18nc("Context: dynamic sensors", "Fade")); ///< generate a number that increase every time you call it (e.g. per dab)
const KoID DistanceId("distance", ki18nc("Context: dynamic sensors", "Distance")); ///< generate a number that increase with distance
const KoID TimeId("time", ki18nc("Context: dynamic sensors", "Time")); ///< generate a number that increase with time
const KoID DrawingAngleId("drawingangle", ki18nc("Context: dynamic sensors", "Drawing angle")); ///< number depending on the angle
const KoID RotationId("rotation", ki18nc("Context: dynamic sensors", "Rotation")); ///< rotation coming from the device
const KoID PressureId("pressure", ki18nc("Context: dynamic sensors", "Pressure")); ///< number depending on the pressure
const KoID PressureInId("pressurein", ki18nc("Context: dynamic sensors", "PressureIn")); ///< number depending on the pressure
const KoID XTiltId("xtilt", ki18nc("Context: dynamic sensors", "X-Tilt")); ///< number depending on X-tilt
const KoID YTiltId("ytilt", ki18nc("Context: dynamic sensors", "Y-Tilt")); ///< number depending on Y-tilt

/**
 * "TiltDirection" and "TiltElevation" parameters are written to
 * preset files as "ascension" and "declination" to keep backward
 * compatibility with older presets from the days when they were called
 * differently.
 */
const KoID TiltDirectionId("ascension", ki18nc("Context: dynamic sensors", "Tilt direction")); /// < number depending on the X and Y tilt, tilt direction is 0 when stylus nib points to you and changes clockwise from -180 to +180.
const KoID TiltElevationId("declination", ki18nc("Context: dynamic sensors", "Tilt elevation")); /// < tilt elevation is 90 when stylus is perpendicular to tablet and 0 when it's parallel to tablet

const KoID PerspectiveId("perspective", ki18nc("Context: dynamic sensors", "Perspective")); ///< number depending on the distance on the perspective grid
const KoID TangentialPressureId("tangentialpressure", ki18nc("Context: dynamic sensors", "Tangential pressure")); ///< the wheel on an airbrush device
const KoID SensorsListId("sensorslist", "SHOULD NOT APPEAR IN THE UI !"); ///< this a non user-visible sensor that can store a list of other sensors, and multiply their output

class KisDynamicSensor;
typedef KisSharedPtr<KisDynamicSensor> KisDynamicSensorSP;

enum DynamicSensorType {
    FUZZY_PER_DAB,
    FUZZY_PER_STROKE,
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

    MYPAINT_PRESSURE,
    MYPAINT_FINE_SPEED,
    MYPAINT_GROSS_SPEED,
    MYPAINT_RANDOM,
    MYPAINT_STROKE,
    MYPAINT_DIRECTION,
    MYPAINT_DECLINATION,
    MYPAINT_ASCENSION,
    MYPAINT_CUSTOM,

    UNKNOWN = 255
};

/**
 * Sensors are used to extract from KisPaintInformation a single
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
    KisDynamicSensor(DynamicSensorType type);

public:

    ~KisDynamicSensor() override;

    /**
     * @return the value of this sensor for the given KisPaintInformation
     */
    qreal parameter(const KisPaintInformation& info);
    /**
     * @return the value of this sensor for the given KisPaintInformation
     * curve -- a custom, temporary curve that should be used instead of the one for the sensor
     * customCurve -- if it's a new curve or not; should always be true if the function is called from outside
     * (aka not in parameter(info) function)
     */
    qreal parameter(const KisPaintInformation& info, const KisCubicCurve curve, const bool customCurve);

    /**
     * This function is call before beginning a stroke to reset the sensor.
     * Default implementation does nothing.
     */
    virtual void reset();

    /**
     * @param parent the parent QWidget
     * @param selector is a \ref QWidget that contains a signal called "parametersChanged()"
     */
    virtual QWidget* createConfigurationWidget(QWidget* parent, QWidget* selector);

    /**
     * Creates a sensor from its identifier.
     */
    static KisDynamicSensorSP id2Sensor(const KoID& id, const QString &parentOptionName);
    static KisDynamicSensorSP id2Sensor(const QString& s, const QString &parentOptionName) {
        return id2Sensor(KoID(s), parentOptionName);
    }

    virtual QString minimumLabel(DynamicSensorType sensorType);
    virtual QString maximumLabel(DynamicSensorType sensorType, int max = -1);
    virtual int minimumValue(DynamicSensorType sensorType);
    virtual int maximumValue(DynamicSensorType sensorType, int max = -1);
    virtual QString valueSuffix(DynamicSensorType sensorType);

    static KisDynamicSensorSP createFromXML(const QString&, const QString &parentOptionName);
    static KisDynamicSensorSP createFromXML(const QDomElement&, const QString &parentOptionName);


    /**
     * @return the identifier of this sensor
     */
    static QString id(DynamicSensorType sensorType);

    using KisSerializableConfiguration::fromXML;
    using KisSerializableConfiguration::toXML;

    void toXML(QDomDocument&, QDomElement&) const override;
    void fromXML(const QDomElement&) override;

    void setCurve(const KisCubicCurve& curve);
    const KisCubicCurve& curve() const;
    void removeCurve();
    bool hasCustomCurve() const;

    void setActive(bool active);
    bool isActive() const;

    virtual bool dependsOnCanvasRotation() const;

    virtual bool isAdditive() const;
    virtual bool isAbsoluteRotation() const;

    inline DynamicSensorType sensorType() const { return m_type; }


    /**
     * @return the currently set length or -1 if not relevant
     */
    int length() { return m_length; }
    QString identifier();


public:
    static inline qreal scalingToAdditive(qreal x) {
        return -1.0 + 2.0 * x;
    }

    static inline qreal additiveToScaling(qreal x) {
        return 0.5 * (1.0 + x);
    }

protected:

    virtual qreal value(const KisPaintInformation& info) = 0;

    int m_length;
    QString m_id;

    DynamicSensorType m_type;
    bool m_customCurve;
    KisCubicCurve m_curve;
    bool m_active;

private:
    Q_DISABLE_COPY(KisDynamicSensor)

};

#endif
