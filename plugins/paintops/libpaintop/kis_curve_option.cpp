/* This file is part of the KDE project
 * Copyright (C) 2008 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2011 Silvio Heinrich <plassy@web.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "kis_curve_option.h"

#include <QDomNode>

KisCurveOption::KisCurveOption(const QString& name, KisPaintOpOption::PaintopCategory category,
                               bool checked, qreal value, qreal min, qreal max)
    : m_name(name)
    , m_category(category)
    , m_checkable(true)
    , m_checked(checked)
    , m_useCurve(true)
    , m_useSameCurve(true)
    , m_separateCurveValue(false)
    , m_curveMode(0)
{
    Q_FOREACH (const DynamicSensorType sensorType, KisDynamicSensor::sensorsTypes()) {
        KisDynamicSensorSP sensor = KisDynamicSensor::type2Sensor(sensorType, m_name);
        sensor->setActive(false);
        replaceSensor(sensor);
    }
    m_sensorMap[PRESSURE]->setActive(true);

    setValueRange(min, max);
    setValue(value);
}

KisCurveOption::~KisCurveOption()
{
}

const QString& KisCurveOption::name() const
{
    return m_name;
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
        setting->setProperty("Pressure" + m_name, isChecked());
    }

    if (activeSensors().size() == 1) {
        setting->setProperty(m_name + "Sensor", activeSensors().first()->toXML());
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
        setting->setProperty(m_name + "Sensor", doc.toString());
    }
    setting->setProperty(m_name + "UseCurve", m_useCurve);
    setting->setProperty(m_name + "UseSameCurve", m_useSameCurve);
    setting->setProperty(m_name + "Value", m_value);
    setting->setProperty(m_name + "curveMode", m_curveMode);

}

void KisCurveOption::readOptionSetting(KisPropertiesConfigurationSP setting)
{
    m_curveCache.clear();
    readNamedOptionSetting(m_name, setting);
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

void KisCurveOption::readNamedOptionSetting(const QString& prefix, const KisPropertiesConfigurationSP setting)
{
    if (!setting) return;
    //dbgKrita << "readNamedOptionSetting" << prefix;
    // setting->dump();

    if (m_checkable) {
        setChecked(setting->getBool("Pressure" + prefix, false));
    }
    //dbgKrita << "\tPressure" + prefix << isChecked();

    m_sensorMap.clear();

    // Replace all sensors with the inactive defaults
    Q_FOREACH (const DynamicSensorType sensorType, KisDynamicSensor::sensorsTypes()) {
        replaceSensor(KisDynamicSensor::type2Sensor(sensorType, m_name));
    }

    QString sensorDefinition = setting->getString(prefix + "Sensor");
    if (!sensorDefinition.contains("sensorslist")) {
        KisDynamicSensorSP s = KisDynamicSensor::createFromXML(sensorDefinition, m_name);
        if (s) {
            replaceSensor(s);
            s->setActive(true);
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
                    KisDynamicSensorSP s = KisDynamicSensor::createFromXML(childelt, m_name);
                    if (s) {
                        replaceSensor(s);
                        s->setActive(true);
                        //dbgKrita << "\tchild sensor" << s::id(s->sensorType()) << s->isActive() << "added";
                    }
                }
            }
            node = node.nextSibling();
        }
    }

    // Only load the old curve format if the curve wasn't saved by the sensor
    // This will give every sensor the same curve.
    //dbgKrita << ">>>>>>>>>>>" << prefix + "Sensor" << setting->getString(prefix + "Sensor");
    if (!setting->getString(prefix + "Sensor").contains("curve")) {
        //dbgKrita << "\told format";
        if (setting->getBool("Custom" + prefix, false)) {
            Q_FOREACH (KisDynamicSensorSP s, m_sensorMap.values()) {
                s->setCurve(setting->getCubicCurve("Curve" + prefix));
            }
        }
    }

    // At least one sensor needs to be active
    if (activeSensors().size() == 0) {
        m_sensorMap[PRESSURE]->setActive(true);
    }

    m_value = setting->getDouble(m_name + "Value", m_maxValue);
    //dbgKrita << "\t" + m_name + "Value" << m_value;

    m_useCurve = setting->getBool(m_name + "UseCurve", true);
    //dbgKrita << "\t" + m_name + "UseCurve" << m_useSameCurve;

    m_useSameCurve = setting->getBool(m_name + "UseSameCurve", true);
    //dbgKrita << "\t" + m_name + "UseSameCurve" << m_useSameCurve;

    m_curveMode = setting->getInt(m_name + "curveMode");
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

void KisCurveOption::setCurve(DynamicSensorType sensorType, bool useSameCurve, const KisCubicCurve &curve)
{
    // No switch in state, don't mess with the cache
    if (useSameCurve == m_useSameCurve) {
        if (useSameCurve) {
            Q_FOREACH (KisDynamicSensorSP s, m_sensorMap.values()) {
                s->setCurve(curve);
            }
        }
        else {
            KisDynamicSensorSP s = sensor(sensorType, false);
            if (s) {
                s->setCurve(curve);
            }

        }
    }
    else {
        // moving from not use same curve to use same curve: backup the custom curves
        if (!m_useSameCurve && useSameCurve) {
            // Copy the custom curves to the cache and set the new curve on all sensors, active or not
            m_curveCache.clear();
            Q_FOREACH (KisDynamicSensorSP s, m_sensorMap.values()) {
                m_curveCache[s->sensorType()] = s->curve();
                s->setCurve(curve);
            }
        }
        else { //if (m_useSameCurve && !useSameCurve)
            // Restore the cached curves
            KisDynamicSensorSP s = 0;
            Q_FOREACH (DynamicSensorType sensorType, m_curveCache.keys()) {
                if (m_sensorMap.contains(sensorType)) {
                    s = m_sensorMap[sensorType];
                }
                else {
                    s = KisDynamicSensor::type2Sensor(sensorType, m_name);
                }
                s->setCurve(m_curveCache[sensorType]);
                m_sensorMap[sensorType] = s;
            }
            s = 0;
            // And set the current sensor to the current curve
            if (!m_sensorMap.contains(sensorType)) {
                s = KisDynamicSensor::type2Sensor(sensorType, m_name);
            }
            if (s) {
                s->setCurve(curve);
                s->setCurve(m_curveCache[sensorType]);
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
                if (s->isAdditive()) {
                    components.additive += s->parameter(info);
                    components.hasAdditive = true;
                } else if (s->isAbsoluteRotation()) {
                    components.absoluteOffset = s->parameter(info);
                    components.hasAbsoluteOffset =true;
                } else {
                    sensorValues << s->parameter(info);
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

qreal KisCurveOption::computeRotationLikeValue(const KisPaintInformation& info, qreal baseValue, bool absoluteAxesFlipped) const
{
    const ValueComponents components = computeValueComponents(info);
    return components.rotationLikeValue(baseValue, absoluteAxesFlipped);
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
