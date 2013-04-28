/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
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

void KisPrecisionOption::writeOptionSetting(KisPropertiesConfiguration* settings) const
{
    settings->setProperty(PRECISION_LEVEL, m_precisionLevel);
}

void KisPrecisionOption::readOptionSetting(const KisPropertiesConfiguration* settings)
{
    m_precisionLevel = settings->getInt(PRECISION_LEVEL, 5);
}

int KisPrecisionOption::precisionLevel() const
{
    return m_precisionLevel;
}

void KisPrecisionOption::setPrecisionLevel(int precisionLevel)
{
    m_precisionLevel = precisionLevel;
}
