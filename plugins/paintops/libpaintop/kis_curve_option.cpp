/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "kis_curve_option.h"

#include <QDomNode>

#include "kis_algebra_2d.h"

#include <sensors/kis_dynamic_sensor_distance.h>
#include <sensors/kis_dynamic_sensor_drawing_angle.h>
#include <sensors/kis_dynamic_sensor_fade.h>
#include <sensors/kis_dynamic_sensor_fuzzy.h>
#include <sensors/kis_dynamic_sensor_time.h>
#include <sensors/kis_dynamic_sensors.h>


qreal KisCurveOption::ValueComponents::rotationLikeValue(qreal normalizedBaseAngle, bool absoluteAxesFlipped, qreal scalingPartCoeff, bool disableScalingPart) const {
    const qreal offset =
            !hasAbsoluteOffset ? normalizedBaseAngle :
                                 absoluteAxesFlipped ? 0.5 - absoluteOffset :
                                                       absoluteOffset;

    const qreal realScalingPart = hasScaling && !disableScalingPart ? KisDynamicSensor::scalingToAdditive(scaling) : 0.0;
    const qreal realAdditivePart = hasAdditive ? additive : 0;

    qreal value = KisAlgebra2D::wrapValue(2 * offset + constant * (scalingPartCoeff * realScalingPart + realAdditivePart), -1.0, 1.0);
    if (qIsNaN(value)) {
        qWarning() << "rotationLikeValue returns NaN!" << normalizedBaseAngle << absoluteAxesFlipped;
        value = 0;
    }
    return value;
}

qreal KisCurveOption::ValueComponents::sizeLikeValue() const {
    const qreal offset =
            hasAbsoluteOffset ? absoluteOffset : 1.0;

    const qreal realScalingPart = hasScaling ? scaling : 1.0;
    const qreal realAdditivePart = hasAdditive ? KisDynamicSensor::additiveToScaling(additive) : 1.0;

    return qBound(minSizeLikeValue,
                  constant * offset * realScalingPart * realAdditivePart,
                  maxSizeLikeValue);
}

KisCurveOption::KisCurveOption(const KoID &id, KisPaintOpOption::PaintopCategory category, bool checked, qreal value, qreal min, qreal max)
    : m_id(id)
    , m_category(category)
    , m_checkable(true)
    , m_checked(checked)
    , m_useCurve(true)
    , m_useSameCurve(true)
    , m_separateCurveValue(false)
    , m_curveMode(0)
{
    Q_FOREACH (const DynamicSensorType sensorType, this->sensorsTypes()) {
        KisDynamicSensorSP sensor = this->type2Sensor(sensorType, m_id.id());
        sensor->setActive(false);
        replaceSensor(sensor);
    }
    m_sensorMap[PRESSURE]->setActive(true);

    setValueRange(min, max);
    setValue(value);


    m_commonCurve = defaultCurve();
}

KisCurveOption::~KisCurveOption()
{
}

KisPaintOpOption::PaintopCategory KisCurveOption::category() const
{
    return m_category;
}

qreal KisCurveOption::minValue() const
{
    return m_minValue;
}

qreal KisCurveOption::maxValue() const
{
    return m_maxValue;
}

qreal KisCurveOption::value() const
{
    return m_value;
}

void KisCurveOption::resetAllSensors()
{
    Q_FOREACH (KisDynamicSensorSP sensor, m_sensorMap.values()) {
        if (sensor->isActive()) {
            sensor->reset();
        }
    }
}

void KisCurveOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    if (m_checkable) {
        setting->setProperty("Pressure" + m_id.id(), isChecked());
    }

    if (activeSensors().size() == 1) {
        setting->setProperty(m_id.id() + "Sensor", activeSensors().first()->toXML());
    }
    else {
        QDomDocument doc = QDomDocument("params");
        QDomElement root = doc.createElement("params");
        doc.appendChild(root);
        root.setAttribute("id", "sensorslist");

        Q_FOREACH (KisDynamicSensorSP sensor, activeSensors()) {
            QDomElement childelt = doc.createElement("ChildSensor");
            sensor->toXML(doc, childelt);
            root.appendChild(childelt);
        }
        setting->setProperty(m_id.id() + "Sensor", doc.toString());
    }
    setting->setProperty(m_id.id() + "UseCurve", m_useCurve);
    setting->setProperty(m_id.id() + "UseSameCurve", m_useSameCurve);
    setting->setProperty(m_id.id() + "Value", m_value);
    setting->setProperty(m_id.id() + "curveMode", m_curveMode);
    setting->setProperty(m_id.id() + "commonCurve", QVariant::fromValue(m_commonCurve));
}

