/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2014 Mohit Goyal <mohit.bits2011@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_precision_option.h"

#include "kis_properties_configuration.h"

KisPrecisionOption::KisPrecisionOption(const KisPropertiesConfiguration *setting)
    : m_precisionData(KisBrushModel::PrecisionData::read(setting))
{
}

int KisPrecisionOption::effectivePrecisionLevel(qreal effectiveDabSize) const
{
    if (!m_precisionData.useAutoPrecision) {
        return m_precisionData.precisionLevel;
    } else {
        return effectiveDabSize < 30.0 || !m_hasImprecisePositionOptions ? 5 : 3;
    }
}

void KisPrecisionOption::setHasImprecisePositionOptions(bool value)
{
    m_hasImprecisePositionOptions = value;
}

bool KisPrecisionOption::hasImprecisePositionOptions() const
{
    return m_hasImprecisePositionOptions;
}

int KisPrecisionOption::precisionLevel() const
{
    return m_precisionData.precisionLevel;
}

void KisPrecisionOption::setPrecisionLevel(int precisionLevel)
{
    m_precisionData.precisionLevel = precisionLevel;
}

void KisPrecisionOption::setAutoPrecisionEnabled(int value)
{
    m_precisionData.useAutoPrecision = value;
}

bool KisPrecisionOption::autoPrecisionEnabled()
{
    return m_precisionData.useAutoPrecision;
}

namespace KisBrushModel {
bool operator==(const PrecisionData &lhs, const PrecisionData &rhs)
{
    return lhs.precisionLevel == rhs.precisionLevel &&
            lhs.useAutoPrecision == rhs.useAutoPrecision;
}

PrecisionData KisBrushModel::PrecisionData::read(const KisPropertiesConfiguration *config)
{

    PrecisionData data;
    data.precisionLevel = config->getInt(PRECISION_LEVEL, 5);
    data.useAutoPrecision = config->getBool(AUTO_PRECISION_ENABLED,false);
    return data;
}

void PrecisionData::write(KisPropertiesConfiguration *config) const
{
    config->setProperty(PRECISION_LEVEL, precisionLevel);
    config->setProperty(AUTO_PRECISION_ENABLED, useAutoPrecision);
}
}
