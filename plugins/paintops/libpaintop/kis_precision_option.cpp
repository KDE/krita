/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
 *  Copyright (c) 2014 Mohit Goyal    <mohit.bits2011@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_precision_option.h"

#include "kis_properties_configuration.h"

void KisPrecisionOption::writeOptionSetting(KisPropertiesConfigurationSP settings) const
{
    settings->setProperty(PRECISION_LEVEL, m_precisionLevel);
    settings->setProperty(AUTO_PRECISION_ENABLED,m_autoPrecisionEnabled);
}

void KisPrecisionOption::readOptionSetting(const KisPropertiesConfigurationSP settings)
{
    m_precisionLevel = settings->getInt(PRECISION_LEVEL, 5);
    m_autoPrecisionEnabled = settings->getBool(AUTO_PRECISION_ENABLED,false);
}

int KisPrecisionOption::effectivePrecisionLevel(qreal effectiveDabSize) const
{
    if (!m_autoPrecisionEnabled) {
        return m_precisionLevel;
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
    return m_precisionLevel;
}

void KisPrecisionOption::setPrecisionLevel(int precisionLevel)
{
    m_precisionLevel = precisionLevel;
}

void KisPrecisionOption::setAutoPrecisionEnabled(int value)
{
    m_autoPrecisionEnabled = value;
}

bool KisPrecisionOption::autoPrecisionEnabled()
{
    return m_autoPrecisionEnabled;
}
