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

KisCurveOption::KisCurveOption(const QString & label, const QString& name, const QString & category,
                               bool checked, qreal value, qreal min, qreal max, bool useCurve,
                               bool separateCurveValue
)
        : m_label(label)
        , m_category(category)
        , m_sensor(0)
        , m_name(name)
        , m_checkable(true)
        , m_checked(checked)
        , m_useCurve(useCurve)
        , m_separateCurveValue(separateCurveValue)
{
    setSensor(KisDynamicSensor::id2Sensor(PressureId.id()));
    setMinimumLabel(i18n("0.0"));
    setMaximumLabel(i18n("1.0"));
    setValueRange(min, max);
    setValue(value);
}

KisCurveOption::~KisCurveOption()
{
    delete m_sensor;
}

const QString& KisCurveOption::name() const
{
    return m_name;
}

const QString & KisCurveOption::label() const
{
    return m_label;
}

const QString& KisCurveOption::category() const
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

void KisCurveOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    if(m_checkable)
        setting->setProperty("Pressure" + m_name, isChecked());
    
    setting->setProperty(m_name + "Sensor"  , sensor()->toXML());
    setting->setProperty(m_name + "UseCurve", m_useCurve);
    setting->setProperty(m_name + "Value"   , m_value);
}

void KisCurveOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    readNamedOptionSetting(m_name, setting);
}

void KisCurveOption::readNamedOptionSetting(const QString& prefix, const KisPropertiesConfiguration* setting)
{
    if(m_checkable)
        setChecked(setting->getBool("Pressure" + prefix, false));

    KisDynamicSensor* sensor = KisDynamicSensor::createFromXML(setting->getString(prefix + "Sensor"));
    
    if(sensor)
        setSensor(sensor);

    // only load the old curve format if the curve wasn't saved by the sensor
    if(!setting->getString(prefix + "Sensor").contains("curve")) {
        bool customCurve = setting->getBool("Custom" + prefix, false);

        if(customCurve)
            m_sensor->setCurve(setting->getCubicCurve("Curve" + prefix));
    }
    m_value    = setting->getDouble(m_name + "Value"   , m_maxValue);
    m_useCurve = setting->getBool  (m_name + "UseCurve", true);
}

void KisCurveOption::setSensor(KisDynamicSensor* sensor)
{
    delete m_sensor;
    Q_ASSERT(sensor);
    m_sensor = sensor;
}

KisDynamicSensor* KisCurveOption::sensor() const
{
    return m_sensor;
}

bool KisCurveOption::isCurveUsed() const
{
    return m_useCurve;
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

const KisCurveLabel& KisCurveOption::minimumLabel() const
{
    return m_minimumLabel;
}

const KisCurveLabel& KisCurveOption::maximumLabel() const
{
    return m_maximumLabel;
}

void KisCurveOption::setMinimumLabel(const KisCurveLabel& _label)
{
    m_minimumLabel = _label;
}

void KisCurveOption::setMaximumLabel(const KisCurveLabel& _label)
{
    m_maximumLabel = _label;
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