void KisCurveOption::readOptionSetting(KisPropertiesConfigurationSP setting)
{
    readNamedOptionSetting(m_id.id(), setting);
}

void KisCurveOption::lodLimitations(KisPaintopLodLimitations *l) const
{
    Q_UNUSED(l);
}

int KisCurveOption::intMinValue() const
{
    return 0;
}

int KisCurveOption::intMaxValue() const
{
    return 100;
}

QString KisCurveOption::valueSuffix() const
{
    return i18n("%");
}

void KisCurveOption::updateRange(qreal minValue, qreal maxValue)
{
    m_minValue = minValue;
    m_maxValue = maxValue;
    m_value = qBound(m_minValue, m_value, m_maxValue);
}

void KisCurveOption::readNamedOptionSetting(const QString& prefix, const KisPropertiesConfigurationSP setting)
{
    if (!setting) return;

    KisCubicCurve commonCurve = m_commonCurve;

    //dbgKrita << "readNamedOptionSetting" << prefix;
    // setting->dump();

    if (m_checkable) {
        setChecked(setting->getBool("Pressure" + prefix, false));
    }
    //dbgKrita << "\tPressure" + prefix << isChecked();

    m_sensorMap.clear();

    // Replace all sensors with the inactive defaults
    Q_FOREACH (const DynamicSensorType sensorType, this->sensorsTypes()) {
        replaceSensor(type2Sensor(sensorType, m_id.id()));
    }

    QString sensorDefinition = setting->getString(prefix + "Sensor");
    if (!sensorDefinition.contains("sensorslist")) {
        KisDynamicSensorSP s = KisDynamicSensor::createFromXML(sensorDefinition, m_id.id());
        if (s) {
            replaceSensor(s);
            s->setActive(true);
            commonCurve = s->curve();
            //dbgKrita << "\tsingle sensor" << s::id(s->sensorType()) << s->isActive() << "added";
        }
    }
    else {
        QDomDocument doc;
        doc.setContent(sensorDefinition);
        QDomElement elt = doc.documentElement();
        QDomNode node = elt.firstChild();
        while (!node.isNull()) {
            if (node.isElement())  {
                QDomElement childelt = node.toElement();
                if (childelt.tagName() == "ChildSensor") {
                    KisDynamicSensorSP s = KisDynamicSensor::createFromXML(childelt, m_id.id());
                    if (s) {
                        replaceSensor(s);
                        s->setActive(true);
                        commonCurve = s->curve();
                        //dbgKrita << "\tchild sensor" << s::id(s->sensorType()) << s->isActive() << "added";
                    }
                }
            }
            node = node.nextSibling();
        }
    }

    m_useSameCurve = setting->getBool(m_id.id() + "UseSameCurve", true);

    // Only load the old curve format if the curve wasn't saved by the sensor
    // This will give every sensor the same curve.
    //dbgKrita << ">>>>>>>>>>>" << prefix + "Sensor" << setting->getString(prefix + "Sensor");
    if (!setting->getString(prefix + "Sensor").contains("curve")) {
        //dbgKrita << "\told format";
        if (setting->getBool("Custom" + prefix, false)) {
            Q_FOREACH (KisDynamicSensorSP s, m_sensorMap.values()) {
                s->setCurve(setting->getCubicCurve("Curve" + prefix));
                commonCurve = s->curve();
            }
        } else {
            commonCurve = emptyCurve();
        }
    }

    if (m_useSameCurve) {
        m_commonCurve = setting->getCubicCurve(prefix + "commonCurve", commonCurve);
    }

    // At least one sensor needs to be active
    if (activeSensors().size() == 0) {
        m_sensorMap[PRESSURE]->setActive(true);
    }

    m_value = setting->getDouble(m_id.id() + "Value", m_maxValue);
    //dbgKrita << "\t" + m_name + "Value" << m_value;

    m_useCurve = setting->getBool(m_id.id() + "UseCurve", true);
    //dbgKrita << "\t" + m_name + "UseCurve" << m_useSameCurve;


    //dbgKrita << "\t" + m_name + "UseSameCurve" << m_useSameCurve;

    m_curveMode = setting->getInt(m_id.id() + "curveMode");
    //dbgKrita << "-----------------";
}

