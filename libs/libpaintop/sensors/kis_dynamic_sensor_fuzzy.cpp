/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
 *  Copyright (c) 2014 Mohit Goyal <mohit.bits2011@gmail.com>
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


#include "kis_dynamic_sensor_fuzzy.h"

#include <QDomElement>
#include <brushengine/kis_paint_information.h>

#include <QCheckBox>
#include <QHBoxLayout>


KisDynamicSensorFuzzy::KisDynamicSensorFuzzy(bool fuzzyPerStroke)
    : KisDynamicSensor(fuzzyPerStroke ? FUZZY_PER_STROKE : FUZZY_PER_DAB),
      m_fuzzyPerStroke(fuzzyPerStroke),
      m_isInitialized(false),
      m_savedValue(0.0)
{
}

void KisDynamicSensorFuzzy::reset()
{
    m_isInitialized = false;
}

bool KisDynamicSensorFuzzy::isAdditive() const
{
    return true;
}

qreal KisDynamicSensorFuzzy::value(const KisPaintInformation &info)
{
    if (m_fuzzyPerStroke && m_isInitialized) {
        return m_savedValue;
    }

    qreal result = 0.0;

    if (!info.isHoveringMode()) {
        result = info.randomSource()->generateNormalized();
        result = 2.0 * result - 1.0;

        m_isInitialized = true;
        m_savedValue = result;
    }

    return result;
}

bool KisDynamicSensorFuzzy::dependsOnCanvasRotation() const
{
    return false;
}


