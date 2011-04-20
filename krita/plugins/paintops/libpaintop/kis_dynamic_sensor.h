/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include "kis_curve_label.h"
#include <kis_cubic_curve.h>

class QWidget;
class KisPaintInformation;
class KisSensorSelector;

const KoID FuzzyId("fuzzy", ki18n("Fuzzy")); ///< generate a random number
const KoID SpeedId("speed", ki18n("Speed")); ///< generate a number depending on the speed of the cursor
const KoID FadeId("fade", ki18n("Fade")); ///< generate a number that increase every time you call it (e.g. per dab)
const KoID DistanceId("distance", ki18n("Distance")); ///< generate a number that increase with distance
const KoID TimeId("time", ki18n("Time")); ///< generate a number that increase with time
const KoID DrawingAngleId("drawingangle", ki18n("Drawing angle")); ///< number depending on the angle
const KoID RotationId("rotation", ki18n("Rotation")); ///< rotation coming from the device
const KoID PressureId("pressure", ki18n("Pressure")); ///< number depending on the pressure
const KoID XTiltId("xtilt", ki18n("X-Tilt")); ///< number depending on X-tilt
const KoID YTiltId("ytilt", ki18n("Y-Tilt")); ///< number depending on Y-tilt
const KoID AscensionId("ascension", ki18n("Ascension")); /// < number depending on the X and Y tilt, ascension is 0 when stylus nib points to you and changes clockwise from -180 to +180.
const KoID DeclinationId("declination", ki18n("Declination")); /// < declination is 90 when stylus is perpendicular to tablet and 0 when it's parallel to tablet
const KoID PerspectiveId("perspective", ki18n("Perspective")); ///< number depending on the distance on the perspective grid
const KoID TangentialPressureId("tangentialpressure", ki18n("Tangential pressure")); ///< the wheel on an airbrush device
const KoID SensorsListId("sensorslist", "SHOULD NOT APPEAR IN THE UI !"); ///< this a non user-visible sensor that can store a list of other sensors, and multiply their output

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
    KisDynamicSensor* clone() const;
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
    static KisDynamicSensor* id2Sensor(const KoID&);
    static KisDynamicSensor* id2Sensor(const QString& s) {
        return id2Sensor(KoID(s));
    }
    static KisDynamicSensor* createFromXML(const QString&);
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
    const KisCurveLabel& minimumLabel() const;
    const KisCurveLabel& maximumLabel() const;
    void setCurve(const KisCubicCurve& curve);
    const KisCubicCurve& curve() const;
    void removeCurve();
    bool hasCustomCurve() const;
protected:
    virtual qreal value(const KisPaintInformation& info) = 0;
protected:
    void setMinimumLabel(const KisCurveLabel& _label);
    void setMaximumLabel(const KisCurveLabel& _label);
private:
    const KoID& m_id;
    KisCurveLabel m_minimumLabel, m_maximumLabel;
    bool m_customCurve;
    KisCubicCurve m_curve;
};

#endif