void KisCurveOption::replaceSensor(KisDynamicSensorSP s)
{
    Q_ASSERT(s);
    m_sensorMap[s->sensorType()] = s;
}

KisDynamicSensorSP KisCurveOption::sensor(DynamicSensorType sensorType, bool active) const
{
    if (m_sensorMap.contains(sensorType)) {
        if (!active) {
            return m_sensorMap[sensorType];
        }
        else {
             if (m_sensorMap[sensorType]->isActive()) {
                 return m_sensorMap[sensorType];
             }
        }
    }
    return 0;
}


bool KisCurveOption::isRandom() const
{
    return bool(sensor(FUZZY_PER_DAB, true)) ||
        bool(sensor(FUZZY_PER_STROKE, true));
}

bool KisCurveOption::isCurveUsed() const
{
    return m_useCurve;
}

bool KisCurveOption::isSameCurveUsed() const
{
    return m_useSameCurve;
}

int KisCurveOption::getCurveMode() const
{
    return m_curveMode;
}

KisCubicCurve KisCurveOption::getCommonCurve() const
{
    return m_commonCurve;
}

void KisCurveOption::setSeparateCurveValue(bool separateCurveValue)
{
    m_separateCurveValue = separateCurveValue;
}

bool KisCurveOption::isCheckable()
{
    return m_checkable;
}

bool KisCurveOption::isChecked() const
{
    return m_checked;
}

void KisCurveOption::setChecked(bool checked)
{
    m_checked = checked;
}

void KisCurveOption::setCurveUsed(bool useCurve)
{
    m_useCurve = useCurve;
}

void KisCurveOption::setCurveMode(int mode)
{
    m_curveMode = mode;
}

void KisCurveOption::setUseSameCurve(bool useSameCurve)
{
    m_useSameCurve = useSameCurve;
}

void KisCurveOption::setCommonCurve(KisCubicCurve curve)
{
    m_commonCurve = curve;
}

void KisCurveOption::setCurve(DynamicSensorType sensorType, bool useSameCurve, const KisCubicCurve &curve)
{
    if (useSameCurve == m_useSameCurve) {
        if (useSameCurve) {
            m_commonCurve = curve;
        }
        else {
            KisDynamicSensorSP s = sensor(sensorType, false);
            if (s) {
                s->setCurve(curve);
            }

        }
    }
    else {
        if (!m_useSameCurve && useSameCurve) {
            m_commonCurve = curve;
        }
        else { //if (m_useSameCurve && !useSameCurve)
            KisDynamicSensorSP s = 0;
            // And set the current sensor to the current curve
            if (!m_sensorMap.contains(sensorType)) {
                s = type2Sensor(sensorType, m_id.id());
            } else {
                KisDynamicSensorSP s = sensor(sensorType, false);
            }
            if (s) {
                s->setCurve(curve);
            }

        }
        m_useSameCurve = useSameCurve;
    }
}

void KisCurveOption::setValueRange(qreal min, qreal max)
{
    m_minValue = qMin(min, max);
    m_maxValue = qMax(min, max);
}

void KisCurveOption::setValue(qreal value)
{
    m_value = qBound(m_minValue, value, m_maxValue);
}

KisCurveOption::ValueComponents KisCurveOption::computeValueComponents(const KisPaintInformation& info) const
{
    ValueComponents components;

    if (m_useCurve) {
        QMap<DynamicSensorType, KisDynamicSensorSP>::const_iterator i;
        QList<double> sensorValues;
        for (i = m_sensorMap.constBegin(); i != m_sensorMap.constEnd(); ++i) {
            KisDynamicSensorSP s(i.value());

            if (s->isActive()) {
                qreal valueFromCurve = m_useSameCurve ? s->parameter(info, m_commonCurve, true) : s->parameter(info);
                if (s->isAdditive()) {
                    components.additive += valueFromCurve;
                    components.hasAdditive = true;
                } else if (s->isAbsoluteRotation()) {
                    components.absoluteOffset = valueFromCurve;
                    components.hasAbsoluteOffset =true;
                } else {
                    sensorValues << valueFromCurve;
                    components.hasScaling = true;
                }
            }
        }

        if (sensorValues.count() == 1) {
            components.scaling = sensorValues.first();
        } else {

            if (m_curveMode == 1){           // add
                components.scaling = 0;
                double i;
                foreach (i, sensorValues) {
                    components.scaling += i;
                }
            } else if (m_curveMode == 2){    //max
                components.scaling = *std::max_element(sensorValues.begin(), sensorValues.end());

            } else if (m_curveMode == 3){    //min
                components.scaling = *std::min_element(sensorValues.begin(), sensorValues.end());

            } else if (m_curveMode == 4){    //difference
                double max = *std::max_element(sensorValues.begin(), sensorValues.end());
                double min = *std::min_element(sensorValues.begin(), sensorValues.end());
                components.scaling = max-min;

            } else {                         //multuply - default
                double i;
                foreach (i, sensorValues) {
                    components.scaling *= i;
                }
            }
        }

    }

    if (!m_separateCurveValue) {
        components.constant = m_value;
    }

    components.minSizeLikeValue = m_minValue;
    components.maxSizeLikeValue = m_maxValue;

    return components;
}

