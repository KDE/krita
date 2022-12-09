/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisCurveOptionDataCommon.h"
#include "KisSensorPackInterface.h"

KisCurveOptionDataCommon::KisCurveOptionDataCommon(const QString &_prefix, const KoID &_id, bool _isCheckable, bool _isChecked, qreal _minValue, qreal _maxValue, KisSensorPackInterface *sensorInterface)
    : id(_id),
      prefix(_prefix),
      isCheckable(_isCheckable),
      strengthMinValue(_minValue),
      strengthMaxValue(_maxValue),
      isChecked(_isChecked),
      strengthValue(_maxValue),
      sensorData(sensorInterface)
{
}

KisCurveOptionDataCommon::KisCurveOptionDataCommon(const KoID &_id, bool _isCheckable, bool _isChecked, qreal _minValue, qreal _maxValue, KisSensorPackInterface *sensorInterface)
    : KisCurveOptionDataCommon("", _id, _isCheckable, _isChecked, _minValue, _maxValue, sensorInterface)
{
}

// TODO: rename into constSensors
std::vector<const KisSensorData*> KisCurveOptionDataCommon::sensors() const
{
    return sensorData->constSensors();
}

std::vector<KisSensorData*> KisCurveOptionDataCommon::sensors()
{
    return sensorData->sensors();
}

bool KisCurveOptionDataCommon::read(const KisPropertiesConfiguration *setting)
{
    if (!setting) return false;

    if (prefix.isEmpty()) {
        return readPrefixed(setting);
    } else {
        KisPropertiesConfiguration prefixedSetting;
        setting->getPrefixedProperties(prefix, &prefixedSetting);
        return readPrefixed(&prefixedSetting);
    }
}

void KisCurveOptionDataCommon::write(KisPropertiesConfiguration *setting) const
{
    if (prefix.isEmpty()) {
        writePrefixed(setting);
    } else {
        KisPropertiesConfiguration prefixedSetting;
        writePrefixed(&prefixedSetting);
        setting->setPrefixedProperties(prefix, &prefixedSetting);
    }
}

bool KisCurveOptionDataCommon::readPrefixed(const KisPropertiesConfiguration *setting)
{
    return sensorData->read(*this, setting);
}

void KisCurveOptionDataCommon::writePrefixed(KisPropertiesConfiguration *setting) const
{
    sensorData->write(*this, setting);
}