qreal KisCurveOption::computeSizeLikeValue(const KisPaintInformation& info) const
{
    const ValueComponents components = computeValueComponents(info);
    return components.sizeLikeValue();
}

qreal KisCurveOption::computeRotationLikeValue(const KisPaintInformation& info, qreal baseValue, bool absoluteAxesFlipped, qreal scalingPartCoeff, bool disableScalingPart) const
{
    const ValueComponents components = computeValueComponents(info);
    return components.rotationLikeValue(baseValue, absoluteAxesFlipped, scalingPartCoeff, disableScalingPart);
}

KisCubicCurve KisCurveOption::defaultCurve()
{
    QList<QPointF> points;
    // needs to be set to something, weird curve is better for debugging
    // it will be reset to the curve from the preset anyway though
    points.push_back(QPointF(0,0));
    points.push_back(QPointF(0.25,0.9));
    points.push_back(QPointF(0.5,0));
    points.push_back(QPointF(0.75,0.6));
    points.push_back(QPointF(1,0));
    return KisCubicCurve(points);
}

KisCubicCurve KisCurveOption::emptyCurve()
{
    QList<QPointF> points;
    points.push_back(QPointF(0,0));
    points.push_back(QPointF(1,1));
    return KisCubicCurve(points);
}

QList<KisDynamicSensorSP> KisCurveOption::sensors()
{
    //dbgKrita << "ID" << name() << "has" <<  m_sensorMap.count() << "Sensors of which" << sensorList.count() << "are active.";
    return m_sensorMap.values();
}

QList<KisDynamicSensorSP> KisCurveOption::activeSensors() const
{
    QList<KisDynamicSensorSP> sensorList;
    Q_FOREACH (KisDynamicSensorSP sensor, m_sensorMap.values()) {
        if (sensor->isActive()) {
            sensorList << sensor;
        }
    }
    //dbgKrita << "ID" << name() << "has" <<  m_sensorMap.count() << "Sensors of which" << sensorList.count() << "are active.";
    return sensorList;
}

QList<KoID> KisCurveOption::sensorsIds()
{
    QList<KoID> ids;

    ids << PressureId
        << PressureInId
        << XTiltId
        << YTiltId
        << TiltDirectionId
        << TiltElevationId
        << SpeedId
        << DrawingAngleId
        << RotationId
        << DistanceId
        << TimeId
        << FuzzyPerDabId
        << FuzzyPerStrokeId
        << FadeId
        << PerspectiveId
        << TangentialPressureId;

    return ids;
}

DynamicSensorType KisCurveOption::id2Type(const KoID &id)
{
    if (id.id() == PressureId.id()) {
        return PRESSURE;
    }
    else if (id.id() == PressureInId.id()) {
        return PRESSURE_IN;
    }
    else if (id.id() == XTiltId.id()) {
        return XTILT;
    }
    else if (id.id() == YTiltId.id()) {
        return YTILT;
    }
    else if (id.id() == TiltDirectionId.id()) {
        return TILT_DIRECTION;
    }
    else if (id.id() == TiltElevationId.id()) {
        return TILT_ELEVATATION;
    }
    else if (id.id() == SpeedId.id()) {
        return SPEED;
    }
    else if (id.id() == DrawingAngleId.id()) {
        return ANGLE;
    }
    else if (id.id() == RotationId.id()) {
        return ROTATION;
    }
    else if (id.id() == DistanceId.id()) {
        return DISTANCE;
    }
    else if (id.id() == TimeId.id()) {
        return TIME;
    }
    else if (id.id() == FuzzyPerDabId.id()) {
        return FUZZY_PER_DAB;
    }
    else if (id.id() == FuzzyPerStrokeId.id()) {
        return FUZZY_PER_STROKE;
    }
    else if (id.id() == FadeId.id()) {
        return FADE;
    }
    else if (id.id() == PerspectiveId.id()) {
        return PERSPECTIVE;
    }
    else if (id.id() == TangentialPressureId.id()) {
        return TANGENTIAL_PRESSURE;
    }
    return UNKNOWN;
}

KisDynamicSensorSP KisCurveOption::id2Sensor(const KoID& id, const QString &parentOptionName)
{
    if (id.id() == PressureId.id()) {
        return new KisDynamicSensorPressure();
    }
    else if (id.id() == PressureInId.id()) {
        return new KisDynamicSensorPressureIn();
    }
    else if (id.id() == XTiltId.id()) {
        return new KisDynamicSensorXTilt();
    }
    else if (id.id() == YTiltId.id()) {
        return new KisDynamicSensorYTilt();
    }
    else if (id.id() == TiltDirectionId.id()) {
        return new KisDynamicSensorTiltDirection();
    }
    else if (id.id() == TiltElevationId.id()) {
        return new KisDynamicSensorTiltElevation();
    }
    else if (id.id() == SpeedId.id()) {
        return new KisDynamicSensorSpeed();
    }
    else if (id.id() == DrawingAngleId.id()) {
        return new KisDynamicSensorDrawingAngle();
    }
    else if (id.id() == RotationId.id()) {
        return new KisDynamicSensorRotation();
    }
    else if (id.id() == DistanceId.id()) {
        return new KisDynamicSensorDistance();
    }
    else if (id.id() == TimeId.id()) {
        return new KisDynamicSensorTime();
    }
    else if (id.id() == FuzzyPerDabId.id()) {
        return new KisDynamicSensorFuzzy(false, parentOptionName);
    }
    else if (id.id() == FuzzyPerStrokeId.id()) {
        return new KisDynamicSensorFuzzy(true, parentOptionName);
    }
    else if (id.id() == FadeId.id()) {
        return new KisDynamicSensorFade();
    }
    else if (id.id() == PerspectiveId.id()) {
        return new KisDynamicSensorPerspective();
    }
    else if (id.id() == TangentialPressureId.id()) {
        return new KisDynamicSensorTangentialPressure();
    }
    dbgPlugins << "Unknown transform parameter :" << id.id();
    return 0;
}

KisDynamicSensorSP KisCurveOption::type2Sensor(DynamicSensorType sensorType, const QString &parentOptionName)
{
    switch (sensorType) {
    case FUZZY_PER_DAB:
        return new KisDynamicSensorFuzzy(false, parentOptionName);
    case FUZZY_PER_STROKE:
        return new KisDynamicSensorFuzzy(true, parentOptionName);
    case SPEED:
        return new KisDynamicSensorSpeed();
    case FADE:
        return new KisDynamicSensorFade();
    case DISTANCE:
        return new KisDynamicSensorDistance();
    case TIME:
        return new KisDynamicSensorTime();
    case ANGLE:
        return new KisDynamicSensorDrawingAngle();
    case ROTATION:
        return new KisDynamicSensorRotation();
    case PRESSURE:
        return new KisDynamicSensorPressure();
    case XTILT:
        return new KisDynamicSensorXTilt();
    case YTILT:
        return new KisDynamicSensorYTilt();
    case TILT_DIRECTION:
        return new KisDynamicSensorTiltDirection();
    case TILT_ELEVATATION:
        return new KisDynamicSensorTiltElevation();
    case PERSPECTIVE:
        return new KisDynamicSensorPerspective();
    case TANGENTIAL_PRESSURE:
        return new KisDynamicSensorTangentialPressure();
    case PRESSURE_IN:
        return new KisDynamicSensorPressureIn();
    default:
        return 0;
    }
}

QList<DynamicSensorType> KisCurveOption::sensorsTypes()
{
    QList<DynamicSensorType> sensorTypes;
    sensorTypes
            << PRESSURE
            << PRESSURE_IN
            << XTILT
            << YTILT
            << TILT_DIRECTION
            << TILT_ELEVATATION
            << SPEED
            << ANGLE
            << ROTATION
            << DISTANCE
            << TIME
            << FUZZY_PER_DAB
            << FUZZY_PER_STROKE
            << FADE
            << PERSPECTIVE
            << TANGENTIAL_PRESSURE;
    return sensorTypes;
}


